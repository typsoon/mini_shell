#include <string.h>
#include <siparse.h>

int get_length(argseq *seq)
{
    if (seq == NULL)
    {
        return 0;
    }

    int len = 0;
    argseq *tmp = seq;
    do
    {
        len++;
        tmp = tmp->next;
    } while (tmp != seq);

    return len;
}

void get_args(argseq *seq, char **answer, int len)
{
    argseq *tmp = seq;

    for (int index = 0; index < len; index++)
    {
        answer[index] = tmp->arg;
        tmp = tmp->next;
    }
    answer[len] = NULL;
}