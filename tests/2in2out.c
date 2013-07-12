#include "rtp/rtp.h"
#include "rtp/rtp_callback.h"
#include "rtp/rtpdec.h"
#include "rtp/rtpenc.h"
#include "pdb.h"
#include "video.h"

#include "video_compress.h"
#include "video_compress/libavcodec.h"
#include "video_decompress.h"
#include "video_decompress/libavcodec.h"

#include "utils/resource_manager.h"
#include "utils/worker.h"

#define INITIAL_VIDEO_RECV_BUFFER_SIZE  ((4*1920*1080)*110/100) //command line net.core setup: sysctl -w net.core.rmem_max=9123840

FILE *F_video_rx=NULL;

int main(){
    struct rtp **devices = NULL;
    struct pdb *participants;
    struct pdb_e *cp;
    struct video_frame *frame;

    int required_connections = 2;
    int ndev = required_connections;
    int dev_id=0;

    int ret;

    struct video_frame *tx_frame;

    tx_frame = vf_alloc(1);
    vf_get_tile(tx_frame, 0)->width=640;
    vf_get_tile(tx_frame, 0)->height=480;
    tx_frame->fps=15;
    tx_frame->color_spec=H264;
    tx_frame->interlacing=PROGRESSIVE;

    frame = vf_alloc(1);
    vf_get_tile(frame, 0)->width=640;
    vf_get_tile(frame, 0)->height=480;
    frame->fps=15;
    frame->color_spec=UYVY;
    frame->interlacing=PROGRESSIVE;

    double rtcp_bw = 5 * 1024 * 1024; /* FIXME */
    int ttl = 255;
    char *saveptr = NULL;
    char *addr="127.0.0.1";
    char *mcast_if= NULL;
    struct timeval curr_time;
    struct timeval timeout;
    struct timeval prev_time;
    struct timeval start_time;
    gettimeofday(&start_time, NULL);


    uint32_t ts;
    int recv_port = 5004;
    int send_port = 7004;

    int exit = 1;

    gettimeofday(&prev_time, NULL);

    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;
    uint32_t timestamp;



    devices = (struct rtp **) malloc((required_connections + 1) * sizeof(struct rtp *));
    participants = pdb_init();

    /* Init decompress */
    struct state_decompress *sd;
    sd = (struct state_decompress *) calloc(2, sizeof(struct state_decompress *));
    printf("Trying to initialize decompressor\n");
    initialize_video_decompress();
    printf("Decompressor initialized ;^)\n");
    printf("Trying to initialize decoder\n");

    struct video_desc des;
    printf("Trying to initialize decoder\n");
    if (decompress_is_available(LIBAVCODEC_MAGIC)) {
        sd = decompress_init(LIBAVCODEC_MAGIC);

      	des.width = 640;
        des.height = 480;
        des.color_spec  = H264;
        des.tile_count = 0;
        des.interlacing = PROGRESSIVE;
        des.fps=15;

        decompress_reconfigure(sd, des, 16, 8, 0, vc_get_linesize(des.width, UYVY), UYVY);  //r=16,g=8,b=0
    }
    char * out =  malloc(1920*1080*5);
    printf("Decoder initialized ;^)\n");

    /* Init compress */
    struct compress_state *sc;
    sc = (struct compress_state *) calloc(2, sizeof(struct compress_state *));
    printf("Give me some compressor help\n");
    show_compress_help();
    printf("Trying to initialize encoder\n");
    compress_init("libavcodec:codec=H.264", &sc);

    printf("Encoder initialized ;^)\n");
    
	for(dev_id=0;dev_id<ndev;dev_id++){
		printf("RTP INIT IFACE device %d\n",dev_id);
		devices[dev_id] = rtp_init_if(addr, mcast_if, recv_port+(dev_id*2), send_port+(dev_id*2), ttl, rtcp_bw, 0, rtp_recv_callback, (void *)participants, 0);

		if (devices[dev_id] != NULL) {
			printf("RTP INIT OPTIONS\n");
			if (!rtp_set_option(devices[dev_id], RTP_OPT_WEAK_VALIDATION, 1)) {
				printf("RTP INIT OPTIONS FAIL 1: set option\n");
				return -1;
			}
			if (!rtp_set_sdes(devices[dev_id], rtp_my_ssrc(devices[dev_id]),
					RTCP_SDES_TOOL, PACKAGE_STRING, strlen(PACKAGE_STRING))) {
				printf("RTP INIT OPTIONS FAIL 2: set sdes\n");
				return -1;
			}


			//INITIAL_VIDEO_RECV_BUFFER_SIZE;
			int ret = rtp_set_recv_buf(devices[dev_id], INITIAL_VIDEO_RECV_BUFFER_SIZE);
			if (!ret) {
				printf("RTP INIT OPTIONS FAIL 3: set recv buf \nset command: sudo sysctl -w net.core.rmem_max=9123840\n");
				return -1;
			}

			if (!rtp_set_send_buf(devices[dev_id], 1024 * 56)) {
				printf("RTP INIT OPTIONS FAIL 4: set send buf\n");
				return -1;
			}
			ret=pdb_add(participants, rtp_my_ssrc(devices[dev_id]));
			printf("[PDB ADD] returned result = %d for ssrc = %x\n",ret,rtp_my_ssrc(devices[dev_id]));
		}
	}
    struct recieved_data *rx_data = calloc(1, sizeof(struct recieved_data));

    tx_init();

    int xec=0;

    int i = 0;

    while(exit){
        gettimeofday(&curr_time, NULL);
        timestamp = tv_diff(curr_time, start_time) * 90000;
        for(dev_id=0;dev_id<ndev;dev_id++){
			rtp_update(devices[dev_id], curr_time);
			rtp_send_ctrl(devices[dev_id], timestamp, 0, curr_time);
        }
        timeout.tv_sec = 0;
        timeout.tv_usec = 10000;

        if (!rtp_recv_poll_r(devices, &timeout, timestamp)){
            //printf("\nPACKET NOT RECIEVED\n");
            //sleep(1);
        }
        
        pdb_iter_t it;
        cp = pdb_iter_init(participants, &it);
        int ret;
        //printf("PACKET RECIEVED, building FRAME\n");
        while (cp != NULL ) {
            ret = pbuf_decode(cp->playout_buffer, curr_time, decode_frame, rx_data);
            //printf("DECODE return value: %d\n", ret);
            if (ret) {
                gettimeofday(&curr_time, NULL);
                //printf("\nFRAME RECIEVED (first byte = %x)\n",rx_data->frame_buffer[0][0]);
                frame->tiles[0].data = rx_data->frame_buffer[0];
                frame->tiles[0].data_len = rx_data->buffer_len[0];

                decompress_frame(sd,(unsigned char *) out,(unsigned char *)rx_data->frame_buffer[0],rx_data->buffer_len[0],rx_data->buffer_num[0]);
                frame->tiles[0].data=out;//rx_data->frame_buffer[0];
                frame->tiles[0].data_len = vc_get_linesize(des.width, UYVY)*des.height;//rx_data->buffer_len[0];

                tx_frame = compress_frame(sc, frame, i);
                if (tx_frame == NULL) {
                    printf("frame NULL!!!\n");
                    continue;
                }
                i = (i + 1)%2;
                
                printf("[MAIN to SENDER] data len = %d and first byte = %x\n",frame->tiles[0].data_len,frame->tiles[0].data[0]);
                if(tx_frame->tiles[0].data_len>0)
                    tx_send_base(vf_get_tile(tx_frame, 0), devices[0], get_local_mediatime(), 1, tx_frame->color_spec, tx_frame->fps, tx_frame->interlacing, 0, 0);
                
                //if (xec > 3)
                //  exit = 0;
                //xec++;

            } else {
                //printf("FRAME not ready yet\n");
            }
            pbuf_remove(cp->playout_buffer, curr_time);
            cp = pdb_iter_next(&it);
        }
        pdb_iter_done(&it);
    }

    compress_done(sc);
    decompress_done(sd);

    rtp_done(devices[dev_id]);
    printf("RTP DONE\n");
}
