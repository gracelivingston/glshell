/**
 * Handle the history of commands.
 */

#include "glshell.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const glsh_command_t *glsh_history_recent(glsh_command_history_t *history);
const glsh_command_t *glsh_history_bang(
        glsh_command_history_t *history,
        const glsh_command_t *glcmd);
const glsh_command_t *glsh_history_add(
        glsh_command_history_t *history,
        const glsh_command_t *glcmd);

/**
 * Save the current command history.
 *
 * @param history The current command history.
 * @returns The saved glsh_command_history_t.
 */
const glsh_command_history_t *glsh_history_save(
        const glsh_command_history_t *history
        );

const glsh_command_t *glsh_history(
        glsh_command_history_t *history,
        const glsh_command_t *glcmd) {
        if (!strncmp(GLSH_RECENT, glcmd->args[0], sizeof(GLSH_RECENT))) {
                return glsh_history_recent(history);
        } else if (glcmd->args[0][0] == '!') {
                return glsh_history_bang(history, glcmd);
        } else {
                glsh_history_add(history, glcmd);
                return glcmd;
        }
}


size_t glsh_history_length(const glsh_command_history_t *h);

const glsh_command_t *glsh_history_recent(
        glsh_command_history_t *history) {
        glsh_command_t *h;
        size_t n;

        /* The list is empty in this case. */
        if (history->back == history->front) {
                return NULL;
        }

        n = glsh_history_length(history);

        /* Go through the history list from the least recent to the
         * most recent entries and print them with their indexes. */
        h = history->back;
        do {
                /* Print the command. */
                printf("%lu %s\n", n, h->cmdline);
                n--;

                /* Go back one entry. */
                h--;

                /* Are we before the beginning? */
                if (h < history->history)
                        /* If so, wrap to the end. */
                        h = (history->history +
                             history->nhist - 1);

        } while (h != history->front);

        return NULL;
}

const glsh_command_t *glsh_history_bang(
        glsh_command_history_t *history,
        const glsh_command_t *glcmd) {
        return glcmd;
}

const glsh_command_t *glsh_history_add(
        glsh_command_history_t *history,
        const glsh_command_t *glcmd) {

        /* Move the back pointer back. */
        history->back++;

        /* See if back needs to wrap. */
        if (history->back >= history->history + history->nhist) {
                history->back = history->history;
        }

        /* See if we’ve wrapped. */
        if (history->back <= history->front) {
                history->front++;
        }

        /* See if front needs to wrap. */
        if (history->front > history->history + history->nhist) {
                history->front = history->history;
        }

        /* Insert the command at the back of the list. */
        if (history->back->cmdline) {
                free(history->back->cmdline);
                history->back->cmdline_size = 0;
        }

        if (history->back->args) {
                free(history->back->args);
                history->back->nargs = 0;
        }

        /* Allocate the command line and copy it. */
        history->back->cmdline = calloc(
                glcmd->cmdline_size,
                sizeof(*glcmd->cmdline));
        history->back->cmdline_size = glcmd->cmdline_size;
        strcpy(history->back->cmdline, glcmd->cmdline);

        /* Allocate the args and copy them. */
        history->back->args = calloc(
                glcmd->nargs,
                sizeof(*glcmd->args));
        history->back->nargs = glcmd->nargs;
        memcpy(history->back->args, glcmd->args,
               glcmd->nargs * sizeof(*glcmd->args));

        glsh_history_save(history);
        return glcmd;
}

size_t glsh_history_length(const glsh_command_history_t *history) {
        size_t n;

        if (history->back < history->front) {
                n = history->nhist + 1;
        } else {
                n = 0;
        }

        return n + history->back - history->front;
}

const glsh_command_history_t *glsh_history_save(
        const glsh_command_history_t *history
        ) {

        FILE *hf;
        glsh_command_t *h;

        /* The list is empty in this case. */
        if (history->back == history->front) {
                return NULL;
        }

        if (!(hf = fopen(GLSH_HIST_FILE, "wb"))) {
                return NULL;
        }

        /* Go through the history list from the least recent to the
         * most recent entries and write them to the file. */
        h = history->back;
        do {
                /* Print the command. */
                fprintf(hf, "%s\n", h->cmdline);

                /* Go back one entry. */
                h--;

                /* Are we before the beginning? */
                if (h < history->history)
                        /* If so, wrap to the end. */
                        h = (history->history +
                             history->nhist - 1);

        } while (h != history->front);

        fclose(hf);
        return history;
}

glsh_command_history_t *glsh_history_load(glsh_command_history_t *history) {

        FILE *hf;

        char buf[GLSH_COMMAND_MAX];
        glsh_command_t *cmd;

        if (!(hf = fopen(GLSH_HIST_FILE, "rb"))) {
                return NULL;
        }

        while (!feof(hf)) {
                fgets(buf, GLSH_COMMAND_MAX, hf);
                cmd = glsh_command_alloc(GLSH_COMMAND_MAX, buf, GLSH_ARGS_MAX);
                glsh_history_add(history, cmd);
        }

        fclose(hf);
        return history;
}
