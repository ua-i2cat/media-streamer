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
	int port;
	stream_list_t* streams;
	transmitter_t* transmitter;
    pthread_t server_th;
} rtsp_serv_t;

EXTERNC int c_init_server(rtsp_serv_t* server);

EXTERNC int c_start_server(rtsp_serv_t* server);

#undef EXTERNC

