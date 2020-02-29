/*
	parses s for occurances of %f %g %s %C and replaces
	them with their corresponding contents
*/
void parse_fgsC(char *s, char *ifn, char *par, unsigned int A, unsigned int B);
/*
	parses s for occurances of %o %p %s and replaces
	them with their corresponding contents
*/
void parse_op(char *s, char *ofn);
/*
	returns 1 if s contains '%' character, 0 otherwise
*/
int VarInStr(char *s);

/*
	copies base filename of s to base
	
	base needs to provide sufficient space to hold the basename of s
*/
void basename(char *s, char *base);

/*
	replaces all occurances of var in s by cont
	returns 0 on success, -1 if var not found in s
*/
int rplStr(char *s, char *var, char *cont);

