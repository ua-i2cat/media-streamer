#include "BasicRTSPOnlyServer.hh"


BasicRTSPOnlyServer::BasicRTSPOnlyServer(int port, stream_list_t* streams, 
                        transmitter_t* transmitter){
    if(transmitter == NULL || streams == NULL){
        exit(1);
    }
    
    this->fPort = port;
    this->fStreams = streams;
    this->fTransmitter = transmitter;
    this->rtspServer = NULL;
    this->env = NULL;
    this->srvInstance = this;
}

BasicRTSPOnlyServer* 
BasicRTSPOnlyServer::getInstance(int port, stream_list_t* streams, transmitter_t* transmitter){
    if (srvInstance != NULL){
        return srvInstance;
    }
    return new BasicRTSPOnlyServer(port, streams, transmitter);
}


int BasicRTSPOnlyServer::init_server() {
    
    if (env != NULL || rtspServer != NULL || 
        fStreams == NULL || fTransmitter == NULL){
        exit(1);
    }
    
    TaskScheduler* scheduler = BasicTaskScheduler::createNew();
    env = BasicUsageEnvironment::createNew(*scheduler);

    UserAuthenticationDatabase* authDB = NULL;   
// #ifdef ACCESS_CONTROL
//   // To implement client access control to the RTSP server, do the following:
//   authDB = new UserAuthenticationDatabase;
//   authDB->addUserRecord("username1", "password1"); // replace these with real strings
//   // Repeat the above with each <username>, <password> that you wish to allow
//   // access to the server.
// #endif

    if (fPort == 0){
        fPort = 8554;
    }
  
    rtspServer = RTSPServer::createNew(*env, fPort, authDB);
    if (rtspServer == NULL) {
        *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
        exit(1);
    }
  
    stream_data_t* stream = (stream_data_t*) malloc(sizeof(stream_data_t));
    stream = fStreams->first;
  
    ServerMediaSession* sms;
  
    while (stream != NULL){
        sms = ServerMediaSession::createNew(*env, stream->stream_name, 
					stream->stream_name,
					stream->stream_name);
	
        sms->addSubsession(BasicRTSPOnlySubsession
		       ::createNew(*env, True, stream, fTransmitter));
        rtspServer->addServerMediaSession(sms);
	
        char* url = rtspServer->rtspURL(sms);
        *env << "\nPlay this stream using the URL \"" << url << "\"\n";
        delete[] url;
	
        stream = stream->next;
    }
  
    return 0;
}

int BasicRTSPOnlyServer::update_server(){
    stream_data_t* stream = (stream_data_t*) malloc(sizeof(stream_data_t));
    stream = fStreams->first;
  
    ServerMediaSession* sms;
  
    while (stream != NULL){
        sms = rtspServer->lookupServerMediaSession(stream->stream_name);
        if (sms == NULL){
            sms = ServerMediaSession::createNew(*env, stream->stream_name, 
                    stream->stream_name,
                    stream->stream_name);
    
            sms->addSubsession(BasicRTSPOnlySubsession
               ::createNew(*env, True, stream, fTransmitter));
            rtspServer->addServerMediaSession(sms);
    
            char* url = rtspServer->rtspURL(sms);
            *env << "\nPlay this stream using the URL \"" << url << "\"\n";
            delete[] url;
        }
        stream = stream->next;
    }
}

void *BasicRTSPOnlyServer::start_server(void *args){
    char* watch = (char*) args;
    
	if (env == NULL || rtspServer == NULL){
		return NULL;
	}
	env->taskScheduler().doEventLoop(watch); 
    
    Medium::close(rtspServer);
    delete &env->taskScheduler();
    env->reclaim();
	
	return NULL;
}

//TODO:Update server method