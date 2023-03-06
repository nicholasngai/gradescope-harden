#include "disable_networking.h"
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <seccomp.h>
#include "defs.h"

int disable_networking(pid_t child_pid UNUSED) {
    int ret;

    /* Initialize libseccomp filter. */
    scmp_filter_ctx filter = seccomp_init(SCMP_ACT_ALLOW);
    if (!filter) {
        fprintf(stderr, "Error initializing seccomp filter\n");
        ret = -1;
        goto exit;
    }

    /* Filter out calls to socket with family AF_INET or AF_INET6. */
    ret =
        seccomp_rule_add(filter, SCMP_ACT_ERRNO(EACCES), SCMP_SYS(socket), 1,
                SCMP_CMP(0, SCMP_CMP_EQ, AF_INET));
    if (ret) {
        errno = -ret;
        perror("seccomp_rule_add");
        ret = -1;
        goto exit_free_filter;
    }
    ret =
        seccomp_rule_add(filter, SCMP_ACT_ERRNO(EACCES), SCMP_SYS(socket), 1,
                SCMP_CMP(0, SCMP_CMP_EQ, AF_INET6));
    if (ret) {
        errno = -ret;
        perror("seccomp_rule_add");
        ret = -1;
        goto exit_free_filter;
    }

    /* Load filter. */
    ret = seccomp_load(filter);
    if (ret) {
        errno = -ret;
        perror("seccomp_load");
        ret = -1;
        goto exit_free_filter;
    }

    ret = 0;

exit_free_filter:
    seccomp_release(filter);
exit:
    return ret;
}
