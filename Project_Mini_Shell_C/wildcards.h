#ifndef __myps_h__
#define __myps_h__

int isLetter(char l);
int isDigit(char d);
int sizeCharList(char *list);
char *strcatCharList(char *list_in, char caract1, char caract2);
int charInList(char *liste, char caractere);
int wildcards(char *expreg, char *mot);
int isFolder(char *path, char *name);
char **listValidFiles(char *path, char *expregu, char *prefixe, char **liste);


#endif
