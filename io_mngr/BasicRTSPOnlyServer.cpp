/*
 *  BasicRTSPOnlyServer.cpp
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

#include "BasicRTSPOnlyServer.hh"
extern "C" {
#include "stream.h"
}

BasicRTSPOnlyServer *BasicRTSPOnlyServer::srvInstance = NULL;

BasicRTSPOnlyServer::BasicRTSPOnlyServer(int port, transmitter_t* transmitter)
{
    if(transmitter == NULL) {
        exit(1);
    }
    this->fPort = port;
    this->fTransmitter = transmitter;
    this->rtspServer = NULL;
    this->env = NULL;
    this->srvInstance = this;
}

    BasicRTSPOnlyServer* 
BasicRTSPOnlyServer::initInstance(int port, transmitter_t* transmitter)
{
    if (srvInstance != NULL) {
        return srvInstance;
    }

    return new BasicRTSPOnlyServer(port, transmitter);
}

    BasicRTSPOnlyServer* 
BasicRTSPOnlyServer::getInstance()
{
    if (srvInstance != NULL) {
        return srvInstance;
    }

    return NULL;
}

int BasicRTSPOnlyServer::init_server()
{
    if (env != NULL || rtspServer != NULL || 
            fTransmitter == NULL) {
        exit(1);
    }

    TaskScheduler* scheduler = BasicTaskScheduler::createNew();
    env = BasicUsageEnvironment::createNew(*scheduler);

    UserAuthenticationDatabase* authDB = NULL;   

    if (fPort == 0) {
        fPort = 8554;
    }

    rtspServer = RTSPServer::createNew(*env, fPort, authDB);
    if (rtspServer == NULL) {
        *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
        exit(1);
    }

    return 0;
}

int BasicRTSPOnlyServer::update_server()
{
    stream_t* stream = (stream_t*) malloc(sizeof(stream_t));

    pthread_rwlock_rdlock(&fTransmitter->video_stream_list->lock);
    stream = fTransmitter->video_stream_list->first;

    ServerMediaSession* sms;

    while (stream != NULL) {
        sms = rtspServer->lookupServerMediaSession(stream->stream_name);
        if (sms == NULL) {
            sms = ServerMediaSession::createNew(*env, stream->stream_name, 
                    stream->stream_name,
                    stream->stream_name);

            sms->addSubsession(BasicRTSPOnlySubsession
                    ::createNew(*env, True, stream));
            rtspServer->addServerMediaSession(sms);

            char* url = rtspServer->rtspURL(sms);
            *env << "\nPlay this stream using the URL \"" << url << "\"\n";
            delete[] url;
        }
        stream = stream->next;
    }
    pthread_rwlock_unlock(&fTransmitter->video_stream_list->lock);
}

void *BasicRTSPOnlyServer::start_server(void *args)
{
    char* watch = (char*) args;
    BasicRTSPOnlyServer* instance = getInstance();

    if (instance == NULL || instance->env == NULL || instance->rtspServer == NULL) {
        return NULL;
    }
    instance->env->taskScheduler().doEventLoop(watch); 

    Medium::close(instance->rtspServer);
    delete &instance->env->taskScheduler();
    instance->env->reclaim();

    return NULL;
}

