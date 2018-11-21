#include <stdlib.h>
#include <stdio.h>
#include "buffer2.h"

int main(void) {
	// allocate a buffer to hold 25 ints
	buffer_t* buf=buffer_calloc(25, sizeof(int));
	
	// fill it with ints
	for(int i=0; i<buffer_getIntLength(buf); i++) buffer_setInt(buf, i, 0x10000000*i);
	
	// read it as floats
	for(int i=0; i<buffer_getFloatLength(buf); i++) printf("%f\n", buffer_getFloat(buf, i));
	
	return EXIT_SUCCESS;
}
