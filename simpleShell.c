/** This is an updated, simplifed version of shell for use with the file system program for CPSC321
 * Elizabeth Norman 230139232 Dec 11 2021
 */

#include <stdio.h>
#include "DiskSim.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
/** This is modified from assignment 1. It parses up user input for use of ls, cat and echo commands
 */
char** parseLine(char *line)
{
	int position = 0;
	char **args = calloc(1024, sizeof(char*));

	char *input = strtok(line, " ");

	while (input!=NULL)
	{
		args[position] = input;
		position ++;
		input = strtok(NULL, " ");
	}

	return args;
}

/** This returns a user's numerical input
 */
int getID()
{
	char argID[100];
	fgets(argID, 100, stdin);
	int arg = atoi(argID);
	return arg;
}


/** Function allows users to tell the program which command they desire to use
 */
bool takeInput()
{
	char input[300];
	fgets(input, 300, stdin);		
	input[strcspn(input, "\n")] =0;	
	printf("you selected: %s\n", input);
	
	char arg1[100];
	int arg;

	if (strcmp(input, "exit")== 0) //look up table time. different actions are taken based on input
	{
		return false;
	}
	else if (strcmp(input, "makeFile") == 0)
	{
		printf("insert ID of directory as to where you want file\n");
		arg = getID();
		makeFile(arg);
	}	
	else if (strcmp(input, "writeFile") == 0)
	{
		printf("write text you wish to write to file\n");
		char uinput[4608];
		fgets(uinput, 4608, stdin);
		input[strcspn(input, "\n")] = 0; //this removes the pesky new line fgets provides
		printf("insert inode ID where you want a file written\n");
		arg = getID();
		writeFile(uinput, arg);
	}	
	else if (strcmp(input, "deleteFile") == 0)
	{
		printf("insert inode ID of file you wish to delete\n");
		arg = getID();
		deleteFile(arg);
	}	
	else if (strcmp(input, "createDirectory") == 0)
	{
		printf("insert ID of directory to make child from\n");
		arg = getID();
		createDirectory(arg);
	}	
	else if (strcmp(input, "readFile") == 0)
	{
		printf("insert inode ID of file you wish to read\n");
		arg = getID();
		readFile(arg);
	}
	else
	{
		char **args = parseLine(input); //from assignment 1. allows user to input some shell commands
		if (strcmp(args[0], "ls")==0 || strcmp(args[0], "cat") == 0 || strcmp(args[0], "echo") == 0)
		{
			pid_t pid = fork();
			int childStatus;
			if (pid == 0)
			{
				execvp(args[0], args);
			}	
			else if (pid < 0)
			{
				printf("forking has forked up\n");
			}
			else
			{
				waitpid(pid, &childStatus, WUNTRACED);
			}
		}	
		else
		{
			printf("command not found\n");
		}

		free(args);
	}
}

/** main program
 */
int main()
{
	bool running = true;
	printf("welcome to simple shell\n\n");
	
	printf("first we will go through the assignment directives, then user may use their own commands\n\n");

	
	partition();
	createDirectory(0);
	createDirectory(0);
	createDirectory(0);
	
	//for alex. 
	makeFile(1);
	writeFile("\"I wish none of this had happened\" \"so do all who live to see such times. but that is not for them to decide. all we have to decide is what to do with the time that is given to us\"",0);
	
	makeFile(1);
	writeFile("From yearning years of want on to dark days of wane / we have walked by the light of the northern star /from frozen fjords of rime out to swift seas of raid / we have sailed by the light of the northern star / the northern night has never seen / the southern cross shine bright / across the colder seas under rain ridden skies / we go on by the light of the northern star", 1);
	

	makeFile(1);
	writeFile("I have known many gods. He who denies them is as blind as he who trusts them too deeply. I seek not beyond death. It may be the blackness averred by the Nemedian skeptics, or Crom's realm of ice and cloud, or the snowy plains and vaulted halls of the Nordheimer's Valhalla. I know not, nor do I care. Let me live deep while I live; let me know the rich juices of red meat and stinging wine on my palate, the hot embrace of white arms, the mad exultation of battle when the blue blades flame and crimson, and I am content. Let teachers and philosophers brood over questions of reality and illusion. I know this: if life is illusion, then I am no less an illusion, and being thus, the illusion is real to me. I live, I burn with life, I love, I slay, and am content", 2);
	
	readFile(1);
	
	//user is informed of their choices
	printf("\nProject commands: makeFile, writeFile, deleteFile, readFile, createDirectory, exit to quit\n"); 
	printf("you will be prompted for arguments after you tell command\n");
	printf("you may make a maximum of 128 files, 128 directories and files have a max size of 4608 characters\n");
	printf("original shell commands: ls, cat, echo are also available\n"); //I wanted to see if I could improve


	
	while (running) //using an int was being weird
	{	
		running = takeInput();
		printf("\n>");	
	}
	return 0;

}
