/* *
 * This file is part of Feng
 *
 * Copyright (C) 2009 by LScube team <team@lscube.org>
 * See AUTHORS for more details
 *
 * feng is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * feng is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with feng; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * */

#ifndef FN_RTP_H
#define FN_RTP_H

#include <time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netembryo/wsocket.h>
#include "mediathread/mediathread.h"
#include <bufferpool/bufferpool.h>
#include <fenice/prefs.h>

#define RTP_DEFAULT_PORT 5004
#define RTCP_BUFFERSIZE    1024
#define BUFFERED_FRAMES_DEFAULT 16

typedef enum {
    i_server = 0,
    i_client = 1
} rtcp_index;

typedef struct {
    int RTP;
    int RTCP;
} port_pair;

typedef enum {
    rtp_proto = 0,
    rtcp_proto
} rtp_protos;

typedef struct RTP_transport {
    Sock *rtp_sock;
    Sock *rtcp_sock;
    struct sockaddr_storage last_stg;
    int rtp_ch, rtcp_ch;
    ev_io * rtcp_watcher;
} RTP_transport;

typedef struct RTCP_stats {
    unsigned int RR_received;
    unsigned int SR_received;
    unsigned long dest_SSRC;
    unsigned int pkt_count;
    unsigned int octet_count;
    unsigned int pkt_lost;
    unsigned char fract_lost;
    unsigned int highest_seq_no;
    unsigned int jitter;
    unsigned int last_SR;
    unsigned int delay_since_last_SR;
} RTCP_stats;

typedef struct RTP_session {
    RTP_transport transport;
    uint8_t rtcp_inbuffer[RTCP_BUFFERSIZE];
    size_t rtcp_insize;
    uint8_t rtcp_outbuffer[RTCP_BUFFERSIZE];
    size_t rtcp_outsize;

    //these time vars now are now back here
    double start_time;
    double seek_time;
    double send_time;
    //Dynamic Stream Change
    unsigned int PreviousCount;
    short MinimumReached;
    short MaximumReached;
    // Back references
    int sched_id;
    uint16_t start_seq;
    uint16_t seq_no;

    uint32_t start_rtptime;

    uint8_t pause;
    uint8_t started;

    uint32_t ssrc;

    gchar *sd_filename; //!< resource name, including path from avroot

    //mediathread - TODO: find better placement
    Selector *track_selector;

    // Metadata Begin
#ifdef HAVE_METADATA
    void *metadata;
#endif
    // Metadata End

    //multicast management
    uint8_t is_multicast; //!< 0, treat as usual, >0 do nothing

    //Consumer has transferred itself here
    BPConsumer *cons;
    RTCP_stats rtcp_stats[2];    //client and server
    struct feng *srv;
} RTP_session;

typedef struct RTP_header {
    /* byte 0 */
#if (G_BYTE_ORDER == G_LITTLE_ENDIAN)
    uint8_t csrc_len:4;   /* expect 0 */
    uint8_t extension:1;  /* expect 1, see RTP_OP below */
    uint8_t padding:1;    /* expect 0 */
    uint8_t version:2;    /* expect 2 */
#elif (G_BYTE_ORDER == G_BIG_ENDIAN)
    uint8_t version:2;
    uint8_t padding:1;
    uint8_t extension:1;
    uint8_t csrc_len:4;
#else
#error Neither big nor little
#endif
    /* byte 1 */
#if (G_BYTE_ORDER == G_LITTLE_ENDIAN)
    uint8_t payload:7;    /* RTP_PAYLOAD_RTSP */
    uint8_t marker:1;     /* expect 1 */
#elif (G_BYTE_ORDER == G_BIG_ENDIAN)
    uint8_t marker:1;
    uint8_t payload:7;
#endif
    /* bytes 2, 3 */
    uint16_t seq_no;
    /* bytes 4-7 */
    uint32_t timestamp;
    /* bytes 8-11 */
    uint32_t ssrc;    /* stream number is used here. */
} RTP_header;

typedef int (*RTP_play_action) (RTP_session * sess);

/**
 * @defgroup RTP RTP Layer
 * @{
 */

/**
 * RTP ports management functions
 * @defgroup rtp_port RTP ports management functions
 * @{
 */

void RTP_port_pool_init(feng *srv, int port);
int RTP_get_port_pair(feng *srv, port_pair * pair);
int RTP_release_port_pair(feng *srv, port_pair * pair);

/**
 * @}
 */

/**
 * RTP session management functions
 * @defgroup rtp_session RTP session management functions
 * @{
 */

int RTP_send_packet(RTP_session * session);
void RTP_session_destroy(RTP_session *);

//! RTP_transport functions
//! low-level sent/receive packet functions depending on transport settings.
ssize_t RTP_recv(RTP_session *);

/**
 * @}
 */

/**
 * @}
 */

#endif // FN_RTP_H