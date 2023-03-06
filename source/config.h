#ifndef GRADESCOPE_HARDEN_CONFIG_H
#define GRADESCOPE_HARDEN_CONFIG_H

#include <stdbool.h>

struct config {
    bool disable_networking;
};

int config_read(const char *config_path, struct config *config);
void config_free(struct config *config);

#endif
