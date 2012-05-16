#include "../tracy.h"
#include "../ll.h"

#include <stdio.h>
#include <stdlib.h>

/* For __NR_<SYSCALL> */
#include <sys/syscall.h>
#include <unistd.h>

#define set_hook(NAME) \
    if (tracy_set_hook(tracy, #NAME, hook_##NAME)) { \
        printf("Could not hook "#NAME" syscall\n"); \
        return EXIT_FAILURE; \
    }

int hook_write(struct tracy_event *e) {
    long ret;
    int i;

    if (e->child->pre_syscall) {
        printf("PRE-write\n");
        for (i = 0; i < 10; i++) {
            tracy_inject_syscall(e->child, __NR_getpid, NULL, &ret);
            printf("Return code: %ld\n", ret);
        }
    } else {
        printf("POST-write\n");
        for (i = 0; i < 10; i++) {
            tracy_inject_syscall(e->child, __NR_getpid, NULL, &ret);
            printf("Return code: %ld\n", ret);
        }
    }
    return TRACY_HOOK_CONTINUE;
}

int main(int argc, char** argv) {
    struct tracy *tracy;

    /* Tracy options */
    tracy = tracy_init(TRACY_TRACE_CHILDREN | TRACY_VERBOSE);

    if (argc < 2) {
        printf("Usage: ./tracy-inject-simple <program-name>\n");
        return EXIT_FAILURE;
    }

    /* Hooks */
    set_hook(write);

    argv++; argc--;

    /* Start child */
    if (!fork_trace_exec(tracy, argc, argv)) {
        perror("fork_trace_exec");
        return EXIT_FAILURE;
    }

    /* Main event-loop */
    tracy_main(tracy);

    tracy_free(tracy);

    return EXIT_SUCCESS;
}