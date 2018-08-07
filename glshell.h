#ifndef GLSHELL_H
#define GLSHELL_H

#include <unistd.h>

/**
 * The prompt string glsh_command uses.
 */
#define GLSH_PROMPT "COMMAND->"

/**
 * The shell history file.
 */
#define GLSH_HIST_FILE "history.txt"

/**
 * How many parameters to allocate at once.
 */
#define GLSH_ARGS_MAX 50

/**
 * The maximum length of a command line.
 */
#define GLSH_COMMAND_MAX 1024

/**
 * A command to execute.
 */
typedef struct {
        /** The total length of cmdline. */
        size_t cmdline_size;

        /** The command entered by the user. */
        char *cmdline;

        /** The copy of the command line separated by c \0. */
        char *cmdline_args;

        size_t nargs;

        /** The arguments parsed from cmdline. */
        char **args;
} glsh_command_t;

/**
 * The kind of status from glsh_command_exec.
 */
typedef enum {
        /** The child process exit status. */
        GLSH_CHILD_EXIT,

        /** The errno from fork. */
        GLSH_CHILD_EFORK,

        /** The errno from waitpid. In this case the child will have
         * been sent SIGKILL to prevent zombies.
         */
        GLSH_CHILD_EWAITPID
} glsh_command_stat_t;

/**
 * The result of executing a glsh_command_t.
 */
typedef struct {
        /** The glsh_command_t executed. */
        const glsh_command_t *glcmd;

        /** The kind of status depending on execvp. */
        glsh_command_stat_t stat_type;

        /** The status itself. */
        union {
                /** A child process status. */
                struct {
                        pid_t child;
                        int status;
                } child;

                int status;
        } status;
} glsh_command_exec_t;

/**
 * Prompt for a command and parse it.
 *
 * \retval NULL Exit the shell.
 * \retval glsh_command_t* The read and parsed command.
 *
 * This will read, parse, and return a new glsh_command_t.
 */
glsh_command_t *glsh_command_read();

/**
 * Execute a glsh_command_t, returning its return value.
 *
 * This will fork, and in the child process immediately execute the
 * command, while in the parent wait on the child. When the child
 * exits, this will return the child’s exit code.
 */
int glsh_command_execute(const glsh_command_t *glcmd, glsh_command_exec_t *st);

/**
 * Free a glsh_command_t.
 *
 * @param glsh_command_t* The glsh_command_t* to free.
 * @retval NULL Always returns NULL.
 *
 * This will free any memory used by the passed glsh_command_t*.
 * If it’s already NULL, this does nothing. This always returns NULL
 * so that it can be composed.
 */
glsh_command_t *glsh_command_free(glsh_command_t *cmd);

/** The number of history entries we keep. */
#define GLSH_COMMAND_HIST_SIZE 10

/** The command history list. */
typedef struct {
        /** The total number of entries in the ring. */
        size_t nhist;

        /** The most recent entry in the ring. */
        glsh_command_t *front;

        /** The oldest entry in the ring. */
        glsh_command_t *back;

        /** The history entries themselves. */
        glsh_command_t *history;
} glsh_command_history_t;

#define GLSH_RECENT "recent"

/**
 * Manage the command history.
 *
 * If this is an internal history command (recent), this will handle
 * that command and return NULL. If this is !! or !n, this will return
 * the corresponding glsh_command_t to run instead of the one passed
 * in. Otherwise, this will add the command to the history ring and
 * return it.
 */
const glsh_command_t *glsh_history(
        glsh_command_history_t *,
        const glsh_command_t *);

/**
 * Load the command history.
 *
 * This will load the history from the history file if there is one,
 * and add it to the current command history. This should be called on
 * start-up.
 */
glsh_command_history_t *glsh_history_load(glsh_command_history_t *);

/**
 * Allocate and initialize a new glsh_command_t from the command line.
 *
 * @param cmdline_size The length of the cmdline parameter.
 * @param cmdline The NUL-terminated string of the command line.
 * @param nargs The maximum number of arguments to allocate.
 * @retval glsh_command_t* The allocated and parsed glsh_command_t.
 *
 * This will allocate a glsh_command_t for the passed command line,
 * duplicate it, and then parse the duplicated command line into
 * arguments using glsh_command_parse. It returns the new
 * glsh_command_t on success, and NULL otherwise.
 */
glsh_command_t *glsh_command_alloc(
        size_t cmdline_size,
        const char *cmdline,
        size_t nargs);

#endif
