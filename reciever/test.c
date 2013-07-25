#include "rtp/rtp.h"
#include "rtp/rtp_callback.h"
#include "rtp/rtpdec.h"
#include "rtp/rtpenc.h"
#include "pdb.h"
#include "video.h"
#include "tv.h"

#include "participants.h"
#include "reciever.h"

#include "video_decompress.h"
#include "video_decompress/libavcodec.h"

#define INITIAL_VIDEO_RECV_BUFFER_SIZE  ((4*1920*1080)*110/100) //command line net.core setup: sysctl -w net.core.rmem_max=9123840

int main(){
  participant_list_t *list;
  
  list = init_participant_list();
  
  add_participant(list, 1080, 720, H264, NULL, 0, INPUT);
  add_participant(list, 1080, 720, H264, NULL, 0, INPUT);
  add_participant(list, 1080, 720, H264, NULL, 0, INPUT);
  
  pthread_t input_th;
  
  pthread_create(&input_th, NULL, &start_input, (void *) list);
  
  
  pthread_join(input_th, NULL);
}