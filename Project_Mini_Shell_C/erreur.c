/**
 * erreur
 * 
 */


/*------------------------------------------------------------------------------
  Fichiers include
  ----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

#define MESSAGE_OK
#include "erreur.h"


/*------------------------------------------------------------------------------
  Ecriture des fonctions
  ----------------------------------------------------------------------------*/
void erreur_sys (Erreurs code_retour)
{
   perror (ErreursMsg[code_retour]);
   exit (code_retour);
}
