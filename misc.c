#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "errors.h"
#include "misc.h"

static int nallocs=0;

void *xmalloc(size_t size) {
	void *value;

	value = malloc (size);
	nallocs++;

	if (value == 0) gerror ("\nerror in xmalloc\n");
	return value;
}

void xfree(void *p) {
	nallocs--;
	free(p);
}

extern inline double dMax  ( double x, double y ) { return x > y ? x : y; }
