#ifndef __myps_h__
#define __myps_h__
 /**
  * myps
  * 
 */

/*------------------------------------------------------------------------------
  Constantes
  ----------------------------------------------------------------------------*/
#define ERREUR      -1
#define FIN_FICHIER -5
#define BUFFER_FILE 100
/* environ le nb de caractere a lire pour avoir les infos dans status */


#define AFF_PID   1
#define AFF_TTY   2
#define AFF_PPID  4 
#define AFF_STAT  8
#define AFF_CMD   16
#define AFF_TIME  32 
#define AFF_CPU   64
#define AFF_MEM   128
#define AFF_USER  256
#define AFF_START 512
#define AFF_   11123



#define OPTION_ALL     1
#define OPTION_PID     2
#define OPTION_PPID    4
#define OPTION_TTY     8
#define OPTION_STOP    128

/*------------------------------------------------------------------------------
  Types personnalisés
  ----------------------------------------------------------------------------*/
typedef struct
{
      int fd;
      char buffer[BUFFER_FILE];
      int pos_buffer;
}Fichier;

typedef struct
{
      int pid;
      char comm[255];
      char state;
      int ppid, pgrp, session, tty, tpgid;
      unsigned int flags, minflt, cminflt, majflt, cmajflt;
      int utime, stime, cutime, cstime;
      int priority;
      long int nice;
}Stat_ps;


typedef struct
{
      Stat_ps *tab;
      int lg;
} List_stat;



/*------------------------------------------------------------------------------
  Prototypes de fonctions
  ----------------------------------------------------------------------------*/

extern int main (int, char **);

/*------------------------------------------------------------------------------
  Variables globales
  ----------------------------------------------------------------------------*/

extern int aff_ps;

#endif
