#include "const.h"

#ifndef HEADER_PAGE
#define HEADER_PAGE

typedef struct {
	bool ref;
	bool drty;
	int dat;
	int age;
	bool location;
	int block;
} page;
typedef page* pgptr;

pgptr crt_page(int dat, bool location, int block){
	pgptr p = malloc(sizeof(page));
	page template = {false, false, dat, 0, location, block};
	*p = template;
	return p;
}

void print_page(page pg, int index){
	printf("  ______________ \n");
	printf(" |   Pagina %d   |\n", index);
	printf(" |--------------|\n");
	printf(" |   ref....%d   |\n", pg.ref);
	printf(" |   drty...%d   |\n", pg.drty);
	printf(" |   loc....%d   |\n", pg.location);
	printf(" |   block.%02d   |\n", pg.block);
	printf(" |     age:     |\n");
	printf(" |  <%8d>  |\n", pg.age);
	printf(" |     dat:     |\n");
	printf(" | <%10d> |\n", pg.dat);
	printf(" |______________|\n");
}

#endif
