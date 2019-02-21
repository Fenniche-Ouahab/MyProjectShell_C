#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

#define ONECHAR '?'

// fonction qui vérifie si c'est une lettre de l'alphabet , soit minuscule ou majuscule 
int isLetter(char l)
{
    return (((l >= 'a') && (l <= 'z')) || ((l >= 'A') && (l <= 'Z')));
}

//  fonction qui  vérifie si c'est un chiffre 
int isDigit(char d)
{
    return ( (d >= '0') && (d <= '9'));
}


int sizeCharList(char *list)
{
    char *tmp = NULL;
    for (tmp = list; *tmp; tmp++);
    return tmp - list;
}

// 
char *strcatCharList(char *list_in, char caract1, char caract2)
{
    char tmp;
    int i = 1;
    if (list_in)
    {
        i = sizeCharList(list_in) + 1;
    }
    if (caract1 > caract1)
        return list_in;
    tmp = caract1;
    for (;;)
    {
        if ((isLetter(tmp)) || (isDigit(tmp)))
        {
            i++;
            list_in = (char *)realloc(list_in, i * sizeof(char));
            *(list_in + (i - 2)) = tmp;
            *(list_in + (i - 1)) = '\0';
        }
        tmp++;
        if (tmp > caract1)
            break;
    }
    return list_in;
}

// le caractere est dans la liste
int charInList(char *liste, char caractere)
{
    char *tmp;
    for (tmp = liste; *tmp != '\0'; tmp++)
    {
        if (*tmp == caractere)
            return 1;
    }
    return 0;
}

// fonction qui gère les wildcards 
// prend en param une expression reguliere et le mot a rechercher
int wildcards(char *expreg, char *mot)
{
    char *tmp1 = NULL, *tmp2 = NULL, *tmpp = NULL;
    char *liste_cars = NULL;
    int not = 0;

    for (tmp1 = expreg, tmp2 = mot; (*tmp1 != '\0') && (*tmp2 != '\0'); tmpp = tmp1, tmp1++, tmp2++)
    {
        if ((*tmp1 == '*') && ((!tmpp) || ((tmpp) && (*tmpp != '\\'))))
        {
            for (;;)
            {
                if (*(tmp1) == '*')
                    tmp1++;
                else
                    break;
            }

            for (; *tmp2 != '\0'; tmp2++)
            {
                if (wildcards(tmp1, tmp2))
                    return 1;
            }
            if ((*tmp1 == '\0') && (*tmp2 == '\0'))
                return 1;
            return 0;
        }
        else if ((*tmp1 == '[') && ((!tmpp) || ((tmpp) && (*tmpp != '\\'))))
        {
            tmp1++;
            if (*tmp1 == '^')
            {
                tmp1++;
                not = 1;
            }
            for (;;)
            {
                if ((!isDigit(*tmp1)) && (!isLetter(*tmp1)))
                    return 0;
                if (*(tmp1 + 1) == '-')
                {
                    if ((!isDigit(*(tmp1 + 2))) && (!isLetter(*(tmp1 + 2))))
                        return 0;
                    liste_cars = strcatCharList(liste_cars, *tmp1, *(tmp1 + 2));
                    tmp1 += 2;
                }
                else
                    liste_cars = strcatCharList(liste_cars, *tmp1, *tmp1);
                tmp1++;
                if (*tmp1 == ']')
                    break;
            }

            if (!not)
            {
                if (!charInList(liste_cars, *tmp2))
                {
                    free(liste_cars);
                    return 0;
                }
            }
            else
            {
                if (charInList(liste_cars, *tmp2))
                {
                    free(liste_cars);
                    return 0;
                }
            }
            free(liste_cars);
            not = 0;
        }
        else if (*tmp1 == '\\')
        {
            tmp1++;
            if (*tmp1 != *tmp2)
                return 0;
            if (*tmp1 == '\0')
                tmp1--;
        }
        else if (*tmp1 != *tmp2)
        {

            if (*tmp1 != ONECHAR)
                return 0;
        }
    }
    while (*tmp1 == '*')
        tmp1++;
    return ((*tmp1 == '\0') && (*tmp2 == '\0'));
}

// fonction qui vérifie si c'est un dossier , prend en param un chemin , et le nom de dossier
int isFolder(char *path, char *name)
{
    DIR *rep;
    char *tmp = NULL;

    if (path)
    {
        tmp = (char *)calloc((strlen(path) + strlen(name) + 1), sizeof(char));
        tmp = strcpy(tmp, path);
        tmp = strcat(tmp, name);
    }
    else
    {
        tmp = (char *)calloc((strlen(name) + 1), sizeof(char));
        tmp = strcpy(tmp, name);
    }
    if ((rep = opendir(tmp)) == 0)
    {
        return 0;
    }
    closedir(rep);

    return 1;
}



char **listValidFiles(char *path, char *expregu, char *prefixe, char **liste)
{
    struct dirent *lecture;
    DIR *rep;
    int n = 0, j = 0;
    char *mot = NULL, *tmp = NULL, *newpath = NULL;
    char *expreg = NULL, *exptmp = NULL;


    expreg = strdup(expregu);
    exptmp = expreg;

    //alloc liste si NULL
    if (!liste)
    {
        liste = (char **)malloc(sizeof(char *));
        *liste = NULL;
    }

    //regard si il faut entrer dans repertoire
    newpath = (char *)malloc(sizeof(char));
    newpath[0] = '\0';

    //si ca commence par  ~, on remplace le symbole par le chemin correspondant

    if ((!path) && (expreg[0] == '~'))
    {
        
        char *tmpo = NULL;

        if (expreg[1] == '\0')
          
        path = (char *)calloc(strlen(getenv("HOME")) + 2, sizeof(char));
        tmpo = (char *)calloc(strlen(expreg + 1) + 1 + strlen(getenv("HOME")), sizeof(char));
        tmpo = strcpy(tmpo, getenv("HOME"));
        tmpo = strcat(tmpo, expreg + 1);
        path = strcpy(path, getenv("HOME"));
        path = strcat(path, "/");

        prefixe = (char *)calloc(strlen(path) + 1, sizeof(char));
        prefixe = strcpy(prefixe, path);

        expreg = tmpo;

        expreg = expreg + 11;
    }

    //parcours de l'expression régulière jusqu'à trouver '/' ou '\0'
    for (tmp = expreg; ((*tmp != '\0') && (*tmp != '/')); j++, tmp++)
    {
        newpath = (char *)realloc(newpath, (j + 2) * sizeof(char));
        newpath[j] = *tmp;
        newpath[j + 1] = '\0';
    }

    //cas ou on trouve rien alors qu'on a tt parcouru

    if ((!strcmp(newpath, "\0")) && (path) && (*tmp == '\0'))
    {
        if (*liste)
        {
            return liste;
        }
        else
        {
            if (prefixe)
            {
                mot = (char *)calloc(strlen(expreg) + strlen(prefixe) + 1, sizeof(char));
                mot = strcpy(mot, prefixe);
            }
            else
                mot = (char *)calloc(strlen(expreg) + 1, sizeof(char));

            mot = strcat(mot, expreg);
            liste = (char **)realloc(liste, (n + 2) * sizeof(char *));
            liste[n] = mot;
            liste[n + 1] = NULL;
            free(prefixe);
            free(path);
            prefixe = NULL;
            path = NULL;

            return liste;
        }
    }

    //quand c'est un repertoire

    if ((*tmp == '/') && (strcmp(expreg + strlen(newpath), "/")))
    {

        //update path et prefixe
        if ((!path) && (!strcmp(newpath, "\0")))
        {
            path = (char *)calloc(2, sizeof(char));
            prefixe = (char *)calloc(2, sizeof(char));
            path[0] = '/';
            path[1] = '\0';
            prefixe[0] = '/';
            prefixe[1] = '\0';
            expreg++;
            return listValidFiles(path, expreg, prefixe, liste);
        }

        if ((rep = opendir((path) ? path : ".")) == 0)
        {

            exit(5);
        }

        //parcours du répertoire pour trouver un fichier ou répertoire qui permet de continuer l'expression régulière
        while ((lecture = readdir(rep)))
        {

            if ((wildcards(newpath, lecture->d_name)) && (isFolder(path, lecture->d_name)))
            {
                //cas ou on a trouver un rep
                 if ((strcmp(newpath, "*")) || ((!strcmp(newpath, "*")) && (strcmp(lecture->d_name, ".")) && (strcmp(lecture->d_name, ".."))))
                {
                       int p = 0, q = 0;

                    //mise à jour du prefixe et le reste ppur lautre appel
                    if (!prefixe)
                    {
                        prefixe = (char *)realloc(prefixe, (3 + strlen(lecture->d_name)) * sizeof(char));
                        if (tmp == expreg)
                        {
                            prefixe = strcpy(prefixe, "/");
                        }
                        else
                            prefixe = strcpy(prefixe, "\0");
                    }
                    else
                    {
                        prefixe = (char *)realloc(prefixe, (strlen(prefixe) + 2 + strlen(lecture->d_name)) * sizeof(char));
                        p = strlen(prefixe);
                    }

                    prefixe = strcat(prefixe, lecture->d_name);

                    prefixe = strcat(prefixe, "/");

                    if (!path)
                    {

                        path = realloc(path, (4 + strlen(lecture->d_name)) * sizeof(char));
                        path = strcpy(path, "./");

                        path = strcat(path, lecture->d_name);

                        q = 2;
                    }
                    else
                    {
                        path = realloc(path, (strlen(path) + 2 + strlen(lecture->d_name)) * sizeof(char));
                        q = strlen(path);
                        path = strcat(path, lecture->d_name);
                    }

                    path = strcat(path, "/");

                    //on rappelle la fonction avec le path modifier

                    char *npath = NULL;
                    char *npref = NULL;

                    //sauvegarde les path et du prefixe
                    npath = (char *)calloc(strlen(path) + 1, sizeof(char));
                    npref = (char *)calloc(strlen(prefixe) + 1, sizeof(char));

                    npath = strcpy(npath, path);
                    npref = strcpy(npref, prefixe);

                    //nouvel appel à la fonction
                    liste = listValidFiles(npath, expreg + (strlen(newpath) + 1), npref, liste);

                    free(prefixe);
                    prefixe = NULL;

                    free(path);
                    path = NULL;
                    free(exptmp);
                }
            }
        }

        //return resultat
        if (*liste)
        {
            if (prefixe)
                free(prefixe);
            prefixe = NULL;
            if (rep)
                free(rep);
            if (lecture)
                free(lecture);
            if (newpath)
                free(newpath);

            return liste;
        }
        else
        {

            if (prefixe)
            {
                mot = (char *)calloc(strlen(expreg) + strlen(prefixe) + 1, sizeof(char));
                mot = strcpy(mot, prefixe);
                free(prefixe);
                prefixe = NULL;
            }
            else
                mot = (char *)calloc(strlen(expreg) + 1, sizeof(char));
            mot = strcat(mot, expreg);
            liste[n] = mot;
            liste[n + 1] = NULL;
            if (rep)
                free(rep);
            if (lecture)
                free(lecture);
            if (newpath)
                free(newpath);
            
            return liste;
        }
    }

    //cas où exp reg sans symbole /

    if ((rep = opendir((path) ? path : ".")) == 0)
    {
        exit(5);
    }

    //parcourt repertoire tant quon ne la pas explorer entierement

    if (!strcmp(expreg + strlen(newpath), "/"))
        expreg[strlen(newpath)] = '\0';
    char **tmp2 = NULL;
    if (liste)
    {
        for (tmp2 = liste; *tmp2; tmp2++, n++);
    }

    while ((lecture = readdir(rep)))
    {

        //si ca correspond, on le met dans la liste liste

        if ((wildcards(expreg, lecture->d_name)) && (strcmp(lecture->d_name, "..")) && (strcmp(lecture->d_name, ".")))
        {

            if (!prefixe)
                mot = (char *)calloc(strlen(lecture->d_name) + 1, sizeof(char));
            else
            {
                mot = (char *)calloc(strlen(lecture->d_name) + 1 + strlen(prefixe), sizeof(char));
                mot = strcpy(mot, prefixe);
            }

            mot = strcat(mot, lecture->d_name);

            liste = (char **)realloc(liste, (n + 2) * sizeof(char *));
            liste[n] = mot;
            liste[n + 1] = NULL;
            n++;
        }
    }

    if (rep)
        free(rep);
    if (lecture)
        free(lecture);
    if (newpath)
        free(newpath);

    //retourne la liste dont il ya tt les elements qui correspond, ou l'exprégulière si elle existe pas
    if (*liste)
    {
        free(prefixe);
        free(path);
        prefixe = NULL;
        path = NULL;
        free(exptmp);
        return liste;
    }
    else
    {

        if (prefixe)
        {

            mot = (char *)calloc(strlen(expreg) + strlen(prefixe) + 1, sizeof(char));
            mot = strcpy(mot, prefixe);
        }else
            mot = (char *)calloc(strlen(expreg) + 1, sizeof(char));
        mot = strcat(mot, expreg);
        liste = (char **)realloc(liste, (n + 2) * sizeof(char *));
        liste[n] = mot;
        liste[n + 1] = NULL;
        free(prefixe);
        free(path);
        prefixe = NULL;
        path = NULL;
        free(exptmp);
        return liste;
    }
}
