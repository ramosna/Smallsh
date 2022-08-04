// Name: Nathan Ramos
// Assignment 3
// Description: This file contains main and a few other functions for the program smallsh.
// smallsh is a small shell with three built in commands cd, status, and exit. smallsh also
// supports input and output redirection along with the ability to put processes in the background. 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "input_functions.h"
#include "input_struct.h"

// global variable for switching foreground and background processes
bool globalSwitch = true;
// global variabe for last function
pid_t lastChild;
bool commandLine = false;

// struct used to hold current running background PIDs, used to easily make LL 
struct ongoing
{
	pid_t childPID;
	struct ongoing* next;
};

// linked list struct that holds all ongoing background child processes
struct masterList
{
	struct ongoing* head;
	struct ongoing* tail;
};

// function for the build in cd command, passes the users input to chdir to switch directories
void cdFunc(char* arr[], int size)
{
	if (size == 1)
	{
		chdir(getenv("HOME"));
	}
	else
	{
		chdir(arr[1]);
	}
}

// this function redirects the input to the file specified by user input
// Based on Module 5 Exploration: Processes and I/O
void redirectIn(char* directIn)
{
	// opening up file to read input
	int sourceFD = open(directIn, O_RDONLY);
	// prints error message if unable to open specified file, returns error 1
	if (sourceFD == -1) {
		perror("unable to open file");
		exit(1);
	}
	// Redirect stdin to source file
	int resultI = dup2(sourceFD, 0);
	// if failed to redirect stdin, print error message and retunr error message 1
	if (resultI == -1) {
		perror("source dup2()");
		exit(1);
	}
}

// this function redirects the output to the file specified by user input
// Based on Module 5 Exploration: Processes and I/O
void redirectOut(char* directOut)
{
	// opening up file to write output, set to write, create if none, and trunc if exists
	int targetFD = open(directOut, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	// prints error message if unable to open specified file, returns error 1
	if (targetFD == -1) {
		perror("unable to open file");
		exit(1);
	}
	// redirect stdout to source file
	int resultO = dup2(targetFD, 1);
	// if failed to redirect stdout, print error message and retunr error message 1
	if (resultO == -1) {
		perror("target dup2()");
		exit(1);
	}
}

// this function iterates through linked list of background PID structs and checks if they are done
void harvestChild(struct masterList* children)
{
	// checking to see if there are any current background processes, head would be null if not
	if (children->head != NULL)
	{
		// head of linked list of PID structs
		struct ongoing* node = children->head;
		// initializing previous incase we need to remove nodes
		struct ongoing* prev;

		// iterating through all stucts in LL 
		while (node != NULL)
		{
			// getting the PID from the struct
			pid_t spawnPid = node->childPID;
			// initializing status variable
			int childStatus;

			// using waitpid with WNOHANG flag to see if processes had terminated
			pid_t prossStat = waitpid(spawnPid, &childStatus, WNOHANG);
			// if processes has not terminated moving to next struct
			if (prossStat == 0)
			{
				prev = node;
				node = node->next;
			}
			// if process has terminated 
			else
			{
				// checking to see exit status of child, seeing if terminated normally or by signal
				if (WIFEXITED(childStatus)) {
					printf("background pid %d is done: exit value %d\n", spawnPid, WEXITSTATUS(childStatus));
					fflush(stdout);
				}
				else {
					printf("background pid %d is done: terminated by signal %d\n", spawnPid, WTERMSIG(childStatus));
					fflush(stdout);
				}
				// removing struct from linked list since processes has terminated
				struct ongoing* temp = node;
				// if the node is the head, removing it
				if (node == children->head) {
					children->head = node->next;
					if (node == children->tail) {
						children->tail = node->next;
					}
					node = node->next;
					free(temp);
				}
				// if not the head then removing it from list
				else
				{
					prev->next = node->next;
					if (node == children->tail) {
						children->tail = prev;
					}
					node = node->next;
					free(temp);
				}
			}
		}
	}
}

// setting up handler to ingnore SIDTSTP 
// Based on lecture material Signal Handling API
void ignoreSIDTSTP(void)
{
	// ignoring SIGTSTOP for all child processes
	struct sigaction SIGTSTOP_ignore = { {0} };
	SIGTSTOP_ignore.sa_handler = SIG_IGN;
	sigaction(SIGTSTP, &SIGTSTOP_ignore, NULL);
}

// heavily based on lecture materail in module 4 Exploration: Process API - Executing a New Program
void commandPross(struct command* run, struct masterList* list, int* statusNum, bool* statBool)
{
	// setting status variable
	int childStatus;

	// Fork a new process
	pid_t spawnPid = fork();

	// switch statement to handle error in fork, child processes and parent processes
	switch (spawnPid) {
	// error handler if there was an error with the forking 
	case -1:
		perror("fork()\n");
		exit(1);
		break;
	// handling the child processes
	case 0:
		// setting up to ignore SIDTSTP in child processes
		ignoreSIDTSTP();
		// setting child process to default handling of SIGINT if foreground process
		if (*run->background == false)
		{
			struct sigaction default_action = { {0} };
			default_action.sa_handler = SIG_DFL;
			sigaction(SIGINT, &default_action, NULL);
		}
		// setting the child processes input/output if background mode is disabled, is a foreground processes,
		// or if the user specified both input/output
		if (*run->background == false || (run->input != NULL && run->output != NULL) || globalSwitch == false) {
			if (run->input != NULL)
			{
				redirectIn(run->input);
			}
			if (run->output != NULL) {
				redirectOut(run->output);
			}
		}
		// if a background processes
		else
		{
			// if input redirection was specified
			if (run->input != NULL)
			{
				redirectIn(run->input);
			}
			// if not specified set to dev/null
			else
			{
				char* redirect = "/dev/null";
				redirectIn(redirect);
			}
			// if output rediction was specified
			if (run->output != NULL) {
				redirectOut(run->output);
			}
			// if not specified set to dev/null
			else
			{
				char* redirect = "/dev/null";
				redirectOut(redirect);
			}
		}

		// using execvp to execute user command since using an array
		execvp(run->inputArr[0], run->inputArr);
		// if execvp errors, prints error and sets error value to 1
		perror(run->inputArr[0]);
		exit(1);
		break;
	// the parent process
	default:
		// if child is a not background process or in foreground only mode
		if (*run->background == false || globalSwitch == false) {
			// Wait for child's termination
			spawnPid = waitpid(spawnPid, &childStatus, 0);
			// checking the exit status of the child
			if (WIFEXITED(childStatus)) {
				// setting status variable 
				*statusNum = WEXITSTATUS(childStatus);
				// setting status of bool variable to terminated normally
				if (*statBool == false) {
					*statBool = true;
				}
			}
			// if terminated by signal
			else {
				// setting status variable
				*statusNum = WTERMSIG(childStatus);
				// setting bool to terminated by a signal
				if (*statBool == true) {
					*statBool = false;
				}
				// printing out if terminated by signal
				printf("terminated by signal %d\n", *statusNum);
				fflush(stdout);
			}
		}
		// for background processes
		else {
			// printing pID to terminal
			printf("background pid is %d\n", spawnPid);
			fflush(stdout);
			// not waiting for status to terminate
			pid_t prossStat = waitpid(spawnPid, &childStatus, WNOHANG);
			// if process has not terminated creating a struct to add to link list
			if (prossStat == 0)
			{
				// creating struct with pid
				struct ongoing* newBack = malloc(sizeof(struct ongoing));
				newBack->childPID = spawnPid;
				// if list is empty
				if (list->head == NULL) {
					list->head = newBack;
					list->tail = newBack;
				}
				else {
					// if list is not empty
					list->tail->next = newBack;
					list->tail = newBack;
				}
			}
			// if the process has terminated then printing its PID and termination status
			else
			{
				if (WIFEXITED(childStatus)) {
					printf("background pid %d is done: exit value %d\n", spawnPid, WEXITSTATUS(childStatus));
					fflush(stdout);
				}
				else {
					printf("background pid %d is done: terminated by signal %d\n", spawnPid, WTERMSIG(childStatus));
					fflush(stdout);
				}
			}
		}
		break;
	}
}

// cleans up and exits program, terminates background processes and frees memory
void exitProcess(struct masterList* finish, struct command* endInp)
{
	struct ongoing* node = finish->head;

	// kills background processes with signal
	while (node != NULL)
	{
		struct ongoing* temp = node;
		node = node->next;
		//Kill idea from stackoverflow.com/questions/14110738/how-to-terminate-a-child-process-which-is-running-another-program-by-doing-exec
		kill(temp->childPID, SIGTERM);
		free(temp);
	}

	free(finish);

	// exits program
	exit(EXIT_SUCCESS);
}

// singal handler for SIGTSTP: this function handles the process when switching from foreground only mode
void changeBackground(int signo)
{
	int childStatus;

	// if currently allowing background processes switching to foreground only mode
	if (globalSwitch == true)
	{
		// setting global variable to false
		globalSwitch = false;
		// indicating entering foreground mode
		char* output = "Entering foreground-only mode (& is now ignored)\n";
		// waiting until last foreground process finishes in order to print message
		waitpid(lastChild, &childStatus, 0);
		// if sitting on command line creating new line
		if (commandLine == true) {
			write(STDOUT_FILENO, "\n", 1);
		}
		// writing out message, using write because printf not reentrant
		write(STDOUT_FILENO, output, 49);
	}
	// if currently in foreground mode and switching out
	else
	{
		// setting global variable to true
		globalSwitch = true;
		// message to print
		char* output = "Exiting foreground-only mode\n";
		// waiting until last process has finished
		waitpid(lastChild, &childStatus, 0);
		if (commandLine == true) {
			write(STDOUT_FILENO, "\n", 1);
		}
		// writing out message, using write because printf not reentrant
		write(STDOUT_FILENO, output, 29);
	}
	// if sitting on command line reprint the colon
	if (commandLine == true)
	{
		char* colon = ": ";
		write(STDOUT_FILENO, colon, 2);
	}

}

// this function initializes signal handlers at start of program
void initial_signals(void)
{
	// SIGINT signal ignore
	// Taken from lecture material Module 5 Exploration: Signal Handling API
	struct sigaction ignore_action = { {0} };
	ignore_action.sa_handler = SIG_IGN;
	sigaction(SIGINT, &ignore_action, NULL);

	//SIGTSTP initial handler
	// Taken from lecture material Module 5 Exploration: Signal Handling API
	struct sigaction SIGTSTP_toggle = { {0} };
	SIGTSTP_toggle.sa_handler = changeBackground;
	// Block all catchable signals while singal handler is running
	sigfillset(&SIGTSTP_toggle.sa_mask);
	// SA_RESTART flag set to pick where left off
	SIGTSTP_toggle.sa_flags = SA_RESTART;
	sigaction(SIGTSTP, &SIGTSTP_toggle, NULL);
}

int main(void)
{
	// initializing linked list of background PID structs
	struct masterList* backProcesses = malloc(sizeof(struct masterList));
	backProcesses->head = NULL;
	backProcesses->tail = NULL;

	// setting status variables
	// used for keeping track of last foreground processes exit status
	int statusVar = 0;
	// keeping track of whether it was normal exit or signal exit
	bool exitStat = true;

	// initializing signals
	initial_signals();

	// program loop
	while (true)
	{
		// checking background processes
		harvestChild(backProcesses);

		// making space for user input
		char userInp[2049];
		// printing prompt
		printf(": ");
		fflush(stdout);
		// indicates program is on command line
		commandLine = true;
		// getting user input
		fgets(userInp, 2048, stdin);
		// indicats not on command line
		commandLine = false;
		// getting rid of the new line character
		char* enter = strchr(userInp, '\n');
		if (enter != NULL) {
			*enter = '\0';
		}
		// if user input was not comment or blank entry
		if (userInp[0] != '#' && strlen(userInp) != 0)
		{

			// variable for holding modified user input
			char* modInp;

			// if no expansion needed for user input
			if (strstr(userInp, "$$") == NULL)
			{
				modInp = userInp;
			}
			// if expansion needed for user input
			else
			{
				modInp = expand(userInp);
			}
			// creating struct to hold all command detials
			struct command* details = malloc(sizeof(struct command));
			details->input = NULL;
			details->output = NULL;
			details->background = malloc(sizeof(bool));
			*details->background = false;

			// getting size of input, checking for input, output, and background command
			details->size = inputSize(modInp, &details->input, &details->output, &details->background);

			// initializing array of command values
			char* arr[details->size + 1];
			// adding each argument to array
			parseInput(arr, details->size, modInp);

			// adding array to command struct
			details->inputArr = arr;

			// exit process
			if (strcmp(arr[0], "exit") == 0)
			{
				exitProcess(backProcesses, details);
			}
			// cd built in command
			else if (strcmp(arr[0], "cd") == 0)
			{
				cdFunc(arr, details->size);
			}
			// status command
			else if (strcmp(arr[0], "status") == 0)
			{
				if (exitStat == true)
				{
					// printing normal exit value
					printf("exit value %d\n", statusVar);
					fflush(stdout);
				}
				else
				{
					// printing if terminated by signal
					printf("terminated by signal %d\n", statusVar);
					fflush(stdout);
				}
			}
			else
			{
				// processing the command if not built in
				commandPross(details, backProcesses, &statusVar, &exitStat);
			}
		}
	}
}