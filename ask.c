#include <stdio.h>
#include <string.h>
#include "config.h"
#include "ask.h"

void askd (char *text, double *data) {
	double temp;
	char input[DOUBLEN];
	int ok = FALSE, l;

	do {
		printf ("%s (%g) ?", text, *data);
		fgets (input, DOUBLEN - 1, stdin);
		l = strlen (input);
		/* remove newline */
		if (input[l - 1] == '\n') input[l - 1] = '\0';
		/* RETURN-Taste */
		if (input[0] == '\0') ok = TRUE;
		else if (sscanf (input, "%lg", &temp) == 1) {
			ok = TRUE;
			*data = temp;
		}
	}
	while (ok == FALSE);
}

void asks (char *text, char *data) {
	char input[ASKLINELEN];
	int l;

	printf ("%s (\"%s\") ?", text, data);
	fgets (input, ASKLINELEN - 1, stdin);
	l = strlen (input);
	/* remove newline */
	if (input[l - 1] == '\n') input[l - 1] = '\0';
	/* RETURN-Taste */
	if (input[0] != '\0') strcpy (data, input);
}

void aski (char *text, int *data) {
	int temp;
	char input[DOUBLEN];
	int ok = FALSE, l;

	do {
		printf ("%s (%d) ?", text, *data);
		fgets (input, DOUBLEN - 1, stdin);
		l = strlen (input);
		/* remove newline */
		if (input[l - 1] == '\n') input[l - 1] = '\0';
		/* RETURN-Taste */
		if (input[0] == '\0') ok = TRUE;
		else if (sscanf (input, "%d", &temp) == 1) {
			ok = TRUE;
			*data = temp;
		}
	}
	while (ok == FALSE);
}
