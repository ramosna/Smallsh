# Smallsh

POSIX shell with built-in commands. All other commands run as child processes using fork() and exec(). Includes signal handlers, variable expansion, input and output redirection, and backgruond and foreground processing

## Features

### Built In Commands

  CD: This command takes one argument for either relative or absolute paths. If no argument is entered it takes you to the root directory specified by the HOME enviroment variable.
  
  Status: This returns the exit status of the last foreground process that has been run.
  
  Exit: By typing exit all background and foreground process will be terminated and the application will exit.

## Compiling & Running
How to compile smallsh:

In order to compile the project, first place the files in a directory making sure that all the c files, 
header files, and the Makefile are all present.

Second run the command "make" while in the directory containing the files. 
This will create an executable called "smallsh".

In order to run the executable and the program, run the command "./smallsh".

The executable does not take anything else as an argument. You just need to run the file. 

Additionally, the executable has been provided in folder.
