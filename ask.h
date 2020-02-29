#ifndef __ask_h
#define __ask_h

#define TRUE 1
#define FALSE 0

void askd (char *text, double *data);
void asks (char *text, char *data);
void aski (char *text, int *data); 
void gerror (const char *err);
void gerror1 (const char *err, const char *s); 

#endif
