#ifndef __IO_h
#define __IO_h

int exists(char *fn);
int getNoC(char *fn, int *comma);
void probe_file(char *fn, double *fs, int *ndata, double *mean, unsigned int t, unsigned int A, unsigned int B, int comma);
void read_file(char *ifn, double ulsb, double mean, int start, int nread, int comma);
void close_file();
double *get_data();
void saveResult(tCFG * cfg, tDATA * data, tGNUTERM * gt, tWinInfo *wi, int argc, char *argv[]);
int write_gnufile(char *gfn, char *ofn, char *vfn, char *ifn, char *s, 
			double fmin, double fmax, double dmin, double dmax,
			char *id, int c);

#endif
