#ifndef GRADESCOPE_HARDEN_DISBALE_NETWORKING_H
#define GRADESCOPE_HARDEN_DISBALE_NETWORKING_H

#include <sys/types.h>

int disable_networking(pid_t child_pid, int comm_sock);

#endif
