# Smallsh

A small POSIX shell with built-in commands. Any command that is not built in runs as child processes using fork() and exec(). Includes background and foreground processing, signal handlers, variable expansion, and input and output redirection 

![Smallsh](gifs/smallsh.gif)

## Features

### Built In Commands

  CD: This command takes one argument for either relative or absolute paths. If no argument is entered it takes you to the root directory specified by the HOME environment variable.
  
  Status: This returns the exit status of the last foreground process that has been run.
  
  Exit: By typing exit all background and foreground process will be terminated and the application will exit.

### Capabilities

Variable Expansion: Any instance of "$$" in a command will be replaced by the process ID of the shell.

The shell can handle both lines that are blank and also lines that begin with # as comments.

Has the ability to do input and output redirection with < and > just like a bash shell.

Commands with '&' at the end are run as background processes. The shell does not wait for the process to terminate and will continue on. Thus, the shell is able to run multiple process at the same time. 

All other commands are executed as child processes using fork() and exec().

### Signal Handlers

Includes signal handlers for SIGINT (CTRL+C) and SIGTSTP (CTRL+Z) 

SIGINT is ignored by shell and all background processes

SIGINT terminates all foreground child processes, printing out process ID of the process and the signal that killed it

All child processes, both foreground and background, ignore SIGTSTP

SIGTSTP turns shell into foreground only mode where '&' is ignored until shell receives SIGTSTP again

## Compiling & Executing
How to compile smallsh:

In order to compile the project, first place the files in a directory making sure that all the c files, 
header files, and the Makefile are all present.

Second run the command "make" while in the directory containing the files. 
This will create an executable called "smallsh".

In order to run the executable and the program, run the command "./smallsh".

The executable does not take anything else as an argument. You just need to run the file. 

Additionally, the executable has been provided in folder.
