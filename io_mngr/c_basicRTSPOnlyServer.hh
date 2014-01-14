/*
 *  c_basicRTSPOnlyServer.hh
 *  Copyright (C) 2013  Fundació i2CAT, Internet i Innovació digital a Catalunya
 *
 *  This file is part of io_mngr.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Authors:  Jordi "Txor" Casas Ríos <txorlings@gmail.com>,
 *            David Cassany <david.cassany@i2cat.net>
 */

/**
 * @file c_basicRTSPOnlyServer.hh
 * @brief 
 */

#ifndef __C_BASIC_RTSP_ONLY_SERVER_HH__
#define __C_BASIC_RTSP_ONLY_SERVER_HH__

#ifdef __cplusplus
extern "C" {
#endif
#include "transmitter.h"

    typedef struct rtsp_serv {
        uint port;
        transmitter_t* transmitter;
        pthread_t server_th;
        uint8_t watch;
        uint8_t run;
    } rtsp_serv_t;

    int c_start_server(rtsp_serv_t* server);

    void c_stop_server(rtsp_serv_t* server);

    int c_update_server(rtsp_serv_t* server); 

    rtsp_serv_t* init_rtsp_server(uint port, transmitter_t *transmitter);

#ifdef __cplusplus
}
#endif

#endif //__C_BASIC_RTSP_ONLY_SERVER_HH__

