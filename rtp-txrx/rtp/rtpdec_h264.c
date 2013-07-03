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
  
  char *dst = NULL;
  
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
      buffers->frame_buffer[substream] = (char*) malloc(total_length);
      dst = buffers->frame_buffer[substream];
    }
    
    
    while (cdata != NULL) {
      pckt = cdata->data;
    
      nal  = (uint8_t) pckt->data[0];
      type = nal & 0x1f;
  
      if (type >= 1 && type <= 23) {
	type = 1;
      }
      
      const uint8_t *src = NULL;
      
      switch (type) {
	case 0:
	//One packet one NAL
	case 1: 
	  if (pass == 0){
	    total_length += sizeof(start_sequence) + pckt->data_len;
	  } else {
	    memcpy(dst, start_sequence, sizeof(start_sequence));
	    memcpy(dst + sizeof(start_sequence), pckt->data, pckt->data_len);
	    dst += pckt->data_len + sizeof(start_sequence);
	  }
	  break;
	//One packet multiple NALs
	case 24:
	  pckt->data++;
	  pckt->data_len--;

          src = (const uint8_t *) pckt->data;
          int src_len        = pckt->data_len;

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
                      assert(dst);
                      memcpy(dst, start_sequence, sizeof(start_sequence));
                      memcpy(dst + sizeof(start_sequence), src, nal_size);
                      dst += nal_size + sizeof(start_sequence);
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
	  pckt->data++;
	  pckt->data_len--;                 // skip the fu_indicator
	  if (pckt->data_len > 1) {
            // these are the same as above, we just redo them here for clarity
	    src = (const uint8_t *) cdata->data;
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

          // skip the fu_header
	    pckt->data++;
	    pckt->data_len--;

            //if (start_bit)
            //    COUNT_NAL_TYPE(data, nal_type);
	    if (pass == 0){
	      if (start_bit) {
                total_length += sizeof(start_sequence) + sizeof(nal) + pckt->data_len;
	      } else {
		total_length += pckt->data_len;
	      }
	    } else {
	      if (start_bit) {
		/* copy in the start sequence, and the reconstructed nal */
		assert(dst);
		memcpy(dst, start_sequence, sizeof(start_sequence));
		dst[sizeof(start_sequence)] = reconstructed_nal;
		memcpy(dst + sizeof(start_sequence) + sizeof(nal), pckt->data, pckt->data_len);
		dst += sizeof(start_sequence) + sizeof(nal) + pckt->data_len;
	      } else {
		assert(dst);
		memcpy(dst, pckt->data, pckt->data_len);
		dst += pckt->data_len;
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