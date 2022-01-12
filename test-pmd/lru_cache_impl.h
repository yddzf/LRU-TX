#ifndef LRUCACHEIMPL_H
#define LRUCACHEIMPL_H

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
// #include "flow_key.h"
// #define kfree free

#define THRESHOLD 6

// #define MAX_ETH_FRAME 1514
// #define ERROR_FILE_OPEN_FAILED -1
// #define ERROR_MEM_ALLOC_FAILED -2
// #define ERROR_PCAP_PARSE_FAILED -3
// #define BUFSIZE 10240
// #define STRSIZE 1024

// #define MAX_SIZE 100000

// typedef unsigned short  u_short;
// typedef unsigned long u_int32;

#define KEY_SIZE 32

// extern void *lruCache;

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

typedef struct cacheEntryS {
	struct flow_key key;  

	int packet_count;
	short flag;

	struct cacheEntryS *hashListPrev;  
	struct cacheEntryS *hashListNext;   
	struct cacheEntryS *lruListPrev;    
	struct cacheEntryS *lruListNext;   
}cacheEntryS;


typedef struct LRUCacheS {

	int cacheCapacity; 
	//int cacheCapacity_short;
	int lruListSize;   
	//int lruListSize_short;

	cacheEntryS **hashMap;  

	cacheEntryS *lruListHead;  
	cacheEntryS *lruListTail;   
}LRUCacheS;


// inline int flow_key_equal_ref(struct flow_key* lhs, struct flow_key* rhs) {
// 	return lhs->srcip == rhs->srcip && lhs->dstip == rhs->dstip && lhs->srcport == rhs->srcport &&
// 		lhs->dstport == rhs->dstport && lhs->protocol == rhs->protocol;
// }

int flow_key_equal(struct flow_key lhs, struct flow_key rhs);

uint32_t TW_hash(uint32_t key);

uint32_t flow_key_hash_4(struct flow_key key);

// cacheEntryS *newCacheEntry(char *key);
cacheEntryS *newCacheEntry(struct flow_key key);
void freeCacheEntry(cacheEntryS* entry);
int LRUCacheCreate(int capacity, void **lruCache);
int LRUCacheDestory(void *lruCache);
void removeFromList(LRUCacheS *cache, cacheEntryS *entry);
cacheEntryS* insertToListHead(LRUCacheS *cache, cacheEntryS *entry);
void freeList(LRUCacheS* cache);
void updateLRUList(LRUCacheS *cache, cacheEntryS *entry);
unsigned int hashKey(LRUCacheS *cache, struct flow_key key);
cacheEntryS *getValueFromHashMap(LRUCacheS *cache, struct flow_key key);
void insertentryToHashMap(LRUCacheS *cache, cacheEntryS *entry);
void removeEntryFromHashMap(LRUCacheS *cache, cacheEntryS *entry);
cacheEntryS* LRUCacheSet(void *lruCache, struct flow_key key, cacheEntryS **pEntryPointer);
//static struct flow_key LRUCacheGet(void *lruCache, char *key);
int LRUCacheLength(void *lruCache);


// void LRU_update( LRUCacheS* lruCache, struct flow_key* key);
// LRUCacheS*  init_LRU (int LRU_Capacity );
int init_LRU (int LRU_Capacity );
void clean_LRU(LRUCacheS* lruCache);

// int init_LRU (int LRU_Capacity);
// void clean_LRU(LRUCacheS* lruCache);
void my_label_sketch(struct flow_key key);

extern void *lruCache;
#endif