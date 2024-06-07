#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_INPUT 1024
#define PROMPT "($) "

extern char **environ;

void display_prompt(void);
char *read_line(void);
char **split_line(char *line);
void execute_command(char **args);
char *find_command_in_path(char *command);
void print_env(void);
void change_directory(char *path);
void handle_logical_operators(char **args);
void handle_variables(char **args);

/**
 * main - Entry point of the simple shell
 * @argc: Argument count
 * @argv: Argument vector
 *
 * Return: 0 on success, 1 on error
 */
int main(int argc, char **argv)
{
	char *line;
	char **args;
	int status = 1;

	if (argc > 1)
	{
		FILE *fp = fopen(argv[1], "r");

		if (!fp)
		{
			perror("simple_shell");
			return (1);
		}
		while ((line = read_line()) != NULL)
		{
			args = split_line(line);
			execute_command(args);
			free(line);
			free(args);
		}
		fclose(fp);
		return (0);
	}

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
		handle_variables(args);
		if (args[0] != NULL)
		{
			if (strcmp(args[0], "exit") == 0) /* Handle exit command */
			{
				if (args[1] != NULL)
				{
					int exit_status = atoi(args[1]);

					free(line);
					free(args);
					exit(exit_status);
				}
				else
				{
					free(line);
					free(args);
					exit(0);
				}
			}
			else if (strcmp(args[0], "env") == 0) /* Handle env command */
			{
				print_env();
			}
			else if (strcmp(args[0], "cd") == 0) /* Handle cd command */
			{
				change_directory(args[1]);
			}
			else if (strcmp(args[0], "alias") == 0) /* Handle alias command */
			{
				/* Implement alias functionality */
			}
			else if (strcmp(args[0], "#") == 0) /* Handle comments */
			{
				/* Ignore comments */
			}
			else
			{
				handle_logical_operators(args);
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
	fflush(stdout); /* Ensure prompt is displayed immediately */
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
	ssize_t line_length;

	line_length = getline(&line, &bufsize, stdin);
	if (line_length == -1)
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

	/* Remove newline character if present */
	if (line_length > 0 && line[line_length - 1] == '\n')
		line[line_length - 1] = '\0';

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
	char *token, *copy, *saveptr;

	if (!tokens)
	{
		fprintf(stderr, "simple_shell: allocation error\n");
		exit(EXIT_FAILURE);
	}

	copy = strdup(line);
	if (!copy)
	{
		perror("split_line");
		exit(EXIT_FAILURE);
	}

	token = strtok_r(copy, " \t\r\n\a", &saveptr);
	while (token != NULL)
	{
		tokens[position] = strdup(token);
		if (!tokens[position])
		{
			perror("split_line");
			exit(EXIT_FAILURE);
		}
		position++;

		if (position >= bufsize)
		{
			bufsize += MAX_INPUT;
			tokens = realloc(tokens, bufsize * sizeof(char *));
			if (!tokens)
			{
				perror("split_line");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok_r(NULL, " \t\r\n\a", &saveptr);
	}
	tokens[position] = NULL;
	free(copy);
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
		if (execve(command_path, args, environ) == -1)
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
}

/**
 * find_command_in_path - Finds a command in the system PATH
 * @command: The command to find
 *
 * Return: The full path to the command if found, NULL otherwise
 */
char *find_command_in_path(char *command)
{
	char *path = getenv("PATH");
	char *token;
	char *full_path = malloc(MAX_INPUT);

	if (!full_path)
	{
		perror("find_command_in_path");
		exit(EXIT_FAILURE);
	}

	token = strtok(path, ":");
	while (token != NULL)
	{
		snprintf(full_path, MAX_INPUT, "%s/%s", token, command);
		if (access(full_path, X_OK) == 0)
		{
			return (full_path);
		}
		token = strtok(NULL, ":");
	}

	free(full_path);
	return (NULL);
}

/**
 * print_env - Prints the environment variables
 */
void print_env(void)
{
	char **env = environ;

	while (*env)
	{
		printf("%s\n", *env);
		env++;
	}
}

/**
 * change_directory - Changes the current directory
 * @path: The directory to change to
 */
void change_directory(char *path)
{
	if (path == NULL)
	{
		path = getenv("HOME");
	}

	if (chdir(path) != 0)
	{
		perror("cd");
	}
}

/**
 * handle_logical_operators - Handles logical operators in the command
 * @args: An array of arguments
 */
void handle_logical_operators(char **args)
{
	int i;

	for (i = 0; args[i] != NULL; i++)
	{
		if (strcmp(args[i], "&&") == 0)
		{
			args[i] = NULL;
			execute_command(args);
			if (args[i + 1] != NULL)
			{
				handle_logical_operators(&args[i + 1]);
			}
			return;
		}
		else if (strcmp(args[i], "||") == 0)
		{
			args[i] = NULL;
			execute_command(args);
			if (args[i + 1] != NULL)
			{
				handle_logical_operators(&args[i + 1]);
			}
			return;
		}
	}

	execute_command(args);
}

/**
 * handle_variables - Handles variable substitution in the command
 * @args: An array of arguments
 */
void handle_variables(char **args)
{
	int i;

	for (i = 0; args[i] != NULL; i++)
	{
		if (args[i][0] == '$')
		{
			char *env_var = getenv(&args[i][1]);

			if (env_var)
			{
				args[i] = strdup(env_var);
			}
			else
			{
				args[i] = "";
			}
		}
	}
}

