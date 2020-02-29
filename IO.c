/********************************************************************************
 *	IO.c  -  handle all input/output for lpsd.c				*
 ********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "config.h"
#include "misc.h"
#include "errors.h"
#include "tics.h"
#include "debug.h"
#include "IO.h"
#include "StrParser.h"


static FILE *ifp = 0;			/* input file pointer */
static char curline[DATALEN];		/* currently read input data line */
static double curdata;			/* contains current data */
static double curtime;			/* contains current time */
static double dts;			/* sum of delta t's */
static double dt2s;			/* sum of delta t^2's */
static double *data = 0;		/* pointer to all data */
static int (*read_data) (void);		/* pointer to function reading input data */
static unsigned int timecol;		/* column 1 contains time in s */
static unsigned int colA;		/* read data from column A */
static unsigned int colB;		/* read data from column B */

static void replaceComma(char *s);
static int read_t_A_B(void);
static int read_A_B(void);
static int read_A(void);

/********************************************************************************
 *	replaces commas by decimal dots						*
 *	parameters:								*
 *		char *s		pointer to string to be changed			*
 ********************************************************************************/
static void replaceComma(char *s) {
	unsigned int n;
	
	for (n = 0; s[n] > 0; n++)
		if (s[n] == ',')
			s[n] = '.';

}

/********************************************************************************
 *	reads time and two columns from a file					*
 *	returns:								*
 *		1 on success							*
 *		0 on failure							*
 ********************************************************************************/
static int read_t_A_B(void)
{
	unsigned int n, ok = 0;
	char s[DATALEN];
	char *col;
	double dataA, dataB;
	
	strcpy(&s[0],&curline[0]);
	col=strtok(s,DATADEL);			/* ATTENTION: s gets altered by strtok */
	if (sscanf(col,"%lg",&curtime)==1) 
		ok = 1;

//	printf("time=%f\t",curtime);

	for (n=1; n<colA;n++) col=strtok(NULL,DATADEL);
	if (sscanf(col,"%lg",&dataA)==1) 
		ok = 1; else ok=0;
//	printf("dataA=%f\t",dataA);
	for (n=0; n<colB-colA;n++) col=strtok(NULL,DATADEL);
	if (sscanf(col,"%lg",&dataB)==1) 
		ok = 1; else ok=0;
//	printf("dataB=%f\n",dataB);
	curdata=dataB-dataA;

	return (ok);
}

/********************************************************************************
 *	reads time and one column from a file					*
 *	returns:								*
 *		1 on success							*
 *		0 on failure							*
 ********************************************************************************/
static int read_t_A(void)
{
	unsigned int n, ok = 0;
	char s[DATALEN];
	char *col;
	
	strcpy(&s[0],&curline[0]);

	col=strtok(s,DATADEL);			/* ATTENTION: s gets altered by strtok */
	if (sscanf(col,"%lg",&curtime)==1) 
		ok = 1;

//	printf("time=%f\t",curtime);

	for (n=1; n<colA;n++) col=strtok(NULL,DATADEL);
	if (sscanf(col,"%lg",&curdata)==1) 
		ok = 1; else ok=0;
//	printf("dataA=%f\t",curdata);

	return (ok);
}

/********************************************************************************
 *	reads two columns from a file						*
 *	returns:								*
 *		1 on success							*
 *		0 on failure							*
 ********************************************************************************/
static int read_A_B(void)
{
	unsigned int n, ok = 0;
	char s[DATALEN];
	char *col;
	double dataA, dataB;

	strcpy(&s[0],&curline[0]);
	
	col=strtok(s,DATADEL);			/* ATTENTION: s gets altered by strtok */
	for (n=1; n<colA;n++) col=strtok(NULL,DATADEL);
	if (sscanf(col,"%lg",&dataA)==1) 
		ok = 1; else ok=0;
//	printf("dataA=%f\t",dataA);
	for (n=0; n<colB-colA;n++) col=strtok(NULL,DATADEL);
	if (sscanf(col,"%lg",&dataB)==1) 
		ok = 1; else ok=0;
//	printf("dataB=%f\n",dataB);
	curdata=dataB-dataA;

	return (ok);
}

/********************************************************************************
 *	reads one column from a file						*
 *	returns:								*
 *		1 on success							*
 *		0 on failure							*
 ********************************************************************************/
static int read_A(void)
{
	unsigned int n, ok = 0;
	char s[DATALEN];
	char *col;

	strcpy(&s[0],&curline[0]);
	
	col=strtok(s,DATADEL);			/* ATTENTION: s gets altered by strtok */
	for (n=1; n<colA;n++) col=strtok(NULL,DATADEL);
	if (sscanf(col,"%lg",&curdata)==1) 
		ok = 1; else ok=0;
//	printf("dataA=%f\n",curdata);

	return (ok);
}

/********************************************************************************
 *	reads one line from a file					*
 *	parameters:								*
 *		int comma		1 if comma is decimal point, 0 otherwise*
 *	returns:								*
 *		2 if first character is comment character #			*
 *		1 on success							*
 *		0 on failure							*
 *		copies line of data into curline				*
 ********************************************************************************/
static int read_lof(int comma) {
	int ok=0;
	
	if (0 != fgets(&curline[0], DATALEN, ifp)) {		/* read max. DATALEN-1 characters */
		ok=1;
		/* replace commas by decimal points */
		if (comma == 1) replaceComma(curline);
		/* test for comment character */
		if (curline[0]=='#') ok=2;
	}
	return (ok);

}

/* returns 1 if file fn exists, 0 otherwise */
int exists(char *fn)
{
	int ok = 0;
	ifp = fopen(fn, "r");
	if (ifp != 0) {
		ok = 1;
		fclose(ifp);
	}
	return (ok);
}

/********************************************************************************
 *	opens a file and returns the number of its columns
 *
 *	recognized formats:
 *
 *		y
 *		y y
 *		y y y....
 *
 *	Parameter
 *		fn	name of file
 *		comma	1 if comma is used as decimal point, 0 otherwise
 *	Returns	
 *		-1	file type not detected	
 *		0	t y text file
 *		1	y text file
 *
 *	delimiters after the last column confuse getNoC - they get counted as
 *	another column
 ********************************************************************************/
int getNoC(char *fn, int *comma)
{
	FILE *fp;
	char s[DATALEN];
	int n=-1;
	char *col;
	
	fp = fopen(fn, "rb");
	if (fp != 0) {
		fgets(s, DATALEN, fp);	/* read max. DATALEN-1 characters */
		/* first line of file has been read */
		*comma = (int) strchr(s, ',') ? 1 : 0;
		/* replace commas by decimal points */
		if (*comma == 1) replaceComma(s);
		/* how many columns does file have? */
		col=strtok(s,DATADEL);			/* ATTENTION: s gets altered by strtok */
		if (col!=NULL) {
			for (n=1; ((col=strtok(NULL,DATADEL))!=NULL);n++);
		} else n=0;
		fclose(fp);
//		printf("Number of columns: %d\n",n);
	}
	return (n);
}

/********************************************************************************
 *	reads file *fn, counts number of data points and determines mean of data
 *										
 *	Parameters
 *		fn	name of file						
 *		fs	sample frequency
 *		t	1 first column contains time in s
 *		A	number of column to process
 *		B	number of column to process in combination
 *		comma	if 1, then decimal point is a comma
 *										
 *	Returns
 * 		ndata	number of datapoints read
 *		mean	mean value of data points
 ********************************************************************************
 	Naming convention	source code	publication
				fs		f_s
				ndata		N
 ********************************************************************************/
void probe_file(char *fn, double *fs, int *ndata, double *mean, unsigned int t, unsigned int A, unsigned int B, int comma)
{
	int nread = 0;
	double lasttime = 0.0;
	char errmsg[200];
	
	*mean = 0.0;
	timecol=t;
	colA=A,
	colB=B;

	/* open file */
	ifp = fopen(fn, "r");
	if (ifp == 0)
		gerror1("Error opening %s", fn);
	
	/* select reading routine */
	if ((timecol==1) & (colB>0)) read_data=read_t_A_B;
	else if ((timecol==1) & (colB==0)) read_data=read_t_A;
	else if ((timecol==0) & (colB>0)) read_data=read_A_B;
	else if ((timecol==0) & (colB==0)) read_data=read_A;

	if (read_data==NULL) gerror("No file reading routine selected!\n");

	curtime = 0;
	dts = 0;
	dt2s = 0;
	while (0 < read_lof(comma)) {
		if (curline[0]!='#') {
			if (1==read_data()) {
				*mean += curdata;
				if (nread > 0) {
					dts += curtime - lasttime;
					dt2s += (curtime - lasttime) * (curtime - lasttime);
				}
				lasttime = curtime;
				nread++;
			} else {
				sprintf(errmsg,"Error reading file in data line %d\n",nread);
				gerror(errmsg);
			}	
		}
	}

	*mean = *mean / (double) nread;
	*ndata = nread;
	*fs = (double) (nread - 1) / dts;
	/* close file */
	fclose(ifp);
	ifp = 0;
}

/********************************************************************************
 *	reads a data file into memory and multiplies by it by ulsb	
 *		ifn	name of file
 *		ulsb	scaling factor to multiply data with						
 *		mean	mean value of data in file
 *		start	line number of first line to use (beginning with 0)
 *		nread	number of lines to read
 *		comma	if 1, then the decimal point is a comma
 ********************************************************************************
 	Naming convention	source code	publication
			 	nread 		N
 ********************************************************************************/
void read_file(char *ifn, double ulsb, double mean, int start, int nread, int comma)
{
	int i;
	int rslt;
	
	ifp = fopen(ifn, "r");
	if (ifp == 0)
		gerror1("Error opening %s", ifn);
	data = (double *) xmalloc(nread * sizeof(double));
	/* check if reading routine has been selected */
	if (read_data==NULL) gerror("No file reading routine selected!\n");

	/* skip lines before start */
	for (i = 0; i < start; i++) {
		if (2==read_lof(comma)) start++;		/* when comment was read, read one more line */
	}
	
	/* read data into data array */
	for (i = 0; i < nread; i++) {
		rslt=read_lof(comma);
		if (2==rslt) i--;				/* when comment was read, read one more line */
		if (1==rslt) { 					/* if data was read, process it */
			if (1==read_data())	
				data[i] = (curdata - mean)*ulsb;
		}	
	}
	fclose(ifp);
	ifp = 0;

}

/*
	returns start address of data
*/
double *get_data()
{
	return (&data[0]);
}

void close_file()
{
	if (0 != ifp)
		fclose(ifp);
	if (0 != data)
		xfree(data);
}

/*
	writes a comment line to *ofp with information on what quantity will be saved in what column
	
	input: 	ofp	output file name
		gt	gnuplot terminal
*/
static void writeHeaderLine(FILE *ofp, tGNUTERM * gt) {
	unsigned int c;
	char tmp[SLEN];
	
	fprintf(ofp,"# ");;
	for (c = 0; c < strlen((*gt).fmt); c++) {
		switch ((*gt).fmt[c]) {
		case 'f':
			fprintf(ofp, "Frequency [Hz]	");
			break;
		case 'd':
			fprintf(ofp, "LSD	");
			break;
		case 'D':
			fprintf(ofp, "PSD	");
			break;
		case 's':
			fprintf(ofp, "LS	");
			break;
		case 'S':
			fprintf(ofp, "PS	");
			break;
		case 'N':
			fprintf(ofp, "NUM_AVG	");
			break;
		case 'u':
			fprintf(ofp, "STD_LSD	");
			break;
		case 'U':
			fprintf(ofp, "VAR_PSD	");
			break;
		case 'v':
			fprintf(ofp, "STD_LS	");
			break;
		case 'V':
			fprintf(ofp, "VAR_PS	");
			break;
		case 'R':
			fprintf(ofp, "RBW [Hz]	");
			break;
		case 'b':
			fprintf(ofp, "Bin	");
			break;
		default:
			strcpy(&tmp[0],&((*gt).fmt[0]));
			(*gt).fmt[c+1]=0;
			message1("\tWARNING: Identifier %s in gnuplot terminal not recognized",&((*gt).fmt[c]));
			strcpy(&((*gt).fmt[0]),&tmp[0]);
			break;
		}
	}
	fprintf(ofp,"\n");;
}

/*
	writes the output data that lpsd calculated to file
	
	input:	ofp	input file pointer
		cfg	configuration information for nspec
		data	actual data to write
		gt	gnuplot terminal - what to write
*/
static void writeData(FILE *ofp, tCFG * cfg, tDATA * data, tGNUTERM * gt) {
	int i;
	unsigned int c;
	
	for (i = 0; i < (*cfg).nspec; i++) {
		for (c = 0; c < strlen((*gt).fmt); c++) {
			switch ((*gt).fmt[c]) {
			case 'f':
				fprintf(ofp, "%.18e	", (*data).fspec[i]);
				break;
			case 'd':
				fprintf(ofp, "%.18e	", sqrt((*data).psd[i]));
				break;
			case 'D':
				fprintf(ofp, "%.18e	", (*data).psd[i]);
				break;
			case 's':
				fprintf(ofp, "%.18e	", sqrt((*data).ps[i]));
				break;
			case 'S':
				fprintf(ofp, "%.18e	", (*data).ps[i]);
				break;
			case 'N':
				fprintf(ofp, "%d	", (*data).avg[i]);
				break;
			case 'u':
				fprintf(ofp, "%.18e	", sqrt((*data).varpsd[i]));
				break;
			case 'U':
				fprintf(ofp, "%.18e	", (*data).varpsd[i]);
				break;
			case 'v':
				fprintf(ofp, "%.18e	", sqrt((*data).varps[i]));
				break;
			case 'V':
				fprintf(ofp, "%.18e	", (*data).varps[i]);
				break;
			case 'R':
				fprintf(ofp, "%e	",
					(*cfg).fsamp / (double) (*data).nffts[i]);
				break;
			case 'b':
				fprintf(ofp, "%e	", (*data).bins[i]);
				break;
			default:
				break;
			}
		}
		fprintf(ofp, "\n");
	}
}


/*
	prints the command line the program was called with to a string
	
	inputs:	dst	pointer to write result to
			must point to variable that has enough space to write data to
		argc	number of command line arguments
		argv	argument strings
*/
void printCommandLine(char *dst, int argc, char *argv[]) {
	int i,b;
	
	b=0;
	for (i=0;i<argc;i++) {
		sprintf(&dst[b],"%s ",argv[i]);
		b=strlen(dst);
	}
			
}

/*
	write information to string: 
*/
static void writeComment(char *dest, tCFG *cfg, tWinInfo *wi, tGNUTERM *gt, tDATA *data, int argc, char *argv[]) {
	char *stm;		/* string with time and date */
	time_t tm;		/* time in epoch */
	char c[CLEN];
	char cmdline[CMTLEN];
	
	/* date, time, and lpsd version */
	tm = time(NULL);
	stm = ctime(&tm);
	sprintf(&dest[0], "# output from %s, generated ", LPSD_VERSION);
	strcat(dest, stm);
	
	/* command line */
	printCommandLine(&cmdline[0],argc, argv);
	sprintf(&dest[strlen(dest)],"# Command line: %s\n#",cmdline);

	/* info on files, window, data, output */
	printConfig(&c[0],*cfg, *wi, *gt, *data);
	rplStr(&c[0], "\n", "\n# ");
	strcat(dest,c);
	sprintf(&dest[strlen(dest)],"\n");
}

/*
	# identifiers for output file format
	#
	# f	frequency
	# d	linear spectral density
	# D	power spectral density
	# s	linear spectrum
	# S	power spectrum
	# N	number of averages
	# u	standard deviation of linear spectral density
	# U	variance of power spectral density
	# v	standard deviation of linear spectrum
	# V	variance of power spectrum
	# R	resolution bandwidth
	# b	bin number

	The format of the output file is stored in gt[gti].fmt

*/
void writeOutputFile(tCFG * cfg, tDATA * data, tGNUTERM * gt, tWinInfo *wi, int argc, char *argv[]) {
	FILE *ofp;
	char cmt[CMTLEN];

	ofp = fopen((*cfg).ofn, "w");
	if (0 == ofp)
		gerror1("Error opening", (*cfg).ofn);

	writeComment(&cmt[0], cfg, wi, gt, data, argc, argv);
	fprintf(ofp,"%s",cmt);

	writeHeaderLine(ofp, gt);
	writeData(ofp, cfg, data, gt);
	
	fclose(ofp);
}

static double getLSD(tDATA *data, int i) {
	return sqrt((*data).psd[i]);
}

static double getLS(tDATA *data, int i) {
	return sqrt((*data).ps[i]);
}

static double getPSD(tDATA *data, int i) {
	return (*data).psd[i];
}

static double getPS(tDATA *data, int i) {
	return (*data).ps[i];
}

/*
	scan gt.fmt string and determine calibration of first y column
	scan data and return minimum and maximum value for first y column
*/
static void getSpan(tCFG *cfg, tDATA *data, tGNUTERM *gt, double *ymin, double *ymax) {
	int i;
	int c;
	double (*getData)(tDATA *data, int i)=getLSD;
	
	/* scan gt.fmt string and determine calibration of first y column */
	for (c = strlen((*gt).fmt)-1; c>=0; c--) {
		switch ((*gt).fmt[c]) {
		case 'd':				/* LSD */
			getData = getLSD;
			break;
		case 'D':				/* PSD */
			getData = getPSD;
			break;
		case 's':				/* LS */
			getData = getLS;
			break;
		case 'S':				/* PS*/
			getData = getPS;
			break;
		default:
			break;
		}
	}
	
	/* scan data and return minimum and maximum value for first y column */
	*ymin=1e300; *ymax=-1e300;
	for (i=0;i<(*cfg).nspec; i++) {
		if (getData(data, i)<*ymin) *ymin=getData(data, i);
		if (getData(data, i)>*ymax) *ymax=getData(data, i);
	}
}

/*
	write general information
	parse and write gnuplot commands

	%x xtics determined by lpsd
	%y ytics determined by lpsd
	%f input data file
	%g input file name without extension
	%o output file name
	%p output file name without extension
	%s paramater string from lpsd command line

*/
void writeGnuplotFile(tCFG *cfg, tDATA *data, tGNUTERM *gt, tWinInfo *wi, int argc, char *argv[]) {
	FILE *gfp;
	char cmt[CMTLEN];
	char ifnb[FNLEN];
	char ofnb[FNLEN];
	char xtics[TICLEN];
	char ytics[TICLEN];
	double ymin, ymax;
	
	gfp = fopen((*cfg).gfn, "w");
	if (0 == gfp)
		gerror1("Error opening", (*cfg).gfn);
	
	/* write general information */
	writeComment(&cmt[0], cfg, wi, gt, data, argc, argv);
	fprintf(gfp,"%s",cmt);

	/* replace %x, %y, %f %g, %o, %p, %s in gnuplot command string */
	basename((*cfg).ifn, ifnb);			/* copy input file basename to ifnb */
	basename((*cfg).ofn, ofnb);			/* copy output file basename to ofnb */
	rplStr(&(*gt).cmds[0],"%f",(*cfg).ifn);
	rplStr(&(*gt).cmds[0],"%g",ifnb);
	rplStr(&(*gt).cmds[0],"%s",(*cfg).param);
	rplStr(&(*gt).cmds[0],"%o",(*cfg).ofn);
	rplStr(&(*gt).cmds[0],"%p",ofnb);

	maketics(xtics, 'x', (int) floor(log10((*cfg).fmin)) - 1, ceil((int) log10((*cfg).fmax)) + 1);
	getSpan(cfg, data, gt, &ymin, &ymax);
	maketics(ytics, 'y', (int) floor(log10(ymin)) - 1, ceil((int) log10(ymax)) + 1);
	rplStr(&(*gt).cmds[0],"%x",xtics);
	rplStr(&(*gt).cmds[0],"%y",ytics);
	/* write gnuplot commands */
	fprintf(gfp,"%s",(*gt).cmds);
	
	fclose(gfp);
}

void saveResult(tCFG * cfg, tDATA * data, tGNUTERM * gt, tWinInfo *wi, int argc, char *argv[])
{
	double ymin, ymax;
	
	/* write output file with colums specified in gt */
	writeOutputFile(cfg, data, gt, wi, argc, argv);

	/* find minimum and maximum of first y column */
	getSpan(cfg, data, gt, &ymin, &ymax);
	
	/* write gnuplot file */
	writeGnuplotFile(cfg, data, gt, wi, argc, argv);
}
