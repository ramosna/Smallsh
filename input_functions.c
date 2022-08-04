// File Description: This file contains all the functions used to parse user input

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "input_functions.h"
#include "input_struct.h"


// This function returns the number of words in a user command, it also adds input/output files to the command
// struct. Along with marking if the command should be run as a background process
int inputSize(char* input, char** reInp, char** reOut, bool** back)
{
	// var to count number of words in command
	int count = 0;

	// var to copy input to tokenize
	char* copy = calloc(strlen(input) + 1, sizeof(char));
	strcpy(copy, input);
	
	// bools to check to see if there is input/output redirection
	bool in = false;
	bool out = false;
	// pointer for strtok_r
	char* saveptr;
	// first token, seperates by spaces
	char* token = strtok_r(copy, " ", &saveptr);

	// if there is an empty string
	if (token == NULL) {
		free(copy);
		return 0;
	}
	// if there is not an empty string
	else
	{
		// cutting up the input string to individual words
		while (token != NULL)
		{
			// checking to see if current word is input command, indicating to bool that it is
			if (strcmp(token, "<") == 0) {
				in = true;
				token = strtok_r(NULL, " ", &saveptr);
			}
			// checking to see if current word is output command, indicating to bool that it is
			else if (strcmp(token, ">") == 0) {
				out = true;
				token = strtok_r(NULL, " ", &saveptr);
			}
			// checking to see if current word is background command and if it is at the end of the user input
			// indicating to bool that it is
			else if (strcmp(token, "&") == 0) {
				token = strtok_r(NULL, " ", &saveptr);
				if (token == NULL) {
					**back = true;
				}
				else {
					count++;
				}
			}
			// if previous word was input redirect, save current word as file name
			else if (in == true) {
				*reInp = calloc(strlen(token) + 1, sizeof(char));
				strcpy(*reInp, token);
				in = false;
				token = strtok_r(NULL, " ", &saveptr);
			}
			// if previous word was output redirect, save current word as file name
			else if (out == true) {
				*reOut = calloc(strlen(token) + 1, sizeof(char));
				strcpy(*reOut, token);
				out = false;
				token = strtok_r(NULL, " ", &saveptr);
			}
			// counting all other words as part of the command
			else {
				count++;
				token = strtok_r(NULL, " ", &saveptr);
			}

		}
	}
	free(copy);
	return count;
}

// this function expands the expansion variables in the users input
char* expand(char* word)
{
	// this bool indicates when all $$ have been caught
	bool compl = false;
	// this variable is used to determine if the whole input string has been covered
	int length = strlen(word);
	// variable for expansion
	char* variable = "$$";

	// turning pid into string
	pid_t numpid = getpid();
	char pid[50];
	sprintf(pid, "%d", numpid);

	// this variable is to hold the expanded string
	char* done = NULL;
	// finding the first instance of $$
	char* ptr = strstr(word, variable);

	// loop to iterate through and find all expansions
	while (compl == false)
	{
		// setting the first value $ to \0 so that sprintf can copy and stop there
		*ptr = '\0';
		ptr += 2;
		// if first instance of $$ then setting beginning of the string into the done variable
		if (done == NULL) {
			done = calloc(strlen(word) + strlen(pid) + 1, sizeof(char));
			sprintf(done, "%s%s", word, pid);
		}
		else {
			// creating temp to hold what part of the string has already been parsed
			char* temp = calloc(strlen(done) + 1, sizeof(char));
			strcpy(temp, done);
			// releasing done
			free(done);
			// adding previously copied section of the string, new section, and pid to the new string
			done = calloc(strlen(temp) + strlen(word) + strlen(pid) + 1, sizeof(char));
			sprintf(done, "%s%s%s", temp, word, pid);
			free(temp);
		}
		// subtracting newly copied part from total length
		length -= (ptr - word);
		// moving the pointer of how much the string has been covered
		word = ptr;
		// if length is 0 then the whole string has been traversed, set bool to complete
		if (length == 0) {
			compl = true;
		}
		else {
			// finding the next instance of $$
			ptr = strstr(word, variable);
			// if there are no more instances of $$, but there is more to the input, copy the rest to the result string
			if (ptr == NULL) {
				char* temp = calloc(strlen(done) + 1, sizeof(char));
				strcpy(temp, done);
				free(done);
				done = calloc(strlen(temp) + strlen(word) + 1, sizeof(char));
				sprintf(done, "%s%s", temp, word);
				free(temp);
				compl = true;
			}
		}
	}
	return done;
}


// this function fills the user input array with pointers to each word entered
void parseInput(char* list[], int indices, char* args)
{
	// index for loop and array access
	int index = 0;
	// pointer for strtok_r
	char* saveptr;

	// copying string to not destroy original
	char* copy = calloc(strlen(args) + 1, sizeof(char));
	strcpy(copy, args);
	// initial division
	char* token = strtok_r(copy, " ", &saveptr);
	// looping through until all arguments are put into the array
	while (index < indices)
	{
		list[index] = token;
		token = strtok_r(NULL, " ", &saveptr);
		index++;
	}
	// setting the last index in the array to NULL for exec() function
	list[indices] = NULL;
}