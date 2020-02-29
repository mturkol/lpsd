#ifndef __config_h
#define __config_h

#define LPSD_VERSION "LPSD 05.12.2019"

#define DEFIFN "p5.txt"		/* lpsd.c	- default input file name */
#define DEFOFN "%g-%C-lpsd.txt"	/* lpsd.c	- default output file name */
#define DEFGFN "%g-%C-lpsd.gnu"	/* lpsd.c	- default gnuplot filename */
#define DEFWFN "FFTW-wisdom"	/* lpsd.c	- default FFTW wisdom file name */
#define DEFWT -2 		/* lpsd.c	- default window type: -2 Kaiser, -1 flat top, or 0..30  */
#define DEFPSLL	120		/* lpsd.c	- default peak side lobe level */
#define DEFOVLP	-1.0		/* lpsd.c	- default overlap for FFTs -1 calculate optimum overlap */
#define DEFFRES -1.0		/* lpsd.c	- default frequency resolution [Hz] 
							-1 determine highest frequency resolution */
#define DEFULSB 1.0		/* lpsd.c	- default scaling factor for gnuplot */
#define DEFLR 1			/* lpsd.c	- 0 no linear regression, 1 linear regression as default, 
							2 subtract line through first and last data point */
#define DEFTMIN 0
#define DEFTMAX -1
#define DEFFMIN -1
#define DEFFMAX -1	
#define DEFDESAVG 100		/* desired number of averages */
#define DEFMINAVG 10		/* minimum number of averages */
#define DEFSBIN -1		/* use smallest bin given by window function */
#define DEFMETHOD 0		/* METHOD to calculate frequency METHOD */
#define DEFFSAMP 1e4		/* lpsd.c	- default sampling frequency */
#define DEFNSPEC 500		/* lpsd.c	- default number of frequencies in spectrum */

#define DATADEL " \t\n"		/* IO.c		- delimiters in datafiles: space, tab, and newline *** 28.06.2007 newline added */
#define DATALEN 1000		/* IO.c		- length of a single line in ASCII data files */
#define MAXGNUTERM 100		/* max. number of plot environments */
#define FNLEN 256		/* lpsd.c	- length of filename strings */
#define CMTLEN 5000		/* lpsd.c	- length of gnuplot comments */
#define ERRMSGLEN 512		/* errors.c	- max. length of error messages */
#define DOUBLEN 40		/* ask.c	- */
#define ASKLINELEN 250		/* ask.c	- */
#define TICLEN 5000		/* tics.c	- maximum length of tics strings */
#define PSTEP 0.2		/* time interval after which to print progress information on spectrum calculation */

#define SLEN 100		/* config.c	- length of strings */
#define CLEN 5000		/* config.c	- length of gunplot commands */

typedef struct {
	int type;
	double req_psll;
	char name[20];
	double psll;
	double rov;
	double nenbw;
	double w3db;
	double winsum;			/* FFTW */
	double winsum2;			/* FFTW */
	double flatness;
	double sbin;			/* smallest bin to be used in spectrum estimation */
} tWinInfo;

typedef struct {
	char identifier[SLEN];		/* is displayed within lpsd */
	char cmds[CLEN];		/* gnuplot commands */
	char fmt[SLEN];			/* format of the output file */
} tGNUTERM;

typedef struct {
	int usedefs;			/* use default values if 1, ask for remaining values otherwise */
	char ifn[FNLEN];		/* input file name */
	unsigned short int askifn;
	char ofn[FNLEN];		/* output file name */
	unsigned short int askofn;
	char gfn[FNLEN];		/* gnuplot file name */
	unsigned short int askgfn;
	char wfn[FNLEN];		/* wisdom file for FFTW */
	char param[SLEN];		/* parameter string */
	int WT;				/* window function; -2 Kaiser, -1 flat top, 0..30 */
	unsigned short int askWT;
	int LR;				/* 0 no linear regression, 1 perform linear regression */
	int nspec;			/* number of samples in spectrum */
	int nfft;			/* FFTW: dimension of FFT */
	unsigned short int asknspec;
	double fsamp;			/* sampling frequency */
	unsigned short int askfsamp;
	unsigned short int cmdfsamp;	/* 1 if specified on command line, true otherwise */
	double fres;			/* frequency resolution for FFT */
	unsigned short int askfres;
	unsigned short int cmdfres;
	double reqPSLL;			/* requested peak side lobe level by user */
	unsigned short int askreqPSLL;
	double ovlp;			/* overlap */
	unsigned short int askovlp;
	unsigned short int cmdovlp;
	double ulsb; 			/* scaling factor for the input data */
	unsigned short int askulsb;

	double tmin;			/* start time for spectrum estimation */
	unsigned short int asktmin;
	double tmax;			/* stop time for spectrum estimation; -1 : use all data */
	unsigned short int asktmax;
	double fmin;
	unsigned short int askfmin;
	double fmax;
	unsigned short int askfmax;
	int desAVG;			/* desired number of averages for spectral estimation */
	unsigned short int askdesAVG;
	unsigned short int cmddesAVG;
	int minAVG;			/* minimum number of averages for spectral estimation */
	unsigned short int askminAVG;
	unsigned short int cmdminAVG;
	int METHOD;			/* method to calculate frequency nodes */
	unsigned short int askMETHOD;
	unsigned short int cmdMETHOD;
	int ngnuterm;			/* number of gnuplot terminals */
	unsigned short int askgt;
	int gt;				/* gnuplot terminal to be used */
	double sbin;			/* smallest bin to be used in spectrum estimation */
	unsigned short int asksbin;
	unsigned int time;		/* 1 - first column in datafile contains time in s */
	unsigned short int asktime;	
	unsigned int colA;		/* process column A */
	unsigned short int askcolA;
	unsigned int colB;		/* process column B if B>0 & B>A */
	unsigned short int askcolB;	
} tCFG;	

typedef struct {
	double *fspec;			/* frequencies where spectra are calculated */
	double *bins;			/* frequency bins in DFTs */
	double *ps;			/* power spectrum */
	double *psd;			/* power spectral density */
	double *varps;			/* variance of power spectrum */
	double *varpsd;			/* variance of power spectral density */
	double *fft_ps;			/* FFTW: complete power spectrum of FFTW */
	double *fft_varps;		/* FFTW: complete variance of power spectrum of FFTW */
	int *avg;			/* debug information: number of averages */
	int *nffts;			/* list of nffts for DFTs */
	int NoC;			/* number of columns in data file */
	double mean;			/* mean value of input data */
	int ndata;			/* number of data in input file */
	int nread;			/* length of time series used for spectrum estimation */
	int comma;			/* 1 - comma as decimal delimiter; 0 - decimal points */

} tDATA;

int readConfigFile();
void getConfig(tCFG *c);
void getGNUTERM(int i, tGNUTERM *dest);
void printConfig(char *dest, tCFG cfg, tWinInfo wi, tGNUTERM gt, tDATA data);

#endif
