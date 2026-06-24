#ifdef TARGET_WEB

#include "os/os.h"

#include <emscripten.h>
#include <stdlib.h>

NORETURN void os_abort(S32 exit_code) {
	exit(exit_code);
}

U64 os_now_microseconds(void) {
	return (U64)(emscripten_get_now() * 1000.0);
}

U64 os_now_milliseconds(void) {
	return (U64)emscripten_get_now();
}

void* os_memory_reserve(U64 size) {
	return malloc(size);
}

void os_memory_unreserve(void* memory, U64 size) {
	NoOp(size);
	free(memory);
}

void os_memory_commit(void* memory, U64 size) {
	NoOp(memory);
	NoOp(size);
}

void os_memory_uncommit(void* memory, U64 size) {
	NoOp(memory);
	NoOp(size);
}

#endif // TARGET_WEB
