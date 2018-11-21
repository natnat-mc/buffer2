/**
 * @name buffer2
 * this library allows the use of buffers in Lua
 * these buffers can be resized dynamically and accessed as arrays of any type
 */

/**
 * list of functions in the main library:
 * new: creates a buffer
 * calloc: creates a buffer filled with zeroes
 * getsize: returns the size of a buffer
 * setsize: resizes a buffer
 * getlength: returns the length of a buffer used as an array
 * setlength: resizes a buffer so that its length would be a specific value
 * gettype: returns the type of the array
 * settype: sets the type of the array
 * get: returns the value at a given index, of a given type
 * set: sets the value at a given index, of a given type
 * iter: an iterator which can be used on any table-like object
 */

/**
 * list of properties of buffer objects:
 * size: the size of the buffer, in bytes
 * length: the length of the buffer used as an array
 * type: the type of objects stored in the buffer, when used as an array
 */

/**
 * list of methods on buffer objects:
 * getsize: returns its size property
 * setsize: sets its size property
 * getlength: returns its length property
 * setlength: sets its length property
 * gettype: returns its type property
 * settype: sets its type property
 * get: reads at a given index, as a given type
 * set: writes at a given index, as a given type
 * @remark other functions from the main library will be available as buffer methods, but will cause undefined behavior if called an potentially throw
 */

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "buffer2.h"

#include <stdint.h>
#include <string.h>
#include <limits.h>

// function type markers
#define API static
#define INTERNAL static
#define OTHER static
#define ENTRYPOINT

//BEGIN type constants

// useful constants
#define typename(sgn, name) T_##sgn##name
#define typeid(name) TYPE_##name

// class name
#define BUFFER_CLASS "buffer2"

// findstr struct
typedef struct {
	int val;
	char* str;
} findstr_t;

// signedness
#define TYPE_UNSIGNED 0x00
#define TYPE_SIGNED 0x10

// standard types
#define TYPE_CHAR 0x0
#define T_UCHAR unsigned char
#define T_SCHAR signed char

#if LUA_MAXINTEGER>=SHRT_MAX
#define TYPE_SHORT 0x1
#define T_USHORT unsigned short
#define T_SSHORT signed short
#endif

#if LUA_MAXINTEGER>=INT_MAX
#define TYPE_INT 0x2
#define T_UINT unsigned int
#define T_SINT signed int
#endif

#if LUA_MAXINTEGER>=LONG_MAX
#define TYPE_LONG 0x3
#define T_ULONG unsigned long
#define T_SLONG signed long
#endif

#if LUA_MAXINTEGER>=LLONG_MAX
#define TYPE_LONGLONG 0x4
#define T_ULONGLONG unsigned long long
#define T_SLONGLONG signed long long
#endif

#define TYPE_FLOAT 0x5
#define T_UFLOAT float
#define T_SFLOAT float

#if LUA_FLOAT_TYPE>=LUA_FLOAT_DOUBLE
#define TYPE_DOUBLE 0x6
#define T_UDOUBLE double
#define T_SDOUBLE double
#endif

// fixed-width types
#define TYPE_8 0x7
#define T_U8 uint8_t
#define T_S8 int8_t

#if LUA_MAXINTEGER>=32767
#define TYPE_16 0x8
#define T_U16 uint16_t
#define T_S16 int16_t
#endif

#if LUA_MAXINTEGER>=2147483647
#define TYPE_32 0x9
#define T_U32 uint32_t
#define T_S32 int32_t
#endif

#if LUA_MAXINTEGER>=9223372036854775807
#define TYPE_64 0xa
#define T_U64 uint64_t
#define T_S64 int64_t
#endif
//END type constants

//BEGIN function prototypes

// setup functions
ENTRYPOINT int luaopen_buffer2(lua_State *L);
INTERNAL int setupMeta(lua_State *L);
INTERNAL int setupLib(lua_State *L);

// internal functions
INTERNAL int isValidType(int type);
INTERNAL buffer_t *bufferFromArg(lua_State *L);
INTERNAL int typeFromArg(lua_State *L, buffer_t *buf, int arg);
INTERNAL int startswith(const char* str, const char* beginning);
INTERNAL int findstr(const char* str, findstr_t* list);
INTERNAL int getLength(buffer_t *buf, int type);
INTERNAL int typeSize(int type);

// size (in bytes) getter/setter
API int api_bufferGetSize(lua_State *L);
API int api_bufferSetSize(lua_State *L);

// length (according to type) getter/setter
API int api_bufferGetLength(lua_State *L);
API int api_bufferSetLength(lua_State *L);

// type getter/setter
API int api_bufferGetType(lua_State *L);
API int api_bufferSetType(lua_State *L);

// value getter/setter
API int api_bufferGet(lua_State *L);
API int api_bufferSet(lua_State *L);

// buffer creator
API int api_bufferNew(lua_State *L);
API int api_bufferCalloc(lua_State *L);

// metamethods
API int meta_index(lua_State *L);
API int meta_newindex(lua_State *L);
API int meta_len(lua_State *L);
API int meta_ipairs(lua_State *L);
API int meta_gc(lua_State *L);

// other Lua functions
OTHER int other_iter(lua_State *L);
//END function prototypes

//BEGIN internal functions
/**
 * @name isValidType
 * checks if a type is valid for the current configuration
 * excludes types that are too wide for the current Lua configuration
 * @param type: int, internal representation of the type
 * @returns int, 0 if invalid, nonzero if valid
 */
int isValidType(int type) {
	// check if there are illegal bits
	if((type&0x1f)!=type) return 0;
	
	// check individual types
	switch(type&0xf) {
		// standard types
		case TYPE_CHAR:
			return 1;
#ifdef TYPE_SHORT
		case TYPE_SHORT:
			return 1;
#endif
#ifdef TYPE_INT
		case TYPE_INT:
			return 1;
#endif
#ifdef TYPE_LONG
		case TYPE_LONG:
			return 1;
#endif
#ifdef TYPE_LONGLONG
		case TYPE_LONGLONG:
			return 1;
#endif
		case TYPE_FLOAT:
			return 1;
#ifdef TYPE_DOUBLE
		case TYPE_DOUBLE:
			return 1;
#endif
		// fixed-width types
		case TYPE_8:
			return 1;
#ifdef TYPE_16
		case TYPE_16:
			return 1;
#endif
#ifdef TYPE_32
		case TYPE_32:
			return 1;
#endif
#ifdef TYPE_64
		case TYPE_64:
			return 1;
#endif
		default:
			return 0;
	}
}

/**
 * @name bufferFromArg
 * unwraps the buffer_t contained in Lua arg1
 * throws on error
 * @param L: lua_State, the Lua instance
 * @returns buffer_t*, a pointer to the buffer_t
 */
buffer_t *bufferFromArg(lua_State *L) {
	return luaL_checkudata(L, 1, BUFFER_CLASS);
}

/**
 * @name typeFromArg
 * reads a type from Lua arg#arg
 * defaults to the type set by the buffer
 * throws on error
 * @param L: lua_State, the Lua instance
 * @param buf: buffer_t*, a pointer to the buffer*
 * @param arg: int, the index of the argument
 * @returns int, the type
 */
int typeFromArg(lua_State *L, buffer_t *buf, int arg) {
	if(lua_isnumber(L, arg)) {
		// treat the argument as a direct type and return it if it is valid
		int type=luaL_checkinteger(L, arg);
		if(isValidType(type)) return type;
	} else if(lua_isstring(L, arg)) {
		// treat the argument as a string type and return it if it is valid
		size_t len=0;
		const char* str=luaL_checklstring(L, arg, &len);
		
		// check if the type is signed
		int signedness=startswith(str, "signed ");
		if(signedness) {
			str+=7; // length of "signed "
		} else {
			if(startswith(str, "unsigned ")) str+=9; // length of "unsigned "
		}
		
		// get type from name
		int type=findstr(str, (findstr_t[]) {
			{TYPE_CHAR, "char"}
#ifdef TYPE_SHORT
			,{TYPE_SHORT, "short"}
#endif
#ifdef TYPE_INT
			,{TYPE_INT, "int"}
#endif
#ifdef TYPE_LONG
			,{TYPE_LONG, "long"}
#endif
#ifdef TYPE_LONGLONG
			,{TYPE_LONGLONG, "long long"}
#endif
			,{TYPE_FLOAT, "float"}
#ifdef TYPE_DOUBLE
			,{TYPE_DOUBLE, "double"}
#endif
			,{TYPE_8, "8"}
			,{TYPE_8, "int8"}
#ifdef TYPE_16
			,{TYPE_16, "16"}
			,{TYPE_16, "int16"}
#endif
#ifdef TYPE_32
			,{TYPE_32, "32"}
			,{TYPE_32, "int32"}
#endif
#ifdef TYPE_64
			,{TYPE_64, "64"}
			,{TYPE_64, "int64"}
#endif
			,{-1, NULL}
		});
		
		// return the type if it is valid
		if(type!=-1) {
			type|=signedness<<4;
			if(isValidType(type)) return type;
		}
	} else if(lua_isnoneornil(L, arg)&&buf!=NULL) {
		// return the type stored in the buffer
		return buffer_getUser(buf)&0x1f;
	}
	// if we're still here, there was an error that we need to throw
	return luaL_argerror(L, arg, "must be a valid type");
}

/**
 * @name startswith
 * determines if a C string starts with a sub-sequence
 * @param str: char*, the string in which to read
 * @param beginning: char*, the string to be detected inside str
 * @returns int, 1 if present, 0 if absent
 */
int startswith(const char* str, const char* beginning) {
	while(*beginning!='\0') {
		if(*str!=*beginning) return 0;
		str++;
		beginning++;
	}
	return 1;
}

/**
 * @name findstr
 * matches a string against a number of others to find a corresponding number
 * the list must be terminated by an element with a string value of NULL; its number will be the return value
 * @param str: char*, the string to test
 * @param list: findstr_t*, the list of number-string pairs
 * @returns the number associated with the string
 */
int findstr(const char* str, findstr_t* list) {
	while(list->str!=NULL) {
		if(!strcmp(str, list->str)) return list->val;
		list++;
	}
	return list->val;
}

#define lenForType(type) case TYPE_##type: \
	return buffer_getLength(buf, typename(U, type));
/**
 * @name getLength
 * determines the length of a buffer according to its type
 * @param buf: buffer_t*, the buffer
 * @param type: int, the type
 * @returns int, the length, -1 if unable to determine it
 */
int getLength(buffer_t *buf, int type) {
	switch(type&0xf) {
		lenForType(CHAR)
#ifdef TYPE_SHORT
		lenForType(SHORT)
#endif
#ifdef TYPE_INT
		lenForType(INT)
#endif
#ifdef TYPE_LONG
		lenForType(LONG)
#endif
#ifdef TYPE_LONGLONG
		lenForType(LONGLONG)
#endif
		lenForType(FLOAT)
#ifdef TYPE_DOUBLE
		lenForType(DOUBLE)
#endif
		lenForType(8)
#ifdef TYPE_16
		lenForType(16)
#endif
#ifdef TYPE_32
		lenForType(32)
#endif
#ifdef TYPE_64
		lenForType(64)
#endif
	}
	return -1;
}
#undef lenForType

#define sizeForType(type) case TYPE_##type: \
	return sizeof(typename(U, type));
/**
 * @name typeSize
 * determines the size of the type
 * @param type: int, the type
 * @returns int, the size, -1 if unable to determine it
 */
int typeSize(int type) {
	switch(type&0xf) {
		sizeForType(CHAR)
#ifdef TYPE_SHORT
		sizeForType(SHORT)
#endif
#ifdef TYPE_INT
		sizeForType(INT)
#endif
#ifdef TYPE_LONG
		sizeForType(LONG)
#endif
#ifdef TYPE_LONGLONG
		sizeForType(LONGLONG)
#endif
		sizeForType(FLOAT)
#ifdef TYPE_DOUBLE
		sizeForType(DOUBLE)
#endif
		sizeForType(8)
#ifdef TYPE_16
		sizeForType(16)
#endif
#ifdef TYPE_32
		sizeForType(32)
#endif
#ifdef TYPE_64
		sizeForType(64)
#endif
	}
	return -1;
}
#undef sizeForType
//END internal functions

//BEGIN buffer creator
/**
 * @ref buffer.new(size)
 * @arg1: int, size
 * @rer1: buffer, buf
 */
int api_bufferNew(lua_State *L) {
	int size=luaL_checkinteger(L, 1);
	if(size<=0) return luaL_argerror(L, 1, "size must be positive");
	
	buffer_t* buf=(buffer_t*) lua_newuserdata(L, sizeof(buffer_t));
	if(!buffer_allocData(buf, size, 0)) {
		buf->alloc=0;
		return luaL_error(L, "failed to allocate buffer");
	}
	luaL_setmetatable(L, BUFFER_CLASS);
	return 1;
}

/**
 * @ref buffer.calloc(len, elem)
 * @arg1: int, len
 * @arg2: int|string, elem
 * @rer1: buffer, buf
 */
int api_bufferCalloc(lua_State *L) {
	// get size and/or type
	int len=luaL_checkinteger(L, 1);
	if(len<=0) return luaL_argerror(L, 1, "length must be positive");
	int elem, type=TYPE_UNSIGNED|TYPE_CHAR;
	if(lua_isnumber(L, 2)) elem=luaL_checkinteger(L, 2);
	else {
		type=typeFromArg(L, NULL, 2);
		elem=typeSize(type);
	}
	if(elem<=0) return luaL_argerror(L, 2, "element size must be positive");
	
	// allocate and create
	buffer_t* buf=(buffer_t*) lua_newuserdata(L, sizeof(buffer_t));
	if(!buffer_callocData(buf, len, elem, 0)) {
		buf->alloc=0;
		return luaL_error(L, "failed to allocate buffer");
	}
	luaL_setmetatable(L, BUFFER_CLASS);
	
	// set type
	lua_pushinteger(L, type);
	lua_pushcfunction(L, api_bufferSetType);
	lua_insert(L, -3);
	lua_copy(L, -2, 1);
	lua_call(L, 2, 0);
	lua_settop(L, 1);
	
	return 1;
}

//END buffer creator

//BEGIN size getter/setter
/**
 * @ref buf.size
 * @ref buf:getsize()
 * @ref buffer.getsize(buf)
 * @arg1: buffer, buf
 * @ret1: int, size
 */
int api_bufferGetSize(lua_State *L) {
	buffer_t *buf=bufferFromArg(L);
	lua_pushinteger(L, buffer_getSize(buf));
	return 1;
}

/**
 * @ref buf.size=val
 * @ref buf:setsize(val)
 * @ref buffer.setsize(buf, val)
 * @arg1: buffer, buf
 * @arg2: int, val
 */
int api_bufferSetSize(lua_State *L) {
	buffer_t *buf=bufferFromArg(L);
	int size=luaL_checkinteger(L, 2);
	if(size<=0) return luaL_argerror(L, 2, "the size must be positive");
	if(!buffer_resize(buf, size)) return luaL_error(L, "error while resizing buffer");
	return 0;
}
//END size getter/setter

//BEGIN length getter/setter
/**
 * @ref #buf
 * @ref buf.length
 * @ref buf:getlength([type])
 * @ref buffer.getlength(buf, [type])
 * @arg1: buffer, buf
 * @arg2: string|int?, type
 * @ret1: int, length
 */
int api_bufferGetLength(lua_State *L) {
	buffer_t *buf=bufferFromArg(L);
	int type=typeFromArg(L, buf, 2);
	int len=getLength(buf, type);
	if(len==-1) return luaL_error(L, "unable to get length");
	lua_pushinteger(L, len);
	return 1;
}

/**
 * @ref buf.length=val
 * @ref buf:setlength(val, [type])
 * @ref buffer.setlength(buf, val, [type])
 * @arg1: buffer, buf
 * @arg2: int, val
 * @arg3: string|int?, type
 */
int api_bufferSetLength(lua_State *L) {
	buffer_t *buf=bufferFromArg(L);
	int len=luaL_checkinteger(L, 2);
	int type=typeFromArg(L, buf, 3);
	int size=typeSize(type);
	if(size==-1) return luaL_error(L, "unable to get size of type for resizing");
	if(len<=0) return luaL_argerror(L, 2, "the length must be positive");
	if(!buffer_resize(buf, size*len)) return luaL_error(L, "error while resizing buffer");
	return 0;
}
//END length getter/setter

//BEGIN type getter/setter
/**
 * @ref buf.type
 * @ref buf:gettype()
 * @ref buffer.gettype(buf)
 * @arg1: buffer, buf
 * @ret1: int, type
 */
int api_bufferGetType(lua_State *L) {
	buffer_t *buf=bufferFromArg(L);
	lua_pushinteger(L, buffer_getUser(buf)&0x1f);
	return 1;
}

/**
 * @ref buf.type=val
 * @ref buf:settype(val)
 * @ref buffer.settype(buf, val)
 * @arg1: buffer, buf
 * @arg2: string|int, type
 */
int api_bufferSetType(lua_State *L) {
	buffer_t *buf=bufferFromArg(L);
	int type=typeFromArg(L, buf, 2);
	int user=buffer_getUser(buf)&~0x1f;
	buffer_setUser(buf, user|type);
	return 0;
}
//END type getter/setter

//BEGIN value getter/setter
#define get(type, sgn, luatype) case typeid(type): \
	ok=1; \
	lua_push##luatype(L, buffer_get(buf, idx, typename(sgn, type))); \
	break;
/**
 * @ref buf[idx]
 * @ref buf:get(idx, [type])
 * @ref buffer.get(buf, idx, [type])
 * @arg1: buffer, buf
 * @arg2: int, idx
 * @arg2: string|int?, type
 * @ret1: number, value
 */
int api_bufferGet(lua_State *L) {
	buffer_t *buf=bufferFromArg(L);
	int idx=luaL_checkinteger(L, 2)-1;
	int type=typeFromArg(L, buf, 3);
	
	if(idx<0||idx>=getLength(buf, type)) return 0;
	
	int ok=0;
	if(type&TYPE_SIGNED) {
		switch(type&0xf) {
			get(CHAR, S, integer)
#ifdef TYPE_SHORT
			get(SHORT, S, integer)
#endif
#ifdef TYPE_INT
			get(INT, S, integer)
#endif
#ifdef TYPE_LONG
			get(LONG, S, integer)
#endif
#ifdef TYPE_LONGLONG
			get(LONGLONG, S, integer)
#endif
			get(FLOAT, S, number)
#ifdef TYPE_DOUBLE
			get(DOUBLE, S, number)
#endif
			get(8, S, integer)
#ifdef TYPE_16
			get(16, S, integer)
#endif
#ifdef TYPE_32
			get(32, S, integer)
#endif
#ifdef TYPE_64
			get(64, S, integer)
#endif
		}
	} else {
		switch(type&0xf) {
			get(CHAR, U, integer)
#ifdef TYPE_SHORT
			get(SHORT, U, integer)
#endif
#ifdef TYPE_INT
			get(INT, U, integer)
#endif
#ifdef TYPE_LONG
			get(LONG, U, integer)
#endif
#ifdef TYPE_LONGLONG
			get(LONGLONG, U, integer)
#endif
			get(FLOAT, U, number)
#ifdef TYPE_DOUBLE
			get(DOUBLE, U, number)
#endif
			get(8, U, integer)
#ifdef TYPE_16
			get(16, U, integer)
#endif
#ifdef TYPE_32
			get(32, U, integer)
#endif
#ifdef TYPE_64
			get(64, U, integer)
#endif
		}
	}
	if(ok) return 1;
	else return luaL_error(L, "unable to get value");
}
#undef get

#define set(type, luatype) case typeid(type): \
	ok=1; \
	buffer_set(buf, idx, (typename(U, type)) luaL_check##luatype(L, 3), typename(U, type)); \
	break; \
/**
 * @ref buf[idx]=val
 * @ref buf:set(idx, val, [type])
 * @ref buffer.set(buf, idx, val, [type])
 * @arg1: buffer, buf
 * @arg2: int, idx
 * @arg3: number, val
 * @arg4: string|int?, type
 */
int api_bufferSet(lua_State *L) {
	buffer_t *buf=bufferFromArg(L);
	int idx=luaL_checkinteger(L, 2)-1;
	int type=typeFromArg(L, buf, 4);
	
	if(idx<0||idx>=getLength(buf, type)) return 0;
	
	int ok=0;
	switch(type&0xf) {
		set(CHAR, integer)
#ifdef TYPE_SHORT
		set(SHORT, integer)
#endif
#ifdef TYPE_INT
		set(INT, integer)
#endif
#ifdef TYPE_LONG
		set(LONG, integer)
#endif
#ifdef TYPE_LONGLONG
		set(LONGLONG, integer)
#endif
		set(FLOAT, number)
#ifdef TYPE_DOUBLE
		set(DOUBLE, number)
#endif
		set(8, integer)
#ifdef TYPE_16
		set(16, integer)
#endif
#ifdef TYPE_32
		set(32, integer)
#endif
#ifdef TYPE_64
		set(64, integer)
#endif
	}
	
	if(!ok) return luaL_error(L, "unable to set value");
	else return 0;
}
#undef set
//END value getter/setter

//BEGIN metamethods
/**
 * @name __index
 * @ref buf[key]
 * @arg1: buffer, buf
 * @arg2: any, key
 * @ret: any
 */
int meta_index(lua_State *L) {
	if(lua_isnumber(L, 2)) return api_bufferGet(L);
	else {
		// try getting from the first upvalue (the getter list)
		lua_settop(L, 3);
		lua_copy(L, 2, 3);
		if(lua_gettable(L, lua_upvalueindex(1))) {
			// call the function
			lua_remove(L, 2);
			lua_insert(L, 1);
			lua_call(L, 1, LUA_MULTRET);
			return lua_gettop(L);
		}
		lua_pop(L, 1);
		
		// try getting from the second upvalue (the library)
		if(lua_gettable(L, lua_upvalueindex(2))) return 1;
		return 0;
	}
}

/**
 * @name __newindex
 * @ref buf[key]=val
 * @arg1: buffer, buf
 * @arg2: any, key
 * @arg3: any, val
 */
int meta_newindex(lua_State *L) {
	if(lua_isnumber(L, 2)) return api_bufferSet(L);
	else {
		// try getting from the first upvalue (the setter list)
		lua_settop(L, 4);
		lua_copy(L, 2, 4);
		if(lua_gettable(L, lua_upvalueindex(1))) {
			// call the function
			lua_remove(L, 2);
			lua_insert(L, 1);
			lua_call(L, 2, LUA_MULTRET);
			return lua_gettop(L);
		}
		return 0;
	}
}

/**
 * @name __len
 * @ref #buf
 * @arg1: buffer, buf
 * @ret1: int, length
 */
int meta_len(lua_State *L) {
	lua_settop(L, 1);
	return api_bufferGetLength(L);
}

/**
 * @name __ipairs
 * @ref ipairs(buf)
 * @arg1: any, buf
 * @ret1: function, iter
 * @ret2: any, buf
 * @ret3: int, 0
 */
int meta_ipairs(lua_State *L) {
	// keep only the buffer on the stack
	lua_settop(L, 1);
	
	// insert the iterator at the bottom of the stack
	lua_pushcfunction(L, other_iter);
	lua_insert(L, 1);
	
	// push 0
	lua_pushinteger(L, 0);
	
	return 3;
}

/**
 * @name __gc
 * @arg1: buffer, buf
 */
int meta_gc(lua_State *L) {
	buffer_t *buf=bufferFromArg(L);
	buffer_destroyData(buf);
	return 0;
}
//END metamethods

//BEGIN other Lua functions
/**
 * @name iter
 * @ref i, v=iter(tab, idx)
 * iterates through a table or table-like object
 * @arg1: table, tab
 * @arg2: number, idx
 * @ret1: number, i
 * @ret2: any, v
 */
int other_iter(lua_State *L) {
	lua_Integer idx=luaL_checkinteger(L, 2)+1;
	lua_pushinteger(L, idx);
	if(lua_gettable(L, 1)) {
		lua_pushinteger(L, idx);
		lua_insert(L, -2);
		return 2;
	} else return 1;
}
//END other Lua functions

//BEGIN setup functions
/**
 * @name luaopen_buffer2
 * this is the entrypoint which is called when the module is loaded
 * creates the library and the metatable, and returns the library only
 */
int luaopen_buffer2(lua_State *L) {
	// clear the stack
	lua_settop(L, 0);
	
	// create the library
	setupLib(L);
	
	// create the metatable
	setupMeta(L);
	
	// return the library
	return 1;
}

/**
 * @name setupLib
 * creates the buffer2 library and leaves it on the stack
 */
int setupLib(lua_State *L) {
	// create table with all the functions
	static luaL_Reg lib[]={
		{"new", api_bufferNew},
		{"calloc", api_bufferCalloc},
		{"getsize", api_bufferGetSize},
		{"setsize", api_bufferSetSize},
		{"getlength", api_bufferGetLength},
		{"setlength", api_bufferSetLength},
		{"gettype", api_bufferGetType},
		{"settype", api_bufferSetType},
		{"get", api_bufferGet},
		{"set", api_bufferSet},
		{"iter", other_iter},
		{NULL, NULL}
	};
	luaL_newlib(L, lib);
	
	// create table with all the types
	lua_newtable(L);
	
	lua_pushinteger(L, TYPE_CHAR);
	lua_setfield(L, -2, "char");
#ifdef TYPE_SHORT
	lua_pushinteger(L, TYPE_SHORT);
	lua_setfield(L, -2, "short");
#endif
#ifdef TYPE_INT
	lua_pushinteger(L, TYPE_INT);
	lua_setfield(L, -2, "int");
#endif
#ifdef TYPE_LONG
	lua_pushinteger(L, TYPE_LONG);
	lua_setfield(L, -2, "long");
#endif
#ifdef TYPE_LONGLONG
	lua_pushinteger(L, TYPE_LONGLONG);
	lua_setfield(L, -2, "long long");
#endif
	lua_pushinteger(L, TYPE_FLOAT);
	lua_setfield(L, -2, "float");
#ifdef TYPE_DOUBLE
	lua_pushinteger(L, TYPE_DOUBLE);
	lua_setfield(L, -2, "double");
#endif
	lua_pushinteger(L, TYPE_8);
	lua_setfield(L, -2, "8");
	lua_pushinteger(L, TYPE_8);
	lua_setfield(L, -2, "int8");
#ifdef TYPE_16
	lua_pushinteger(L, TYPE_16);
	lua_setfield(L, -2, "16");
	lua_pushinteger(L, TYPE_16);
	lua_setfield(L, -2, "int16");
#endif
#ifdef TYPE_32
	lua_pushinteger(L, TYPE_32);
	lua_setfield(L, -2, "32");
	lua_pushinteger(L, TYPE_32);
	lua_setfield(L, -2, "int32");
#endif
#ifdef TYPE_64
	lua_pushinteger(L, TYPE_64);
	lua_setfield(L, -2, "64");
	lua_pushinteger(L, TYPE_64);
	lua_setfield(L, -2, "int64");
#endif
	
	lua_setfield(L, 1, "types");
	return 1;
}

/**
 * @name setupMeta
 * creates the metatable for buffers
 */
int setupMeta(lua_State *L) {
	// create metatable
	luaL_newmetatable(L, BUFFER_CLASS);
	
	// simple methods
	lua_pushcfunction(L, meta_len);
	lua_setfield(L, 2, "__len");
	lua_pushcfunction(L, meta_gc);
	lua_setfield(L, 2, "__gc");
	lua_pushcfunction(L, meta_ipairs);
	lua_setfield(L, 2, "__ipairs");
	
	
	// __index
	static luaL_Reg getters[]={
		{"size", api_bufferGetSize},
		{"length", api_bufferGetLength},
		{"type", api_bufferGetType},
		{NULL, NULL}
	};
	luaL_newlib(L, getters);
	lua_settop(L, lua_gettop(L)+1);
	lua_copy(L, 1, -1);
	lua_pushcclosure(L, meta_index, 2);
	lua_setfield(L, 2, "__index");
	
	// __newindex
	static luaL_Reg setters[]={
		{"size", api_bufferSetSize},
		{"length", api_bufferSetLength},
		{"type", api_bufferSetType},
		{NULL, NULL}
	};
	luaL_newlib(L, setters);
	lua_pushcclosure(L, meta_newindex, 1);
	lua_setfield(L, 2, "__newindex");
	
	lua_pop(L, 1);
	return 0;
}
//END setup functions
