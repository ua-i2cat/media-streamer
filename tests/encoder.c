#include <stdio.h>
#include <string.h>
#include "video_compress.h"
#include "video_compress/libavcodec.h"

int main (){
  struct compress_state *sc;
  sc = (struct compress_state *) calloc(2, sizeof(struct compress_state *));
  
  printf("Give me some compressor help\n");
  show_compress_help();
  
  printf("Trying to initialize encoder\n");
  compress_init("libavcodec:codec=H.264", &sc);
  printf("Encoder initialized ;^)\n");
 
  printf("Free allocated memory\n");
  compress_done(sc);
 
  printf("Done successfully ;^)\n"); 
  return 0;
}
