#include "glshell.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

glsh_command_exec_t *command_wait(glsh_command_exec_t *);

int glsh_command_execute(const glsh_command_t *glcmd, glsh_command_exec_t *st)
{
        int r;

        st->glcmd = glcmd;
        st->stat_type = GLSH_CHILD_EXIT;

        if ((r = fork()) > 0) {
                /* In the parent now, so we wait on the child. */
                st->status.child.child = r;
                st = command_wait(st);
        } else if (r == 0) {
                /* In the child, so we’re executing the command or
                 * exiting with -errno. */
                if ((execvp(glcmd->args[0], glcmd->args)) == -1) {
                        /* execvp failed so we exit with that. */
                        _exit(-errno);
                }
        } else {
                st->stat_type = GLSH_CHILD_EFORK;
                st->status.status = errno;
        }

        return 0;
}

glsh_command_exec_t *command_wait(glsh_command_exec_t *st) {
        int cst, wst;

        /* Wait for the child status. waitpid returns the child pid
         * when the child exits, when the child is signaled,
         * suspended, and resumed. */
        while ((wst = waitpid(st->status.child.child, &cst, 0)) > 0) {
                /* wait() returned the child pid, so we have to see if
                 * the child exited. */

                if (!(WIFEXITED(cst))) {
                        /* The child has not yet exited, so this is
                         * a signal notification. */
                        continue;
                } else {
                        /* The child has exited, so we set the exit
                         * status of st. */
                        st->status.child.status = WEXITSTATUS(cst);
                        return st;
                }
        }

        /* We really shouldn’t get an error, but if we do, well, here
         * we go. */
        kill(st->status.child.child, SIGKILL);

        st->stat_type = GLSH_CHILD_EWAITPID;
        st->status.status = errno;
        return st;
}
