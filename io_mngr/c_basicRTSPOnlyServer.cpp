/*
 *  c_basicRTSPOnlyServer.cpp - C client to BasicRTSPOnlyServer implementation.
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

#include "c_basicRTSPOnlyServer.hh"
#include "BasicRTSPOnlyServer.hh"

rtsp_serv_t *init_rtsp_server(uint port, transmitter_t *transmitter)
{
    rtsp_serv_t *server;
    if ((server = (rtsp_serv_t*) malloc(sizeof(rtsp_serv_t))) == NULL) {
        error_msg("init_rtsp_server out of memory!");
        return NULL;
    }
    server->port = port;
    server->transmitter = transmitter;
    server->watch = 0;
    server->run = false;

    return server;
}

int c_start_server(rtsp_serv_t *server)
{
    int ret;

    BasicRTSPOnlyServer *srv = BasicRTSPOnlyServer::initInstance(server->port, server->transmitter);
    srv->init_server();
    ret = pthread_create(&server->server_th, NULL, BasicRTSPOnlyServer::start_server, &server->watch);
    if (ret == 0) {
        server->run = true;
    } else {
        server->run = false;
    }

    return ret;
}

void c_stop_server(rtsp_serv_t *server)
{
    server->watch = 1;
    if (server->run){
        pthread_join(server->server_th, NULL);
    }
}

void c_update_server(rtsp_serv_t *server)
{
    if ((BasicRTSPOnlyServer *srv = BasicRTSPOnlyServer::getInstance()) == NULL) {
        exit(1);
    } else {
        srv->update_server();
    }
}

