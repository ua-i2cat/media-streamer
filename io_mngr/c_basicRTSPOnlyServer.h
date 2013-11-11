#ifndef C_BASIC_RTSP_ONLY_SERVER_H
#define C_BASIC_RTSP_ONLY_SERVER_H
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "transmitter.h"

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC typedef struct rtsp_serv {
	uint port;
	stream_list_t* streams;
	transmitter_t* transmitter;
    pthread_t server_th;
    uint8_t watch;
    uint8_t run;
} rtsp_serv_t;

EXTERNC int c_start_server(rtsp_serv_t* server);

EXTERNC void c_stop_server(rtsp_serv_t* server);

EXTERNC int c_update_server(rtsp_serv_t* server); 

EXTERNC rtsp_serv_t* init_rtsp_server(uint port, stream_list_t *streams, transmitter_t *transmitter);

#undef EXTERNC

