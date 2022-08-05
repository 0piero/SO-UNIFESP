#include "handler.h"

void *run_a(void *in){
	int ind;
	int p = *((int *) in);
	ind = get_page(p);
	print_page((ram.pages[ind]), ind);
}

void run_b(int p){
	int ind;
	ind = get_page(p);
	print_page((ram.pages[ind]), ind);
}
int registros[NUM_ITERS];
int main(){
	int ind;
	int times[4];
	//pthread_t threads[4];

	crt_memory(RAM_SIZE, &ram);
	
	printf("\n\n=====================INICIAL===================\n");
	
	crt_handler(0, 0, RAM_SIZE, SWAP_SIZE);
	print_all_in_mem();
	read_swap(0, &(ram.pages[0]));
	read_swap(1, &(ram.pages[1]));
	read_swap(2, &(ram.pages[2]));
	read_swap(3, &(ram.pages[3]));

	print_all_in_mem();
	print_addresses();
	
	for(ind=0; ind<100; ind++){
		usleep(1*1000);
		printf("\n\n=====================PASSADA===================\n");
												
		for(int j=0;j<NUM_ITERS;j++){
			double rand_numb = randn(RAM_SIZE/2, RAM_SIZE/8);
			int x = (int)rand_numb;
			printf("NUM ALEATORIO: %d\n", x);
			run_b(x);
		}
		update_ages();
		release_all();
		registros[ind] = misses;
		VIRTUAL_TIME++;
	}
	FILE* f = fopen("misses.txt","w+");
	for(int j=0; j<NUM_ITERS;j++){
		fprintf(f,"%d\n", registros[j]);
	}
	fclose(f);
	//save_all_in_mem();
	//printf("\nmisses: %d\n", misses);
	//print_all_in_mem();
	//print_addresses();
	return 0;
}