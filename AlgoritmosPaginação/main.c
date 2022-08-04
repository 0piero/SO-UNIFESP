#include "handler.h"

void run_a(int p){
	int ind;
	ind = get_page(p);
	print_page((ram.pages[ind]), ind);
	release_page(ind);
}

int main(){
	int ind;
	/*pgptr pg1 = crt_page(123, 0, 3);
	pgptr pg2 = crt_page(4564167, 1, 2);
	pgptr pg3 = crt_page(125456, 0, 3);
	pgptr pg4 = crt_page(786, 1, 2);*/
	crt_memory(RAM_SIZE, &ram);
	
	printf("\n\n=====================INICIAL===================\n");

	
	crt_handler(0, 0, RAM_SIZE, SWAP_SIZE);
	print_all_in_mem();

	
	for(ind=0; ind<2; ind++){
		usleep(100*1000);
		printf("\n\n=====================PASSADA===================\n");
		run_a(rand()%3);
		run_a(rand()%6);
		run_a(rand()%9);
		run_a(rand()%16);
		update_pages();
	}
	printf("\nmisses: %d\n", misses);
	print_all_in_mem();
	
	return 0;
}
