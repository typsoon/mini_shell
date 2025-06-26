#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/types.h>
#include <limits.h>
#include "builtins_impl.h"
#include "config.h"
#include "builtins.h"
#define CURRENT_DIR_SYMBOL "."
#define SILENT_CHAR_PREFIX '.'
#define HOME_ENV_VAR "HOME"
#define EXEC_SUCCESS EXIT_SUCCESS

bool more_than_one_arg(char *args[])
{
    if (args[1] != NULL)
    {
        return true;
    }
    return false;
}

bool more_than_two_args(char *args[])
{
    if (more_than_one_arg(args) && args[2] != NULL)
    {
        return true;
    }
    return false;
}

int lexit(char *args[])
{
    if (more_than_one_arg(args))
    {
        return BUILTIN_ERROR;
    }
    exit(EXEC_SUCCESS);
    return EXEC_SUCCESS;
}

int lls(char *args[])
{
    if (more_than_one_arg(args))
    {
        return BUILTIN_ERROR;
    }

    DIR *cwd = opendir(CURRENT_DIR_SYMBOL);
    // DIR *cwd = opendir(".");
    if (cwd == NULL)
    {
        return BUILTIN_ERROR;
    }

    struct dirent *dir_contents;
    while ((dir_contents = readdir(cwd)) != NULL)
    {
        if (dir_contents->d_name[0] != SILENT_CHAR_PREFIX)
        {
            fprintf(stdout, "%s\n", dir_contents->d_name);
        }
    }

    // errno = 0;
    // if (errno != 0 || closedir(cwd) != EXEC_SUCCESS)
    if (closedir(cwd) != EXEC_SUCCESS)
    {
        return BUILTIN_ERROR;
    }

    return EXEC_SUCCESS;
}

int lcd(char *args[])
{
    char *temp = args[1];
    if (temp == NULL)
    {
        temp = getenv(HOME_ENV_VAR);
    }
    else if (more_than_two_args(args))
    {
        return BUILTIN_ERROR;
    }

    if (chdir(temp) != EXEC_SUCCESS)
    {
        return BUILTIN_ERROR;
    }
    return EXEC_SUCCESS;
}

#define PARSING_ERROR 1
#define PARSING_SUCCESS 0

int parse_str(int *result, char *input)
{
    char *endptr;
    errno = 0;
    long result_long = strtol(input, &endptr, 10);

    if (result_long > INT_MAX || result_long < INT_MIN)
    {
        errno = ERANGE;
        return BUILTIN_ERROR;
        BUILTIN_ERROR;
    }

    *result = (int)result_long;

    if (*endptr != '\0' || errno == EINVAL || errno == ERANGE)
    {
        return BUILTIN_ERROR;
    }
    return EXEC_SUCCESS;
}

int lkill(char *args[])
{
    if (!more_than_one_arg(args))
    {
        return BUILTIN_ERROR;
    }

    int signal_number = SIGTERM;
    pid_t pid;

    if (more_than_two_args(args))
    {
        if (args[3] != NULL ||
            args[1][0] != '-' ||
            parse_str(&signal_number, args[1] + 1) == PARSING_ERROR ||
            parse_str(&pid, args[2]) == PARSING_ERROR)
        {
            return BUILTIN_ERROR;
        }
    }
    else if (parse_str(&pid, args[1]) == PARSING_ERROR)
    {
        return BUILTIN_ERROR;
    }

    if (kill(pid, signal_number) != 0)
    {
        return BUILTIN_ERROR;
    }

    return EXEC_SUCCESS;
}
