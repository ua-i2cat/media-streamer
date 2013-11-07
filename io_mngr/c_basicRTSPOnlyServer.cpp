#include "c_basicRTSPOnlyServer.h"

#include "BasicRTSPOnlyServer.hh"

int c_start_server(rtsp_serv_t* server){
    BasicRTSPOnlyServer *srv = BasicRTSPOnlyServer::getInstance(server->port, server->streams, server->transmitter);
    srv->init_server();
    return pthread_create(&server->server_th, NULL, srv->start_server, &server->watch);
}

rtsp_serv_t *init_rtsp_server(uint port, stream_list_t *streams, transmitter_t *transmitter){
    rtsp_serv_t *server = (rtsp_serv_t*) malloc(sizeof(rtsp_serv_t));
    server->port = port;
    server->streams = streams;
    server->transmitter = transmitter;
    server->watch = 0;
    return server;
}

void c_stop_server(rtsp_serv_t* server){
    server->watch = 1;
    pthread_join(server->server_th, NULL);
}
