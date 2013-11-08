#include "c_basicRTSPOnlyServer.h"

#include "BasicRTSPOnlyServer.hh"

// int c_init_server(rtsp_serv_t* server){
// 	return init_server(server->port, server->streams, server->transmitter);
// }

int c_start_server(rtsp_serv_t* server){
    init_server(server->port, server->streams, server->transmitter);
    return pthread_create(&server->server_th, NULL, start_server, NULL);
}

rtsp_serv_t *init_rtsp_server(uint port, stream_list_t *streams, transmitter_t *transmitter){
    rtsp_serv_t *server = (rtsp_serv_t*) malloc(sizeof(rtsp_serv_t));
    server->port = port;
    server->streams = streams;
    server->transmitter = transmitter;
    return server;
}