#ifndef LRUCACHEIMPL_H
#define LRUCACHEIMPL_H

#include <stdio.h>
#include <string.h>
#include "flow_key.h"
#define kfree free

#define THRESHOLD 7

#define MAX_ETH_FRAME 1514
#define ERROR_FILE_OPEN_FAILED -1
#define ERROR_MEM_ALLOC_FAILED -2
#define ERROR_PCAP_PARSE_FAILED -3
#define BUFSIZE 10240
#define STRSIZE 1024

#define MAX_SIZE 100000

typedef unsigned short  u_short;
typedef unsigned long u_int32;
//#include <semaphore.h>

#define KEY_SIZE 32
//#define VALUE_SIZE 100

typedef struct cacheEntryS {
	struct flow_key key;  
	//char data[VALUE_SIZE];  
	int packet_count;
	short flag;

	//sem_t entry_lock;

	struct cacheEntryS *hashListPrev;  
	struct cacheEntryS *hashListNext;   
	struct cacheEntryS *lruListPrev;    
	struct cacheEntryS *lruListNext;   
}cacheEntryS;


typedef struct LRUCacheS {
	//sem_t cache_lock;
	int cacheCapacity; 
	//int cacheCapacity_short;
	int lruListSize;   
	//int lruListSize_short;

	cacheEntryS **hashMap;  

	cacheEntryS *lruListHead;  
	cacheEntryS *lruListTail;   
}LRUCacheS;












static cacheEntryS *newCacheEntry(char *key);
void freeCacheEntry(cacheEntryS* entry);
static int LRUCacheCreate(int capacity, void **lruCache);
static int LRUCacheDestory(void *lruCache);
void removeFromList(LRUCacheS *cache, cacheEntryS *entry);
cacheEntryS* insertToListHead(LRUCacheS *cache, cacheEntryS *entry);
void freeList(LRUCacheS* cache);
static void updateLRUList(LRUCacheS *cache, cacheEntryS *entry);
unsigned int hashKey(LRUCacheS *cache, char* key);
cacheEntryS *getValueFromHashMap(LRUCacheS *cache, char *key);
static void insertentryToHashMap(LRUCacheS *cache, cacheEntryS *entry);
static void removeEntryFromHashMap(LRUCacheS *cache, cacheEntryS *entry);
static cacheEntryS* LRUCacheSet(void *lruCache, char *key, cacheEntryS **pEntryPointer);
//static struct flow_key LRUCacheGet(void *lruCache, char *key);
inline int LRUCacheLength(void *lruCache);


void LRU_update( LRUCacheS* lruCache, struct flow_key* key);
LRUCacheS*  init_LRU (int LRU_Capacity );
void clean_LRU(LRUCacheS* lruCache);
#endif