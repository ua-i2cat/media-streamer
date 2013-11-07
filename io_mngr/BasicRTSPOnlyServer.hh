#ifndef _BASIC_RTSP_ONLY_SERVER_HH
#define _BASIC_RTSP_ONLY_SERVER_HH

#include <RTSPServer.hh>
#include <BasicUsageEnvironment.hh>
#include "BasicRTSPOnlySubsession.hh"



class BasicRTSPOnlyServer {
private:
    BasicRTSPOnlyServer(int port, stream_list_t* streams, transmitter_t* transmitter);
    
public:
    static BasicRTSPOnlyServer* getInstance(int port, stream_list_t* streams, transmitter_t* transmitter);
    int init_server();

    void *start_server(void *args);
    
    int update_server();
    
private:
    
    static BasicRTSPOnlyServer* srvInstance;
    int fPort;
    stream_list_t* fStreams;
    transmitter_t* fTransmitter;
    RTSPServer* rtspServer;
    UsageEnvironment* env;

};


#endif