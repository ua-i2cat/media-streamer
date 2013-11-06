#include "BasicRTSPOnlySubsession.hh"
#include <BasicUsageEnvironment.hh>
#include <RTSPServer.hh>
#include <GroupsockHelper.hh>


BasicRTSPOnlySubsession*
BasicRTSPOnlySubsession::createNew(UsageEnvironment& env,
				Boolean reuseFirstSource,
				stream_data_t* stream, 
                transmitter_t* transmitter){
	return new BasicRTSPOnlySubsession(env, reuseFirstSource,stream, transmitter);
}
 
BasicRTSPOnlySubsession
::BasicRTSPOnlySubsession(UsageEnvironment& env,
				Boolean reuseFirstSource,
				stream_data_t* stream, 
                transmitter_t* transmitter
                         )
  : ServerMediaSubsession(env),
    fSDPLines(NULL), 
    fReuseFirstSource(reuseFirstSource), fLastStreamToken(NULL) {
	fDestinationsHashTable = HashTable::create(ONE_WORD_HASH_KEYS);
	gethostname(fCNAME, sizeof fCNAME);
	this->fStream = stream;
    this->fTransmitter = transmitter;
	fCNAME[sizeof fCNAME-1] = '\0'; // just in case
}

BasicRTSPOnlySubsession::~BasicRTSPOnlySubsession() {
  
	delete[] fSDPLines;
	while (1) {
		Destinations* destinations
			= (Destinations*)(fDestinationsHashTable->RemoveNext());
		if (destinations == NULL) break;
		delete destinations;
	}
	delete fDestinationsHashTable;
}

char const* BasicRTSPOnlySubsession::sdpLines() {
	if (fSDPLines == NULL){
		setSDPLines();
	}
	return fSDPLines;
}

void BasicRTSPOnlySubsession
::setSDPLines() {

	//TODO: should be more dynamic
	unsigned estBitrate = 5000;
	char const* mediaType = "video";
	uint8_t rtpPayloadType = 96;
	AddressString ipAddressStr(fServerAddressForSDP);
	char* rtpmapLine = "a=rtpmap:96 H264/90000\n";
	char const* auxSDPLine = "";

	char const* const sdpFmt =
		"m=%s %u RTP/AVP %u\r\n"
		"c=IN IP4 %s\r\n"
		"b=AS:%u\r\n"
		"%s"
		"a=control:%s\r\n";
	unsigned sdpFmtSize = strlen(sdpFmt)
		+ strlen(mediaType) + 5 /* max short len */ + 3 /* max char len */
		+ strlen(ipAddressStr.val())
		+ 20 /* max int len */
		+ strlen(rtpmapLine)
		+ strlen(trackId());
	char* sdpLines = new char[sdpFmtSize];

	sprintf(sdpLines, sdpFmt,
		mediaType, // m= <media>
		fPortNumForSDP, // m= <port>
		rtpPayloadType, // m= <fmt list>
		ipAddressStr.val(), // c= address
		estBitrate, // b=AS:<bandwidth>
		rtpmapLine, // a=rtpmap:... (if present)
		trackId()); // a=control:<track-id>
	
	fSDPLines = sdpLines;
}

void BasicRTSPOnlySubsession::getStreamParameters(unsigned clientSessionId,
		      netAddressBits clientAddress,
		      Port const& clientRTPPort,
		      Port const& clientRTCPPort,
		      int tcpSocketNum,
		      unsigned char rtpChannelId,
		      unsigned char rtcpChannelId,
		      netAddressBits& destinationAddress,
		      u_int8_t& /*destinationTTL*/,
		      Boolean& isMulticast,
		      Port& serverRTPPort,
		      Port& serverRTCPPort,
		      void*& streamToken) {
	
	if (destinationAddress == 0) {
		destinationAddress = clientAddress;
	}
	
	struct in_addr destinationAddr;
	destinationAddr.s_addr = destinationAddress;
	
	Destinations* destinations;
	
	destinations = new Destinations(destinationAddr, clientRTPPort, clientRTCPPort);
	fDestinationsHashTable->Add((char const*)clientSessionId, destinations);
}


void BasicRTSPOnlySubsession::startStream(unsigned clientSessionId,
						void* streamToken,
						TaskFunc* rtcpRRHandler,
						void* rtcpRRHandlerClientData,
						unsigned short& rtpSeqNum,
						unsigned& rtpTimestamp,
						ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
						void* serverRequestAlternativeByteHandlerClientData) {

	Destinations* dst = 
		(Destinations*)(fDestinationsHashTable->Lookup((char const*)clientSessionId));
	if (dst == NULL){
		return;
	} else {
		pthread_rwlock_wrlock(&fTransmitter->participants->lock);
        add_transmitter_participant(fTransmitter, clientSessionId,  inet_ntoa(dst->addr), ntohs(dst->rtpPort.num()));
		add_participant_stream(fTransmitter->participants->first, fStream);
        init_transmission(fTransmitter->participants->first, fTransmitter);
		pthread_rwlock_unlock(&fTransmitter->participants->lock);
	}
}

void BasicRTSPOnlySubsession::pauseStream(unsigned clientSessionId, void* streamToken){
// 	Destinations* dst = 
// 		(Destinations*)(fDestinationsHashTable->Lookup((char const*)clientSessionId));
// 	if (dst == NULL){
// 		return;
// 	} else {
// 		stream_data_t *participant;
// 		pthread_rwlock_wrlock(&list->lock);
// 		remove_participant_ssrc(list, clientSessionId);
// 		pthread_rwlock_unlock(&list->lock);
// 	}
}

void BasicRTSPOnlySubsession::deleteStream(unsigned clientSessionId, void*& streamToken){
// 	Destinations* dst = 
// 		(Destinations*)(fDestinationsHashTable->Lookup((char const*)clientSessionId));
// 	if (dst == NULL){
// 		return;
// 	} else {
// 		stream_data_t *participant;
// 		pthread_rwlock_wrlock(&list->lock);
// 		remove_participant_ssrc(list, clientSessionId);
// 		pthread_rwlock_unlock(&list->lock);
// 	}
}