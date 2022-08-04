#include "table.h"

#ifndef HEADER_MEM
#define HEADER_MEM

typedef struct {
	pgptr pages;
	int size;
	int pos;
} memory;

memory ram;
int addresses[SWAP_SIZE]; // Tabela de MV = MS -> MR

memory crt_memory(int size){
	memory m = {malloc(size*sizeof(pgptr)), size, 0};
	return m;
}

void alloc_memory(memory* mem ,pgptr pg){
	mem->pages[mem->pos] = *pg;
	mem->pos = (mem->pos + 1) % mem->size;
}

void write_swap(int pos, pgptr pg){
	int algs = 5; // int algs = log10(pos)+1;
	char *arquivo_nome = (char*)malloc((4+algs)*sizeof(char));
	FILE *sw;
	pg->location = RAM;
	pg->block = pos; // era "= ram_adr;"
	sprintf(arquivo_nome, "%d%s", pos, ".pag");
	sw = fopen(arquivo_nome,"w");
	fprintf(sw, "%d\n%d\n%d\n%d\n%d\n", pg->ref, pg->drty, pg->dat, pg->location, pg->block);
	fclose(sw);
}

void read_swap(int pos, pgptr pg){
	int algs = 5; // int algs = log10(pos)+1;
	int aux_drty, aux_ref, aux_location;
	char *arquivo_nome = (char*)malloc((4+algs)*sizeof(char));
	FILE *sw;
	sprintf(arquivo_nome, "%d%s", pos, ".pag");
	sw = fopen(arquivo_nome,"r");
	fscanf(sw, "%d\n%d\n%d\n%d\n%d\n", &aux_ref, &aux_drty, &((*pg).dat), &aux_location, &((*pg).block));
	(*pg).ref = aux_ref;
	(*pg).drty = aux_drty;
	(*pg).location = aux_location;
	(*pg).age = 0;
	fclose(sw);
}

void swap_memory(int ram_adr, int swap_adr){
	pgptr buff = &ram.pages[ram_adr];

	printf("AAAAA %d \n", buff->block);
 
 	// ATIVAR QUANDO O BUFF ESTIVER ARRUMADO
	//addresses[(*buff).block] = -1;
	//addresses[swap_adr] = ram_adr;
	
	(*buff).location = SWAP;
	(*buff).block = swap_adr;

	read_swap(swap_adr, &(ram.pages[ram_adr]));
	write_swap(swap_adr, (buff));
}

void print_all_in_mem(){
	int i;
	printf("Paginas carregadas em memoria {\n");
	for(i=0; i<RAM_SIZE; i++){
		print_page((ram.pages[i]), i);
	}
	printf("}\n\n");
}

void save_all_in_mem(){
	int i;
	for(i=0; i<RAM_SIZE; i++){
		write_swap((ram.pages[i]).block, &(ram.pages[i]));
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
