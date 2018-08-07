#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "glshell.h"

int main(int argc, char **argv) {
        glsh_command_t *cmd;
        glsh_command_exec_t status;
        glsh_command_history_t history;

        history.history = calloc(
                GLSH_COMMAND_HIST_SIZE,
                sizeof(glsh_command_t));

        history.nhist = GLSH_COMMAND_HIST_SIZE;
        history.front = history.back = history.history;

        glsh_history_load(&history);

        while ((cmd = glsh_command_read())) {
                if (!glsh_history(&history, cmd)) {
                        continue;
                }

                if (glsh_command_execute(cmd, &status)) {
                        fprintf(stderr,
                                "Status error: %s\n",
                                strerror(errno));
                        continue;
                } else if (status.stat_type == GLSH_CHILD_EXIT) {
                        continue;
                } else {
                        printf("System error: %s\n",
                               strerror(status.status.status));
                }
        }

        fprintf(stderr, "Exiting: %s\n", strerror(errno));
}
