#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "errors.h"

void message (const char *err) {
	fprintf (stderr, "%s\n", err);
}

void message1 (const char *err, const char *s) {
	char errmsg[ERRMSGLEN];
	sprintf (errmsg, err, s);
	fputs (errmsg, stderr);
	putc ('\n', stderr);
}

void gerror(const char *err) {
	fprintf (stderr, "%s\n", err);
	exit(1);
}

void gerror1 (const char *err, const char *s) {
	char errmsg[ERRMSGLEN];
	sprintf (errmsg, err, s);
	fputs (errmsg, stderr);
	putc ('\n', stderr);
	exit(1);
}

void gerror2(const char *err,char *err2) {
	fprintf (stderr, "%s %s\n", err,err2);
	exit(1);
}
