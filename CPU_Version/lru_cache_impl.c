#include "lru_cache_impl.h"


//int counts = 0;

cacheEntryS *entryPointer = NULL;

cacheEntryS *tmpCacheEntry = NULL;
const int L = 9700;

//cacheEntryS Queue[MAX_SIZE];

static cacheEntryS* newCacheEntry(struct flow_key key)
{

	cacheEntryS* entry = (cacheEntryS*)malloc(sizeof(cacheEntryS));
	if (NULL == entry) {
		printf("error!entry cannot be init!\n");
		return NULL;
	}
	memset(entry, 0, sizeof(*entry));
	entry->key = key;
	//strcpy(entry->key, key);
	entry->flag = 0;
	//entry->sum_bytes = data;
	entry->packet_count = 1;

	
	return entry;
}


void freeCacheEntry(cacheEntryS* entry)
{
	if (NULL == entry) return;
	kfree(entry);
}


static int LRUCacheCreate(int capacity, void **lruCache)
{
	LRUCacheS* cache = NULL;
	if (NULL == (cache = malloc(sizeof(*cache)))) {
		printf("error!\n");
		return -1;
	}
	memset(cache, 0, sizeof(*cache));
	cache->cacheCapacity = capacity;
	cache->hashMap = malloc(sizeof(cacheEntryS*)*capacity);
	if (NULL == cache->hashMap) {
		kfree(cache);
		printf("error!\n");
		return -1;
	}
	memset(cache->hashMap, 0, sizeof(cacheEntryS*)*capacity);

	*lruCache = cache;
	return 0;
}


static int LRUCacheDestory(void *lruCache)
{
	LRUCacheS* cache = (LRUCacheS*)lruCache;
	if (NULL == cache) return 0;
	//kfree hashMap
	if (cache->hashMap) {
		kfree(cache->hashMap);
	}
	//kfree linklist
	freeList(cache);
	//kfree(cache);
	return 0;
}

/**********************************
***********************************/

void removeFromList(LRUCacheS *cache, cacheEntryS *entry)
{

	if (cache->lruListSize == 0) {
		return;
	}

	if (entry == cache->lruListHead && entry == cache->lruListTail) {

		cache->lruListHead = cache->lruListTail = NULL;
	
	}
	else if (entry == cache->lruListHead) {
		cache->lruListHead = entry->lruListNext;
		cache->lruListHead->lruListPrev = NULL;
	}
	else if (entry == cache->lruListTail) {
		cache->lruListTail = entry->lruListPrev;
		cache->lruListTail->lruListNext = NULL;
	}
	else {
		entry->lruListPrev->lruListNext = entry->lruListNext;
		entry->lruListNext->lruListPrev = entry->lruListPrev;
	}
	cache->lruListSize--;
}


cacheEntryS* insertToListHead(LRUCacheS *cache, cacheEntryS *entry)
{
	cacheEntryS *removedEntry = NULL;
	++cache->lruListSize;

	if (cache->lruListSize > cache->cacheCapacity) {

		removedEntry = cache->lruListTail;

		removeFromList(cache, cache->lruListTail);
	}

	if (cache->lruListHead == NULL && cache->lruListTail == NULL) {
		cache->lruListHead = cache->lruListTail = entry;
	}
	else {
		entry->lruListNext = cache->lruListHead;
		entry->lruListPrev = NULL;
		cache->lruListHead->lruListPrev = entry;
		cache->lruListHead = entry;
	}
	//printf("Remove: %d, %s\n", cache->lruListSize, removedEntry->key);
	return removedEntry;
}


static void freeList(LRUCacheS* cache)
{


	if (0 == cache->lruListSize) {
		return;
	}

	cacheEntryS *entry = cache->lruListHead;

	while (entry) {
		cacheEntryS *temp = entry->lruListNext;
		freeCacheEntry(entry);
		entry = temp;
	}
	cache->lruListSize = 0;
}


static void updateLRUList(LRUCacheS *cache, cacheEntryS *entry)
{

	removeFromList(cache, entry);


	insertToListHead(cache, entry);
}

/*********************************************************************************

**********************************************************************************/


inline unsigned int hashKey(LRUCacheS *cache, struct flow_key key)
{

	uint32_t hashCode = flow_key_hash_4(key);

	return (uint32_t)hashCode % (cache->cacheCapacity);

}




cacheEntryS *getValueFromHashMap(LRUCacheS *cache, struct flow_key key)
{

	cacheEntryS *entry = cache->hashMap[hashKey(cache, key)];


	while (entry) {
		if (!flow_key_equal(entry->key, key)) {
			break;
		}
		entry = entry->hashListNext;
	}

	return entry;
}


static void insertentryToHashMap(LRUCacheS *cache, cacheEntryS *entry)
{


	cacheEntryS *n = cache->hashMap[hashKey(cache, entry->key)];
	if (n != NULL) {
		if (entry->hashList == NULL){
			entry->hashListNext = n;
			n->hashListPrev = entry;
		}
		else{
			entry->hashListPrev->hashListNext = entry->hashListNext;
			entry->hashListNext->hashListPrev = entry->hashListPrev;
			entry->hashListNext = n;
			n->hashListPrev = entry;
		}
	}

	cache->hashMap[hashKey(cache, entry->key)] = entry;
}


static void removeEntryFromHashMap(LRUCacheS *cache, cacheEntryS *entry)
{
	

	if (NULL == entry || NULL == cache || NULL == cache->hashMap) {
		return;
	}
	cacheEntryS *n = cache->hashMap[hashKey(cache, entry->key)];



	while (n) {
		if (flow_key_equal(n->key, entry->key) == 0) { 
			if (n->hashListPrev) {
				n->hashListPrev->hashListNext = n->hashListNext;
			}
			else {
				cache->hashMap[hashKey(cache, entry->key)] = n->hashListNext;
			}
			if (n->hashListNext) {
				n->hashListNext->hashListPrev = n->hashListPrev;
			}
			return;
		}
		n = n->hashListNext;
	}
}

/*******************************************************************************

********************************************************************************/

static cacheEntryS* LRUCacheSet(void *lruCache, struct flow_key key, cacheEntryS **pEntryPointer)
{
	LRUCacheS *cache = (LRUCacheS *)lruCache;

	cacheEntryS *entry = getValueFromHashMap(cache, key);
	if (entry != NULL) {
		//printf("entry != NULL\n");
		//entry->curr_bytes += data;
		entry->packet_count++;
		//strncpy(entry->data, data, VALUE_SIZE);
		if (entry->flag == 0) {
			//printf("entry->flag == 0\n");
			if ((*pEntryPointer) != NULL )
				if( flow_key_equal(entry->key, (*pEntryPointer)->key) == 0 ) {
					//printf("entry == (*pEntryPointer)\n");
					(*pEntryPointer) = (*pEntryPointer)->lruListPrev;
				}
			updateLRUList(cache, entry);
		}
		if (entry->flag == 1) {
			//printf("entry->flag == 1\n");
			if ((*pEntryPointer)->packet_count < THRESHOLD) {
				//printf("(*pEntryPointer)->sum_bytes < THRESHOLD\n");
				cacheEntryS *temp = NULL;
				temp = *pEntryPointer;

				(*pEntryPointer) = (*pEntryPointer)->lruListPrev;
				removeEntryFromHashMap(cache, (*pEntryPointer)->lruListNext);
				(*pEntryPointer)->lruListNext = (*pEntryPointer)->lruListNext->lruListNext;
				(*pEntryPointer)->lruListNext->lruListPrev = (*pEntryPointer);
				entry->flag = 0;
				cache->lruListSize--;
				tmpCacheEntry = temp;
				//freeCacheEntry(temp);
			}
			else {
				//printf("(*pEntryPointer)->sum_bytes >= THRESHOLD\n");
				(*pEntryPointer)->flag = 1;
				(*pEntryPointer) = (*pEntryPointer)->lruListPrev;
				entry->flag = 0;
			}
			updateLRUList(cache, entry);
		}

	}
	else {
		//printf("entry == NULL\n");

		if (tmpCacheEntry == NULL) {
			entry = newCacheEntry(key);
		}
		else {
			memset(tmpCacheEntry, 0, sizeof(*tmpCacheEntry));
			tmpCacheEntry->key = key;
			//strcpy(entry->key, key);
			tmpCacheEntry->flag = 0;
			//entry->sum_bytes = data;
			tmpCacheEntry->packet_count = 1;
			
			//tmpCacheEntry->hashListPrev = NULL;
			//tmpCacheEntry->hashListNext = NULL;
			//tmpCacheEntry->lruListPrev = NULL;
			//tmpCacheEntry->lruListNext = NULL;
			entry = tmpCacheEntry;
		}
		

		
		if ((*pEntryPointer) == NULL) {
			//printf("(*pEntryPointer) == NULL\n");
			cacheEntryS *removedEntry = insertToListHead(cache, entry);
			if (NULL != removedEntry) {


				removeEntryFromHashMap(cache, removedEntry);

				tmpCacheEntry = removedEntry;
				//freeCacheEntry(removedEntry);
			}

			insertentryToHashMap(cache, entry);


			return removedEntry;
		}
		else {

			if ((*pEntryPointer)->packet_count < THRESHOLD) {
				//printf("(*pEntryPointer)->sum_bytes < THRESHOLD\n");
				cacheEntryS *temp = NULL;
				temp = *pEntryPointer;

				if ((*pEntryPointer)->lruListNext == NULL) {
					//*(*pEntryPointer) = *((*pEntryPointer)->lruListPrev);
					(*pEntryPointer) = (*pEntryPointer)->lruListPrev;
					removeEntryFromHashMap(cache, (*pEntryPointer)->lruListNext);
					(*pEntryPointer)->lruListNext = NULL;
					((LRUCacheS *)(void *)lruCache)->lruListTail = (*pEntryPointer);
				}
				else {
					(*pEntryPointer) = (*pEntryPointer)->lruListPrev;
					removeEntryFromHashMap(cache, (*pEntryPointer)->lruListNext);
					(*pEntryPointer)->lruListNext = (*pEntryPointer)->lruListNext->lruListNext;
					(*pEntryPointer)->lruListNext->lruListPrev = (*pEntryPointer);
				}
				cache->lruListSize--;
				cacheEntryS *removedEntry = insertToListHead(cache, entry);
				insertentryToHashMap(cache, entry);
				tmpCacheEntry = temp;
				//freeCacheEntry(temp);
			}
			else {
				//printf("(*pEntryPointer)->sum_bytes >= THRESHOLD\n");
				(*pEntryPointer)->flag = 1;
				//printf("(*pEntryPointer) hash value is: %d\n", hashKey(lruCache, (*pEntryPointer)->key));
				(*pEntryPointer) = (*pEntryPointer)->lruListPrev;
				//printf("(*pEntryPointer)->lruListPrev hash value is: %d\n", hashKey(lruCache, (*pEntryPointer)->key));
				cacheEntryS *removedEntry = insertToListHead(cache, entry);
				//printf("Tail is :%s\n", removedEntry->key);
				if (NULL != removedEntry) {

					removeEntryFromHashMap(cache, removedEntry);
					//freeCacheEntry(removedEntry);
				}

				insertentryToHashMap(cache, entry);

				tmpCacheEntry = removedEntry;

				return removedEntry;
			}
		}
	}
	return NULL;
}





void LRUCachePrint(void *lruCache)
{
	LRUCacheS *cache = (LRUCacheS *)lruCache;
	if (NULL == cache || 0 == cache->lruListSize) {
		return;
	}
	printf("\n>>>>>>>>>>>>>>>\n");
	printf("cache (key data):\n");
	cacheEntryS* entry = cache->lruListHead;

	while (entry) {
		printf("(%u,%u,%u,%u,%u  %d)", entry->key.srcip, entry->key.dstip, entry->key.protocol, entry->key.srcport, entry->key.dstport,entry->packet_count);
		printf("\n");
		entry = entry->lruListNext;
	
	}

	printf("\n<<<<<<<<<<<<<<<\n");
}


int LRUCacheLength(void *lruCache)
{
	LRUCacheS *cache = (LRUCacheS *)lruCache;
	return cache->lruListSize;
}

void LRU_update(LRUCacheS* lruCache, struct flow_key* key) {


	int LRUCurrLength = lruCache->lruListSize;

	if (LRUCurrLength == L)
		entryPointer = ((LRUCacheS *)(void *)lruCache)->lruListTail;
	else if (LRUCurrLength < L)
		entryPointer = NULL;
	 LRUCacheSet(lruCache, *key, &entryPointer);
	//printf("updated lruCache->lruListTail hash value is: %d\n", hashKey(lruCache, (((LRUCacheS *)(void *)lruCache)->lruListTail)->key));
	//printf("The updated LRUCurrLength is:%d\n", LRUCurrLength1);


}

LRUCacheS* init_LRU(int LRU_Capacity) {
	void *lruCache;
	if (0 != LRUCacheCreate(LRU_Capacity, &lruCache))
	{
		printf("error!!!\n");
		return 0;
	}
	//printf("init_LRU success\n");
	return (LRUCacheS *)lruCache;
}

void clean_LRU(LRUCacheS* lruCache) {

	LRUCacheDestory(lruCache);
	//printf("clean_LRU success\n");
}
