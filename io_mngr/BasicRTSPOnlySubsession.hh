/*
 *  BasicRTSPOnlySubsession.hh
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
 * @file BasicRTSPOnlySubsession.hh
 * @brief RTSP server for io_mngr, it manages the RTSP streams.
 *
 */

#ifndef __BASIC_RTSP_ONLY_SUBSESSION_HH__
#define __BASIC_RTSP_ONLY_SUBSESSION_HH__

#ifndef _SERVER_MEDIA_SESSION_HH
#include <ServerMediaSession.hh>
#endif
extern "C" {
#include "stream.h"
}


/**
 * BasicRTSPOnlySubsession class is .
 */
class BasicRTSPOnlySubsession: public ServerMediaSubsession {

    private:
        Boolean fReuseFirstSource;
        void* fLastStreamToken;
        char fCNAME[100];
        stream_t *fStream;

        /**
         * Generates the SDP text and stores it on fSDPLines class member.
         */
        void setSDPLines();

    protected:
        char *fSDPLines;
        HashTable *fDestinationsHashTable;

        BasicRTSPOnlySubsession(UsageEnvironment& env, Boolean reuseFirstSource,
                stream_t *stream);

        virtual ~BasicRTSPOnlySubsession();	
        
        /**
         * Getter for fSDPLines, it generate the SDP text if needed.
         * @return char const * which contains the SDP text.
         */
        virtual char const *sdpLines();
        
        virtual void getStreamParameters(unsigned clientSessionId,
                netAddressBits clientAddress,
                Port const& clientRTPPort,
                Port const& clientRTCPPort,
                int tcpSocketNum,
                unsigned char rtpChannelId,
                unsigned char rtcpChannelId,
                netAddressBits& destinationAddress,
                u_int8_t& destinationTTL,
                Boolean& isMulticast,
                Port& serverRTPPort,
                Port& serverRTCPPort,
                void*& streamToken);
        
        virtual void startStream(unsigned clientSessionId, void* streamToken,
                TaskFunc* rtcpRRHandler, void* rtcpRRHandlerClientData,
                unsigned short& rtpSeqNum,
                unsigned& rtpTimestamp, 
                ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
                void* serverRequestAlternativeByteHandlerClientData);

        virtual void deleteStream(unsigned clientSessionId, void*& streamToken);

    public:
        /**
         * Creates a new instance of a BasicRTSPOnlySubsession.
         * @param env .
         * @param reuseFirstSource .
         * @return stream .
         */
        static BasicRTSPOnlySubsession*
            createNew(UsageEnvironment &env,
                    Boolean reuseFirstSource,
                    stream_t* stream); 
};


/**
 * Destinations class is .
 */
class Destinations {

    public:
        Boolean isTCP;
        struct in_addr addr;
        Port rtpPort;
        Port rtcpPort;
        int tcpSocketNum;
        unsigned char rtpChannelId, rtcpChannelId;

        Destinations(struct in_addr const& destAddr,
                Port const& rtpDestPort,
                Port const& rtcpDestPort)
            : isTCP(False), addr(destAddr), rtpPort(rtpDestPort), rtcpPort(rtcpDestPort) {
            }
        Destinations(int tcpSockNum, unsigned char rtpChanId, unsigned char rtcpChanId)
            : isTCP(True), rtpPort(0) /*dummy*/, rtcpPort(0) /*dummy*/,
            tcpSocketNum(tcpSockNum), rtpChannelId(rtpChanId), rtcpChannelId(rtcpChanId) {
            }
};

#endif //__BASIC_RTSP_ONLY_SUBSESSION_HH__

