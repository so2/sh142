#include <stdio.h>
#include "defines.h"
#include "functions.h"
#include <errno.h>
#define MAXLINE 4096

extern int errno;

typedef void (*sighandler_t)(int);

static char *my_argv[100], *my_envp[100];
static char *search_path[10];
static char *default_env;

void handleUserCommand()
{
 //       if( chk_pipe()==1)
   //     {

        if (checkBuiltInCommands() == 0) {
		//printf("%d",commandArgc);
		// puts("chkbuiltin works");

                launchJob(commandArgv, "STANDARD", 0, FOREGROUND);
                              //puts(commandArgv[1]);
        }
}

 //int check_pipe()
 //{
 void pipelining(int flag)
 {
    char    line[MAXLINE];
    FILE    *fpin, *fpout;
    char arg1[50],arg2[50];
    int i;
   if(flag==2)
   {
	    strcat(arg1,commandArgv[0]);
	    strcpy(arg2,commandArgv[2]);
   }
    else
   {
	   for(i=0;i<flag-1;i++)
	   {
		   strcat(arg1,commandArgv[i]);
		   strcat(arg1," ");
	   }
	   strcpy(arg2,commandArgv[commandArgc-1]);
   }
  // puts(commandArgv[0]);
   //puts(arg1);
   //puts(arg2);

    if ((fpin = popen(arg1, "r")) == NULL)
        printf("can't open %s", arg1);

    if ((fpout = popen(arg2, "w")) == NULL)
        printf("popen error");


    while (fgets(line, MAXLINE, fpin) != NULL) {
        if (fputs(line, fpout) == EOF)
            printf("fputs error to pipe");
    }
    if (ferror(fpin))
        printf("fgets error");
    if (pclose(fpout) == -1)
       printf("");


    //return 1;
}


int checkBuiltInCommands()
{
        if (strcmp("exit", commandArgv[0]) == 0) {
                setenv("PATH",default_env,1);
		exit(EXIT_SUCCESS);
        }
        if (strcmp("cd", commandArgv[0]) == 0) {

                changeDirectory();
                return 1;
        }
        if (strncmp("PATH=", commandArgv[0], 5) == 0) {

                putenv(commandArgv[0]);
                return 1;
        }
        if (strncmp("DATA=", commandArgv[0], 5) == 0) {

                putenv(commandArgv[0]);
                return 1;
        }
       /* if (strcmp("in", commandArgv[0]) == 0) {
                launchJob(commandArgv + 2, *(commandArgv + 1), STDIN, FOREGROUND);
                return 1;
        }
        if (strcmp("out", commandArgv[0]) == 0) {
                launchJob(commandArgv + 2, *(commandArgv + 1), STDOUT, FOREGROUND);
                return 1;
        }*/
        if (strcmp("bg", commandArgv[0]) == 0) {
                if (commandArgv[1] == NULL)
                        return 0;
                if (strcmp("in", commandArgv[1]) == 0)
                        launchJob(commandArgv + 3, *(commandArgv + 2), STDIN, BACKGROUND);
                else if (strcmp("out", commandArgv[1]) == 0)
                        launchJob(commandArgv + 3, *(commandArgv + 2), STDOUT, BACKGROUND);
                else
                        launchJob(commandArgv + 1, "STANDARD", 0, BACKGROUND);
                return 1;
        }
        if (strcmp("fg", commandArgv[0]) == 0) {
                if (commandArgv[1] == NULL)
                        return 0;
                int jobId = (int) atoi(commandArgv[1]);
                t_job* job = getJob(jobId, BY_JOB_ID);
                if (job == NULL)
                        return 0;
                if (job->status == SUSPENDED || job->status == WAITING_INPUT)
                        putJobForeground(job, TRUE);
                else                                                                                                // status = BACKGROUND
                        putJobForeground(job, FALSE);
                return 1;
        }
        if (strcmp("jobs", commandArgv[0]) == 0) {
                printJobs();
                return 1;
        }
        if (strcmp("kill", commandArgv[0]) == 0)
        {
                if (commandArgv[1] == NULL)
                        return 0;
                killJob(atoi(commandArgv[1]));
                return 1;
        }
        //int i;
        ////for( i=0;i<=commandArgc;i++)
        //{
        //char *str;
        //strcpy(str,commandArgv);
        if( commandArgc == 3 )
        {
		if(strcmp("|", commandArgv[1]) == 0  )
            {
            pipelining(2);
            return 1;
            }
            //return 1;
        }
	if( commandArgc > 3 )
{
		if(strcmp("|", commandArgv[commandArgc-2]) == 0 )
		{
			//puts("works");
			//printf("%d",commandArgc);
			pipelining(commandArgc-1);
			return 1;
		}
	}
        //break;
        //}



        return 0;
}

void copy_argv(char **argv)
{
	int index = 0;
	for(;argv[index] != NULL; index++) {
		my_argv[index] = (char *)malloc(sizeof(char) * (strlen(argv[index]) + 1));
		memcpy(my_argv[index], argv[index], strlen(argv[index]));
	}
}

void call_execve(char *cmd[])
{
	int i;
	//printf("cmd is %s\n", *cmd);
	if(fork() == 0) {
		i = execve(*cmd, my_argv, my_envp);
		//printf("errno is %d\n", errno);
		if(i < 0) {
			printf("%s: %s\n", cmd[0], "command not found");
			exit(1);
		}
	} else {
		wait(NULL);
	}
}

void executeCommand(char *command[], char *file, int newDescriptor,
                    int executionMode)
{
        int commandDescriptor;
        if (newDescriptor == STDIN) {
                commandDescriptor = open(file, O_RDONLY, 0600);
                dup2(commandDescriptor, STDIN_FILENO);
                close(commandDescriptor);
        }
        if (newDescriptor == STDOUT) {
                commandDescriptor = open(file, O_CREAT | O_TRUNC | O_WRONLY, 0600);
                dup2(commandDescriptor, STDOUT_FILENO);
                close(commandDescriptor);
        }
        if (execvp(*command, command) == -1)
                perror("SHELL142");
       
/* 
        copy_argv(command);
		if(attach_path(*command) == 0) {
			call_execve(command);
		} else {
			printf("%s: command not found\n", command);
		}
*/
}


void launchJob(char *command[], char *file, int newDescriptor,
               int executionMode)
{
        pid_t pid;
        pid = fork();
        switch (pid) {
        case -1:
                perror("SHELL142");
                exit(EXIT_FAILURE);
                break;
        case 0:
                signal(SIGINT, SIG_DFL);
                signal(SIGQUIT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
                signal(SIGCHLD, &signalHandler_child);
                signal(SIGTTIN, SIG_DFL);
                usleep(20000);
                setpgrp();
                if (executionMode == FOREGROUND)
                        tcsetpgrp(SHELL142_TERMINAL, getpid());
                if (executionMode == BACKGROUND)
                        printf("[%d] %d\n", ++numActiveJobs, (int) getpid());

                executeCommand(command, file, newDescriptor, executionMode);

                exit(EXIT_SUCCESS);
                break;
        default:
                setpgid(pid, pid);

                jobsList = insertJob(pid, pid, *(command), file, (int) executionMode);

                t_job* job = getJob(pid, BY_PROCESS_ID);

                if (executionMode == FOREGROUND) {
                        putJobForeground(job, FALSE);
                }
                if (executionMode == BACKGROUND)
                        putJobBackground(job, FALSE);
                break;
        }
}

void putJobForeground(t_job* job, int continueJob)
{
        job->status = FOREGROUND;
        tcsetpgrp(SHELL142_TERMINAL, job->pgid);
        if (continueJob) {
                if (kill(-job->pgid, SIGCONT) < 0)
                        perror("kill (SIGCONT)");
        }

        waitJob(job);
        tcsetpgrp(SHELL142_TERMINAL, SHELL142_PGID);
}

void putJobBackground(t_job* job, int continueJob)
{
        if (job == NULL)
                return;

        if (continueJob && job->status != WAITING_INPUT)
                job->status = WAITING_INPUT;
        if (continueJob)
                if (kill(-job->pgid, SIGCONT) < 0)
                        perror("kill (SIGCONT)");

        tcsetpgrp(SHELL142_TERMINAL, SHELL142_PGID);
}

void waitJob(t_job* job)
{
        int terminationStatus;
        while (waitpid(job->pid, &terminationStatus, WNOHANG) == 0) {
                if (job->status == SUSPENDED)
                        return;
        }
        jobsList = delJob(job);
}

void killJob(int jobId)
{
        t_job *job = getJob(jobId, BY_JOB_ID);
        kill(job->pid, SIGKILL);
}

void changeDirectory()
{
        if (commandArgv[1] == NULL) {
                chdir(getenv("HOME"));
        } else {
                if (chdir(commandArgv[1]) == -1) {
                        printf(" %s: no such directory\n", commandArgv[1]);
                }
        }
}

void fill_argv(char *tmp_argv)
{
	char *foo = tmp_argv;
	int index = 0;
	char ret[100];
	bzero(ret, 100);
	while(*foo != '\0') {
		if(index == 10)
			break;

		if(*foo == ' ') {
			if(my_argv[index] == NULL)
				my_argv[index] = (char *)malloc(sizeof(char) * strlen(ret) + 1);
			else {
				bzero(my_argv[index], strlen(my_argv[index]));
			}
			strncpy(my_argv[index], ret, strlen(ret));
			strncat(my_argv[index], "\0", 1);
			bzero(ret, 100);
			index++;
		} else {
			strncat(ret, foo, 1);
		}
		foo++;
		/*printf("foo is %c\n", *foo);*/
	}
	my_argv[index] = (char *)malloc(sizeof(char) * strlen(ret) + 1);
	strncpy(my_argv[index], ret, strlen(ret));
	strncat(my_argv[index], "\0", 1);
}

void copy_envp(char **envp)
{
	int index = 0;
	for(;envp[index] != NULL; index++) {
		my_envp[index] = (char *)malloc(sizeof(char) * (strlen(envp[index]) + 1));
		memcpy(my_envp[index], envp[index], strlen(envp[index]));
	}
}

void get_path_string(char **tmp_envp, char *bin_path)
{
	int count = 0;
	char *tmp;
	while(1) {
		tmp = strstr(tmp_envp[count], "PATH");
		if(tmp == NULL) {
			count++;
		} else {
			break;
		}
	}
        strncpy(bin_path, tmp, strlen(tmp));
}

void insert_path_str_to_search(char *path_str) 
{
	int index=0;
	char *tmp = path_str;
	char ret[100];

	while(*tmp != '=')
		tmp++;
	tmp++;

	while(*tmp != '\0') {
		if(*tmp == ':') {
			strncat(ret, "/", 1);
			search_path[index] = (char *) malloc(sizeof(char) * (strlen(ret) + 1));
			strncat(search_path[index], ret, strlen(ret));
			strncat(search_path[index], "\0", 1);
			index++;
			bzero(ret, 100);
		} else {
			strncat(ret, tmp, 1);
		}
		tmp++;
	}
}

int attach_path(char *cmd)
{
	char ret[100];
	int index;
	int fd;
	bzero(ret, 100);
	for(index=0;search_path[index]!=NULL;index++) {
		strcpy(ret, search_path[index]);
		strncat(ret, cmd, strlen(cmd));
		//printf("=ret:%s= ",ret);
		if((fd = open(ret, O_RDONLY)) > 0) {
			strncpy(cmd, ret, strlen(ret));
			close(fd);
			return 0;
		}
	}
	return 0;
}

void free_argv()
{
	int index;
	for(index=0;my_argv[index]!=NULL;index++) {
		bzero(my_argv[index], strlen(my_argv[index])+1);
		my_argv[index] = NULL;
		free(my_argv[index]);
	}
}



void init()
{
        SHELL142_PID = getpid();
        SHELL142_TERMINAL = STDIN_FILENO;
        SHELL142_IS_INTERACTIVE = isatty(SHELL142_TERMINAL);

        if (SHELL142_IS_INTERACTIVE) {
                while (tcgetpgrp(SHELL142_TERMINAL) != (SHELL142_PGID = getpgrp()))
                        kill(SHELL142_PID, SIGTTIN);

                signal(SIGQUIT, SIG_IGN);
                signal(SIGTTOU, SIG_IGN);
                signal(SIGTTIN, SIG_IGN);
                signal(SIGTSTP, SIG_IGN);
                signal(SIGINT, SIG_IGN);
                signal(SIGCHLD, &signalHandler_child);

                setpgid(SHELL142_PID, SHELL142_PID);
                SHELL142_PGID = getpgrp();
                if (SHELL142_PID != SHELL142_PGID) {
                        printf("Error, the shell is not process group leader");
                        exit(EXIT_FAILURE);
                }
                if (tcsetpgrp(SHELL142_TERMINAL, SHELL142_PGID) == -1)
                        tcgetattr(SHELL142_TERMINAL, &SHELL142_TMODES);

                currentDirectory = (char*) calloc(1024, sizeof(char));
        } else {
                printf("Could not make SHELL142 interactive. Exiting..\n");
                exit(EXIT_FAILURE);
        }
}

void printarr(char **a)
{
        int i = 0;
        for(;a[i] != NULL; i++) {
                printf("%s \n", a[i]);
        }
}

int main(int argc, char **argv, char **envp)
{
	int linelength = 100;
	int linenum = 0;

	char *path_str = (char *)malloc(sizeof(char) * 256);
	
	int count = 0;	
	char *tmp;

	int nrows = 10;
	int ncols = 100;
	int row;
	char **contents;
	contents = malloc(nrows * sizeof(char *));
	for (row = 0; row < nrows; row++)
	{
		contents[row] = malloc(ncols * sizeof(char));
	}
	
	FILE *fp = fopen(".sh142","r");
	if(fp){
		// exists
		while (feof(fp) == 0)
		{
			linenum++;
			fgets(contents[linenum], linelength, fp);
			char *pos;
			
			if ((pos=strchr(contents[linenum], '\n')) != NULL)
			{
				*pos = '\0';
			}
			if ((pos=strchr(contents[linenum], '\r')) != NULL)
			{
				*pos = '\0';
			}
			

			tmp = strstr(contents[linenum], "PATH");
			if(tmp != NULL) {
				strncpy(path_str, tmp, strlen(tmp));

default_env = getenv("PATH");
//printf("%s",default_env);

//printf("%s",getenv("PATH"));
//putenv(path_str);
//printf("%s",path_str);
				break;
			}
		}
		fclose(fp);

	} else {
		// doesnt exist read it from env variable
		copy_envp(envp);
		get_path_string(my_envp, path_str);	
		//printf("no config file");
	}

	if(strlen(path_str) != 0)
	{
		insert_path_str_to_search(path_str);
	}
	else
	{
		//printf("no PATH set");
	}

        init();
        welcomeScreen();
        shellPrompt();
        while (TRUE) {
                userInput = getchar();
                switch (userInput) {
                case '\n':
                        shellPrompt();
                        break;
                default:
                        getTextLine();
                        handleUserCommand();
                        shellPrompt();
                        break;
                }
        }
        printf("\n");
        return 0;
}

void signalHandler_child(int p)
{
        pid_t pid;
        int terminationStatus;
        pid = waitpid(-1, &terminationStatus, WUNTRACED | WNOHANG);
        if (pid > 0) {
                t_job* job = getJob(pid, BY_PROCESS_ID);
                if (job == NULL)
                        return;
                if (WIFEXITED(terminationStatus)) {
                        if (job->status == BACKGROUND) {
                                printf("\n[%d]+  Done\t   %s\n", job->id, job->name);
                                jobsList = delJob(job);
                        }
                } else if (WIFSIGNALED(terminationStatus)) {
                        printf("\n[%d]+  KILLED\t   %s\n", job->id, job->name);
                        jobsList = delJob(job);
                } else if (WIFSTOPPED(terminationStatus)) {
                        if (job->status == BACKGROUND) {
                                tcsetpgrp(SHELL142_TERMINAL, SHELL142_PGID);
                                changeJobStatus(pid, WAITING_INPUT);
                                printf("\n[%d]+   suspended [wants input]\t   %s\n",
                                       numActiveJobs, job->name);
                        } else {
                                tcsetpgrp(SHELL142_TERMINAL, job->pgid);
                                changeJobStatus(pid, SUSPENDED);
                                printf("\n[%d]+   stopped\t   %s\n", numActiveJobs, job->name);
                        }
                        return;
                } else {
                        if (job->status == BACKGROUND) {
                                jobsList = delJob(job);
                        }
                }
                tcsetpgrp(SHELL142_TERMINAL, SHELL142_PGID);
        }
}
