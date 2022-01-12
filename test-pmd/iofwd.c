/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <sys/queue.h>
#include <sys/stat.h>

#include <rte_common.h>
#include <rte_byteorder.h>
#include <rte_log.h>
#include <rte_debug.h>
#include <rte_cycles.h>
#include <rte_memory.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_atomic.h>
#include <rte_branch_prediction.h>
#include <rte_memcpy.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_interrupts.h>
#include <rte_pci.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_string_fns.h>
#include <rte_flow.h>


#include <rte_mbuf_core.h>

#include "testpmd.h"
#include "lru_cache_impl.h"

#define ALL_32_BITS 0xffffffff
#define BIT_8_TO_15 0x0000ff00

/*
 * Forwarding of packets in I/O mode.
 * Forward packets "as-is".
 * This is the fastest possible forwarding operation, as it does not access
 * to packets data.
 */

union ipv4_5tuple_host {
	struct {
		uint8_t  pad0;
		uint8_t  proto;
		uint16_t pad1;
		uint32_t ip_src;
		uint32_t ip_dst;
		uint16_t port_src;
		uint16_t port_dst;
	};
	xmm_t xmm;
};

#if defined(__SSE2__)
static inline xmm_t
em_mask_key(void *key, xmm_t mask)
{
	__m128i data = _mm_loadu_si128((__m128i *)(key));

	return _mm_and_si128(data, mask);
}
#elif defined(__ARM_NEON)
static inline xmm_t
em_mask_key(void *key, xmm_t mask)
{
	int32x4_t data = vld1q_s32((int32_t *)key);

	return vandq_s32(data, mask);
}
#elif defined(__ALTIVEC__)
static inline xmm_t
em_mask_key(void *key, xmm_t mask)
{
	xmm_t data = vec_ld(0, (xmm_t *)(key));

	return vec_and(data, mask);
}
#else
#error No vector engine (SSE, NEON, ALTIVEC) available, check your toolchain
#endif

static rte_xmm_t mask0;

static void
pkt_burst_io_forward(struct fwd_stream *fs)
{
	struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
	uint16_t nb_rx;
	uint16_t nb_tx;
	uint32_t retry;
    uint16_t  i;
	uint64_t start_tsc = 0;

	/*
	 * Receive a burst of packets and forward them.
	 */
	nb_rx = rte_eth_rx_burst(fs->rx_port, fs->rx_queue,
			pkts_burst, nb_pkt_per_burst);
	inc_rx_burst_stats(fs, nb_rx);
	if (unlikely(nb_rx == 0))
		return;
	fs->rx_packets += nb_rx;
	mask0 = (rte_xmm_t){.u32 = {BIT_8_TO_15, ALL_32_BITS,
				ALL_32_BITS, ALL_32_BITS} };

 	for (i = 0; i < nb_rx; i++){
		union ipv4_5tuple_host key;
		struct flow_key key_s;
		struct flow_key key_ss[2];
		struct rte_ipv4_hdr *ipv4_hdr_lru;
		struct rte_mbuf *m = pkts_burst[i];
		rte_prefetch0(rte_pktmbuf_mtod(m, void *));
		ipv4_hdr_lru = rte_pktmbuf_mtod(m, struct rte_ipv4_hdr *);
		ipv4_hdr_lru = (uint8_t *)ipv4_hdr_lru + offsetof(struct rte_ipv4_hdr, time_to_live);
		key.xmm = em_mask_key(ipv4_hdr_lru, mask0.x);
		key_s.srcip = rte_be_to_cpu_32(key.ip_src);
		key_s.dstip = rte_be_to_cpu_32(key.ip_dst);
		key_s.srcport = rte_be_to_cpu_16(key.port_src);
		key_s.dstport = rte_be_to_cpu_16(key.port_dst);
		key_s.protocol = key.proto;
		// key_ss[0].srcip = 2;
		// key_ss[0].dstip = 3;
		// key_ss[0].srcport = 4;
		// key_ss[0].dstport = 5;
		// key_ss[0].protocol = 6;
		// key_ss[1].srcip = 3;
		// key_ss[1].dstip = 4;
		// key_ss[1].srcport = 5;
		// key_ss[1].dstport = 6;
		// key_ss[1].protocol = 7;
		// //int inde = 0;
		// if (i % 2 == 0)
		// 	my_label_sketch(key_ss[0]);
		// else
		// 	my_label_sketch(key_ss[1]);
		// //inde++;

		/* process each recevied packet in LRU-TX */
		my_label_sketch(key_s);
	}
	nb_tx = rte_eth_tx_burst(fs->tx_port, fs->tx_queue,
			pkts_burst, nb_rx);
	/*
	 * Retry if necessary
	 */
	if (unlikely(nb_tx < nb_rx) && fs->retry_enabled) {
		retry = 0;
		while (nb_tx < nb_rx && retry++ < burst_tx_retry_num) {
			rte_delay_us(burst_tx_delay_time);
			nb_tx += rte_eth_tx_burst(fs->tx_port, fs->tx_queue,
					&pkts_burst[nb_tx], nb_rx - nb_tx);
		}
	}
	fs->tx_packets += nb_tx;
	inc_tx_burst_stats(fs, nb_tx);
	if (unlikely(nb_tx < nb_rx)) {
		fs->fwd_dropped += (nb_rx - nb_tx);
		do {
			rte_pktmbuf_free(pkts_burst[nb_tx]);
		} while (++nb_tx < nb_rx);
	}

	get_end_cycles(fs, start_tsc);
}

struct fwd_engine io_fwd_engine = {
	.fwd_mode_name  = "io",
	.port_fwd_begin = NULL,
	.port_fwd_end   = NULL,
	.packet_fwd     = pkt_burst_io_forward,
};
