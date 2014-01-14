/*
 *  BasicRTSPOnlyServer.hh
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

#ifndef __BASIC_RTSP_ONLY_SERVER_HH__
#define __BASIC_RTSP_ONLY_SERVER_HH__

#include <RTSPServer.hh>
#include <BasicUsageEnvironment.hh>
#include "BasicRTSPOnlySubsession.hh"
extern "C" {
#include "transmitter.h"
}

class BasicRTSPOnlyServer {

    private:
        BasicRTSPOnlyServer(int port, transmitter_t* transmitter);

    public:
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

