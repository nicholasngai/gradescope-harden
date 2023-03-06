#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include "config.h"
#include "disable_networking.h"

/* The launch function, which should be called by both the parent and the child
 * with the same arguments other than child_pid. */
static int launch_func(pid_t child_pid, const struct config *config) {
    int ret;

    if (config->disable_networking) {
        ret = disable_networking(child_pid);
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

    pid_t child = fork();
    if (child == -1) {
        perror("fork");
        ret = -1;
        goto exit_free_config;
    } else if (child == 0) {
        /* Child. */

        /* Call launch_func as child. */
        if (launch_func(0, &config)) {
            abort();
        }

        /* Execute the original /autograder/run_autograder that was moved to
         * /autograder/run_autograder.orig by our setup.sh */
        char *argv[] = {"/autograder/run_autograder.orig", NULL};
        if (execv(argv[0], argv)) {
            abort();
        }
    } else {
        /* Parent. */

        /* Call launch_func as parent. */
        ret = launch_func(child, &config);
        if (ret) {
            goto exit_kill_child;
        }

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
exit_free_config:
    config_free(&config);
exit:
    return ret;
}
