#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_INPUT 1024
#define PROMPT "($) "

void display_prompt(void);
char *read_line(void);
char **split_line(char *line);
void execute_command(char **args);

int main(void)
{
    char *line;
    char **args;
    int status = 1;

    while (status)
    {
        display_prompt();
        line = read_line();
        if (line == NULL) /* Handle EOF (Ctrl+D) */
        {
            printf("\n");
            break;
        }
        args = split_line(line);
        if (args[0] != NULL)
        {
            execute_command(args);
        }

        free(line);
        free(args);
    }

    return 0;
}

void display_prompt(void)
{
    printf("%s", PROMPT);
}

char *read_line(void)
{
    char *line = NULL;
    size_t bufsize = 0; /* size_t for getline */

    if (getline(&line, &bufsize, stdin) == -1)
    {
        if (feof(stdin))  /* End of file (Ctrl+D) */
        {
            free(line);
            return NULL;
        }
        else
        {
            perror("read_line");
            exit(EXIT_FAILURE);
        }
    }

    return line;
}

char **split_line(char *line)
{
    int bufsize = MAX_INPUT, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens)
    {
        fprintf(stderr, "simple_shell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, " \t\r\n\a");
    while (token != NULL)
    {
        tokens[position] = token;
        position++;

        if (position >= bufsize)
        {
            bufsize += MAX_INPUT;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens)
            {
                fprintf(stderr, "simple_shell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, " \t\r\n\a");
    }
    tokens[position] = NULL;
    return tokens;
}

void execute_command(char **args)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0)
    {
        /* Child process */
        if (execvp(args[0], args) == -1)
        {
            perror("simple_shell");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        /* Error forking */
        perror("simple_shell");
    }
    else
    {
        /* Parent process */
        do
        {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}

