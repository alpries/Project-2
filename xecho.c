#include "posix-calls.h"

void
printdec(unsigned int x)
{
	char digit[] = "0123456789";
	char buf[24];
	int len = 0;
	char *ptr = &buf[23];
	do {
		ptr--;
		len++;
		*ptr = digit[x % 10];
		x /= 10;
	} while (x > 0);
	Lwrite(1, ptr, len);
}

int
lenstr(char *str)
{
	int ret;
	for (ret = 0; *(str + ret) != 0; ret++) {
		;
	}
	return ret;
}

void
wrstr(char *str)
{
	int len = lenstr(str);
	Lwrite(1, str, len);
}

void
wrstrarray(char str[])
{
	// int len = lenstr(str);
	char c;
	wrstr("{");
	for (int j = 0; (c = str[j]) !=0 ; j++) {
		if (c == '\t') {
			wrstr("\'\\t\'");
		} else if (c == '\n') {
			wrstr("\'\\n\'");
		} else if (c == '\r') {
			wrstr("\'\\r\'");
		} else if (c == '\\') {
			wrstr("\'\\\\\'");
		} else if (c == '\'') {
			wrstr("\'\\\'\'");
		} else if (c < ' ' || c > '~') {
			printdec(c);
		} else {
			wrstr("\'");
			Lwrite(1, &c, 1);
			wrstr("\'");
		}
		wrstr(", ");
	}
	wrstr("\'\\0\'};");
}

int
Lmain(int argc, char *argv[], char *envp[])
{
	wrstr("argc == ");
	printdec(argc);
	wrstr("\n");

	for (int k = 0; k < argc; k++) {
		wrstr("argv[");
		printdec(k);
		wrstr("] : \"");
		wrstr(argv[k]);
		wrstr("\" == ");
		wrstrarray(argv[k]);
		wrstr("\n");
	}

	int envc;
	for (envc = 0; envp[envc] != NULL; envc++) {
		;
	}
	wrstr("envc == ");
	printdec(envc);
	wrstr("\n");

	for (int k = 0; k < envc; k++) {
		if (k == 0 || k == envc - 1) {
			wrstr("envp[");
			printdec(k);
			wrstr("] : \"");
			wrstr(envp[k]);
			wrstr("\"\n");
		} else if ( k == 1 && 1 < envc - 1) {
			wrstr("...\n");
		}
	}

	return 0;
}

