#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <curl/curl.h>

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "MediaSink.hh"
#include "MediaSession.hh"
#include "raptorrtp/giMediaSession.hh"


/*************************************************************************
 * Defines
 *************************************************************************/
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 100000


/*************************************************************************
 * Structures
 *************************************************************************/

struct url_data {
    size_t size;
    char* data;
};



/*************************************************************************
 * Classes
 *************************************************************************/

class StreamClientState
{
public:
    StreamClientState( );
    virtual ~StreamClientState( );

public:
    MediaSubsessionIterator *iter;
    giMediaSession          *session;
    MediaSubsession         *subsession;
    TaskToken               streamTimerTask;
    double                  duration;
};


StreamClientState::StreamClientState( )
  : iter( NULL ),
    session( NULL ),
    subsession( NULL ),
    streamTimerTask( NULL ),
    duration( 0.0 )
{ }

StreamClientState::~StreamClientState( )
{
    delete iter;
}



class liveTestSink: public MediaSink
{
public:
    static liveTestSink* createNew(UsageEnvironment &env, MediaSubsession &subsession, char const* streamId = NULL);

private:
    liveTestSink( UsageEnvironment &env, MediaSubsession &subsession, char const* streamId );

    virtual ~liveTestSink( );

    static void afterGettingFrame( void* clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds );

    void afterGettingFrame( unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds );

private:
    virtual Boolean continuePlaying( );

private:
    u_int8_t* fReceiveBuffer;
    MediaSubsession &fSubsession;
    char* fStreamId;
    std::ofstream *outfile;
};


liveTestSink* liveTestSink::createNew( UsageEnvironment& env, MediaSubsession& subsession, char const* streamId )
{
  return new liveTestSink( env, subsession, streamId );
}

liveTestSink::liveTestSink( UsageEnvironment& env, MediaSubsession& subsession, char const* streamId )
  : MediaSink( env ),
    fSubsession( subsession )
{
    fStreamId = strDup( streamId );
    fReceiveBuffer = new u_int8_t[ DUMMY_SINK_RECEIVE_BUFFER_SIZE ];

    envir( ) << "New liveTestSink Stream \"" << fStreamId << "\"; ";

}

liveTestSink::~liveTestSink()
{
  delete[] fReceiveBuffer;
  delete[] fStreamId;
}

void liveTestSink::afterGettingFrame( void* clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds )
{
    liveTestSink* sink = ( liveTestSink* )clientData;
    sink->afterGettingFrame( frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds );
}

void liveTestSink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned /*durationInMicroseconds*/)
{
    // Usually process data here, but just drop it and continue, to request the next frame of data:
    continuePlaying( );
}

Boolean liveTestSink::continuePlaying( )
{
    if ( fSource == NULL ) return False;

    // Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
    fSource->getNextFrame(fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE, afterGettingFrame, this, onSourceClosure, this );
    return True;
}





/*************************************************************************
 * Global variables
 *************************************************************************/
StreamClientState scs;
UsageEnvironment* env;
char exitWatchVariable;


/*************************************************************************
 * Application
 *************************************************************************/


// Handler to catch Ctrl-C
void handler(int s)
{
    printf("Caught signal %d\n",s);

    exitWatchVariable = 1;
}


// Used by libcurl to write data to buffer
size_t write_data( void *ptr, size_t size, size_t nmemb, struct url_data *data )
{
    size_t index = data->size;
    size_t n = ( size * nmemb );
    char* tmp;

    data->size += ( size * nmemb );

    fprintf( stderr, "data at %p size=%ld nmemb=%ld\n", ptr, size, nmemb );

    tmp = ( char* ) realloc( data->data, data->size + 1 );

    if( tmp )
    {
        data->data = tmp;
    }
    else
    {
        if( data->data )
        {
            free( data->data );
        }
        fprintf( stderr, "Failed to allocate memory.\n" );
        return 0;
    }

    memcpy( ( data->data + index ), ptr, n );
    data->data[ data->size ] = '\0';

    return size * nmemb;
}


// Use libcrul to get the SDP file from the server
char * getSdp ( char* url )
{
    CURL *curl;
    CURLcode res;

    struct url_data data;

    data.size = 0;
    data.data = (char *) malloc( 4096 );
    if( NULL == data.data )
    {
        fprintf( stderr, "Failed to allocate memory.\n" );
        return NULL;
    }

    data.data[0] = '\0';

 
    curl = curl_easy_init( );
    if( curl )
    {
        curl_easy_setopt( curl, CURLOPT_URL, url );
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, write_data );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, &data );
        
        res = curl_easy_perform( curl );

        if( res != CURLE_OK )
          fprintf( stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror( res ) );
 
        curl_easy_cleanup( curl );
    }

    return data.data;
}



main()
{
    char * sdpDescription;
    giMediaSubsession *sub = NULL;

    // Setup a signal to catch Ctrl-C inorder to exit cleanly
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    // Set the exit flag to 0
    exitWatchVariable = 0;

    // Begin by setting up our usage environment:
    TaskScheduler* scheduler = BasicTaskScheduler::createNew();

    env = BasicUsageEnvironment::createNew(*scheduler);

    // Get the service description file from the server
    sdpDescription = getSdp("http://192.168.100.100/streamA.sdp");

    fprintf( stderr, "SDP = %s\n", sdpDescription ); 

    // Create a new media session
    scs.session = giMediaSession::createNew( *env, sdpDescription );

    if ( scs.session == NULL )
    {
        *env << "Failed to create a MediaSession object from the SDP description: " << env->getResultMsg() << "\n";
        exit(1);
    }
    else if ( !scs.session->hasSubsessions( ) )
    {
        *env << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
        exit(1);
    }


    /* Initialise each media subsession */
    MediaSubsessionIterator * iter = new MediaSubsessionIterator( *scs.session );

    while( ( sub = dynamic_cast<giMediaSubsession*>( iter->next( ) ) ) != NULL )
    {

        if ( sub != NULL )
        {
            if ( !sub->initiate( ) )
            {
                *env << "Failed to initiate the \"" << sub << "\" subsession: " << env->getResultMsg( ) << "\n";
            }
            else
            {
                *env << "Initiated the \"" << sub << "\" subsession\n";
            }
        }

        *env << "Set up the \"" << sub << "\" subsession (";
        if ( sub->rtcpIsMuxed( ) )
        {
            *env << "client port " << sub->clientPortNum();
        }
        else
        {
            *env << "client ports " << sub->clientPortNum() << "-" << sub->clientPortNum()+1;
        }
        *env << ")\n";

        if( strcmp(sub->codecName(),"MP2T") == 0 )
        {
            // Connect the MP2T subsession to a test sink.
            sub->sink = liveTestSink::createNew(*env, *sub);

        }
        else if( strcmp(sub->codecName(),"RAPTORFEC") == 0 )
        {
            MediaSubsessionIterator* tempIter = new MediaSubsessionIterator( *scs.session );

            giMediaSubsession *temSubsession = dynamic_cast<giMediaSubsession*>(tempIter->next( ));

            while(temSubsession != NULL)
            {
                if( strcmp(temSubsession->codecName(),"MP2T") == 0 )
                {
                    // Connect the RAPTORFEC subsession to the MP2T session.
                    sub->sink = temSubsession->raptorRtpSource();
                    break;
                }
            }
            delete tempIter;
        }

        if ( sub->sink == NULL )
        {
            *env << "Failed to create a data sink for the \"" << sub << "\" subsession: " << env->getResultMsg() << "\n";
            break;
        }

        *env << "Created a data sink for the \"" << sub << "\" subsession\n";

        sub->sink->startPlaying( *( sub->readSource( ) ), NULL, NULL );
    };


    env->taskScheduler( ).doEventLoop( &exitWatchVariable );

    delete iter; iter = NULL;

    env->reclaim();

    delete scheduler;  scheduler = NULL;



    printf("EXIT \n");
}
