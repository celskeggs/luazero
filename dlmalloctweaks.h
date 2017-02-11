#ifndef LUAZERO_DLMALLOCTWEAKS_H
#define LUAZERO_DLMALLOCTWEAKS_H

#define LACKS_SYS_TYPES_H
#define NO_MALLOC_STATS 1
#define LACKS_ERRNO_H
#define LACKS_TIME_H
#define LACKS_STDLIB_H
#define LACKS_STRING_H
#define LACKS_SYS_MMAN_H
#define LACKS_FCNTL_H
#define LACKS_UNISTD_H
#define LACKS_SYS_PARAM_H

#include <stddef.h>
#include </usr/include/libsyscall.h>
#include <stdint.h>
#include "helpers.h"

#define MAP_PRIVATE 0x02
#define MAP_ANONYMOUS 0x20
#define PROT_READ 0x1
#define PROT_WRITE 0x2

#define ENOMEM 12
#define EINVAL 22

static intptr_t errno = 0;

static inline void *mmap(void *addr, size_t len, int prot, int flags, int fd, ptrdiff_t off) {
    void *ret = lsc_mmap(addr, len, (unsigned long) prot, (unsigned long) flags, (unsigned long) fd,
                         (unsigned long) off);
    if ((intptr_t) ret < 0) {
        errno = -(intptr_t) ret;
        return (void *) -1;
    }
    return ret;
}

static inline void *mremap(void *addr, size_t old_len, size_t new_len, int flags) {
    void *ret = lsc_mremap(addr, old_len, new_len, (unsigned long) flags, NULL);
    if ((intptr_t) ret < 0) {
        errno = -(intptr_t) ret;
        return (void *) -1;
    }
    return ret;
}

static inline int munmap(void *addr, size_t length) {
    intptr_t ret = lsc_munmap(addr, length);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    return 0;
}

static void *cbrk = NULL;

static inline int brk(void *addr) {
    void *newb = cbrk = (void*) lsc_brk(addr);
    if (newb < addr) {
        errno = ENOMEM;
        return -1;
    }
    return 0;
}

static inline void *sbrk(intptr_t increment) {
    void *old = cbrk;
    if (increment > 0) {
        if ((uintptr_t) old + (uintptr_t) increment < (uintptr_t) old) {
            errno = ENOMEM;
            return (void *) -1;
        }
    } else {
        if ((uintptr_t) -increment > (uintptr_t) old) {
            errno = ENOMEM;
            return (void *) -1;
        }
    }
    if (brk(old + increment) < 0) {
        return (void *) -1;
    }
    return old;
}

#endif //LUAZERO_DLMALLOCTWEAKS_H
