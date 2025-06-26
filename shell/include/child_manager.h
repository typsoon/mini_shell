#ifndef CHILD_MANAGER_H
#define CHILD_MANAGER_H
#include <sys/types.h>

void child_handler(int sig_no);

// void sigint_handler(int sig_no);

const size_t alive_in_fg_count();

#endif