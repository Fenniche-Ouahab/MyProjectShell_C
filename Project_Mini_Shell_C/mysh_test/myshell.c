#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "myshell.h"
#include "common.h"
#include "erreur.h"
#include "dyn_string.h"
//#include "myls.h"
//#include "myps.h"
#include "list_fg.h"


// Ce programme permet de simuler le fonctionnement d'un shell

#define syserror(x) perror(errormsg[x])
#define fatalsyserror(x) syserror(x), exit(x)
#define ROUGE(m) "\033[01;31m"m"\033[0m"
#define VERT(m) "\033[01;32m"m"\033[0m"


typedef enum
{
   CMD_EXEC,
   CMD_EXIT,
   CMD_MYPWD,
   CMD_MYJOBS,
   CMD_MYFG,
   CMD_CD,
   CMD_SETENV,
   CMD_SET,
   CMD_MYLS,
   CMD_MYPS,
   CMD_STATUS
} commandes;

// Structure pour la commande status
typedef struct {
	int proc;
	int value;
	int failed;
} StatusProc;

StatusProc statusproc;

char * errormsg[] = {
	"No error",
	ROUGE("Impossible to fork process"),
	ROUGE("Exec failed"),
	ROUGE("Pipe failed")
};

// Utiliser lors du traitement de la ligne de commande dans le fils
// Prend en parametre un pointeur de tableau de char correspond a la commande ainsi que ses arguments
// Et le nombre d'arguments pour celle-ci.
void traiterCmdInFork(char *cmd[SIZE],int *nbcmd, int *nbpipe, int *nbpipetraiter, int p[MAXPIPE][2], int *redirection, char *chemin){
	char *tab[SIZE], **ps;
	int i=0, fd;
	for(;i<*nbcmd;i++,cmd++){
		tab[i]=*cmd;
	}
	*nbcmd=0;


	if(*nbpipe>0){
		printf("NBPIPE:%d NBPT:%d\n",*nbpipe,*nbpipetraiter);
		if(!*nbpipetraiter){
			//Cas premiere commande
			printf("debut\n");
			printf("Ferme 0 dup in\n");
			close(p[0][0]);
			dup2(p[0][1],STDOUT_FILENO);
			close(p[0][1]);
			for(i=1;i<*nbpipe;i++){
				printf("close: %d\n",i);
				close(p[i][0]);
				close(p[i][1]);
			}
		} else if(*nbpipetraiter==*nbpipe) {
			//Cas derniere commande
			printf("DER\n");
			for(i=0;i<*nbpipetraiter-1;i++){
				printf("close:%d\n",i);
				close(p[i][0]);
				close(p[i][1]);
			}
			printf("close dup in:%d\n",*nbpipetraiter-1);
			close(p[*nbpipetraiter-1][1]);
			dup2(p[*nbpipetraiter-1][0],STDIN_FILENO);
			close(p[*nbpipetraiter-1][0]);
		} else {
			//de deuxieme commande a l'avant derniere commande
			printf("MID\n");
			for(i=0;i<*nbpipetraiter-1;i++){
				printf("close:%d\n",i);
				close(p[i][0]);
				close(p[i][1]);
			}
			printf("close dup in:%d\n",*nbpipetraiter-1);
			close(p[*nbpipetraiter-1][1]);
			dup2(p[*nbpipetraiter-1][0],STDIN_FILENO);
			close(p[*nbpipetraiter-1][0]);
			printf("close dup out:%d\n",*nbpipetraiter);
			close(p[*nbpipetraiter][0]);
			dup2(p[*nbpipetraiter][1],STDOUT_FILENO);
			close(p[*nbpipetraiter][1]);
			for(i=*nbpipetraiter+1;i<*nbpipe;i++){
				printf("close:%d\n",i);
				close(p[i][0]);
				close(p[i][1]);
			}
		}
	}

	switch ( *redirection) {
		case 1:
			fd = open(chemin, O_RDWR|O_CREAT, 0666);
			close(STDOUT_FILENO);
			dup(fd);
			break;
		case 2:
			fd = open(chemin, O_RDWR|O_CREAT|O_APPEND,0666);
			close(STDOUT_FILENO);
			dup(fd);
			break;
		case 3:
			fd = open(chemin, O_RDWR|O_CREAT, 0666);
			close(STDERR_FILENO);
			dup(fd);
			break;
		case 4:
			fd = open(chemin, O_RDWR|O_CREAT|O_APPEND,0666);
			close(STDERR_FILENO);
			dup(fd);
			break;
		case 5:
			fd = open(chemin, O_RDWR|O_CREAT, 0666);
			close(STDOUT_FILENO);
			dup(fd);
			close(STDERR_FILENO);
			dup(fd);
			break;
		case 6:
			fd = open(chemin, O_RDWR|O_CREAT, 0666);
			close(STDOUT_FILENO);
			dup(fd);
			close(STDERR_FILENO);
			dup(fd);
			break;
		case 7:
			fd = open(chemin, O_RDONLY);
			close(STDIN_FILENO);
			dup(fd);
			break;

		default:

			break;
	}

	execvp(*tab, tab);
	syserror(2);
	exit(FAILEDEXEC);
}


// Permet de gerer les commandes non traité dans un fork grace execvp
// Par exemple cd, status ,...
// Prend en parametre un pointeur de tableau de char correspond a la commande ainsi que ses arguments

int traiterInFather(char *cmd[SIZE]){
	char currentdir[SIZE];
	if(!strncmp(*cmd,"cd",2)){
		chdir(*(cmd+1));
		getcwd(currentdir, SIZE);
		return 1;
	} else if(!strncmp(*cmd,"status",6)){
		if (!statusproc.proc){
			printf("Aucune commande execute avant\n");
		} else if(!statusproc.failed){
			printf("%d termine avec comme code retour %d\n",statusproc.proc,statusproc.value);
		} else {
			printf("%d termine anormalement\n",statusproc.proc);
		}
		return 1;
	} else if(!strncmp(*cmd,"exit",4)){
		//a ameliorer
		exit(0);
		return 1;
	} else if(!strncmp(*cmd,"myls",4)){
		//programme a part qu'on arrive pas a lancer ici
		printf("le programme a été réaliser dans le dossier myls_test, on arrive pas a le lancer ici! \n");
		
		exit(0);
		return 1;
	} else if(!strncmp(*cmd,"myps",4)){
		//programme a part qu'on arrive pas a lancer ici
		printf("le programme a été réaliser dans le dossier myps_test, on arrive pas a le lancer ici! \n");
		exit(0);
		return 1;
	}
	return 0;
}

void func_status(int p,int value,int failed){
	statusproc.proc=p;
	statusproc.value=value;
	statusproc.failed=failed;
}

void exit_shell(int sigv){
	char answer[SIZE];
	if(sigv==SIGINT){
		//a ameliorer
		printf("\nVoulez-vous quitter? (oui ou non):");
		fgets(answer,SIZE-1,stdin);
		answer[strlen(answer)-1]='\0';
		if(!strncmp(answer,"oui",3)){
			exit(0);
		}
	}
}


/**
 * Affiche la liste des tâches en background
 * param argc : nombre d'arguments
 * param **argv : tableau des arguments


int myjobs (int argc, char **argv)
{
   list_fg_aff (list);
   exit (0);
}
 */

int main(void) {
	char lgcmd[SIZECMDLINE], command[SIZE], *tabcmd[SIZE], currentdir[SIZE], *chemin, *newcmd, *s,**ps;
	unsigned char *tmp;
	pid_t p;
	int i, j, nbcar, nbcmdprev,status, waitstatus, waitprev, nbargcmd, nbpipe, nbpipetraiter, redirection, tube[MAXPIPE][2];
	nbcmdprev=0;

	signal(SIGINT, exit_shell);

	for(;;) {
		getcwd(currentdir, SIZE);
		printf("%s>",currentdir);
		memset(lgcmd,0,SIZECMDLINE);
		fgets(lgcmd, SIZECMDLINE-1, stdin);
		for(s=lgcmd; isspace(*s); s++);
		s[strlen(s)-1]='\0'; //retire \n a la fin de la ligne de commande
		//printf("Command line:%s\n",s);
		nbargcmd=0;
		nbpipe=0;
		nbpipetraiter=-1;
		nbcar=0;
		status=0;
		waitprev=0;
		waitstatus=0;
		redirection=0;
		i=0;
		newcmd=s;
		for(;*s;s++,nbcar++){
			//on avance jusqua fin d'une commande
			if(*s==';' || *(s+1)=='\0' || *s=='|' || *s=='&' || *s=='>' || (*s=='2' && *(s+1)=='>')|| *s=='<'){
				redirection=0;
				if(*s=='2' && *(s+1)=='>') {
					*s++='\0';
					*s++='\0';
					if(*s=='>'){ //cas 2>>
						*s++='\0';
						redirection=4;
					} else { //cas 2>
						redirection=3;
					}
					s--;
				} else if(*s=='>'){
					*s++='\0';
					if(*s=='>'){ //cas >>
						*s++='\0';
						if(*s=='&'){//cas >>&
							redirection=6;
						} else {
							s--;
							redirection=2;
						}
					} else if(*s=='&'){//>&
						redirection=5;
					} else { //cas >
						s--;
						redirection=1;
					}
				} else if(*s=='<'){
					redirection=7;
				} else if(*s=='|' && *(s+1)!='|'){ // Cas du pipe
          if(pipe(tube[nbpipe]) == -1){
						fatalsyserror(3);
					}
					nbpipe++;
				} else if(*s=='|' && *(s+1)=='|'){ //cas ||
					*s++='\0';
					waitprev=2;
				} else if(*s=='&' && *(s+1)=='&'){ //cas &&
					*s++='\0';
					waitprev=1;
				} else if(*(s+1)=='\0'){
					s++;
					nbcar++;
				}
				memset(command,0,strlen(command));
				for (j = 0; j < nbcmdprev; j++) {
					memset(tabcmd[j],0,strlen(tabcmd[j]));
				}
				memcpy(command,newcmd,nbcar);
				*s='\0';
				for(tmp=command; isspace(*tmp); tmp++);
				for(i=0; *tmp && tmp != (unsigned char *)s; i++) {
					tabcmd[i]=tmp;
					nbargcmd++;
					//stopper le while quand on rencontre un ; ou & ou |
					while (!isspace(*tmp) && *tmp!='\0' && *tmp!=';' && *tmp!='|' && *tmp!='&') {
						tmp++;
					}
					//traiter &
					if(*tmp=='|'){
						waitprev=2;
						*tmp++='\0';
						break;
					} else if(*tmp==';'){
						waitprev=0;
						*tmp++='\0';
						break;
					} else if(*tmp=='&'){
						waitprev=1;
						*tmp++='\0';
						break;
					} else if(*tmp!='\0'){
						*tmp++='\0';
					}
					while (isspace(*tmp)) {
						tmp++;
					}
				}
				s++;
				if(redirection){ // on recup le chemin
					while(isspace(*s)){
						s++;
					}
					chemin=s;
					while (!isspace(*s) && *s!='\0' && *s!=';' && *s!='|' && *s!='&') {
						s++;
					}
					// s--;
					*s='\0';
          if(*s=='\0' && !isspace(*(s+1))){
            s++;
          }
					while(isspace(*s) || *s=='&' || *s=='|' || *s==';'){
						s++;
					}
				} else {
					chemin=NULL;
				}
				newcmd=s;
				nbcar=0;
				nbcmdprev=i;
				nbpipetraiter++;
				//printf("PIPE:%d, traiter:%d\n",nbpipe,nbpipetraiter);
				if(!traiterInFather(tabcmd)){
					p=fork();
					if (p == ERR) fatalsyserror(1);
					if(p) {
						wait(&status);
						if(WIFEXITED(status)) {
							if ((status=WEXITSTATUS(status)) != FAILEDEXEC) {
								for(i=0;i<nbpipe;i++){
									close(tube[i][1]);
								} if(waitprev==1){
									waitstatus = -1;
								} else if(waitprev==2) {
									waitstatus = -2;
								} else {
									waitstatus = 0;
								}
								printf(VERT("exit status of ["));
								for (ps = tabcmd; *ps; ps++){
									if(**ps)
										printf("%s ", *ps);
								}
								printf(VERT("\b]=%d\n"), status);
								//printf("\n");
								func_status(p,status,0);
							}
						}
						else {
							func_status(p,-1,1);
							puts(ROUGE("Abnormal exit"));
						}
						nbargcmd=0;
					} else {
						printf("Command fork:");
						for (ps = tabcmd; *ps; ps++) printf("%s ", *ps);
						printf("\n" );
						if((waitstatus==-1 && status==0) || (waitstatus==-2 && status!=0) ||(waitstatus==0 && status==0)){

							traiterCmdInFork(tabcmd, &nbargcmd, &nbpipe, &nbpipetraiter, tube, &redirection, chemin);

						} else {
							//stop le fils quand on ne doit pas executer commande
							exit(DONTEXEC);
						}
					}
				}
			}
		}
	}
	exit(0);
}
