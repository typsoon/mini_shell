#include <stdlib.h>
#include <sys/wait.h>
#include <config.h>

#include <stdio.h>
#include <errno.h>

#include "notes_holder.h"

void child_handler(int sig_no)
{
    int stat_loc;

    pid_t child_pid;
    while ((child_pid = waitpid(-1, &stat_loc, WNOHANG)) != -1 && child_pid != 0)
    {
        bool present_in_fg = is_in_fg(child_pid);

        // if foreground child
        if (present_in_fg)
        {
            remove_fg_process(child_pid);
        }
        else
        {
            if (WIFSIGNALED(stat_loc))
            {
                make_note(child_pid, true, WTERMSIG(stat_loc));
            }
            else if (WIFEXITED(stat_loc))
            {
                make_note(child_pid, false, WEXITSTATUS(stat_loc));
            }
            else
            {
                exit(EXEC_FAILURE);
            }
        }
    }
}

// void sigint_handler(int sig_no)
// {
//     pid_t pid;
//     while ((pid = waitpid(-1, NULL, 0)) != -1)
//     {
//     }
// }
