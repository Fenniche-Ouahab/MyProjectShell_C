 /**
  * myps
  * 
 */

/*------------------------------------------------------------------------------
  Fichiers include
  ----------------------------------------------------------------------------*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>

#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>

#include "myps.h"
#include "output_ps.h"

#define LG_PROC 5 /*  =  longueur de la chaine "/proc" */
#define LG_STAT 5 /*  =  longueur de la chaine "/stat" */

#define VIDE -1

#define OPTION_ALL     1
#define OPTION_PID     2
#define OPTION_PPID    4
#define OPTION_TTY     8
#define OPTION_STOP    128


#define OPT_HELP (0-2)
#define OPT_VERSION (0-3)
#define OPT_PPID (0-4)


/*------------------------------------------------------------------------------
  Types personalisés
  ----------------------------------------------------------------------------*/
/* Structure des options de myps */
typedef struct
{
      /*  paramettres des options */
      int tty;
      int pid; 
      int ppid;
	  int euid;

} Param_ps;



/*------------------------------------------------------------------------------
  Variables globales
  ----------------------------------------------------------------------------*/


static Param_ps params; /* contient les param des options */
unsigned int options_ps; /* Masque des paramètres */

static char const optstring[] = "Aeuxp:t:V";
/*
 * x : liste le processus n'ayant pas de terminal associé. Ils sont repérés par ``?'' sous la colonne TT, en particulier les démons système et les processus rattachés à init (proc-num=1).
 * u : spécifie le nom du propriétaire du processus.
 * a : liste les processus détenus par les autres utilisateurs.
 * t : liste uniquement les processus attachés à un terminal particulier (ex:ps -axtco).
 */

static struct option const long_options[] =
{
   {"help",           no_argument, NULL, OPT_HELP}, 
   {"version",        no_argument, NULL, 'V'}, 
   {"all",            no_argument, NULL, 'A'}, // -A identique a -e d'apres (man ps)
   {"pid",            no_argument, NULL, 'p'}, // identique a -p et --pid
   {"tty",            no_argument, NULL, 't'}, // identique a -t et --tty
   {"euid",            no_argument, NULL, 'u'}, // - u selection par euid ou nom, identique a U et --user (effective user Id (alias uid)
												// x register format (affiche les processus qui n'ont pas de terminal de contrôle). 
   {"ppid",            no_argument, NULL, OPT_PPID},
   {NULL, 0, NULL, 0}
};

int aff_ps;



/*------------------------------------------------------------------------------
  Ecriture des fonctions
  ----------------------------------------------------------------------------*/


/**
 * Remplit le buffer de caractères.
 * param *f : le fichier à lire
 */
void remplit_buffer (Fichier *f)
{
   if(f->fd != ERREUR)
   {
      if ( read (f->fd, f->buffer, BUFFER_FILE) == ERREUR)
         printf ("EXIT_FIC_READ\n");
      f->pos_buffer = 0;
   }
}


/**
 * Ouvre un fichier en lecture.
 * param *nom_fichier : la chaine de caractère correspondant au fichier à ouvrir
 * return : une structure de Fichier qui contient le descripteur du fichier
 *          ainsi qu'un premier buffer remplit.
 */
Fichier ouvre_fichier_lecture (char *nom_fichier)
{
   Fichier f;
   f.fd = open (nom_fichier, O_RDONLY);
   if (f.fd == ERREUR)
      printf("EXIT_FIC_OPEN_R\n");
   else
      remplit_buffer (&f);
   return f;
}


/**
 * Ferme un fichier précédement ouvert.
 * param *f : le Fichier à fermer
 */
void ferme_fichier (Fichier *f)
{
   if(f->fd != ERREUR)
      if (close (f->fd)) printf("EXIT_FIC_CLOSE\n");
}


/** 
 * 
 * param *d_name : chaine de caractere contenant un PID
 * return : le nom non-relatif du fichier de stat
 */
char* concat_nom_rep(char* d_name)
{
   int lg;
   char* temp = NULL;
   int lng_d_name = strlen(d_name);

   lg = LG_PROC + 1 + lng_d_name + 1 + LG_STAT  ; /* = /proc/<d_name>/stat  */
   temp = (char *) malloc ( (lg + 1 ) * sizeof(char) );
   snprintf(temp, lg , "%s/%s/%s", "/proc", d_name, "stat");
   temp [lg] = '\0';

   return temp;
}

/**
 * Récupère les valeurs contenues dans un fichier stat
 * param *d_name : chaine de caractere contenant un PID
 * return : une structure Stat_ps remplie
 */
Stat_ps scan_stat(char* d_name)
{
   Fichier fic_stat;
   Stat_ps stat;

   d_name = concat_nom_rep(d_name);
   fic_stat = ouvre_fichier_lecture (d_name);
   sscanf(fic_stat.buffer,
          "%d "
          "%s "
          "%c "
          "%d %d %d %d %d" /* state, ppid, pgrp, session, tty, tpgid */
          "%u %u %u %u %u" /* flags, minflt, cminflt, majflt, cmajflt */
          "%d %d %d %d "   /* utime, stime, cutime, cstime */
          "%d %ld", /* priority, nice */
          &stat.pid,
          stat.comm,
          &stat.state, &stat.ppid, &stat.pgrp, &stat.session, &stat.tty, &stat.tpgid,
          &stat.flags, &stat.minflt, &stat.cminflt, &stat.majflt, &stat.cmajflt,
          &stat.utime, &stat.stime, &stat.cutime, &stat.cstime,
          &stat.priority, &stat.nice
      );
   ferme_fichier (&fic_stat);
   free(d_name);

   return stat;
}

/**
 * return : 1 si la chaine passé en parametre est un chiffre, 0 sinon
 */ 
char is_a_number(char* ch)
{
   int i;
   char bool = 1;
   for( i = strlen(ch) ; i ; i-- )
      if  (ch[i-1] < '0' || ch[i-1] > '9' )
         bool = 0;

   return bool;
}


/**
 * Parcours le répertoire /proc afin de récupérer et d'afficher, pour chaque 
 * répertoire appartenant à un processus (où le nom est un chiffre) et qui 
 * correspond aux requetes effectuées en paramétres, ses propriétes.
 */
void open_proc()
{
   struct dirent * objet=NULL;
   DIR * rep ;
   Stat_ps stat;

   /* Tentative d'ouverture du répertoire */
   rep = opendir ("/proc");
   if (rep == NULL)
   {
      fprintf (stderr,"myps: Probleme à l'ouverture du dossier proc \n");
      exit (1);
   }
     
   while ( (objet = readdir (rep)) )
   {
      /* tant qu on a un chiffre, c'est un rep de processus */
      if (is_a_number(objet->d_name))
      {
         stat = scan_stat (objet->d_name);

         /* test si selon option il faut ajouter a la liste a afficher */
         if ( (options_ps & OPTION_ALL) 
              || ( ( (options_ps & OPTION_TTY) || !(options_ps & OPTION_PID) ) ? (params.tty == (stat.tty&(0xff))) : 0 )  
              || ( (options_ps & OPTION_PID) ? (params.pid == (stat.pid)) : 0 )   
            )
            affiche_ps_stat (stat);
      }
   }

/* Fermeture du répertoire */
   closedir (rep);
}


/**
 * retourne le numéro du TTY de ce processus, afin de reconnaitre et afficher 
 * les processus du même TTY de myshell
 */
int get_tty_local()
{
   Stat_ps stat;
   char* d_name= malloc (sizeof(char)*6);
   int pid = getpid();
   sprintf(d_name, "%u", pid);
   concat_nom_rep(d_name);
   stat = scan_stat(d_name);
   free (d_name);
   
   return stat.tty&(0xff);
}

/**
 * Initialise les parametres
 */ 
void init_params_ps()
{
   params.tty  = get_tty_local();
   params.pid  = VIDE;
   params.ppid = VIDE;
}


/**
 * récupére les parametres et place les masks de selection ou d'affichage selon ceux-ci
 */
int myps (int argc, char **argv)
{
   int c;
   int option_index = 0;
   options_ps = 0;
   init_params_ps();
   aff_ps = 0;
   aff_ps |=  AFF_PID | AFF_TTY | AFF_TIME | AFF_CMD;

   for (;;)
   {
      c = getopt_long (argc, argv, optstring,
                       long_options, &option_index);
      if (c == -1)
         break;

      switch (c)
      {
         case 'A':
         case 'e':
            options_ps |=  OPTION_ALL;
            break;

         case 'p':
            if (is_a_number(optarg))
            {
               options_ps |=  OPTION_PID;
               sscanf(optarg, "%d", &params.pid);
            }
            else
               erreur_bad_arg("le pid doit etre un entier");
            break;
            
         case 't':
            if (is_a_number(optarg))
            {
               options_ps |=  OPTION_TTY;
               sscanf(optarg, "%d", &params.tty);;
            }
            else
               erreur_bad_arg("tty doit etre un entier");
            break; 

         case OPT_HELP:
            options_ps |= OPTION_STOP;
            break;
                    
         case 'V':
            options_ps |= OPTION_STOP;
            version_ps();
            break;
		 case 'u': 
			break;
         case 'x': 
			break;

         case '?':
            options_ps |= OPTION_STOP;
		printf("\n");
            erreur_bad_arg("Option non reconue");
            break;
      }
   }

   if (optind < argc)
   {
      printf ("Arguments ne constituant pas des options: ");
      while (optind < argc)
         printf ("%s ", argv[optind++]);
      printf ("\n");
   }


   if (options_ps & OPTION_STOP)
      usage_ps();
   else
   {
      affiche_ps ();
      open_proc ();
   }
   
      

   exit (0);
}
