#include <stdlib.h>
#include <string.h>
#ifdef __linux__
	#include <argp.h>
#else
	#include "argp.h"
#endif
#include "errors.h"
#include "config.h"

const char *argp_program_version = LPSD_VERSION;
const char *argp_program_bug_address = "<m.troebs@gmx.de>";

/* A description of the arguments we accept. */
static char args_doc[] = "";

/* Program documentation. */
static char doc[] = "lpsd - a program to calculate spectral estimates by Michael Troebs\
 and Gerhard Heinzel, (c) 2003, 2004.\nImprovements on accuracy, bug-fixes and\
 corrections in algorithm flow were made by Mert Turkol, 2019, 2020.\
\vlpsd can be controlled by command line options or interactively";

/* The options we understand. */
static struct argp_option options[] = {
	{"davg",    'a', "# of des. avgs", 0, "desired number of averages", 			0},
	{"colA",    'A', "# of column",    0, "number of column to process",			0},
	{"tmin",    'b', "tmin", 0, "start time in seconds",					0},
	{"colB",    'B', "# of column",    0, "process column B - column A",			0},	
	{"param",   'c', "param", 0, "parameter string",					0},
	{"usedefs", 'd', 0,       0, "use defaults",						0},
	{"tmax",    'e', "tmax", 0, "stop time in seconds", 					0},
	{"fsamp",   'f', "sampl. freq.", 0, "sampling frequency in Hertz",			0},
	{"gnuplot", 'g', "gnuplot file",  0, "gnuplot file name",				0},
	{"method",  'h', "0, 1",   0, "method for frequency calculation: 0-LPSD, 1-FFT",	0},
	{"input",   'i', "input file",  0, "input file name",					0},
	{"fres",    'j', "FFT freq. res.", 0, "Frequency resolution for FFT", 			0},
	{"sbin",    'k', "sbin", 0, "smallest frequency bin",					0},
	{"ovlp",    'l', "overlap", 0, "segment overlap in %",	 				0},
	{"mavg",    'm', "# of min. avgs", 0, "minimum number of averages", 			0},
	{"nspec",   'n', "# in spectr.", 0, "number of values in spectrum", 			0},
	{"output",  'o', "output file",  0, "output file name",					0},
	{"psll",    'p', "psll", 0, "peak side lobe level in dB", 				0},
	{"quiet",   'q', 0,       0, "Don't produce output on screen",				0},
	{"lr",      'r', "0,1",   0, "linear regression; 1 yes, 0 no", 				0},
	{"fmin",    's', "fmin", 0, "start frequency in spectrum", 				0},
	{"fmax",    't', "fmax", 0, "stop frequency in spectrum", 				0},
	{"time",    'T', 0, 0, "file contains time in s in first column",			0},	
	{"gnuterm", 'u', "gnuterm", 0, "number of gnuplot terminal", 				0},
	{"window",  'w', "wind. func.", 0, "window function; -2 Kaiser, -1 flat top, 0..30",  	0},
	{"scale",   'x', "factor", 0,"scaling factor",						0},
	{0,0,0,0,0,0}
};

/* Parse a single option. */
static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
  	/* Get the input argument from argp_parse, which we
	   know is a pointer to our arguments structure. */
	tCFG *arguments = state->input;

	switch (key) {
	case 'q':
	#ifndef __WIN32__ 
		stdout = fopen("/dev/null", "r");
	#endif
		break;
	case 'T':
		arguments->time=1;
		arguments->asktime=0;
		break;
	case 'A':
		arguments->colA=atoi(arg);
		arguments->askcolA=0;
		break;
	case 'B':
		arguments->colB=atoi(arg);
		arguments->askcolB=0;
		break;
	case 'g':
		strcpy(arguments->gfn,arg);
		break;
	case 'c':
		strcpy(arguments->param,arg);
		break;
	case 'i':
		strcpy(arguments->ifn,arg);
		arguments->askifn=0;
		break;
	case 'j':
		arguments->fres=atof(arg);
		arguments->askfres=0;
		arguments->cmdfres=1;
		break;
	case 'o':
		strcpy(arguments->ofn,arg);
		arguments->askofn=0;
		break;
	case 'r':
		arguments->LR=atoi(arg);
		break;
	case 'd':
		arguments->usedefs=1;
		break;
	case 'w':
		arguments->WT=atoi(arg);
		arguments->askWT=0;
		break;
	case 'n':
		arguments->nspec=atoi(arg);
		arguments->asknspec=0;
		break;
	case 'a':
		arguments->desAVG=atoi(arg);
		arguments->cmddesAVG=1;
		break;
	case 'm':
		arguments->minAVG=atoi(arg);
		arguments->cmdminAVG=1;
		break;
	case 'h':
		arguments->METHOD=atoi(arg);
		arguments->askMETHOD=0;
		arguments->cmdMETHOD=1;
		break;
	case 'u':
		arguments->gt=atoi(arg);
		arguments->askgt=0;
		break;
	case 'l':
		arguments->ovlp=atof(arg);
		arguments->askovlp=0;
		arguments->cmdovlp=1;
		break;
	case 'p':
		arguments->reqPSLL=atof(arg);
		arguments->askreqPSLL=0;
		break;
	case 'b':
		arguments->tmin=atof(arg);
		arguments->asktmin=0;
		break;
	case 'e':
		arguments->tmax=atof(arg);
		arguments->asktmax=0;
		break;
	case 's':
		arguments->fmin=atof(arg);
		arguments->askfmin=0;
		break;
	case 't':
		arguments->fmax=atof(arg);
		arguments->askfmax=0;
		break;
	case 'k':
		arguments->sbin=atof(arg);
		arguments->asksbin=0;
		break;
	case 'f':
		arguments->fsamp=atof(arg);
		arguments->askfsamp=0;
		arguments->cmdfsamp=1;
		break;
	case 'x':	
		arguments->ulsb=atof(arg);
		arguments->askulsb=0;
		break;
		
    	case ARGP_KEY_END:
      		break;

	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

void parseArgs(int argc, char *argv[], tCFG *cfg)
{
	/* Parse our arguments; every option seen by parse_opt will be
	   reflected in cfg */
	argp_parse(&argp, argc, argv, 0, 0, cfg);

}

