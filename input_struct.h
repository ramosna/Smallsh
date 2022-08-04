#ifndef COMMAND
#define COMMAND
// This struct holds the different parts of a users input command
struct command
{
	char** inputArr;
	int size;
	char* input;
	char* output;
	bool* background;

};
#endif