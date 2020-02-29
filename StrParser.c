/*

	parse a string for the following variables and replace them by their content

# string variables available within this file
#
# %x xtics determined by lpsd
# %y ytics determined by lpsd
# %f input data file
# %g input file name without extension
# %o output file name
# %p output file name without extension
# %C column number(s) to be processed
# %s paramater string from lpsd command line

# identifiers for output file format
#
# f	frequency
# d	linear spectral density
# D	power spectral density
# s	linear spectrum
# S	power spectrum
# N	number of averages
# A	standard deviation or variance, depending on dDsS
# R	resolution bandwidth
# b	bin number

*/
#include <stdio.h>
#include <string.h>
#include "StrParser.h"
#include "config.h"

/*
	returns 1 if s contains '%' character, 0 otherwise
*/
int VarInStr(char *s) {
	int p;
	p=(int)strchr(s,'%');
	return(p!=0);
}

/*
	replaces all occurances of var in s by cont
	returns 0 on success, -1 if var not found in s
*/
int rplStr(char *s, char *var, char *cont) {
	char *tmp,*rest;
	char rslt[CMTLEN];
	int error=0;
	
	tmp=strstr(s,var);
	if (tmp==0) {				/* no occurance of var in s */
		error=-1;
		strcpy(rslt,s);
	} else {				/* occurance of var found */
		rest=&tmp[strlen(var)];		/* remaining string after var */
		rplStr(rest, var, cont);	/* replace var by cont in rest */
		memcpy(&rslt[0],&s[0],tmp-s);	/* copy characters before var to rslt */
		rslt[tmp-s]=0;			
		strcat(rslt,cont);		/* append cont to rslt */
		strcat(rslt,rest);		/* append rest to result */
	}
	strcpy(s,rslt);
	return(error);
}

/*
	copies base filename of s to base
	
	base needs to provide sufficient space to hold the basename of s
*/
void basename(char *s, char *base) {
	char fc[FNLEN];
	char *fn;

	strcpy(fc, s);
	fn = strrchr(fc, '.');
	if (fn != 0)
		*fn = 0;
	strcpy(base, fc);
} 

/*
	parses s for occurances of %f %g %s %C and replaces
	them with their corresponding contents
*/
void parse_fgsC(char *s, char *ifn, char *par, unsigned int A, unsigned int B) {
	char ifnb[FNLEN];
	char C[FNLEN];

	basename(ifn, ifnb);			/* copy input file basename to ifnb */
	if (B==0) sprintf(C,"%d",A);
	else sprintf(C,"%d-%d",B,A);
	rplStr(s,"%f",ifn);
	rplStr(s,"%g",ifnb);
	rplStr(s,"%C",C);
	rplStr(s,"%s",par);
}

/*
	parses s for occurances of %o %p %s and replaces
	them with their corresponding contents
*/
void parse_op(char *s, char *ofn) {
	char ofnb[FNLEN];

	basename(ofn, ofnb);			/* copy output file basename to ofnb */
	rplStr(s,"%o",ofn);
	rplStr(s,"%p",ofnb);
}

/*
int main() {
	char ifn[FNLEN] = "my.cal.txt";
	char ofn[FNLEN] = "%g-lpsd-%g.txt%g";
	char ifnb[FNLEN];
	
	basename(ifn,ifnb);
	printf("ifn=%s\n",ifn);
	printf("ifnb=%s\n",ifnb);
	printf("ofn=%s\n",ofn);
	printf("VarInStr(ofn)=%d\n",VarInStr(ofn));
	rplStr(ofn,"%g",ifnb);
	printf("ofn=%s\n",ofn);
	printf("VarInStr(ofn)=%d\n",VarInStr(ofn));
	return(0);

}
*/
