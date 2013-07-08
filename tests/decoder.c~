#include <stdio.h>
#include <string.h>
#include "video_decompress.h"
#include "video_decompress/libavcodec.h"

int main (){
  struct state_decompress *sd;
  sd = (struct state_decompress *) calloc(2, sizeof(struct state_decompress *));
  
  printf("Trying to initialize decompressor\n");
 
  initialize_video_decompress();
  printf("Decompressor initialized ;^)\n");
 
  printf("Trying to initialize decoder\n");
  if (decompress_is_available(LIBAVCODEC_MAGIC)){
    sd = decompress_init(LIBAVCODEC_MAGIC);
  }
  printf("Decoder initialized ;^)\n");
 
  printf("Free allocated memory\n");
  decompress_done(sd);
 
  printf("Done successfully ;^)\n"); 
}