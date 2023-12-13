#include <stdio.h>
#include <stdlib.h>

int main(void)
{
size_t n = 10;
char *buf = NULL;

getline(&buf, &n, stdin);


free(buf);

return 0;
}
