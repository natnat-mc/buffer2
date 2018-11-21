# C side
In C, this library almost exclusively uses macros for accessing buffers, in order to avoid the function call overhead.  
This library exposes a `buffer_t` type which contains a size, an allocated size, and an user-defined int, as well as the data pointer.  
Buffers can be read and written as any type, as long as the given type can be read and written with a standard assignment.

## Buffer creation and destruction
These functions allow you to create and destroy buffers.
These functions are actually macros, but all of their arguments are evaluated only once.

### `buffer_t* buffer_alloc(int size)`
Allocates a buffer of a given size, and returns a pointer to it if everything went well.

### `buffer_t* buffer_calloc(int len, int elem)`
Allocates a buffer large enough to fit `len` elements of `elem` bytes, and fills it with zeros.

### `buffer_t* buffer_wrap(void* ptr, int len)`
Wraps a buffer around a pointer, for ease of use.
Such buffer can be read and written to, but destroying or resizing them will result in undefined behavior.
To destroy such buffer, simply `free` them.

### `void buffer_destroy(buffer_t* buf)`
Destroys a buffer, deallocating its internal memory and `free`ing the pointer.
You shouldn't use a deallocated buffer, and should remove all references to it.

## Advanced buffer creation and destruction
These functions are **not** to be used directly, but they still may be useful.

### `buffer_t* buffer_allocStruct()`
Allocates a `buffer_t` struct, but doesn't initialize it.

### `buffer_t* buffer_allocData(buffer_t* buf, int size, int destroy)`
Allocates the data portion of an uninitialized buffer, optionally `free`ing the buffer if this fails.

### `buffer_t* buffer_callocData(buffer_t* buf, int len, int elem, int destroy)`
Same than `buffer_allocData`, but using `calloc` instead of `malloc`.

### `buffer_t* buffer_wrapData(buffer_t* buf, void* ptr, int len)`
Wraps an unitialized buffer around a pointer.
Again, this means that you shouldn't destroy the buffer nor resize it.

### `void buffer_destroyData(void* buf)`
Destroys the data portion of a buffer without `free`ing it.

## Buffer size manipulation
These functions allow you to read and modify the size of existing buffers.
Some of these functions are actually macros for performance reasons, so you should be careful with double evaluation.

### `int buffer_getSize(buffer_t* buf)`
Returns the size of a buffer.
This is a macro and thus returns a lvalue, but you **should not** modify it yourself.

### `int buffer_resize(buffer_t* buf, int size)`
Resizes a buffer to a given number of bytes.
Returns `0` on failure and the allocated size on success.
The allocated size may be different from the available size for performance reasons.
This function doesn't fill anything with zeros, so you should take care of it.

### `int buffer_enlarge(buffer_t* buf, int delta)`
Resizes a buffer to add or remove a given ammount of bytes to its size.
This internally calls `buffer_resize`, so the same rules do apply.

### `int buffer_getAllocatedSize(buffer_t* buf)`
Returns the allocated size of a buffer.
Again, this is a lvalue which **should not** be modified.

## Raw buffer access
These functions allow reading from and writing to buffers directly.
These functions are actually macros for performance reasons, but all their arguments are evaluated only once.

### `void* buffer_getPointer(buffer_t* buf)`
Returns a pointer to the data contained in a buffer.
Care must be taken not to read or write out of bounds.

### `int buffer_getUser(buffer_t* buf)`
Returns the uservalue of a buffer.

### `void buffer_setUser(buffer_t* buf)`
Sets the uservalue of a buffer.

## Typed buffer access
These functions allow reading from and writing to buffers with explicit type checking.
These functions are actually macros for performance reasons, but all their arguments (except for `type`) are evaluated only once.

### `type* buffer_getArray(buffer_t* buf, type)`
Returns a typed pointer which can be used to read from or write to a buffer.
`type` can be any type of which it is possible to create an array.

### `type* buffer_getTypeArray(buffer_t* buf)`
Returns a typed pointer which can be used to read from or write to a buffer.
`type` can be any of `char`, `short`, `int`, `long`, `long long`, `float`, `double`.

### `int buffer_getLength(buffer_t* buf, type)`
Returns the length of the array returned by `buffer_getArray` with the same arguments.

### `int buffer_getTypeLength(buffer_t* buf)`
Returns the length of the array returned by `buffer_getTypeArray` with the same arguments.

### `type buffer_get(buffer_t* buf, int index, type)`
Returns the value at the specified index, of the specified type as a lvalue which **can** be modified.
Bounds are not checked, so you **must** ensure you don't read outside of them.
Indexes work the same way here as in an array.

### `type buffer_getType(buffer_t* buf, int index)`
Same as `buffer_get(buf, index, type)`.

### `void buffer_set(buffer_t* buf, int index, type value, type)`
Writes a value at a given index, of the specified type.
Internally uses `buffer_get`, so the same rules also apply.

### `void buffer_setType(buffer_t* buf, int index, type value)`
Same as `buffer_set(buf, index, value, type)`.
