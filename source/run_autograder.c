#include <stdio.h>
#include <errno.h>
#include "config.h"

int main(void) {
    struct config config;
    int ret;

    /* Read config. */
    ret = config_read("/autograder/source/gradescope-harden.yml", &config);
    if (ret) {
        fprintf(stderr, "Error reading config file!\n");
        goto exit;
    }

    config_free(&config);
exit:
    return ret;
}
