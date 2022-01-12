#include <windows.h>
#include <stdio.h>

#include "lru_cache_impl.h"

const long count = 20000;
const int work_round = 5;
struct flow_key* flows;
const Elemtype* traffics;
char flow[25000][32];

extern cacheEntryS* tmpCacheEntry;

struct flow_key* flow_gen(size_t size) {
	struct flow_key* array = (struct flow_key*) malloc(sizeof(struct flow_key)* size);
		
	for (int i = 0; i < size; ++i) {
		array[i].srcip = rand_uint32();
		array[i].dstip = rand_uint32();
		array[i].srcport = rand_uint16();
		array[i].dstport = rand_uint16();
		array[i].protocol = rand_uint16();
	}
	return array;
}

Elemtype* traffic_gen(size_t size) {
	Elemtype* array = (Elemtype*)malloc(sizeof(Elemtype)*size);
	//FILE* f = fopen("E:\\test\\CPU_Cycle_Test\\udp.txt", "r");
	errno_t err;
	FILE* f;
	if((err = fopen_s(&f, ".\\udp.txt", "r")) != 0)
		printf("The file udp.txt was not opened\n");
	else
		printf("The file udp.txt was opened\n");
	char buf[30];
	for (int i = 0; i < size; ++i) {
		fgets(buf, 30, f);
		int traffic;
		sscanf_s(buf, "%d", &traffic);
		array[i] = traffic;
	}

	fclose(f);

	return array;
}

double LRU_time;

void do_work_1(const int w) {
    int packet = 0;
	
	int LRU_Capacity = 10000;
	
	tmpCacheEntry = NULL;
	LARGE_INTEGER begin, end, frequency;

	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&begin);

    LRUCacheS* lruCache = init_LRU (LRU_Capacity );
	
	for (int i = 0; i < count; ++i) {
		Elemtype t0 = traffics[i];
		Elemtype t = t0;

		while (t0) {
			t = t0 > 500 ? 500 : t0;
			t0 -= t;
			LRU_update(lruCache, &flows[i]);
			++packet;		
		}	
	}
	
	QueryPerformanceCounter(&end);
	//printf("LRU_cache_size is : %d\n", lruCache->lruListSize);
	//LRUCachePrint(lruCache);
    
	LRU_time += (double)packet / ((double)(end.QuadPart - begin.QuadPart) / frequency.QuadPart);
	clean_LRU(lruCache);
}


void do_work(const int w) {

	LRU_time = 0;
	printf("====================================================\n");
	for (int i = 0; i<work_round; i++) {
		do_work_1(w);
	}
	
	printf("LRU_throughput is :%d pps\n",  (int)(LRU_time / work_round ) );
}

int main() {
	flows = flow_gen(count);
	traffics = traffic_gen(count);

	struct flow_key a;
	a.srcip = 10;
	a.dstip = 13;
	a.protocol = 33;
	a.srcport = 11;
	a.dstport = 22;
	// printf("%u\n", flow_key_hash(a, 16));
	// printf("%u\n", flow_key_hash(a, 16));
	while (1) {
		for (int d = 1000; d <= 3000; d += 1000) {
			do_work(d);
		}
		system("pause");
	}

}
