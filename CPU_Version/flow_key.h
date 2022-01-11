#ifndef FLOW_KEY_H
#define FLOW_KEY_H 1

#include "util.h"
#include <stdbool.h>
#include <stddef.h>
#include <math.h>

#define MAX_FUNC_NUM 10

struct flow_key {
	uint32_t srcip;
	uint32_t dstip;
	union {
		struct {
			uint16_t srcport;
			uint16_t dstport;
		};
		uint32_t port;
	};
	uint16_t protocol;
};
inline uint32_t flow_key_hash_old(struct flow_key key) {
	int hashCode = (int)key.srcip;
	hashCode = (hashCode * 397) ^ (int)key.dstip;
	hashCode = (hashCode * 397) ^ (int)key.srcport;
	hashCode = (hashCode * 397) ^ (int)key.dstport;
	hashCode = (hashCode * 397) ^ (int)key.protocol;
	return (uint32_t)hashCode;
}

inline uint32_t flow_key_hash(struct flow_key key, uint32_t bits) {
	uint32_t hash = hash_32(key.srcip, bits);
	hash ^= hash_32(key.dstip, bits);
	hash ^= hash_32(key.port, bits);
	hash ^= hash_32(key.protocol, bits);
	//hash ^= hash_32(key.srcport, bits);
	//hash ^= hash_32(key.dstport, bits);
	
	return hash;
}

inline uint32_t flow_key_hash_2(struct flow_key key, uint32_t mod) {
	int bits = (uint32_t)log2f(mod);;
	uint32_t hash = hash_32(key.srcip, bits);
	hash ^= hash_32(key.dstip, bits);
	hash ^= hash_32(*((uint32_t*)&(key.srcport)), bits);
	//hash ^= hash_32(key->dstport, bits);
	//hash ^= hash_32(key->protocol, bits);
	return hash >> (32 - bits);
}

inline int flow_key_equal_ref(struct flow_key* lhs, struct flow_key* rhs) {
	return lhs->srcip == rhs->srcip && lhs->dstip == rhs->dstip && lhs->srcport == rhs->srcport &&
		lhs->dstport == rhs->dstport && lhs->protocol == rhs->protocol;
}

inline int flow_key_equal(struct flow_key lhs, struct flow_key rhs) {
	return memcmp(&lhs, &rhs, sizeof(struct flow_key));
	return lhs.srcip == rhs.srcip && lhs.dstip == rhs.dstip && lhs.srcport == rhs.srcport &&
		lhs.dstport == rhs.dstport && lhs.protocol == rhs.protocol;
}

inline uint32_t TW_hash(uint32_t key) {
	key = ~key + (key << 15); 
	key = key ^ (key >> 12);
	key = key + (key << 2);
	key = key ^ (key >> 4);
	key = key * 2057;
	key = key ^ (key >> 16);
	return key;
}


inline uint32_t BJ_hash(uint32_t key) {
	key = (key + 0x7ed55d16) + (key << 12);
	key = (key ^ 0xc761c23c) ^ (key >> 19);
	key = (key + 0x165667b1) + (key << 5);
	key = (key + 0xd3a2646c) ^ (key << 9);
	key = (key + 0xfd7046c5) + (key << 3); 
	key = (key ^ 0xb55a4f09) ^ (key >> 16);
	return key;
}


inline uint32_t flow_key_hash_4(struct flow_key key) {
	uint32_t hash = TW_hash(key.srcip);
	hash ^= TW_hash(key.dstip);
	hash ^= TW_hash(key.port);
	hash ^= TW_hash(key.srcport);
	hash ^= TW_hash(key.dstport);
	return hash;
}


inline uint32_t flow_key_hash_3(struct flow_key key) {
	uint32_t hash = BJ_hash(key.srcip);
	hash ^= BJ_hash(key.dstip);
	hash ^= BJ_hash(key.port);
	hash ^= BJ_hash(key.srcport);
	hash ^= BJ_hash(key.dstport);
	return hash;
}


#endif
