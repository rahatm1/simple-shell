/*
 * main.c
 *
 * A simple program to illustrate the use of the GNU Readline library
 */

#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <readline/history.h>
#include <readline/readline.h>

size_t BUFFER_LEN = 256;
size_t ARGS_LEN = 15;

typedef struct {
    char *name;
    pid_t processId;
    int status;
    bool running;
} process;

/* Separates command into arguments */
void cmdSeparator(char *args[], char *cmd)
{
    args[0] = strtok(cmd, " ");
    int i = 0;
    while (args[i] != NULL)
    {
        i++;
        args[i] = strtok(NULL, " ");
    }
}

/* Remove First Element and shift left */
void arrayUnshift(char *a[]) {
	for (size_t i = 0; i < ARGS_LEN; i++) {
		a[i] = a[i+1];
	}
	a[ARGS_LEN] = "";
}

/* Remove Given Element and shift left */
void processShift(process a[], int start)
{
	for (int i = start; i < 5; i++) {
		a[i] = a[i+1];
	}
    // a[processNum] = {0};
}

/* Check for process termination */
void checkDeath(process processList[], int *processNum)
{
    int tempStatus;
    for (int i = 0; i < *processNum; i++)
    {
        /* Clean Zombie Processes */
        if (waitpid(processList[i].processId, &tempStatus, WNOHANG) > 0)
        {
            printf("%d: %s has been terminated\n", i, processList[i].name);
            free(processList[i].name);

            processShift(processList, i);
            *processNum = *processNum - 1;
        }
    }
}

int main ( void )
{
	process processList[5];
	int processNum = 0;

	for (;;)
    {
		/* Get Current Working Dir */
		char dir[BUFFER_LEN];
		getcwd(dir, BUFFER_LEN);
		strncat(dir, ">", BUFFER_LEN);

        checkDeath(processList, &processNum);

		/* Shell Prompt */
		char *cmd = readline(dir);

		char *args[ARGS_LEN];
		cmdSeparator(args, cmd);
		if (args[0] == NULL) {
            free(cmd);
			continue;
		}

		/* Change Directory */
		if (strncmp(args[0], "cd", BUFFER_LEN) == 0)
		{
			if (chdir(args[1]) != 0)
			{
				printf("Failed to change dir\n");
			}
		}
        /* Print Background Process List */
		else if (strncmp(args[0], "bglist", BUFFER_LEN) == 0)
		{
			for (int i = 0; i < processNum; i++) {
                char runStatus = processList[i].running ? 'R' : 'S';
				printf("%d[%c]: %s\n", i, runStatus, processList[i].name);
			}
			printf("Total Background Jobs: %d\n", processNum);
		}
        /* Kill background Process */
        else if (strncmp(args[0], "bgkill", BUFFER_LEN) == 0)
        {
            int toKill = atoi(args[1]);
            if (toKill >= 0 && toKill <= processNum)
            {
                /*Kill and remove process from processList */
                if (kill(processList[toKill].processId, SIGTERM) == 0)
                {
                    waitpid(processList[toKill].processId, 0, 0);
                    free(processList[toKill].name);
                    processShift(processList, toKill);
                    processNum--;
                }
                else
                {
                    printf("Failed to Kill process\n");
                }

            }
            else
            {
                printf("invalid process Number\n");
            }
        }
        /* Stop background Process */
        else if (strncmp(args[0], "stop", BUFFER_LEN) == 0)
        {
            int toStop = atoi(args[1]);
            if (toStop >= 0 && toStop <= processNum)
            {
                if (!processList[toStop].running)
                {
                    printf("Process is already stopped\n");
                }
                else if (kill(processList[toStop].processId, SIGSTOP) == 0)
                {
                    processList[toStop].running = false;
                }
                else
                {
                    printf("Failed to Stop process\n");
                }
            }
            else
            {
                printf("invalid process Number\n");
            }
        }
        /* Start stopped process */
        else if (strncmp(args[0], "start", BUFFER_LEN) == 0)
        {
            int toStart = atoi(args[1]);
            if (toStart >= 0 && toStart <= processNum)
            {
                if (processList[toStart].running)
                {
                    printf("Process is already Running\n");
                }
                else if (kill(processList[toStart].processId, SIGCONT) == 0)
                {
                    processList[toStart].running = true;
                }
                else
                {
                    printf("Failed to Start process\n");
                }
            }
            else
            {
                printf("invalid process Number\n");
            }
        }
		else
		{
            /* Check for background process request */
			bool bgProcess = false;
			if (strncmp(args[0], "bg", BUFFER_LEN) == 0)
			{
				bgProcess = true;
				arrayUnshift(args); //remove "bg" from args
            }

            /* Fork process here*/
			pid_t child_pid = fork();
			if (child_pid == 0)
			{
				execvp(args[0], args);
				perror("execvp didn't execute!");   /* execve() only returns on error */
				return -1;
			}
			else
			{
                /* Store background process info to processlist */
				if (bgProcess) {
                    //TODO: Free memory
                    processList[processNum].name = malloc(strlen(args[0])+1);
                    strcpy(processList[processNum].name, args[0]);

                    processList[processNum].processId = child_pid;
                    processList[processNum].running = true;
                    processNum++;
				}
                else
                {
                    int	status;
					wait (&status);
				}
			}
		}

		free (cmd);
	}
}
