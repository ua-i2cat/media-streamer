/*
 *  BasicRTSPOnlyServer.hh - RTSP server for media-streamer.
 *  Copyright (C) 2013  Fundació i2CAT, Internet i Innovació digital a Catalunya
 *
 *  This file is part of media-streamer.
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
 * @brief RTSP server for media-streamer.
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
 * BasicRTSPOnlyServer class is a wrapper to RTSPServer to enable the dinamic use of participants and streams from a transmitter on media-streamer.
 */
class BasicRTSPOnlyServer {

    private:
        static BasicRTSPOnlyServer* srvInstance;
        int fPort;
        transmitter_t* fTransmitter;
        RTSPServer* rtspServer;
        UsageEnvironment* env;

        BasicRTSPOnlyServer(int port, transmitter_t* transmitter);

    public:
        /**
         * Initializes the server.
         * @param port Port to listen.
         * @param transmitter Transmitter instance.
         * @return The server instance.
         */
        static BasicRTSPOnlyServer* initInstance(int port, transmitter_t* transmitter);

        /**
         * Initialize the RTSP server, if no port was configured it uses 8554 by default.
         */
        void init_server();

        /**
         * Callback to start the server.
         * @param args Stop condition argument.
         * @return NULL.
         */
        static void *start_server(void *args);

        /**
         * Updates the RTSP server media information for each current stream.
         */
        void update_server();

        /**
         * Getter for the server instance.
         * @return The server instance or NULL.
         */
        static BasicRTSPOnlyServer* getInstance();
};

#endif //__BASIC_RTSP_ONLY_SERVER_HH__

