#ifndef _BASIC_RTSP_ONLY_SERVER_HH
#define _BASIC_RTSP_ONLY_SERVER_HH

#include <RTSPServer.hh>
#include <BasicUsageEnvironment.hh>
#include "BasicRTSPOnlySubsession.hh"



class BasicRTSPOnlyServer {
private:
    BasicRTSPOnlyServer(int port, transmitter_t* transmitter);
    
public:
    static BasicRTSPOnlyServer* initInstance(int port, transmitter_t* transmitter);
    static BasicRTSPOnlyServer* getInstance();
    
    int init_server();

    static void *start_server(void *args);
    
    int update_server();
    
private:
    
    static BasicRTSPOnlyServer* srvInstance;
    int fPort;
    transmitter_t* fTransmitter;
    RTSPServer* rtspServer;
    UsageEnvironment* env;

};


#endif