#include "glshell.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Parse a command line into a glsh_command_t.
 *
 * This will split cmdline on whitespace characters into a list of
 * arguments. If cmdline doesn’t end in a \c '\\n' or is empty, this
 * will return NULL.
 *
 * @param glcmd The glsh_command_t with glsh_command_t->cmdline set.
 * @retval NULL cmdline doesn’t end with a newline or is an empty string.
 * @retval glsh_command_t* The command and arguments parsed from cmdline.
 */
glsh_command_t *glsh_command_parse(glsh_command_t *glcmd);

glsh_command_t *glsh_command_read() {
        char cmdline[GLSH_COMMAND_MAX];

        /* Display the prompt and ensure it’s flushed. */
        printf("%s ", GLSH_PROMPT);
        fflush(stdout);

        /* Read the command line. If the command is empty, return NULL
         * to report end of input. */
        if((!fgets(cmdline, GLSH_COMMAND_MAX, stdin))) {
                return NULL;
        };

        /* Allocate the new glsh_command_t. */
        return glsh_command_alloc(GLSH_COMMAND_MAX, cmdline, GLSH_ARGS_MAX);
}

glsh_command_t *glsh_command_alloc(
        size_t cmdline_size,
        const char *cmdline,
        size_t nargs) {

        glsh_command_t *cmd;

        /* Allocate memory for the command line */
        cmd = calloc(1, sizeof(*cmd));

        cmd->cmdline_size = cmdline_size;
        cmd->cmdline = strndup(cmdline, cmd->cmdline_size);

        /* Duplicate the command line for arguments. */
        cmd->cmdline_args = strndup(cmdline, cmdline_size);

        /* Allocate memory for the pointers into cmdline_args. */
        cmd->args = calloc(nargs, sizeof(*cmd->args));

        if (glsh_command_parse(cmd)) {
                return glsh_command_free(cmd);
        }

        return cmd;
}

glsh_command_t *glsh_command_free(glsh_command_t *cmd)
{
        if (!cmd)
                return NULL;

        if(cmd->args)
                free(cmd->args);

        if (cmd->cmdline)
                free(cmd->cmdline);

        if(cmd->cmdline_args)
                free(cmd->cmdline_args);

        return NULL;
}

glsh_command_t *glsh_command_parse(glsh_command_t *glcmd) {
        char **arg;
        char *c;

        /* Return if cmdline isn’t valid. */
        if (glcmd->cmdline[0] < ' ') {
                return NULL;
        }

        /* Start with the beginning of the command line copy, then
         * write ‘\0’ for each argument. */
        c = glcmd->cmdline_args;
        arg = glcmd->args;

        /* For each argument in cmdline_args, point the corresponding
         * arg to the beginning and NUL-terminate it. */
        while (arg - glcmd->args < glcmd->nargs - 1) {
                *arg = c;
                while ((!(*c == '\0' ||
                          *c == ' ' ||
                          *c == '\t' ||
                          *c == '\n')))
                        c++;
                arg++;

                if (*c == '\n') {
                        glcmd->cmdline[c - glcmd->cmdline_args] = '\0';
                        *c = '\0';
                        break;
                }
                *c = '\0';
                c++;
        }

        /* Set the final argument to the NULL pointer. */
        *arg = NULL;
        return 0;
}
