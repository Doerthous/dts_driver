#include "at24cxx.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <lib/uart_printf.h>

int at24cxx_test(at24cxx_t *at24cxx)
{
	static uint8_t buff[4096];
	uint8_t *in = buff;
	uint8_t *out = buff + 2048;
	int i;
	
	
	memset(buff, 0, 4096);
	
	//srand(time(NULL));
    
    for (i = 0; i < at24cxx->capacity; i++) {
		in[i] = rand() % 256;
	}

    if (at24cxx_write(at24cxx, 0, in, at24cxx->capacity) != at24cxx->capacity) {
		return 0;
	}
	
	if (at24cxx_read(at24cxx, 0, out, at24cxx->capacity) != at24cxx->capacity) {
		return 0;
	}
	
	if (memcmp(in, out, at24cxx->capacity) != 0) {
		return 0;
	}
	
	for (i = 0; i < at24cxx->capacity; i++) {
		in[i] = rand() % 256;
	}
	
	if (at24cxx_write(at24cxx, 111, in, at24cxx->capacity) != at24cxx->capacity) {
		return 0;
	}
	
	if (at24cxx_read(at24cxx, 111, out, at24cxx->capacity) != at24cxx->capacity) {
		return 0;
	}
	
	if (memcmp(in, out, at24cxx->capacity) != 0) {
		return 0;
	}
		
	return 1;
}
