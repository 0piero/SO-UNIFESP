#include "memory.h"
#include "normal.h"
#ifndef HEADER_PR
#define HEADER_PR

// Mudei de process para handler pq vai ter q ter varios processos
// usando a mesma memoria real e swap, tendo q ter ate o conflito de
// uma querer mudar uma pagina referenciada e n podendo pq ta referenciada

typedef struct {
	pgptr* pages;
	int size;
}handler;
typedef handler * hndptr;

handler crt_handler(int ref_ram, int ref_swap, int amount_ram, int amount_swap){
	handler p = {malloc(sizeof(page)*(amount_ram + amount_swap)), amount_ram + amount_swap};
	int index = 0;
	misses = 0;
	
	srand(time(NULL));
	for(int i = 0; i < amount_ram; i++, index++){
		p.pages[index] = crt_page(rand(), RAM, ref_ram + i);
		alloc_memory(&ram, p.pages[index]);
		write_swap(i, p.pages[index]);
		addresses[i] = i;
	}

	for(int i = amount_ram; i < amount_swap; i++, index++){
		
		p.pages[index] = crt_page(rand(), SWAP, ref_swap + i);
		write_swap(i, p.pages[index]);
		addresses[i] = -1;
	}
	return p;
}

int get_removable_frame_NUR(){ // NUR
	int ret, i, j=0;
	int** cls_i = (int **)malloc(4*sizeof(int*));
	for(int j = 0; j < 4; j++) cls_i[j] = (int*)malloc(sizeof(int));
	char count_cls[4] = {0};
	for(i=0;i<RAM_SIZE;i++){
		int cls_val = ((ram.pages[i]).ref)*4+((ram.pages[i]).drty)*2;
		cls_val = cls_val/2;
		cls_i[cls_val][count_cls[cls_val]] = i;
		count_cls[cls_val]++;
		cls_i[cls_val] = (int*)realloc(cls_i[cls_val/2], (count_cls[cls_val]+1)*sizeof(int));
	}
	while(!(count_cls[j]!=0)){j++;}
	ret = cls_i[j][rand()%count_cls[j]];
	free(cls_i);
	return ret;
}

int get_removable_frame_AGING(){ // Aging
	int i, oldest = 0;
	for(i=0; i<RAM_SIZE; i++){
		if((ram.pages[oldest]).age > (ram.pages[i]).age){
			oldest = i;
		}
	}
	return oldest;
}

int load_page(int ind){
	int new_index;
	if(ALGORITHM=="NUR"){
		new_index = get_removable_frame_NUR();	
	}
	else{
		new_index = get_removable_frame_AGING();		
	}
	swap_memory(new_index, ind);
	return new_index;
}

int get_page(int ind){
	int x = addresses[ind];
	printf("\nProcurando pagina: %d\n", ind);
	if(x < 0){
		printf("-------PAGE MISS--------\n-Pagina requisitada: %d-\n", ind);
		x = load_page(ind);
		printf("-Colocada no frame: %d-\n", x);
		(ram.pages[x]).ref = 1;
		misses++;
		return x;
	}
	printf("Pagina encontrada em: %d\n\n", x);
	(ram.pages[x]).ref = true;
	ref_times[ram.pages[x].block] = VIRTUAL_TIME;
	return x;
}

int release_page(int ind){
	int x = addresses[ind];
	if(x==-1){return 0;}
	if(VIRTUAL_TIME - ref_times[ram.pages[x].block] > RELEASE_THRESHOLD){
		(ram.pages[x]).ref = 0;
	}
	else{ref_times[ram.pages[x].block]++;}
	return 0;
}

void release_all(){
	int i;
	for(i=0; i<SWAP_SIZE; i++){
		release_page(i);
	}
}

void print_addresses(){
	int i;
	printf("Tabela MV -> MR :\n");
	for(i=0; i<SWAP_SIZE; i++){
		printf(" - %d -> %d\n", i, addresses[i]);
	}
}

void update_ages(){
	int i;
	//Aging
	for(i=0; i<RAM_SIZE; i++){
		(ram.pages[i]).age = (ram.pages[i]).age/10;
		if((ram.pages[i]).ref){
			(ram.pages[i]).age += 10000000;
		}
	}
}
#endif
