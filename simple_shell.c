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
char *find_command_in_path(char *command);

/**
 * main - Entry point of the simple shell
 *
 * Return: 0 on success, 1 on error
 */
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
			if (strcmp(args[0], "exit") == 0) /* Handle exit command */
			{
				status = 0;
			}
			else
			{
				execute_command(args);
			}
		}
		free(line);
		free(args);
	}
	return (0);
}

/**
 * display_prompt - Displays the shell prompt
 */
void display_prompt(void)
{
	printf("%s", PROMPT);
}

/**
 * read_line - Reads a line of input from stdin
 *
 * Return: The line read from stdin
 */
char *read_line(void)
{
	char *line = NULL;
	size_t bufsize = 0; /* size_t for getline */

	if (getline(&line, &bufsize, stdin) == -1)
	{
		if (feof(stdin))  /* End of file (Ctrl+D) */
		{
			free(line);
			return (NULL);
		}
		else
		{
			perror("read_line");
			exit(EXIT_FAILURE);
		}
	}
	return (line);
}

/**
 * split_line - Splits a line into tokens
 * @line: The line to split
 *
 * Return: An array of tokens
 */
char **split_line(char *line)
{
	int bufsize = MAX_INPUT, position = 0;
	char **tokens = malloc(bufsize * sizeof(char *));
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
			tokens = realloc(tokens, bufsize * sizeof(char *));
			if (!tokens)
			{
				fprintf(stderr, "simple_shell: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, " \t\r\n\a");
	}
	tokens[position] = NULL;
	return (tokens);
}

/**
 * execute_command - Executes a command
 * @args: An array of arguments
 */
void execute_command(char **args)
{
	pid_t pid;
	int status;
	char *command_path;

	command_path = find_command_in_path(args[0]);
	if (command_path == NULL)
	{
		/* Command not found */
		fprintf(stderr, "simple_shell: %s: command not found\n", args[0]);
		return;
	}

	pid = fork();
	if (pid == 0)
	{
		/* Child process */
		if (execvp(command_path, args) == -1)
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
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	free(command_path);
}

/**
 * find_command_in_path - Finds a command in the PATH
 * @command: The command to find
 *
 * Return: The full path of the command if found, NULL otherwise
 */
char *find_command_in_path(char *command)
{
	char *path, *path_copy, *token, *full_path;
	size_t command_len, dir_len;

	path = getenv("PATH");
	if (path == NULL)
	{
		return (NULL);
	}

	path_copy = strdup(path);
	if (path_copy == NULL)
	{
		perror("simple_shell");
		exit(EXIT_FAILURE);
	}

	command_len = strlen(command);
	token = strtok(path_copy, ":");
	while (token != NULL)
	{
		dir_len = strlen(token);
		full_path = malloc(dir_len + command_len + 2); /* +2 for '/' and '\0' */
		if (full_path == NULL)
		{
			free(path_copy);
			perror("simple_shell");
			exit(EXIT_FAILURE);
		}

		strcpy(full_path, token);
		strcat(full_path, "/");
		strcat(full_path, command);

		if (access(full_path, X_OK) == 0)
		{
			free(path_copy);
			return (full_path);
		}

		free(full_path);
		token = strtok(NULL, ":");
	}
	free(path_copy);
	return (NULL);
}

