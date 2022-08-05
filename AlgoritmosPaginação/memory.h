#include "page.h"
#ifndef HEADER_MEM
#define HEADER_MEM
typedef struct {
	pgptr pages;
	int size;
	int pos;
} memory;
memory ram;

void crt_memory(int size, memory* m){
	memory mem = {malloc(size*sizeof(page)), size, 0};
	*m = mem;
}

void alloc_memory(memory* mem ,pgptr pg){
	mem->pages[mem->pos] = *pg;
	mem->pos = (mem->pos + 1) % mem->size;
}

void write_swap(int pos, pgptr pg){
	int algs = 5;
	char *arquivo_nome = (char*)calloc((4+algs), sizeof(char));
	FILE *sw;
	pg->location = SWAP;
	//pg->block = pos; // era "= ram_adr;"
	//printf("Novo block: %d\n", pg->block);
	sprintf(arquivo_nome, "%d%s", pos, ".pag");
	sw = fopen(arquivo_nome,"w");

	fprintf(sw, "%d\n%d\n%d\n%d\n%d\n", 0, 0, pg->dat, pg->location, pg->block);
	ref_times[pg->block]=0;
	fclose(sw);
}

void read_swap(int pos, pgptr pg){
	int algs = 5;
	int aux_drty, aux_ref, aux_location;
	char *arquivo_nome = (char*)calloc((4+algs), sizeof(char));
	FILE *sw;
	sprintf(arquivo_nome, "%d%s", pos, ".pag");
	sw = fopen(arquivo_nome,"r");
	fscanf(sw, "%d\n%d\n%d\n%d\n%d\n", &aux_ref, &aux_drty, &(*pg).dat, &aux_location, &(*pg).block);
	ref_times[pg->block]=VIRTUAL_TIME;
	(*pg).ref = aux_ref;
	(*pg).drty = aux_drty;
	(*pg).location = RAM;
	(*pg).age = 0;
	fclose(sw);
}

void save_all_in_mem(){
	int i;
	for(i=0; i<RAM_SIZE; i++){
		write_swap((ram.pages[i]).block, &(ram.pages[i]));
	}
	printf("}\n\n");
}

void swap_memory(int ram_adr, int swap_adr){
	//printf("RAM_ADR, SWAP ADR : %d %d\n", ram_adr, swap_adr);
	pgptr buff = &ram.pages[ram_adr];
	page p = *buff;
	//printf("Página selecionada: %d\n", p.block);
	addresses[(*buff).block] = -1;
	addresses[swap_adr] = ram_adr;

	read_swap(swap_adr, &(ram.pages[ram_adr]));
	write_swap(p.block, &p);
}

void print_all_in_mem(){
	int i;
	printf("Paginas carregadas em memoria {\n");
	for(i=0; i<RAM_SIZE; i++){
		print_page((ram.pages[i]), i);
	}
	printf("}\n\n");
}

int get_real_address(int ind){
	int i;

	for(i=0; i<RAM_SIZE; i++){
		
		if((ram.pages[i]).block == ind){
			return i;	
		}
	}
	
	return -1;
}


#endif
