#include "myls.h"

void display_file_recursively(struct FileList *file_list, int count, int mode)
/*
* afficher pour l'option -R.
* NOTE: pour plusieurs répertoires, utiliser le chemin absolu.
 */
{
        char path[MAX_PATH_LEN] = { 0 };
        char temp[MAX_PATH_LEN] = { 0 };
        char filename[MAX_FILENAME_LEN] = { 0 };
        struct FileList list[MAX_FILE_COUNT];
        int i = 0, size = 0;

        puts(get_current_dir_name()); /* le chemin absolu */
        strcpy(path, get_current_dir_name());

        if(mode & DETAIL)
                display_file_detail(file_list, count);
        else
                display_file_simply(file_list, count);
        printf("\n");

        for(i = 0; i < count; ++i) {
                strcpy(filename, file_list[i].name);

                /* REMARQUE: "." et ".." est un répertoire, sautez-les! * /
                if((strcmp(filename, ".") == 0) || (strcmp(filename, "..") == 0))
                        continue;
                if(S_ISDIR(file_list[i].info.st_mode)) { /* stocker le chemin absolu */
                        strcpy(temp, path);
                        strcat(path, "/");
                        strcat(path, filename); /*stocker le chemin absolu*/
                        size = get_file_list(path, list, mode);
                        display_file_recursively(list, size, mode);
                        strcpy(path, temp);
                }
        }
}

void display_file_detail(struct FileList *file_list, int count)
/*
 * afficher pour l'option -l.
 */
{
        char *uid_to_name(), *ctime(), *gid_to_name(), *filemode();
        char modestr[11];
        struct stat *info_p;
        int i;

        for(i = 0; i < count; ++i) {
                info_p = &file_list[i].info;

                file_mode_to_string(info_p->st_mode, modestr);

                printf("%s",      modestr);
                printf("%4d ",    (int)info_p->st_nlink);
                printf("%-8s ",   uid_to_name(info_p->st_uid));
                printf("%-8s ",   gid_to_name(info_p->st_gid));
                printf("%8ld ",   (long)info_p->st_size);
                printf("%.12s ",  4 + ctime(&info_p->st_mtime));

                if(S_ISDIR(file_list[i].info.st_mode)) /* dossiers   afficher en bleu  */
                        printf("\e[34m%s", file_list[i].name);
                else                                   /* autre afficher en vert */
                        printf("\e[92m%s", file_list[i].name);
                printf("\e[39m\n"); /* default blanc */
        }
}

void display_file_simply(struct FileList *file_list, int count)
/*
* afficher sans l'option -R ou -l.
 */
{
        struct winsize size;
        int max_name_length = 0, i;
        int len = 0;
        int cols ; /*colonnes séparées */

        for(i = 0; i < count; ++i) {
                len = strlen(file_list[i].name);
                if( len > max_name_length)
                        max_name_length = len;
        }

        /* Obtenir la taille du terminal avec ioctl (2) */
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
        cols = size.ws_col;
        cols /= (max_name_length + 1);

        for(i = 0; i < count; ++i) {
                if(i != 0 && (i % cols == 0))
                        puts(" ");
                /*
				 * utilise "% * s": la précision n'est pas spécifiée dans le format
                 * chaîne, mais en tant qu'argument de valeur entière supplémentaire
                 * précédant l'argument à formater.
                 */
                if(S_ISDIR(file_list[i].info.st_mode))
                        printf("\e[34m%*s", -(max_name_length + 1),
                                file_list[i].name);
                else
                        printf("\e[92m%*s", -(max_name_length + 1),
                                file_list[i].name);
        }
        printf("\e[39m\n");
}
