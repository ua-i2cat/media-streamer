/*
 *  BasicRTSPOnlyServer.hh - Main RTSP server for io_mngr.
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
 * @file BasicRTSPOnlyServer.hh
 * @brief RTSP server for io_mngr, it manages the RTSP streams.
 *
 */

#ifndef __BASIC_RTSP_ONLY_SERVER_HH__
#define __BASIC_RTSP_ONLY_SERVER_HH__

#include <RTSPServer.hh>
#include <BasicUsageEnvironment.hh>
#include "BasicRTSPOnlySubsession.hh"
extern "C" {
#include "transmitter.h"
}

/**
 * BasicRTSPOnlyServer class contains the main engine for RTSP protocol handling. It binds to a port and serves a transmitter_t configuration.
 */
class BasicRTSPOnlyServer {

    private:
        BasicRTSPOnlyServer(int port, transmitter_t* transmitter);

    public:
        /**
         * Initializes the server.
         * @param port Port to listen.
         * @param transmitter Transmitter instance whose configuration is published.
         * @return BasicRTSPOnlyServer* The server instance.
         */
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

#endif //__BASIC_RTSP_ONLY_SERVER_HH__

