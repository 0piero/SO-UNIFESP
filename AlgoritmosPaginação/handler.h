#include "memory.h"

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

int *addresses;
int misses;

handler crt_handler(int ref_ram, int ref_swap, int amount_ram, int amount_swap){
	handler p = {malloc(sizeof(page)*(amount_ram + amount_swap)), amount_ram + amount_swap};
	int index = 0;
	misses = 0;
	addresses = malloc(sizeof(int)*(amount_swap));
	
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
	int i, start = rand() % RAM_SIZE;
	for(i=start; i<start+RAM_SIZE; i++){
		if(!((ram.pages[i%RAM_SIZE]).ref) && !((ram.pages[i%RAM_SIZE]).drty)){
			return i;
		}
	}
	for(i=start; i<start+RAM_SIZE; i++){
		if(!((ram.pages[i%RAM_SIZE]).ref)){
			return i;
		}
	}
	for(i=start; i<start+RAM_SIZE; i++){
		if(!((ram.pages[i%RAM_SIZE]).drty)){
			return i;
		}
	}
	return start;


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
	int new_index = get_removable_frame_NUR();
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
	(ram.pages[x]).ref = 1;
	return x;
}

void release_page(int ind){
	int x = addresses[ind];
	(ram.pages[x]).ref = 0;
}

void undirtify(){
	int i;
	for(i=0; i<RAM_SIZE; i++){
		(ram.pages[i]).drty = 0;
	}
}

void release_all(){
	int i;
	for(i=0; i<RAM_SIZE; i++){
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

void update_pages(){
	int i;
	//Aging
	for(i=0; i<RAM_SIZE; i++){
		(ram.pages[i]).age = (ram.pages[i]).age/10;
		if((ram.pages[i]).ref){
			(ram.pages[i]).age += 10000000;
		}
	}
	
	//undirtify();

#endif
