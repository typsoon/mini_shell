#include <stdlib.h>
#include <stdio.h>
#include <siparse.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "errors_names.h"
#include "execution_utils.h"
#include "parsing_utils.c"
#include "builtins.h"
#include "builtins_impl.h"
#include "config.h"
#include "notes_holder.h"

void handle_error(const char *fname)
{
    switch (errno)
    {
    case ENOENT:
        fprintf(stderr, "%s%s", fname, NO_SUCH_FILE_OR_DIRECTORY);
        break;

    case EACCES:
        fprintf(stderr, "%s%s", fname, PERMISSION_DENIED);
        break;

    default:
        fprintf(stderr, "%s%s", fname, EXEC_ERROR);
        break;
    }
}

/// returns true if the builtin was found
int try_exec_builtin(const char *builtin, char *argv[])
{
    for (size_t i = 0; builtins_table[i].name != NULL && builtins_table[i].fun != NULL; i++)
    {
        if (strcmp(builtins_table[i].name, builtin) == 0)
        {
            if (builtins_table[i].fun(argv) == BUILTIN_ERROR)
            {
                fprintf(stderr, BUILTIN_ERROR_FORMAT_STR, builtin);
                // handle_error(builtin);
            }

            fflush(stdout);
            return BUILTIN_FOUND;
        }
    }
    return BUILTIN_NOT_FOUND;
}

void handle_redirs(redirseq *redirs)
{
    if (redirs == NULL)
    {
        return;
    }

    redirseq *iter = redirs;
    do
    {
        int oflags = 0;
        if (IS_RIN(iter->r->flags) || IS_ROUT(iter->r->flags) || IS_RAPPEND(iter->r->flags))
        {
            int open_result = 0;

            if (IS_RIN(iter->r->flags))
            {
                oflags |= O_RDONLY;
                close(STDIN_FILENO);
                open_result = open(iter->r->filename, oflags);
            }
            else
            {
                if (!IS_RAPPEND(iter->r->flags))
                {
                    oflags |= O_TRUNC;
                }
                else
                {
                    oflags |= O_APPEND;
                }

                oflags |= O_CREAT;
                oflags |= O_WRONLY;
                close(STDOUT_FILENO);
                open_result = open(iter->r->filename, oflags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            }

            if (open_result == -1)
            {
                handle_error(iter->r->filename);
                exit(EXEC_FAILURE);
            }
        }

        iter = iter->next;
    } while (iter != redirs);
}

#define WRITING 0
#define READING 1
#define UNINITIALIZED_FD -1

void execute_pipeline(pipeline *p)
{
    if (p == NULL || (p->commands == NULL) || p->commands->com == NULL)
    {
        return;
    }

    command *com;
    commandseq *com_seq = p->commands;
    bool is_trivial = true;

    if (com_seq->next != com_seq)
    {
        is_trivial = false;
    }

    int piped_flides[2] = {UNINITIALIZED_FD, UNINITIALIZED_FD};
    int prev_pipe[2];
    // int duplicated_stdout = dup(STDOUT_FILENO);

    sigset_t sigchld_sigset;
    sigemptyset(&sigchld_sigset);
    sigaddset(&sigchld_sigset, SIGCHLD);

    sigset_t restore_mask;
    sigprocmask(0, NULL, &restore_mask);
    sigdelset(&restore_mask, SIGINT);

    sigprocmask(SIG_BLOCK, &sigchld_sigset, NULL);
    do
    {
        prev_pipe[0] = piped_flides[0];
        prev_pipe[1] = piped_flides[1];

        if (com_seq->next != p->commands)
        {
            pipe(piped_flides);
        }

        com = com_seq->com;
        int args_len = get_length(com->args);
        char *parsed_args[args_len + 1];
        get_args(com->args, parsed_args, args_len);

        if (is_trivial && try_exec_builtin(com->args->arg, parsed_args))
        {
            // exit(EXIT_SUCCESS);
            return;
        }

        int fork_res = fork();
        // switch (fork_res)
        switch (fork_res)
        {
        case -1:
            exit(EXEC_FAILURE);
            break;

        case 0:
        {
            if (prev_pipe[0] != UNINITIALIZED_FD)
            {
                dup2(prev_pipe[0], STDIN_FILENO);
                close(prev_pipe[0]);
                close(prev_pipe[1]);
            }

            if (com_seq->next != p->commands)
            {
                dup2(piped_flides[1], STDOUT_FILENO);
                close(piped_flides[0]);
                close(piped_flides[1]);
            }

            handle_redirs(com->redirs);

            if (p->flags & INBACKGROUND)
            {
                setsid();
            }
            sigprocmask(SIG_SETMASK, &restore_mask, NULL);
            execvp(com->args->arg, parsed_args);

            handle_error(com->args->arg);
            exit(EXEC_FAILURE);

            break;
        }

        default:
            if ((p->flags & INBACKGROUND) == 0)
            {
                add_fg_process(fork_res);
            }

            if (prev_pipe[0] != UNINITIALIZED_FD)
            {
                close(prev_pipe[0]);
                close(prev_pipe[1]);
            }

            break;
        }

        com_seq = com_seq->next;
    } while (com_seq != p->commands);

    sigset_t temp_sigset;
    sigprocmask(0, NULL, &temp_sigset);
    sigdelset(&temp_sigset, SIGCHLD);

    while (get_fg_count() > 0)
    {
        // SIGCHLD not blocked
        sigsuspend(&temp_sigset);
    };
    sigprocmask(SIG_UNBLOCK, &sigchld_sigset, NULL);
}

void schedule_pipelineseq_for_exec(pipelineseq *p_seq)
{
    pipelineseq *iter = p_seq;
    if (iter == NULL)
    {
        return;
    }

    do
    {
        execute_pipeline(iter->pipeline);
        iter = iter->next;
    } while (iter != p_seq);
}