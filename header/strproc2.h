#ifndef _strproc2__h_
#define _strproc2__h_

#ifdef __cplusplus
extern "C"{
#endif

typedef struct _string{
	char **strings;
	int size;
}STRING;

char *GetString(const char *format,...);

STRING Split(char *originStr,char sign);
void DeleteString(STRING *pStr);


#ifdef __cplusplus
}
#endif
#endif
