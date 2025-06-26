#ifndef _EXECUTION_UTILS
#define _EXECUTION_UTILS

#define BUILTIN_FOUND 1
#define BUILTIN_NOT_FOUND 0

#include "siparse.h"
// int try_exec_builtin(const char *builtin, char *argv[]);

void schedule_pipelineseq_for_exec(pipelineseq *p_seq);

#endif
