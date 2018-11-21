#ifndef _BUFFER2_H
#define _BUFFER2_H

/* buffer library
 * handles buffers, which are just memory segments with additional information
 * buffers can be resized, and are dynamically allocated
 * please note that any function declared in this library may actually be a macro
 */

/* buffer type definition
 * contains the size, allocated size and position of the buffer
 * its size is 3*sizeof(int)+sizeof(void*), so it should be 24 (with alignment) on a 64bit system
 */
typedef struct buffer_t {
	int size, alloc;
	int user;
	void* ptr;
} buffer_t;

/* struct readers/writers
 * allow reading/writing from a buffer_t pointer
 */
#define buffer_getSize(buf) (((buffer_t*) buf)->size)
#define buffer_getAllocatedSize(buf) (((buffer_t*) buf)->alloc)
#define buffer_getPointer(buf) (((buffer_t*) buf)->ptr)
#define buffer_getUser(buf) (((buffer_t*) buf)->user)
#define buffer_setUser(buf, val) ((buffer_t*) buf)->user=val

/* array getters
 * get the array pointed by the buffer
 */
#define buffer_getCharArray(buf) ((char*) ((buffer_t*) buf)->ptr)
#define buffer_getShortArray(buf) ((short*) ((buffer_t*) buf)->ptr)
#define buffer_getIntArray(buf) ((int*) ((buffer_t*) buf)->ptr)
#define buffer_getLongArray(buf) ((long*) ((buffer_t*) buf)->ptr)
#define buffer_getLongLongArray(buf) ((long long*) ((buffer_t*) buf)->ptr)
#define buffer_getFloatArray(buf) ((float*) ((buffer_t*) buf)->ptr)
#define buffer_getDoubleArray(buf) ((double*) ((buffer_t*) buf)->ptr)
#define buffer_getArray(buf, type) ((type*) ((buffer_t*) buf)->ptr)

/* length getters
 * return the length of the buffer
 */
#define buffer_getCharLength(buf) (buffer_getSize(buf)/sizeof(char))
#define buffer_getShortLength(buf) (buffer_getSize(buf)/sizeof(short))
#define buffer_getIntLength(buf) (buffer_getSize(buf)/sizeof(int))
#define buffer_getLongLength(buf) (buffer_getSize(buf)/sizeof(long))
#define buffer_getLongLongLength(buf) (buffer_getSize(buf)/sizeof(long long))
#define buffer_getFloatLength(buf) (buffer_getSize(buf)/sizeof(float))
#define buffer_getDoubleLength(buf) (buffer_getSize(buf)/sizeof(double))
#define buffer_getLength(buf, type) (buffer_getSize(buf)/sizeof(type))

/* item getters
 * returns directly a lvalue
 * can be used to read OR set, actually
 */
#define buffer_getChar(buf, idx) buffer_getCharArray(buf)[idx]
#define buffer_getShort(buf, idx) buffer_getShortArray(buf)[idx]
#define buffer_getInt(buf, idx) buffer_getIntArray(buf)[idx]
#define buffer_getLong(buf, idx) buffer_getLongArray(buf)[idx]
#define buffer_getLongLong(buf, idx) buffer_getLongLongArray(buf)[idx]
#define buffer_getFloat(buf, idx) buffer_getFloatArray(buf)[idx]
#define buffer_getDouble(buf, idx) buffer_getDoubleArray(buf)[idx]
#define buffer_get(buf, idx, type) buffer_getArray(buf, type)[idx]

/* item setters
 * use the getters as setters
 */
#define buffer_setChar(buf, idx, val) buffer_getChar(buf, idx)=val
#define buffer_setShort(buf, idx, val) buffer_getShort(buf, idx)=val
#define buffer_setInt(buf, idx, val) buffer_getInt(buf, idx)=val
#define buffer_setLong(buf, idx, val) buffer_getLong(buf, idx)=val
#define buffer_setLongLong(buf, idx, val) buffer_getLongLong(buf, idx)=val
#define buffer_setFloat(buf, idx, val) buffer_getFloat(buf, idx)=val
#define buffer_setDouble(buf, idx, val) buffer_getDouble(buf, idx)=val
#define buffer_set(buf, idx, val, type) buffer_get(buf, idx, type)=val

/* buffer struct allocator
 * allocates a buffer struct
 */
#define buffer_allocStruct() ((buffer_t*) malloc(sizeof(buffer_t)))

/* internal buffer data manipulators
 * manipulate internal data of a buffer
 * these are NOT to be called to create buffers, and MUST be called on uninitialized buffers
 * you better have a good reason to use these
 */
buffer_t *buffer_allocData(void* buf, int size, int destroy);
buffer_t *buffer_callocData(void* buf, int len, int elem, int destroy);
buffer_t *buffer_wrapData(void* buf, void* ptr, int len);
void buffer_destroyData(void* buf);

/* buffer allocator
 * creates a buffer and returns a pointer to it
 * note that it is created on the heap and as such you will need to destroy it properly
 */
#define buffer_alloc(size) buffer_allocData(buffer_allocStruct(), size, 1)

/* buffer allocator with fill size
 * creates a buffer with a length and an element size, and fills it with zeros
 * the buffer also needs to be destroyed properly
 */
#define buffer_calloc(len, elem) buffer_callocData(buffer_allocStruct(), len, elem, 1)

/* buffer wrapper
 * wraps a pointer around a memory region
 * if you resize the buffer, then behavior is undefined
 * destroying such buffer is also undefined behavior
 * if the memory region becomes invalid, that is also undefined behavior
 * basically, be careful with these
 */
#define buffer_wrap(ptr, len) buffer_wrapData(buffer_allocStruct(), ptr, len)

/* buffer destroyer
 * destroys properly a buffer
 * you must always destroy allocated buffers
 */
#define buffer_destroy(buf) (buffer_destroyData(buf), free(buf))

/* buffer resizer
 * sets a buffer's size to a given value
 * always preserves the data
 * note that more memory may actually be allocated
 * also note that resizing to a negative size is undefined behavior
 * returns nonzero on success, zero on error
 */
int buffer_resize(void* buffer, int size);

/* buffer enlarge
 * enlarges or shrinks a buffer by a said number of bytes
 * it internally calls buffer_resize, so the same restrictions apply
 */
#define buffer_enlarge(buf, ammount) buffer_resize(buf, buffer_getSize(buf)+ammount)

#endif //_BUFFER2_H
