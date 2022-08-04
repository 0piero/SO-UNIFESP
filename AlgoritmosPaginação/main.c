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

int main(){
	int ind;
	int times[4];
	//pthread_t threads[4];

	ram = crt_memory(RAM_SIZE);
	
	printf("\n\n=====================INICIAL===================\n");
	
	crt_handler(0, 0, RAM_SIZE, SWAP_SIZE);
	print_all_in_mem();
	read_swap(0, &(ram.pages[0]));
	read_swap(1, &(ram.pages[1]));
	read_swap(2, &(ram.pages[2]));
	read_swap(3, &(ram.pages[3]));

	print_all_in_mem();
	print_addresses();
	
	for(ind=0; ind<5; ind++){
		usleep(100*1000);
		printf("\n\n=====================PASSADA===================\n");
		/*times[0] = rand()%3;
		pthread_create(&(threads[0]), NULL, run_a, &(times[0]));
		times[1] = rand()%6;
		pthread_create(&(threads[1]), NULL, run_a, &(times[1]));
		times[2] = rand()%9;
		pthread_create(&(threads[2]), NULL, run_a, &(times[2]));
		times[3] = rand()%16;
		pthread_create(&(threads[3]), NULL, run_a, &(times[3]));*/
		run_b(rand()%3);
		run_b(rand()%6);
		run_b(rand()%9);
		run_b(rand()%SWAP_SIZE);
		update_pages();
		release_all();
	}
	save_all_in_mem();
	printf("\nmisses: %d\n", misses);
	print_all_in_mem();
	print_addresses();
	return 0;
}
