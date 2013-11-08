#include <RTSPServer.hh>
#include <BasicUsageEnvironment.hh>
#include "BasicRTSPOnlySubsession.hh"

int init_server(int port, stream_list_t* streams, transmitter_t* transmitter);

void *start_server(void *args);