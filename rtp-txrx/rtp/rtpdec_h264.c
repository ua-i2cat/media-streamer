#ifdef HAVE_CONFIG_H
#include "config.h"
#include "config_unix.h"
#endif // HAVE_CONFIG_H
#include "debug.h"
#include "perf.h"
#include "rtp/rtp.h"
#include "rtp/rtp_callback.h"
#include "rtp/pbuf.h"
#include "rtp/rtpdec.h"

static const uint8_t start_sequence[] = { 0, 0, 0, 1 };

int decode_frame_h264(struct coded_data *cdata, void *rx_data)
{
  int ret = TRUE;
  rtp_packet *pckt = NULL;
  int substream = 0;
  struct coded_data *orig = cdata;

  uint8_t nal;
  uint8_t type;
  
  int pass;
  int total_length = 0;
  
  uint8_t *dst = NULL;
  int src_len;
  
  struct recieved_data *buffers = (struct recieved_data *) rx_data;
  for (int i = 0; i < (int) MAX_SUBSTREAMS; ++i) {
    buffers->buffer_len[i] = 0;
    buffers->buffer_num[i] = 0;
    buffers->frame_buffer[i] = NULL;
  }
  
  for(pass = 0; pass < 2; pass++){
    
    if (pass > 0){
      cdata = orig;   
      buffers->buffer_len[substream] = total_length;
      buffers->frame_buffer[substream] = (uint8_t*) malloc(total_length);
      dst = buffers->frame_buffer[substream] + total_length;
    }
    
    printf("\n\n\nPASS %d\n\n\n", pass);
    
    while (cdata != NULL) {
      pckt = cdata->data;
      assert(pckt->pt == PT_H264);
      
      nal  = (uint8_t) pckt->data[0];
      type = nal & 0x1f;
      
      printf("\n\n\n\ntype of nal: %d\n", type);
  
      if (type >= 1 && type <= 23) {
	type = 1;
      }
      
      printf("byte 1: %d ", pckt->data[0]);
      printf("byte 2: %d ", pckt->data[1]);
      printf("byte 3: %d ", pckt->data[2]);
      printf("byte 4: %d \n", pckt->data[3]);
      
      const uint8_t *src = NULL;
      
      switch (type) {
	case 0:
	//One packet one NAL
	case 1: 
	  if (pass == 0){
	    total_length += sizeof(start_sequence) + pckt->data_len;
	  } else {
	    dst -= pckt->data_len + sizeof(start_sequence);
	    memcpy(dst, start_sequence, sizeof(start_sequence));
	    memcpy(dst + sizeof(start_sequence), pckt->data, pckt->data_len);
	  }
	  break;
	//One packet multiple NALs
	case 24:
	  src = (const uint8_t *) pckt->data;
          src_len = pckt->data_len;

	  src++;
	  src_len--;
	  
          while (src_len > 2) {
              //uint16_t nal_size = AV_RB16(src);
              uint16_t nal_size;
              memcpy(&nal_size, src, sizeof(uint16_t));

                    // consume the length of the aggregate
              src     += 2;
              src_len -= 2;
		
              if (nal_size <= src_len) {
                  if (pass == 0) {
                      total_length += sizeof(start_sequence) + nal_size;
                  } else {
		      dst -= nal_size + sizeof(start_sequence);
                      memcpy(dst, start_sequence, sizeof(start_sequence));
                      memcpy(dst + sizeof(start_sequence), src, nal_size);
                  }
              } else {
                    //av_log(ctx, AV_LOG_ERROR,
                    //       "nal size exceeds length: %d %d\n", nal_size, src_len);
              }
                    // eat what we handled
              src     += nal_size;
              src_len -= nal_size;

                //if (src_len < 0)
                    //av_log(ctx, AV_LOG_ERROR,
                    //       "Consumed more bytes than we got! (%d)\n", src_len);
          }
          break;
  
	case 25:
	case 26:
	case 27:
	case 29:
	//error unhandled type
	  break;
	case 28:
	  src = (const uint8_t *) pckt->data;
          src_len = pckt->data_len;

	  src++;
	  src_len--;                // skip the fu_indicator
	  
	  if (src_len > 1) {
            // these are the same as above, we just redo them here for clarity
            
	    uint8_t fu_indicator      	= nal;
	    uint8_t fu_header         	= *src;
	    uint8_t start_bit         	= fu_header >> 7;
	    uint8_t end_bit 		= (fu_header & 0x40) >> 6;
	    uint8_t nal_type          	= fu_header & 0x1f;
	    uint8_t reconstructed_nal;

            // Reconstruct this packet's true nal; only the data follows.
            /* The original nal forbidden bit and NRI are stored in this
             * packet's nal. */
	    reconstructed_nal  = fu_indicator & 0xe0;
	    reconstructed_nal |= nal_type;
	    
	    printf("src byte 1: %d ", src[0]); 
	    printf("src byte 2: %d ", src[1]);
	    printf("src byte 3: %d ", src[2]);
	    printf("src byte 4: %d \n", src[3]);
	    
	    printf("fu_indicator: %d\n", fu_indicator);
	    printf("fu_header: %d\n", fu_header);
	    printf("start_bit: %d\n", start_bit);
	    printf("end_bit: %d\n", end_bit);
	    printf("nal_type: %d\n", nal_type);
	    printf("reconstructed_nal: %d\n", reconstructed_nal);
	    
	    // skip the fu_header
	    src++;
	    src_len--;

	    if (pass == 0){
	      if (start_bit) {
                total_length += sizeof(start_sequence) + sizeof(reconstructed_nal) + src_len;
	      } else {
		total_length += src_len;
	      }
	    } else {
	      if (start_bit) {
		/* copy in the start sequence, and the reconstructed nal */
		dst -= sizeof(start_sequence) + sizeof(reconstructed_nal) + src_len;
		memcpy(dst, start_sequence, sizeof(start_sequence));
		memcpy(dst + sizeof(start_sequence), &reconstructed_nal, sizeof(reconstructed_nal));
		memcpy(dst + sizeof(start_sequence) + sizeof(reconstructed_nal), src, src_len);
	      } else {
		dst -= src_len;
		memcpy(dst, src, src_len);
	      }
	    }
	  } else {
	      //av_log(ctx, AV_LOG_ERROR, "Too short data for FU-A H264 RTP packet\n");
	      //result = AVERROR_INVALIDDATA;
	  }
	  break;
      }
      cdata = cdata->nxt;
    }
  }
  return ret;
}