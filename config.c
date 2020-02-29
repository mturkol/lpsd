/********************************************************************************
			config.c
			
  configure lpsd at runtime via a textfile
  
  under linux, use the command
  	export LPSDCFN=/your/dir/psdconfigfilename
  under DOS, use the command
  	set LPSDCFN=c:\your\dir\psdconfigfilename

  what needs to be configured?
	DEFFT		default input file type
	DEFFSAMP	default sampling frequency
	DEFWT		default window type
	DEFPSLL		default peak side lobe level
	DEFULSB		default scaling factor for gnuplot
	DEFLR		default linear regression setting

  structure of config file
	keyword	value 	for numbers
	keyword "value" for strings
  	text after # are comments
	
 ********************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "config.h"
#include "errors.h"
#include "string.h"
#include "misc.h"

#define LPSDCFN "LPSDCFN"		/* lpsd config file name */

static char *CFN;			/* config file name */

static tGNUTERM gt[MAXGNUTERM];		/* gnuplot terminals */
static int gti=0;			/* current index to gnuplot terminals */

typedef struct {
	char *token;
	void (*action) (char*);
} tPARSEPAIR;

static FILE *CFP=0;

static void act_ifn(char*);
static void act_ofn(char*);
static void act_gfn(char *s);
static void act_wfn(char *s);
static void act_wt(char *s);
static void act_nspec(char *s);
static void act_lr(char *s);
static void act_fsamp(char *s);
static void act_fres(char *s);
static void act_ovlp(char *s);
static void act_reqpsll(char *s);
static void act_ulsb(char *s);
static void act_desavg(char *s);
static void act_minavg(char *s);
static void act_METHOD(char *s);
static void act_tmin(char *s);
static void act_tmax(char *s);
static void act_fmin(char *s);
static void act_fmax(char *s);
static void act_sbin(char *s);
static void act_time(char *s);
static void act_colA(char *s);
static void act_colB(char *s);
static void act_format(char *s);
static void act_gnuterm(char *s);

static tPARSEPAIR pplist [] = {
	{"IFN",		act_ifn},
	{"OFN",		act_ofn},
	{"GFN",		act_gfn},
	{"WFN",		act_wfn},
	{"WT",		act_wt},
	{"NSPEC",	act_nspec},
	{"LR",		act_lr},
	{"FSAMP",	act_fsamp},
	{"FRES",	act_fres},
	{"OVLP",	act_ovlp},
	{"PSLL",	act_reqpsll},
	{"ULSB",	act_ulsb},
	{"desAVG",	act_desavg},
	{"minAVG",	act_minavg},
	{"METHOD",	act_METHOD},
	{"TMIN",	act_tmin},
	{"TMAX",	act_tmax},
	{"FMIN",	act_fmin},
	{"FMAX",	act_fmax},
	{"SBIN",	act_sbin},
	{"TIME",	act_time},
	{"COLA",	act_colA},
	{"COLB",	act_colB},
	{"FORMAT",	act_format},
	{"GNUTERM",	act_gnuterm}
};

static const int npplist = sizeof (pplist) / sizeof (tPARSEPAIR);

static tPARSEPAIR gtlist [] = {
	{"FORMAT",	act_format}
};

static const int ngtlist = sizeof (gtlist) / sizeof (tPARSEPAIR);

static tCFG cfg={usedefs:0,
		ifn:DEFIFN,
		askifn:1,
		ofn:DEFOFN,
		askofn:1,
		gfn:DEFGFN,
		wfn:DEFWFN,
		param:"",
		askgfn:0,
		WT:DEFWT,
		askWT:1,
		LR:DEFLR,
		nspec:DEFNSPEC,
		asknspec:1,
		fsamp:DEFFSAMP,
		askfsamp:1,
		cmdfsamp:0,
		reqPSLL:DEFPSLL,
		askreqPSLL:1,
		ovlp:DEFOVLP,
		askovlp:1,
		cmdovlp:0,
		ulsb:DEFULSB,
		askulsb:1,
		tmin:DEFTMIN,
		asktmin:1,
		tmax:DEFTMAX,
		asktmax:1,
		fmin:DEFFMIN,
		askfmin:1,
		fmax:DEFFMAX,
		askfmax:1,
		fres:-1,
		askfres:0,
		cmdfres:0,
		desAVG:DEFDESAVG,
		askdesAVG:0,
		cmddesAVG:0,
		minAVG:DEFMINAVG,
		askminAVG:0,
		cmdminAVG:0,
		sbin:DEFSBIN,
		asksbin:0,
		METHOD:DEFMETHOD,
		askMETHOD:0,
		cmdMETHOD:0,
		ngnuterm:0,
		askgt:1,
		gt:0,
		time:1,
		asktime:0,
		colA:1,
		askcolA:1,
		colB:0,
		askcolB:0};

void getConfig(tCFG *c) {
	memcpy(c,&cfg,sizeof(cfg));

}

void getGNUTERM(int i, tGNUTERM *dest) {
	memcpy(dest,&gt[i],sizeof(gt[i]));
}

/* returns 0 if token is not contained at beginning of s, !0 otherwise */
static int isToken(char *s, char *token) {
	return (int)strstr(s,token);
}

/* 
	copies string contained in s to dest 
	returns an empty string, if no string is contained in s
*/
static void getStringValue(char *dest, char *s) {
	char *start;
	char *stop;
	start=strchr(s,'"')+1;
	stop=strrchr(s,'"');
	if ((start!=0) && (stop!=0)) {
		strncpy(dest,start,(int)(stop-start));
		dest[(int)(stop-start)]=0;
	} else dest[0]=0;
}

/* returns the integer value defined in string s */
static int getIntValue(char *s) {
	char *start;
	start=strchr(s,' ')+1;
	return atoi(start);
}

/* returns the double value defined in string s */
static double getDBLValue(char *s) {
	char *start;
	start=strchr(s,' ')+1;
	return atof(start);
}

/*
static void act_nop(char *s __attribute__ ((unused))) {}
*/

static void act_ifn(char *s) {
	getStringValue(&cfg.ifn[0],s);
	if (s[0]=='?') cfg.askifn=1;
	else cfg.askifn=0;
}

static void act_ofn(char *s) {
	getStringValue(&cfg.ofn[0],s);
	if (s[0]=='?') cfg.askofn=1;
	else cfg.askofn=0;
}

static void act_gfn(char *s) {
	getStringValue(&cfg.gfn[0],s);
	if (s[0]=='?') cfg.askgfn=1;
	else cfg.askgfn=0;
}

static void act_wfn(char *s) {
	getStringValue(&cfg.wfn[0],s);
}

static void act_wt(char *s) {
	cfg.WT=getIntValue(s);
	if (s[0]=='?') cfg.askWT=1;
	else cfg.askWT=0;
}

static void act_nspec(char *s) {
	cfg.nspec=getIntValue(s);
	if (s[0]=='?') cfg.asknspec=1;
	else cfg.asknspec=0;
}

static void act_lr(char *s) {
	cfg.LR=getIntValue(s);
}

static void act_fsamp(char *s) {
	cfg.fsamp=getDBLValue(s);
	if (s[0]=='?') cfg.askfsamp=1;
	else cfg.askfsamp=0;
}

static void act_fres(char *s) {
	cfg.fres=getDBLValue(s);
	if (s[0]=='?') cfg.askfres=1;
	else cfg.askfres=0;
}

static void act_ovlp(char *s) {
	cfg.ovlp=getDBLValue(s);
	if (s[0]=='?') cfg.askovlp=1;
	else cfg.askovlp=0;
}

static void act_reqpsll(char *s) {
	cfg.reqPSLL=getDBLValue(s);
	if (s[0]=='?') cfg.askreqPSLL=1;
	else cfg.askreqPSLL=0;
}

static void act_ulsb(char *s) {
	cfg.ulsb=getDBLValue(s);
	if (s[0]=='?') cfg.askulsb=1;
	else cfg.askulsb=0;
}

static void act_desavg(char *s) {
	cfg.desAVG=getIntValue(s);
}

static void act_minavg(char *s) {
	cfg.minAVG=getIntValue(s);
}

static void act_sbin(char *s) {
	cfg.sbin=getIntValue(s);
}

static void act_time(char *s) {
	cfg.time=getIntValue(s);
	if (s[0]=='?') cfg.asktime=1;
}

static void act_colA(char *s) {
	cfg.colA=getIntValue(s);
	if (s[0]=='?') cfg.askcolA=1;
}

static void act_colB(char *s) {
	cfg.colB=getIntValue(s);
	if (s[0]=='?') cfg.askcolB=1;
}

static void act_METHOD(char *s) {
	cfg.METHOD=getIntValue(s);
	if (s[0]=='?') cfg.askMETHOD=1;
	else cfg.askMETHOD=0;
}

static void act_tmin(char *s) {
	cfg.tmin=getDBLValue(s);
	if (s[0]=='?') cfg.asktmin=1;
	else cfg.asktmin=0;
}

static void act_tmax(char *s) {
	cfg.tmax=getDBLValue(s);
	if (s[0]=='?') cfg.asktmax=1;
	else cfg.asktmax=0;
}

static void act_fmin(char *s) {
	cfg.fmin=getDBLValue(s);
	if (s[0]=='?') cfg.askfmin=1;
	else cfg.askfmin=0;
}

static void act_fmax(char *s) {
	cfg.fmax=getDBLValue(s);
	if (s[0]=='?') cfg.askfmax=1;
	else cfg.askfmax=0;
}

static void act_format(char *s) {
	getStringValue(&gt[gti].fmt[0],s);
}

static void process_line(char *s) {
	strncat(gt[gti].cmds,s,CLEN);
}

static void remove_comment(char *s) {
	int i=0;
	while ((s[i]!='#') && (s[i]!='\0')) i++;
	s[i]='\0';
}

static void act_gnuterm(char *s) {
	int i;
	int TR=0;			/* 1 if token is recognized */

	gt[gti].cmds[0]='\0';
	getStringValue(&gt[gti].identifier[0],s);
	if (strlen(gt[gti].identifier)==0) gerror1("\tMissing identifier in line %s",s);
	while ((0!=fgets(s,CMTLEN,CFP)) && (!isToken(s,"END"))) {	/* read line */
		TR=0;
		i=0;
		if (s[0]!='#') {
			remove_comment(s);
			do {
				if (isToken(s,gtlist[i].token)) {
					TR=1;
					gtlist[i].action(s);
				}
				i++;
			} while ((!TR) && (i<ngtlist));
			if (TR==0) {	/* copy gnuplot command to gnuplot terminal */
				process_line(s);
			}
		}
	}
	if (isToken(s,"END")) {
		gti++;
		cfg.ngnuterm=gti;
	}	
}

/********************************************************************************
 * 	reads configuration from config file					*
 *	returns									*
 *	  0 if config file could not be read					*
 *	  1 on success								*
 ********************************************************************************/
int readConfigFile() {
	int ok=0,i;
	char s[CMTLEN];
	int TR=0;			/* 1 if token is recognized */
	strcpy(gt[0].identifier,"LSD");

	CFN=getenv(LPSDCFN);
	gti=0;				/* number of gnuplot terminals defined so far */
	printf("Reading configuration file %s\n",CFN);
	if (CFN!=0) {		/* if environment variable was set */
		CFP=fopen(CFN,"r");
		if (CFP!=0) {
			while (0!=fgets(s,CMTLEN,CFP)) {	/* read line */
				if ((s[0]!='#') && (s[0]!='\n')) {
					remove_comment(s);
					TR=0;
					i=0;
					do {
						if (isToken(s,pplist[i].token)) {
							TR=1;
							pplist[i].action(s);
						}
						i++;
					} while ((!TR) && (i<npplist));
					if (TR==0) printf("No keyword recognized: %s",s);
				}
			}
			fclose(CFP);
			ok=1;
		} else printf("Error reading config file - using standard settings.\n\n");
	} else printf("Environment variable LPSDCFN is not defined - using standard settings.\n\n");
	if (gti==0) {				/* no gnuplot terminal has been defined at all */
		gti=1;
		strcpy(gt[0].identifier,"standard gnuplot terminal");
		strcpy(gt[0].fmt,"fDUSVRbN"); //was "fdusvN", then "fdusvRbN"
		strcpy(gt[0].cmds,"");
		/*
        # f frequency
        # d linear spectral density
        # D power spectral density
        # s linear spectrum
        # S power spectrum
        # N number of averages
        # u standard deviation of linear spectral density
        # U variance of power spectral density
        # v standard deviation of linear spectrum
        # V variance of power spectrum
        # R resolution bandwidth
        # b bin number
        */
	}
	return(ok);
}

static void printFiles(char *dest, tCFG cfg) {
	sprintf(&dest[0],	    "\n===Configuration===========================================================\n");
	sprintf(&dest[strlen(dest)],"---Files-------------------------------------------------------------------\n");
	sprintf(&dest[strlen(dest)],"input: %s\t",cfg.ifn);
	sprintf(&dest[strlen(dest)],"output: %s\t",cfg.ofn);
	sprintf(&dest[strlen(dest)],"gnuplot: %s\n",cfg.gfn);

}

static void printWindow(char *dest, tCFG cfg, tWinInfo wi) {
	sprintf(&dest[strlen(dest)],"---Window function---------------------------------------------------------\n");
	sprintf(&dest[strlen(dest)],"Name:  %s\t",wi.name);
	sprintf(&dest[strlen(dest)],"PSLL (dB): %.2f\t",wi.psll);
	sprintf(&dest[strlen(dest)],"Overlap (%%): %.2f\n",cfg.ovlp);
	sprintf(&dest[strlen(dest)],"NENBW (bins): %.2f\t",wi.nenbw);
	sprintf(&dest[strlen(dest)],"W3dB (bins): %.2f\t",wi.w3db);
	sprintf(&dest[strlen(dest)],"Flatness (dB): %.2f\n",wi.flatness);
	sprintf(&dest[strlen(dest)],"SBIN (bins): %.2f\n",wi.sbin);
}	

static void printData(char *dest, tCFG cfg, tDATA data) {
	char yn[2][SLEN]={"no","yes"};

sprintf(&dest[strlen(dest)],"---Data--------------------------------------------------------------------\n");
	sprintf(&dest[strlen(dest)],"Regression: %s\t",yn[cfg.LR]);
	sprintf(&dest[strlen(dest)],"Fsamp (Hz): %.2e\t",cfg.fsamp);
	sprintf(&dest[strlen(dest)],"NDATA (samples): %d\n",data.ndata);
	sprintf(&dest[strlen(dest)],"Tmin (s): %.1f\t",cfg.tmin);
	sprintf(&dest[strlen(dest)],"Tmax (s): %.1f\t",cfg.tmax);
	sprintf(&dest[strlen(dest)],"Read (samples): %d\n",data.nread);
	sprintf(&dest[strlen(dest)],"Mean: %.2e\t",data.mean);
	sprintf(&dest[strlen(dest)],"Scaling factor: %.3e\t",cfg.ulsb);
	sprintf(&dest[strlen(dest)],"Time column: %s\n",yn[cfg.time]);
	if (cfg.colB==0) sprintf(&dest[strlen(dest)],"Column: %d\n",cfg.colA);
	else sprintf(&dest[strlen(dest)],"Columns: %d-%d\n",cfg.colB,cfg.colA);
}

static void printOutput(char *dest, tCFG cfg, tGNUTERM gt, tDATA data) {
	char meth[2][SLEN]={"LPSD","FFTW"};
	int avg;

	avg=floor((data.nread-cfg.nfft)/(cfg.ovlp/100.)/cfg.nfft+1);
sprintf(&dest[strlen(dest)],"---Output------------------------------------------------------------------\n");
	sprintf(&dest[strlen(dest)],"Size: %d\t\t",cfg.nspec);
	sprintf(&dest[strlen(dest)],"Fmin (Hz): %.1e\t",cfg.fmin);
	sprintf(&dest[strlen(dest)],"Fmax (Hz): %.1e\n",cfg.fmax);
	sprintf(&dest[strlen(dest)],"SBIN (bins): %.2f\t",cfg.sbin);
	if (cfg.METHOD==0) {
		sprintf(&dest[strlen(dest)],"min. avgs: %d\t\t",cfg.minAVG);
		sprintf(&dest[strlen(dest)],"des. avgs: %d\n",cfg.desAVG);
	} else if (cfg.METHOD==1) {
		sprintf(&dest[strlen(dest)],"avgs: %d\t\t",avg);
		sprintf(&dest[strlen(dest)],"Fres (Hz): %.1e\n",cfg.fres);
	}
	sprintf(&dest[strlen(dest)],"Gnuplot terminal: %s\n",gt.identifier);
	sprintf(&dest[strlen(dest)],"Method: %s\t\t",meth[cfg.METHOD]);
	sprintf(&dest[strlen(dest)],"\n");
	sprintf(&dest[strlen(dest)],"===========================================================================\n");

}

/*
	prints the current configuration to string dest
	dest must provide sufficient space for information
*/
void printConfig(char *dest, tCFG cfg, tWinInfo wi, tGNUTERM gt, tDATA data) {
	
	printFiles(dest, cfg);
	printWindow(dest, cfg, wi);
	printData(dest, cfg, data);
	printOutput(dest, cfg, gt, data);
}

/*
int main(int argc, char *argv[]) {
	CFN=getenv(PSDCFN);
	if (CFN==0) printf("%s not found\n",PSDCFN);
	printf("Reading from config file %s.\n",CFN);
	
	
	return(0);
}
*/
