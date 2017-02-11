#include <stdint.h>
#include <stdbool.h>
#include "helpers.h"
#include </usr/include/libsyscall.h>

#define SIGABRT 6

void abort(void) {
    // TODO: do this "correctly"
    lsc_kill((int) lsc_getpid(), SIGABRT);
    lsc_exit(255);
    __builtin_trap();
}

intptr_t time(intptr_t *tout) {
    return lsc_time((uintptr_t *) tout);
}

void lua_writestring_default(const char *s, size_t len) {
    lsc_write(1, s, len);
}

void lua_writeerror_default(const char *s, size_t len) {
    lsc_write(2, s, len);
}

int deepcall(void (*target)(void *), void *param, deepptr *saveaddr) {
    return (int) lsc_deep_call(target, param, saveaddr);
}

void deepret(deepptr loadaddr) __attribute__((noreturn));
void deepret(deepptr loadaddr) {
    lsc_deep_return(loadaddr);
}
