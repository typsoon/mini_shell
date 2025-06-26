#include "child_manager.h"
#include "config.h"
#include "execution_utils.h"
#include "notes_holder.h"
#include "siparse.h"
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define INPUT_LENGTH 4 * (MAX_LINE_LENGTH + 1)
// This has to be at least 2*INPUT_LENGTH
#define BUF_LENGTH 2 * (INPUT_LENGTH)

#define EXITED_FORMAT_STR                                                      \
  "Background process %i terminated. (exited with status %i)\n"
#define KILLED_FORMAT_STR                                                      \
  "Background process %i terminated. (killed by signal %i)\n"

void print_note(note *nt) {
  if (nt->killed_by_signal) {
    printf(KILLED_FORMAT_STR, nt->pid, nt->num);
  } else {
    printf(EXITED_FORMAT_STR, nt->pid, nt->num);
  }
}

void print_prompt() {
  consume_notes(print_note);
  printf("%s", PROMPT_STR);
  fflush(stdout);
}

int main(int argc, char *argv[]) {
  pipelineseq *ln;

  char buf[BUF_LENGTH];

  struct sigaction sa;

  sa.sa_handler = child_handler;
  sigemptyset(&sa.sa_mask);
  // sa.sa_flags = 0;
  sa.sa_flags = SA_RESTART;

  sigset_t sigint_mask;
  sigemptyset(&sigint_mask);
  sigaddset(&sigint_mask, SIGINT);

  sigprocmask(SIG_BLOCK, &sigint_mask, NULL);

  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    exit(EXEC_FAILURE);
  }

  bool reading_bad_command = false;
  int bytes_waiting_for_exec = 0;

  struct stat stdin_stat;
  if (fstat(STDIN_FILENO, &stdin_stat)) {
    exit(EXEC_FAILURE);
  }

  if (S_ISCHR(stdin_stat.st_mode)) {
    print_prompt();
  }

  sigset_t sigchld_sigset;
  sigemptyset(&sigchld_sigset);
  sigaddset(&sigchld_sigset, SIGCHLD);

  int length;
  while ((length =
              read(STDIN_FILENO, buf + bytes_waiting_for_exec, INPUT_LENGTH))) {
    // read error
    if (length == -1) {
      // TODO: think about this
      // fprintf(stderr, "%s\n", SYNTAX_ERROR_STR);
      continue;
    }

    bytes_waiting_for_exec += length;
    buf[bytes_waiting_for_exec] = '\0';

    char *temp;
    char *cursor = buf;
    while ((temp = strchr(cursor, '\n')) != NULL) {
      *temp = '\0';
      // \n is counted here
      int parsed_line_length = temp - cursor + 1;
      if (reading_bad_command) {
        reading_bad_command = false;
      }
      // We don't want to parse empty string
      else if (parsed_line_length > 1) {
        if ((parsed_line_length > MAX_LINE_LENGTH) ||
            (ln = parseline(cursor)) == NULL) {
          fprintf(stderr, "%s\n", SYNTAX_ERROR_STR);
        } else {
          schedule_pipelineseq_for_exec(ln);
        }
      }
      bytes_waiting_for_exec -= parsed_line_length;
      cursor = temp + 1;
    }

    if (bytes_waiting_for_exec > MAX_LINE_LENGTH) {
      if (!reading_bad_command) {
        reading_bad_command = true;
        fprintf(stderr, "%s\n", SYNTAX_ERROR_STR);
      }

      bytes_waiting_for_exec = 0;
    } else {
      // This works if INPUT_LENGTH > 2*MAX_LINE_LENGTH
      strcpy(buf, cursor);
    }
    cursor = buf;

    if (S_ISCHR(stdin_stat.st_mode) && !bytes_waiting_for_exec) {
      print_prompt();
    }
  }

  if ((ln = parseline(buf)) != NULL) {
    schedule_pipelineseq_for_exec(ln);
  }

  return 0;

  // ln = parseline(
  // 	"ls -las | grep k | wc ; echo abc > f1 ;  cat < f2 ; echo abc >> f3\n");
  // printparsedline(ln);
  // printf("\n");
  // printf("\n");
  // com = pickfirstcommand(ln);
  // printcommand(com, 1);

  // ln = parseline("sleep 3 &");
  // printparsedline(ln);
  // printf("\n");

  // ln = parseline("echo  & abc >> f3\n");
  // printparsedline(ln);
  // printf("\n");
  // com = pickfirstcommand(ln);
  // printcommand(com, 1);
}
