#include <stdio.h>
/**
 * main - Entry point
 * Description: Program to illustrate operators in C
 *
 *
 * Return: Always 0
 */
int main(void)
{
int a = 20, b = 15, c, d, e = 10;
c = a++;
d = a;
printf("%d\n", a < b > c == d);
printf("%d\n", a == c);
printf("%d\n", !(d < e));
printf("%d\n", e > d && c <= a);
printf("%d\n", a);
return (0);
}
