# Lua side
In lua, this library exposes all high level the functions it exposes in C except `buffer_getUser` and `buffer_setUser` which it uses internally.
This library exposes a table, which it returns when `require`d, but doesn't store in the global context.
Buffer instances are userdata with metatables, allowing them to be used more easily.
Buffer instances destroy themselves when collected, and cannot be destroyed manually.
Buffer instances have a type, which is the default type for data read from and written to it.
For simplicity, we will call the library table `buffer2`, the buffer type `buffer` and buffer instances `buf`.

## Misc and internal functions
These functions are not considered part of the core interface, but are still useful.

### `int index, any value buffer2.iter(any tab, int index)`
An iterator capable of iterating through any `table`-like object.
To be more specific, `tab` must expose a length (`#tab` must exist and be an integer) and indexable (`tab[n]` where `n` is an integer between `1` and `#tab` must exist and be a meaningful value, including `nil`).

## Buffer creation functions
These functions create buffer instances.

### `buffer buf buffer2.new(int size)`
Creates a new buffer instance of size `size` and in `char` mode.

### `buffer buf buffer2.calloc(int length, string|int type)`
Creates a new buffer instance of length `length` and in `type` mode.

## Buffer size manipulation
Buffers in lua have two distinct values for `size` and `length`.
A buffer's `size` represents its physical size in bytes whereas its `length` represents the number of items which can be stored into it in its current mode.

### `int size buffer2.getsize(buffer buf)` | `size=buf.size` | `int size buf:getsize()`
Returns the size of the buffer.

### `int length buffer2.getlength(buffer buf, string|int? type)` | `length=#buf` | `length=buf.length` | `int length buf:getlength(string|int? type)`
Returns the length of the buffer.
If the `type` argument is provided, the function will instead return the length the buffer would have if it was in this type.

### `buffer2.setsize(buffer buf, int size)` | `buf.size=size` | `buf:setsize(int size)`
Sets the size of the buffer.

### `buffer2.setlength(buffer buf, int length, string|int? type)` | `buf.length=length` | `buf:setlength(int length, string|int? type)`
Sets the length of the buffer.
If the `type` argument is provided, the function will instead resize the buffer such that its length in `type` mode would be `length`.

## Buffer type manipulation
Buffers in lua have a type, which will govern how it will read data from and write data to them.
Only the `char`, `int8` (also known as `8`) and `float` types, along with their `signed` and `unsigned` counterparts, are guaranteed to be available.
Other types, such as `short`, `int`, `long`, `long long`, `int16` (`16`), `int32` (`32`), `int64` (64) and `double` can be available, depending on platform and configuration.
Before using optional types, check for the presence in the `buffer2.types` table of their base type (without `signed` or `unsigned`).
Floating-point types (`float` and `double`) don't have `signed` and `unsigned` counterparts.
Internally, types are represented as integers, and can be matched with their string representation by using the `buffer2.types` table.

### `int type buffer2.gettype(buffer buf)` | `type=buf.type` | `int type buf:gettype()`
Returns the type of a buffer

### `buffer2.settype(buffer buf, string|int type)` | `buf.type=type` | `buf:settype(string|int type)`
Sets the type of a buffer.
The type can be supplied in its integer or string representation, but it will always be converted to an integer.

## Data reading and writing
Buffers can be used as tables, exposing a length (`#buf`), and being indexable (`a=buf[n]`, `buf[n]=a`).
Buffers can also be read faster using functions.
Buffers can also be iterated with the `ipairs` iterator.

### `type value buffer2.get(buffer buf, int index, string|int? type)` | `value=buf[index]` | `type value buf:get(int index, string|int? type)`
Reads a value from the buffer at the given index, for the given type.
Reading out of bounds will return `nil`.

### `buffer2.set(buffer buf, int index, type value, string|int? type)` | `buf[index]=value` | `buf:set(int index, type value, string|int? type)`
Writes a value into the buffer at the given index, as the given type.
Writing out of bounds will silently fail.
The value will be coerced to its required type if it is a number.
If the value is not a number, then this function will throw an error.

### `function iterator, buffer buf, int index ipairs(buffer buf)` | `for index, value in ipairs(buf) do ... end`
Iterates a buffer as a table of its type.
