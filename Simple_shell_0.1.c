#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

int main(void)
{
char *simp = NULL, *simp_cpy = NULL, *token = NULL;
char *delim = " \n";
size_t t = 0;
int argc = 0, s = 0;
char **argv = NULL;

if (getline(&simp, &t, stdin) == -1)
{
return -1;
}
simp_cpy = strdup(simp);

token = strtok(simp, delim);

while (token)
{
token = strtok(NULL, delim);
argc++;
}

printf("%d\n", argc);

argv = malloc(sizeof(char *) * argc);

token = strtok(simp_cpy, delim);

while (token)
{
argv[s] = token;
token = strtok(NULL, delim);
s++;
}
argv[s] = NULL;

s = 0;

while (argv[s])
{
printf("%s\n", argv[s++]);
}

free(simp), free(simp_cpy), free(argv);
return 0;
}
