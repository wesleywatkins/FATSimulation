/*
  Project 3 for COP4610
  Main Function File
*/

#include "proj3.h"

// main function
int main(int argc, char ** argv) {

	char line[10000]; // for reading command line input
	char * imageFileName; // string for storing the file name
	Image * image = calloc(1, sizeof(*image)); // Image struct for image file

	// check for image file in command line arguments
	if (argc != 2) {
		printf("ERROR: No image file provided.\n");
		exit(1);
	}

	// get image file name
	imageFileName = (char*)calloc(strlen(argv[1])+1,sizeof(char));
	strcpy(imageFileName, argv[1]);

	// read image file in image struct
	readInImageFile(imageFileName, image);
	image->bootSector = readBootSector(imageFileName);

	// free image file name
	free(imageFileName);

	// set root cluster as current cluster
	int rootCluster = hexStringToDec(image->bootSector.RootClus, RootClusSize);
	image->currentCluster = rootCluster;

	int temp[getTotalClusters(image)];
	getFATRegion(image, temp);

	// Print Prompt
	printf("\n\n");
	printPrompt(image);
	while(fgets(line,200,stdin)) {
		if (dev) printf("\n\nline: %s\n", line);
		parseInput(line, image);
		printPrompt(image);
	}

	return 1;
}


// print the prompt with the directory
void printPrompt(const Image * image) {
	char prompt[150] = "/";
	int i = 0;
	for (i = 0; i < image->promptDepth; i++) {
		if (i != 0)
			strcat(prompt, "/");
		strcat(prompt, image->cwd[i]);
	}
	printf("\n%s> ", prompt);
}


// parse input from prompt and call the correct function
void parseInput(char * line, Image * image) {

	// initial variables
	int lineCount, i;

	// remove trailing whitespace & split line into array
	removeTrailingSpace(line);
	char lineArr[100][100];
	split(line, " ", &lineCount, lineArr);

	// grab command, first word in input
	char command[strlen(lineArr[0]) + 1];
	strcpy(command, lineArr[0]);

	// get arguments
	char args[10000];
	int start = strlen(command)+1;
	for (i = start; i < strlen(line); i++) {	
		args[i-start] = line[i];
	}
	args[i-start] = '\0';

	// declare error message variable
	char errorMessage[150];
	bool check = true;

	// check for commands and call the correct function

	if (strcmp(command, "exit") == 0)
		exit_command(image);

	else if (strcmp(command, "info") == 0) 
		info_command(image);

	else if (strcmp(command, "ls") == 0)
		check = ls_command(args, image, errorMessage);

	else if (strcmp(command, "cd") == 0)
		check = cd_command(args, image, errorMessage);

	else if (strcmp(command, "size") == 0)
		check = size_command(args, image, errorMessage);

	else if (strcmp(command, "creat") == 0)
		check = creat_command(args, image, errorMessage);

	else if (strcmp(command, "mkdir") == 0)
		check = mkdir_command(args, image, errorMessage);

	else if (strcmp(command, "open") == 0)
		check = open_command(args, image, errorMessage);

	else if (strcmp(command, "close") == 0)
		check = close_command(args, image, errorMessage);

	else if (strcmp(command, "read") == 0)
		check = read_command(args, image, errorMessage);

	else if (strcmp(command, "write") == 0)
		check = write_command(args, image, errorMessage);

	else if (strcmp(command, "rm") == 0)
		check = rm_command(args, image, errorMessage);
		
	else if (strcmp(command, "rmdir") == 0)
		check = rmdir_command(args, image, errorMessage);

	else
		printf("Command not recognized.\n");

	// print error message (if applicable)
	if (!check)
		printf("Error: %s", errorMessage);
}