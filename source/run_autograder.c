#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include "config.h"
#include "disable_networking.h"

/* The launch function, which should be called by both the parent and the child
 * with the same arguments other than child_pid and comm_sock. child_pid
 * contains the return value from fork (0 for the child or the PID of the child
 * for the parent), and comm_sock contains a UNIX-domain socket to communicate
 * with the other party. */
static int launch_func(pid_t child_pid, int comm_sock,
        const struct config *config) {
    int ret;

    if (config->disable_networking) {
        ret = disable_networking(child_pid, comm_sock);
        if (ret) {
            goto exit;
        }
    }

    ret = 0;

exit:
    return ret;
}

int main(void) {
    struct config config;
    int ret;

    /* Read config. */
    ret = config_read("/autograder/source/gradescope-harden.yml", &config);
    if (ret) {
        fprintf(stderr, "Error reading config file!\n");
        goto exit;
    }

    /* Spawn a socketpair. */
    int pair[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair) == -1) {
        perror("socketpair");
        ret = -1;
        goto exit_free_config;
    }

    pid_t child = fork();
    if (child == -1) {
        perror("fork");
        ret = -1;
        goto exit_close_sockets;
    } else if (child == 0) {
        /* Child. */

        /* Use pair[1] as socket. */
        close(pair[0]);

        /* Call launch_func as child. */
        if (launch_func(0, pair[1], &config)) {
            abort();
        }

        /* Close our socket. */
        close(pair[1]);

        /* Execute the original /autograder/run_autograder that was moved to
         * /autograder/run_autograder.orig by our setup.sh */
        char *argv[] = {"/autograder/run_autograder.orig", NULL};
        if (execv(argv[0], argv)) {
            abort();
        }
    } else {
        /* Parent. */

        /* Use pair[0] as socket. */
        close(pair[1]);
        pair[1] = -1;

        /* Call launch_func as parent. */
        ret = launch_func(child, pair[0], &config);
        if (ret) {
            goto exit_kill_child;
        }

        /* Close our socket. */
        close(pair[0]);
        pair[0] = -1;

        int child_ret;
        if (waitpid(child, &child_ret, 0) == -1) {
            perror("waitpid");
            goto exit_kill_child;
        }

        if (child_ret) {
            ret = child_ret;
            goto exit_kill_child;
        }
    }

exit_kill_child:
    kill(child, SIGKILL);
    waitpid(child, NULL, 0);
exit_close_sockets:
    if (pair[0] != -1) {
        close(pair[0]);
    }
    if (pair[1] != -1) {
        close(pair[1]);
    }
exit_free_config:
    config_free(&config);
exit:
    return ret;
}
