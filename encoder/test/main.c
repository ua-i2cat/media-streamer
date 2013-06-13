#include <stdio.h>
#include <string.h>
#include "video_compress.h"
#include "video_compress/libavcodec.h"

int main (){
  struct compress_state *sc;
  sc = (struct compress_state *) calloc(2, sizeof(struct compress_state *));
  
  printf("Trying to initialize compressor\n");
 
  show_compress_help();
  printf("Compressor initialized ;^)\n");
 
  printf("Trying to initialize encoder\n");
  if (is_compress_none(sc)){
    if (compress_init("string", sc)){
      return 1;
    }
  }
  printf("Encoder initialized ;^)\n");
 
  printf("Free allocated memory\n");
  compress_done(sc);
 
  printf("Done successfully ;^)\n"); 
  return 0;
}