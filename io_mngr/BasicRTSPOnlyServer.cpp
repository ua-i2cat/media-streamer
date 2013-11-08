#include "BasicRTSPOnlyServer.hh"

RTSPServer* rtspServer;
UsageEnvironment* env;

int init_server(int port, stream_list_t* streams, transmitter_t* transmitter) {
	
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

  if (port == 0){
	  port = 8554;
  }
  
  rtspServer = RTSPServer::createNew(*env, port, authDB);
  if (rtspServer == NULL) {
	*env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
	exit(1);
  }
  
  stream_data_t* stream = (stream_data_t*) malloc(sizeof(stream_data_t));
  stream = streams->first;
  
  ServerMediaSession* sms;
  
  while (stream != NULL){
    sms = ServerMediaSession::createNew(*env, stream->stream_name, 
					stream->stream_name,
					stream->stream_name);
	
    sms->addSubsession(BasicRTSPOnlySubsession
		       ::createNew(*env, True, stream, transmitter));
    rtspServer->addServerMediaSession(sms);
	
	char* url = rtspServer->rtspURL(sms);
    *env << "\n\"" << stream->stream_name << "\" stream, from a UDP Transport Stream input source \n\t(";
    *env << "Play this stream using the URL \"" << url << "\"\n";
    delete[] url;
	
	stream = stream->next;
  }
  
  return 0;
}

void *start_server(void *args){
	if (env == NULL || rtspServer == NULL){
		return NULL;
	}
	env->taskScheduler().doEventLoop(); 
	
	return NULL;
}

//TODO:Update server method and stop server method