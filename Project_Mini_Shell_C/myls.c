#include "myls.h"

/*
 * Implémentation simple de la commande linux ls.
 *
 * myls prend en charge les arguments de ligne de commande suivants:
 * -t - trier par date de modification
 * -r - sortie inversée
 * -a - ne masque pas les entrées commençant par "."
 * -R - liste les sous-répertoires de manière récursive
 * -l - utilise un format de liste longue
 * supporte la combinaison de ces options comme -traRl
 * Prend également en charge les couleurs pour distinguer dossier et fichier

*/

static const char *optString = "traRl";

int myls(int argc, char *argv[])
/*
 * Utilisez getopt () pour accéder à l'option de commande.
 * vous pouvez utiliser plusieurs options une fois comme:
 * myls -atlrR
 */
{
        int i;
        int mode = 0;       /* par default aucun param */
        int opt = 0;        /* obtenir des options de commande */
        int file_count = 0; /* obtenir le nombre de fichiers d'un répertoire specefic */

        while((opt = getopt(argc, argv, optString)) != -1) {
                switch(opt) {
                case 't': mode |= SORT_BY_MTIME; break;
                case 'r': mode |= REVERSE;       break;
                case 'a': mode |= ALL;           break;
                case 'R': mode |= RECURSIVE;     break;
                case 'l': mode |= DETAIL;        break;
                default:
                        exit(EXIT_FAILURE);
                }
        }

        if(optind == argc) { /* répertoire courant */
                file_count = get_file_list(".", (struct FileList *)&file_list, mode);
                display((struct FileList *)&file_list, file_count, mode);
        }
        else {              /* spécifier un ou plusieurs répertoires */
                for(i = optind; i < argc; ++i) {
                        if( optind + 1 != argc) /* plus d'un repertoire */
                                printf("%s: \n", argv[i]);
                        file_count = get_file_list(argv[i],
                                                   (struct FileList *)&file_list,
                                                   mode);
                        display((struct FileList *)&file_list, file_count, mode);
                }
        }

        return EXIT_SUCCESS;
}

int get_file_list(char dirname[], struct FileList *file_list, int mode)
/*
 * Stocker tous les fichiers sous le répertoire dans file_list,
 * les trier alphabétiquement par défaut.
 * et retourne le nombre de fichiers.
*/
{
        DIR           *dir_pointer;  /* le répertoire*/
        struct dirent *dirent_ptr;   /* chaque entrée */
        int count = 0;
        char filename[MAX_FILENAME_LEN];

        if((dir_pointer = opendir(dirname)) == NULL) {
                fprintf(stderr, "ls: can not open %s\n", dirname);
                exit(EXIT_FAILURE);
        }
        else {
                /*
		 * change le répertoire de travail, car stat () accède au chemin relatif
                 */
                chdir(dirname);

                /* collecter le nom du fichier et les informations */
                while((dirent_ptr = readdir(dir_pointer)) != NULL) {
                        strcpy(filename, dirent_ptr->d_name);

                        if(filename[0] == '.' && !(mode & ALL))
                                continue;
                        strcpy(file_list[count].name, filename);

                        /*stats le fichier et remplit les informations. */
                        if(stat(filename, &file_list[count].info) == -1)
                                perror(filename);

                        ++count;
                }

                qsort(file_list, count, sizeof(file_list[0]), name_cmp);

                if(mode & SORT_BY_MTIME)
                        qsort(file_list, count, sizeof(file_list[0]), mtime_cmp);

                if(mode & REVERSE)
                        reverse_file_list(file_list, count);

                closedir(dir_pointer);
        }
        return count;
}

void display(struct FileList *file_list, int count, int mode)
/*
 * afficher le fichier avec le mode spécifié (option de commande).
 */
{
        if(mode & RECURSIVE)
                display_file_recursively(file_list, count, mode);
        else if(mode & DETAIL)
                display_file_detail(file_list, count);
        else
                display_file_simply(file_list, count);
}
