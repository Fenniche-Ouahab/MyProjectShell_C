#ifndef __varglob_h__
#define __varglob_h__

extern void varglob_init ();
extern void mysetenv (int argc, char **argv);
extern int mysetenv2 (int argc, char ** argv);
extern char * varglob_get (char * nom);
extern void varglob_free ();


#endif
