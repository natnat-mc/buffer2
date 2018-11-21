#include "buffer2.h"

#include <stdlib.h>

buffer_t *buffer_allocData(void* buffer, int size, int destroy) {
	buffer_t *buf=(buffer_t*) buffer;
	
	if(buf==NULL) return NULL;
	
	buf->size=size;
	buf->alloc=size;
	buf->user=0;
	buf->ptr=malloc(size);
	
	if(buf->ptr==NULL) {
		if(destroy) free(buf);
		else buf->size=buf->alloc=0;
		return NULL;
	}
	
	return buf;
}

buffer_t *buffer_callocData(void* buffer, int length, int elem, int destroy) {
	buffer_t *buf=(buffer_t*) buffer;
	
	if(buf==NULL) return NULL;
	
	buf->size=length*elem;
	buf->alloc=buf->size;
	buf->user=0;
	buf->ptr=calloc(length, elem);
	
	if(buf->ptr==NULL) {
		if(destroy) free(buf);
		else buf->size=buf->alloc=0;
		return NULL;
	}
	
	return buf;
}

buffer_t *buffer_wrapData(void* buffer, void* ptr, int size) {
	buffer_t *buf=(buffer_t*) buffer;
	
	if(buf==NULL) return NULL;
	
	buf->size=size;
	buf->alloc=0;
	buf->user=0;
	buf->ptr=ptr;
	
	return buf;
}

void buffer_destroyData(void* buf) {
	if(buf==NULL) return;
	if(buffer_getAllocatedSize(buf)) free(buffer_getPointer(buf));
}

int buffer_resize(void* buf, int size) {
	buffer_t *buffer=(buffer_t*) buf;
	
	if(size<=0) return 0;
	
	if(buffer->alloc>=size) {
		buffer->size=size;
		return buffer->alloc;
	} else if(buffer->alloc) {
		void* ptr=realloc(buffer->ptr, size);
		if(ptr==NULL) return 0;
		buffer->ptr=ptr;
		buffer->size=size;
		buffer->alloc=size;
		return size;
	} else {
		buffer->ptr=malloc(size);
		if(buffer->ptr==NULL) return 0;
		buffer->size=size;
		buffer->alloc=size;
		return size;
	}
}
