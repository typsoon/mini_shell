#include "notes_holder.h"
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <config.h>
#include <stdlib.h>

struct notes_holder
{
    size_t size;
    struct note notes[MAX_CHILD_COUNT];
};

struct notes_holder n_holder;

void make_note(pid_t pid, bool killed_by_signal, int num)
{
    if (n_holder.size == MAX_CHILD_COUNT)
    {
        exit(EXEC_FAILURE);
    }

    note *inserted_note = &n_holder.notes[n_holder.size];
    inserted_note->pid = pid;
    inserted_note->killed_by_signal = killed_by_signal;
    inserted_note->num = num;

    n_holder.size++;
}

void consume_notes(void (*consumer)(note *))
{
    for (size_t i = 0; i < n_holder.size; i++)
    {
        consumer(n_holder.notes + i);
    }
    n_holder.size = 0;
}

pid_t fg_processes[MAX_CHILD_COUNT];
size_t fg_count = 0;

const size_t get_fg_count()
{
    return fg_count;
}

void add_fg_process(pid_t pid)
{
    if (fg_count == MAX_CHILD_COUNT)
    {
        exit(EXEC_FAILURE);
    }

    fg_processes[fg_count++] = pid;
}

#define NOT_IN_FG -1
int get_ind_in_fg(pid_t pid)
{
    size_t i = 0;
    // while (i < n_holder.size && fg_processes[i] != pid)
    while (i < fg_count && fg_processes[i] != pid)
    {
        i++;
    }

    if (i == fg_count)
    {
        return NOT_IN_FG;
    }
    return i;
}

bool remove_fg_process(pid_t pid)
{
    size_t i = get_ind_in_fg(pid);
    if (i == NOT_IN_FG)
    {
        return false;
    }

    fg_count--;
    while (i < fg_count)
    {
        fg_processes[i] = fg_processes[i + 1];
        i++;
    }

    return true;
}

bool is_in_fg(pid_t pid)
{
    if (get_ind_in_fg(pid) != NOT_IN_FG)
    {
        return true;
    }
    return false;
}