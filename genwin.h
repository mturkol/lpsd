#ifndef __genwin_h
#define __genwin_h

/* ANSI prototypes of externally visible functions: */

void set_window (int type, double req_psll, char *name, double *psll,
		 double *rov, double *nenbw, double *w3db, double *flatness,
		 double *sbin);

void makewinsincos (int nfft, double bin, double *win, double *winsum,
		    double *winsum2, double *nenbw);

void makewin (int nfft, int half, double *win, double *winsum,
	      double *winsum2, double *nenbw);

#endif
