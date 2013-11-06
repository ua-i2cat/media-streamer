#include "c_basicRTSPOnlyServer.h"

#include "BasicRTSPOnlyServer.hh"

int c_init_server(rtsp_serv_t* server){
	return init_server(server->port, server->streams, server->transmitter);
}

int c_start_server(rtsp_serv_t* server){
    return pthread_create(&server->server_th, NULL, start_server, NULL);
}