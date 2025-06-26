#ifndef NOTES_HOLDER_H
#define NOTES_HOLDER_H
#include <sys/types.h>
#include <stdbool.h>

#define MAX_CHILD_COUNT 2048

struct note
{
    pid_t pid;
    bool killed_by_signal;
    int num;
};

typedef struct note note;

void make_note(pid_t pid, bool killed_by_signal, int num);

void consume_notes(void (*consumer)(note *));

const size_t get_fg_count();

void add_fg_process(pid_t pid);

// returns true if the deletion did happen
bool remove_fg_process(pid_t pid);

bool is_in_fg(pid_t pid);

#endif