#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "builtins.h"
#include "builtins_impl.h"

int echo(char *[]);
int undefined(char *[]);

builtin_pair builtins_table[] = {
	{"exit", &lexit},
	{"lecho", &echo},
	{"lcd", &lcd},
	{"lkill", &lkill},
	{"lls", &lls},
	{NULL, NULL}};

int echo(char *argv[])
{
	int i = 1;
	if (argv[i])
		printf("%s", argv[i++]);
	while (argv[i])
		printf(" %s", argv[i++]);

	printf("\n");
	fflush(stdout);
	return 0;
}

int undefined(char *argv[])
{
	fprintf(stderr, "Command %s undefined.\n", argv[0]);
	return BUILTIN_ERROR;
}