# Buffer2
A buffer lib for C and lua.  
Its documentation can be found in [`doc/C.md`](doc/C.md) and [`doc/LUA.md`](doc/LUA.md)

## C example
`examples/example.c`:
```c
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
    
    // destroy the buffer
    buffer_destroy(buf);
	
	return EXIT_SUCCESS;
}
```

## Lua example
`examples/example.lua`
```lua
local buffer2=require 'buffer2'

-- create a buffer to hold 16 shorts
local buf=buffer2.calloc(16, 'short')

-- fill it with powers of two
for i=1, #buf do
	buf[i]=2^(i-1)
end

-- read it as chars
buf.type='char'
for i=1, #buf do
	print(i, buf[i])
end
```