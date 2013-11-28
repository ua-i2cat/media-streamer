#include "BasicRTSPOnlyServer.hh"

BasicRTSPOnlyServer *BasicRTSPOnlyServer::srvInstance = NULL;

BasicRTSPOnlyServer::BasicRTSPOnlyServer(int port, transmitter_t* transmitter){
    if(transmitter == NULL){
        exit(1);
    }
    
    this->fPort = port;
    this->fTransmitter = transmitter;
    this->rtspServer = NULL;
    this->env = NULL;
    this->srvInstance = this;
}

BasicRTSPOnlyServer* 
BasicRTSPOnlyServer::initInstance(int port, transmitter_t* transmitter){
    if (srvInstance != NULL){
        return srvInstance;
    }
    return new BasicRTSPOnlyServer(port, transmitter);
}

BasicRTSPOnlyServer* 
BasicRTSPOnlyServer::getInstance(){
    if (srvInstance != NULL){
        return srvInstance;
    }
    return NULL;
}

int BasicRTSPOnlyServer::init_server() {
    
    if (env != NULL || rtspServer != NULL || 
        fTransmitter == NULL){
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
  
    return 0;
}

int BasicRTSPOnlyServer::update_server(){
    stream_data_t* stream = (stream_data_t*) malloc(sizeof(stream_data_t));
    
    pthread_rwlock_rdlock(&fTransmitter->stream_list->lock);
    stream = fTransmitter->stream_list->first;
  
    ServerMediaSession* sms;
  
    while (stream != NULL){
        sms = rtspServer->lookupServerMediaSession(stream->stream_name);
        if (sms == NULL){
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
    pthread_rwlock_unlock(&fTransmitter->stream_list->lock);
}

void *BasicRTSPOnlyServer::start_server(void *args){
    char* watch = (char*) args;
    BasicRTSPOnlyServer* instance = getInstance();
    
	if (instance == NULL || instance->env == NULL || instance->rtspServer == NULL){
		return NULL;
	}
	instance->env->taskScheduler().doEventLoop(watch); 
    
    Medium::close(instance->rtspServer);
    delete &instance->env->taskScheduler();
    instance->env->reclaim();
	
	return NULL;
}
