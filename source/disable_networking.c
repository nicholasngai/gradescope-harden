#include "disable_networking.h"
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <seccomp.h>
#include <unistd.h>

static int recvfd(int comm_sock, int *fd) {
    int ret;

    /* Receive FD using SCM_RIGHTS message. */
    char iobuf;
    struct iovec io = {
        .iov_base = &iobuf,
        .iov_len = sizeof(iobuf),
    };
    union {
        char buf[CMSG_SPACE(sizeof(*fd))];
        struct cmsghdr align;
    } u;
    struct msghdr msg = {
        .msg_name = NULL,
        .msg_namelen = 0,
        .msg_iov = &io,
        .msg_iovlen = 1,
        .msg_control = u.buf,
        .msg_controllen = sizeof(u.buf),
    };
    if (recvmsg(comm_sock, &msg, 0)) {
        perror("recvmsg");
        ret = -1;
        goto exit;
    }
    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    if (!cmsg || cmsg->cmsg_level != SOL_SOCKET || cmsg->cmsg_type != SCM_RIGHTS
            || cmsg->cmsg_len != CMSG_LEN(sizeof(*fd))) {
        fprintf(stderr, "Invalid SCM_RIGHTS message from sender\n");
        ret = -1;
        goto exit;
    }
    *fd = *((int *) CMSG_DATA(cmsg));

    ret = 0;

exit:
    return ret;
}

static int sendfd(int comm_sock, int fd) {
    int ret;

    /* Send FD using SCM_RIGHTS message. */
    char iobuf = '\0';
    struct iovec io = {
        .iov_base = &iobuf,
        .iov_len = sizeof(iobuf),
    };
    union {
        char buf[CMSG_SPACE(sizeof(fd))];
        struct cmsghdr align;
    } u;
    struct msghdr msg = {
        .msg_name = NULL,
        .msg_namelen = 0,
        .msg_iov = &io,
        .msg_iovlen = 1,
        .msg_control = u.buf,
        .msg_controllen = sizeof(u.buf),
    };
    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(fd));
    *((int *) CMSG_DATA(cmsg)) = fd;
    if (sendmsg(comm_sock, &msg, 0) == -1) {
        perror("sendmsg");
        ret = -1;
        goto exit;
    }

    ret = 0;

exit:
    return ret;
}

int disable_networking(pid_t child_pid, int comm_sock) {
    int ret;

    if (child_pid == 0) {
        /* Child. */

        /* Initialize libseccomp filter. */
        scmp_filter_ctx filter = seccomp_init(SCMP_ACT_ALLOW);
        if (!filter) {
            fprintf(stderr, "Error initializing seccomp filter\n");
            abort();
        }

        /* Filter out calls to socket with family AF_INET or AF_INET6. */
        ret =
            seccomp_rule_add(filter, SCMP_ACT_NOTIFY, SCMP_SYS(socket), 1,
                    SCMP_CMP(0, SCMP_CMP_EQ, AF_INET));
        if (ret) {
            errno = -ret;
            perror("seccomp_rule_add");
            abort();
        }
        ret =
            seccomp_rule_add(filter, SCMP_ACT_NOTIFY, SCMP_SYS(socket), 1,
                    SCMP_CMP(0, SCMP_CMP_EQ, AF_INET6));
        if (ret) {
            errno = -ret;
            perror("seccomp_rule_add");
            abort();
        }

        /* Load filter. */
        ret = seccomp_load(filter);
        if (ret) {
            errno = -ret;
            perror("seccomp_load");
            abort();
        }

        /* Get notify FD. */
        int notify_fd = seccomp_notify_fd(filter);
        if (notify_fd < 0) {
            errno = -notify_fd;
            perror("seccomp_notify_fd");
            abort();
        }

        /* Send the notify FD over the socket. */
        ret = sendfd(comm_sock, notify_fd);
        if (ret) {
            abort();
        }

        /* Close the notify FD. */
        close(notify_fd);

        ret = 0;

        if (child_pid == 0) {
            seccomp_release(filter);
        }
    } else {
        /* Parent. */

        /* Receive the notify FD from the socket. */
        int notify_fd;
        ret = recvfd(comm_sock, &notify_fd);
        if (ret) {
            goto exit;
        }
    }

exit:
    return ret;
}
