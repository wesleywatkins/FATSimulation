/*
  Project 3 for COP4610
  Function Definitions
*/

#include "proj3.h"


// exit from the program
void exit_command(Image * image) {
	free(image->buffer);
	exit(0);
}

// parse the boot sector. Print the field name and corresponding values 
// for each entry, one per line (e.g. Bytes per Sector: 512)
void info_command(const Image * image) {

	int i;

	printf("\njmpBoot: ");
	for (i = 0; i < jmpBootSize; i++)
		printf("%x ", image->bootSector.jmpBoot[i]);
	printf("\n");

	char OEMStr[OEMNameSize+1];
	hexToASCII(image->bootSector.OEMName, OEMNameSize, OEMStr);
	printf("OEMName: %s\n", OEMStr);

	printf("BytsPerSec: %d\n", hexStringToDec(image->bootSector.BytsPerSec, BytsPerSecSize));
	printf("SecPerClus: %d\n", hexStringToDec(image->bootSector.SecPerClus, SecPerClusSize));
	printf("RsvdSecCnt: %d\n", hexStringToDec(image->bootSector.RsvdSecCnt, RsvdSecCntSize));
	printf("NumFATs: %d\n", hexStringToDec(image->bootSector.NumFATs, NumFATsSize));
	printf("RootEntCnt: %d\n", hexStringToDec(image->bootSector.RootEntCnt, RootEntCntSize));
	printf("TotSec16: %d\n", hexStringToDec(image->bootSector.TotSec16, TotSec16Size));
	printf("Media: %x\n", image->bootSector.Media[0]);
	printf("FATSz16: %d\n", hexStringToDec(image->bootSector.FATSz16, FATSz16Size));
	printf("SecPerTrk: %d\n", hexStringToDec(image->bootSector.SecPerTrk, SecPerTrkSize));
	printf("NumHeads: %d\n", hexStringToDec(image->bootSector.NumHeads, NumHeadsSize));
	printf("HiddSec: %d\n", hexStringToDec(image->bootSector.HiddSec, HiddSecSize));
	printf("TotSec32: %d\n", hexStringToDec(image->bootSector.TotSec32, TotSec32Size));
	printf("FATSz32: %d\n", hexStringToDec(image->bootSector.FATSz32, FATSz32Size));
	printf("ExtFlags: %d\n", hexStringToDec(image->bootSector.ExtFlags, ExtFlagsSize));
	printf("FSVer: %d\n", hexStringToDec(image->bootSector.FSVer, FSVerSize));
	printf("RootClus: %d\n", hexStringToDec(image->bootSector.RootClus, RootClusSize));
	printf("FSInfo: %d\n", hexStringToDec(image->bootSector.FSInfo, FSInfoSize));
	printf("BkBootSec: %d\n", hexStringToDec(image->bootSector.BkBootSec, BkBootSecSize));
	printf("Reserved: %d\n", hexStringToDec(image->bootSector.Reserved, ReservedSize));
	printf("DrvNum: %d\n", hexStringToDec(image->bootSector.DrvNum, DrvNumSize));
	printf("Reservedl: %d\n", hexStringToDec(image->bootSector.Reservedl, ReservedlSize));
	printf("BootSig: %d\n", hexStringToDec(image->bootSector.BootSig, BootSigSize));
	printf("VolID: %d\n", hexStringToDec(image->bootSector.VolID, VolIDSize));

	char VolLabStr[VolLabSize+1];
	hexToASCII(image->bootSector.VolLab, VolLabSize, VolLabStr);
	printf("VolLab: %s\n", VolLabStr);

	char FilSysTypeStr[FilSysTypeSize+1];
	hexToASCII(image->bootSector.FilSysType, FilSysTypeSize, FilSysTypeStr);
	printf("FilSysType: %s\n\n", FilSysTypeStr);

}

// print the name field for the directories within the contents of DIRNAME including the “.” and “..” 
// directories. For simplicity, you may print each of the directory entries on separate lines
bool ls_command(char args[], const Image * image, char errorMessage[]) {

	// initial variables
	DirEntry entries[150]; // abirtrary amount of dir entries
	int dirEntryCount = 0; // current dir entry count
	int i, j;

	// if no arguments, then use cwd
	if (strcmp(args,"") == 0) {
		readDirEntriesInDir(image, image->currentCluster, entries, &dirEntryCount);
	}

	// arguments passed
	else {

		// make sure no spaces in dirName
		if (strchr(args, ' ') != NULL) {
			sprintf(errorMessage, "directory name cannot contain spaces\n");
			return false;
		}

		// finding dirname in current directory
		int dirIndex = -1;
		readDirEntriesInDir(image, image->currentCluster, entries, &dirEntryCount);
		for (i = 0; i < dirEntryCount; i++) {
			char dirName[NameSize+1];
			hexToASCII(entries[i].Name, NameSize, dirName);
			removeTrailingSpace(dirName);
			if (entries[i].Attr[0] == 0x10 && strcmp(args,dirName) == 0) {
				dirIndex = i;
				break;
			}
		}
		dirEntryCount = 0; // reset this

		// handle directory not found
		if (dirIndex == -1) {
			sprintf(errorMessage, "directory \"%s\" does not exist\n", args);
			return false;
		}

		// get first cluster # of directory
		unsigned char firstClusNum[FstClusHISize + FstClusLOSize];
		for (i = 0; i < FstClusLOSize; i++)
			firstClusNum[i] = entries[dirIndex].FstClusLO[i];
		for (i = 0; i < FstClusHISize; i++)
			firstClusNum[i+FstClusLOSize] = entries[dirIndex].FstClusHI[i];

		// convert cluster number to integer
		int dirClusNumber = hexStringToDec(firstClusNum, FstClusLOSize+FstClusHISize);

		// read dir entries from firstClusNum
		readDirEntriesInDir(image, dirClusNumber, entries, &dirEntryCount);

	}
	
	//create array of fixed directory entry structs
	DirEntry sortDirectories[dirEntryCount]; 

	//load directory entries into array for sorting 
	for (i = 0; i < dirEntryCount; i++)
		sortDirectories[i] = entries[i]; 

	//sort temp array
	qsort(sortDirectories, dirEntryCount, sizeof(const DirEntry), compareDirs);

	// print out dir entry names
	printf("\n");
	for (i = 0; i < dirEntryCount; i++) {
		if (sortDirectories[i].Attr[0] == 0x10)
			printf(ANSI_COLOR_GREEN);
		char name[NameSize];
		hexToASCII(sortDirectories[i].Name, NameSize, name);
		removeTrailingSpace(name);
		if (strcmp(name, "") == 0) continue; // ignore empty dir entries
		printf("%s\n", name);
		if (sortDirectories[i].Attr[0] == 0x10)
			printf(ANSI_COLOR_RESET);
	}

	// return success
	return true;
}

// changes the current working directory to DIRNAME.
bool cd_command(char args[], Image * image, char errorMessage[]) {

	// initial variables
	DirEntry entries[150]; // abirtrary amount of dir entries
	int dirEntryCount = 0; // current dir entry count
	int i, j;

	// check arguments
	if (strcmp(args,"") == 0) {
		return true;
	}

	// make sure no spaces in dirName
	if (strchr(args, ' ') != NULL) {
		sprintf(errorMessage, "directory name cannot contain spaces\n");
		return false;
	}

	// finding dirname in current directory
	int dirIndex = -1;
	readDirEntriesInDir(image, image->currentCluster, entries, &dirEntryCount);
	for (i = 0; i < dirEntryCount; i++) {
		char dirName[NameSize+1];
		hexToASCII(entries[i].Name, NameSize, dirName);
		removeTrailingSpace(dirName);
		if (entries[i].Attr[0] == 0x10 && strcmp(args,dirName) == 0) {
			dirIndex = i;
			break;
		}
	}
	dirEntryCount = 0; // reset this

	// handle directory not found
	if (dirIndex == -1) {
		sprintf(errorMessage, "directory \"%s\" does not exist\n", args);
		return false;
	}

	// get first cluster # of directory
	unsigned char firstClusNum[FstClusHISize + FstClusLOSize];
	for (i = 0; i < FstClusLOSize; i++)
		firstClusNum[i] = entries[dirIndex].FstClusLO[i];
	for (i = 0; i < FstClusHISize; i++)
		firstClusNum[i+FstClusLOSize] = entries[dirIndex].FstClusHI[i];

	// convert cluster number to integer
	int dirClusNumber = hexStringToDec(firstClusNum, FstClusLOSize+FstClusHISize);

	// set currentCluster to new cluster
	image->currentCluster = dirClusNumber;

	// set prompt
	if (image->currentCluster <= hexStringToDec(image->bootSector.RootClus, RootClusSize))
		image->promptDepth = 0;
	else {
		if (strcmp(args,"..") != 0 && (strcmp(args,".")!= 0)) {
			char name[NameSize+1];
			hexToASCII(entries[dirIndex].Name, NameSize, name);
			removeTrailingSpace(name);
			strcpy(image->cwd[image->promptDepth++], name);
		}
		else
			image->promptDepth--;
	}

	// return success
	return true;
}

// prints the size of the file FILENAME in the current working directory in bytes
bool size_command(char args[], const Image * image, char errorMessage[]) {

	// initial variables
	DirEntry entries[150]; // abirtrary amount of dir entries
	int dirEntryCount = 0; // current dir entry count
	int i, j;

	// check arguments
	if (strcmp(args,"") == 0) {
		sprintf(errorMessage, "no file provided\n");
		return false;
	}

	// make sure no spaces in dirName
	if (strchr(args, ' ') != NULL) {
		sprintf(errorMessage, "file name cannot contain spaces\n");
		return false;
	}

	// finding filename in current directory
	int fileIndex = -1;
	readDirEntriesInDir(image, image->currentCluster, entries, &dirEntryCount);
	for (i = 0; i < dirEntryCount; i++) {
		char fileName[NameSize+1];
		hexToASCII(entries[i].Name, NameSize, fileName);
		removeTrailingSpace(fileName);
		if (entries[i].Attr[0] == 0x20 && strcmp(args,fileName) == 0) {
			fileIndex = i;
			break;
		}
	}
	dirEntryCount = 0; // reset this

	// handle directory not found
	if (fileIndex == -1) {
		sprintf(errorMessage, "file \"%s\" does not exist\n", args);
		return false;
	}

	// read size variable
	int fileSize;
	fileSize = hexStringToDec(entries[fileIndex].FileSize, FileSizeSize);
	printf("Size of file \"%s\": %d\n", args, fileSize);

	// return success
	return true;

}

// creates a file in the current working directsory with a size of 0 bytes and with a name of FILENAME
bool creat_command(char args[], Image * image, char errorMessage[]) {

	// initial variables
	int clusters[1000];
	DirEntry entries[150]; // abirtrary amount of dir entries
	int dirEntryCount = 0; // current dir entry count
	int i, j;

	// check to make sure there is a filename given (strcmp(args,"") != 0)
	if (strcmp(args,"") == 0) {
		sprintf(errorMessage, "no file name provided\n");
		return false;
	}

	// check to make sure the filename does not contain spaces
	if (strchr(args, ' ') != NULL) {
		sprintf(errorMessage, "file name cannot contain spaces\n");
		return false;
	}

	// make sure no other dir entry in current directory has that name
	readDirEntriesInDir(image, image->currentCluster, entries, &dirEntryCount);
	for (i = 0; i < dirEntryCount; i++) {
		char fileName[NameSize+1];
		hexToASCII(entries[i].Name, NameSize, fileName);
		removeTrailingSpace(fileName);
		if (strcmp(args,fileName) == 0) {
			sprintf(errorMessage, "\"%s\" already exits", args);
			return false;
		}
	}
	dirEntryCount = 0; // reset this

	// find available cluster
	int firstCluster = findAvailCluster(image);
	if (firstCluster == -1) { // disk full, no more clusters
		sprintf(errorMessage, "no more space on disk");
		return false;
	}
	unsigned char endingVal[] = {0xFF,0xFF,0xFF,0xFF};
	setFATTableValue(image, firstCluster, endingVal);

	// create a new dir entry for the file
	DirEntry d = createDirEntry(args, 0x20, firstCluster);

	// get associated clusters
	getAssociatedClusters(image, image->currentCluster, clusters);

	// try to add dir entry to one of the clusters
	i = 0;
	bool added = false;
	while (clusters[i] != -1) {
		if (addDirEntryToCluster(image,d,clusters[i])) {
			added = true;
			break;
		}
		i++;
	}

	// if there wasn't enough room in the available clusters
	// add a new cluster and then add the dir entry to that cluster
	if (!added) {
		int newClus = addAdditionalCluster(image,image->currentCluster);
		if (newClus != -1)
			addDirEntryToCluster(image,d,newClus);
	}

	// update Image File
	if (!updateImageFile(image)) {
		printf("ERROR: image file might be corrupted");
		exit(1);
	}

	// print success message
	printf("File \"%s\" created successfully.\n", args);

	// done - return true
	return true;

}

// creates a new directory in the current working directory with the name DIRNAME
bool mkdir_command(char args[], Image * image, char errorMessage[]) {

	// initial variables
	int clusters[1000];
	DirEntry entries[150]; // abitrary amount of dir entries
	int dirEntryCount = 0; // current dir entry count
	int i, j;

	// check to make sure there is a directory name given (strcmp(args,"") != 0)
	if (strcmp(args,"") == 0) {
		sprintf(errorMessage, "no directory name provided\n");
		return false;
	}

	// check to make sure the directory name does not contain spaces
	if (strchr(args, ' ') != NULL) {
		sprintf(errorMessage, "directory name cannot contain spaces\n");
		return false;
	}

	// make sure no other dir entry in current directory has that name
	// also make sure user not passing ".." or "." as directory names
	readDirEntriesInDir(image, image->currentCluster, entries, &dirEntryCount);
	for (i = 0; i < dirEntryCount; i++) {
		char dirName[NameSize+1];
		hexToASCII(entries[i].Name, NameSize, dirName);
		removeTrailingSpace(dirName);
		if (strcmp(args,dirName) == 0 || strcmp(args,".") == 0 || strcmp(args,"..") == 0) {
			sprintf(errorMessage, "\"%s\" already exits", args);
			return false;
		}
	}
	dirEntryCount = 0; // reset this

	// find available cluster
	int dirCluster = findAvailCluster(image);
	if (dirCluster == -1) { // disk full, no more clusters
		sprintf(errorMessage, "no more space on disk");
		return false;
	}
	unsigned char endingVal[] = {0xFF,0xFF,0xFF,0xFF};
	setFATTableValue(image, dirCluster, endingVal);

	// create a new dir entry for the file
	DirEntry d = createDirEntry(args, 0x10, dirCluster);

	// get associated clusters
	getAssociatedClusters(image, image->currentCluster, clusters);

	// try to add dir entry to one of the clusters
	i = 0;
	bool added = false;
	while (clusters[i] != -1) {
		if (addDirEntryToCluster(image,d,clusters[i])) {
			added = true;
			break;
		}
		i++;
	}

	// if there wasn't enough room in the available clusters
	// add a new cluster and then add the dir entry to that cluster
	if (!added) {
		int newClus = addAdditionalCluster(image,image->currentCluster);
		if (newClus != -1)
			addDirEntryToCluster(image,d,newClus);
	}

	// write "." directory
	DirEntry currentDir = createDirEntry(".", 0x10, dirCluster);
	addDirEntryToCluster(image,currentDir,dirCluster);

	// write ".." directory
	DirEntry parentDir = createDirEntry("..", 0x10, image->currentCluster);
	addDirEntryToCluster(image,parentDir,dirCluster);

	// update Image File
	if (!updateImageFile(image)) {
		printf("ERROR: image file might be corrupted");
		exit(1);
	}

	// print success message
	printf("Directory \"%s\" created successfully.\n", args);

	return true;
}

// opens a file named FILENAME in the current working directory. A file can only be read from or
// written to if it is opened first. You will need to maintain a table of opened files and add FILENAME to
// it when open is called MODE is a string and is only valid if it is one of the following:
bool open_command(char args[], Image * image, char errorMessage[]) {

	// initial variables
	DirEntry entries[150];
	int i, mode, argsCount = 0, dirEntryCount = 0;
	char arguments[100][100];
	char filename[150];

	// check to make sure there is a filename given
	if((strcmp(args,"") == 0)){
		sprintf(errorMessage, "command \"open\" is missing arguments: FILENAME MODE\n");
		return false; 
	}

	// split arguments
	split(args, " ", &argsCount, arguments);
	if (argsCount != 2) {
		sprintf(errorMessage, "too many or too few arguments passed\n");
		return false;
	}

	// get filename and mode
	strcpy(filename, arguments[0]);
	if (strcmp(arguments[1], "r") == 0) mode = 0;
	else if (strcmp(arguments[1], "w") == 0) mode = 1;
	else if (strcmp(arguments[1], "rw") == 0) mode = 2;
	else if (strcmp(arguments[1], "wr") == 0) mode = 3;
	else {
		sprintf(errorMessage, "an invalid mode has been used. MODE may be: r w rw wr\n");
		return false;
	}

	// confirm file is not already open
	for (i = 0; i < image->openFilesCnt; i++) {
		if (strcmp(filename, image->openFiles[i].filename) == 0) {
			sprintf(errorMessage, "file \"%s\" is already open\n", filename);
			return false;
		}
	}

	// read in all dir entries from current directory
	readDirEntriesInDir(image, image->currentCluster, entries, &dirEntryCount);
	
	// find specified file index in current directory
	int fileIndex = -1;
	for(i = 0; i < dirEntryCount; i++)
	{	
		char name[NameSize+1];
		hexToASCII(entries[i].Name, NameSize, name);
		removeTrailingSpace(name);
		if (entries[i].Attr[0] == 0x20 && strcmp(filename,name) == 0) {
			fileIndex = i;
			break;
		}
	}

	// handle directory not found
	if (fileIndex == -1) {
		sprintf(errorMessage, "file \"%s\" does not exist\n", filename);
		return false;
	}

	// get cluster number of specified file
	unsigned char firstClusNum[FstClusHISize + FstClusLOSize];
	for (i = 0; i < FstClusLOSize; i++)
		firstClusNum[i] = entries[fileIndex].FstClusLO[i];
	for (i = 0; i < FstClusHISize; i++)
		firstClusNum[i+FstClusLOSize] = entries[fileIndex].FstClusHI[i];

	// convert cluster number to integer
	int clusNumber = hexStringToDec(firstClusNum, FstClusLOSize+FstClusHISize);

	// add open file to array of open files in image struct
	strcpy(image->openFiles[image->openFilesCnt].filename, filename);
	image->openFiles[image->openFilesCnt].firstCluster = clusNumber;
	image->openFiles[image->openFilesCnt].mode = mode;
	image->openFilesCnt++;

	// print success message
	printf("File opened successfully: %s\n", filename);

	// print open files
	printf("Open Files: ");
	for (i = 0; i < image->openFilesCnt; i++)
		printf("%s ", image->openFiles[i].filename);
	printf("\n");

	return true;
}

// closes a file named FILENAME. Needs to remove the file entry from the open file table
bool close_command(char args[], Image * image, char errorMessage[]) {

	// initial variables
	int i;

	// check to make sure there is a filename given
	if((strcmp(args,"") == 0)){
		sprintf(errorMessage, "command \"close\" is missing argument: FILENAME\n");
		return false; 
	}

	// check to make sure the file name does not contain spaces
	if (strchr(args, ' ') != NULL) {
		sprintf(errorMessage, "file name cannot contain spaces\n");
		return false;
	}

	// look for file in currently open files
	int index = -1;
	for (i = 0; i < image->openFilesCnt; i++) {
		if (strcmp(args, image->openFiles[i].filename) == 0) {
			index = i;
			break;
		}
	}

	// handle file does not exists
	if (index == -1) {
		sprintf(errorMessage, "file is not currently open\n");
		return false;
	}

	// remove file from list and shift list over
	for (i = index; i < image->openFilesCnt-1; i++) {
		strcpy(image->openFiles[i].filename, image->openFiles[i+1].filename);
		image->openFiles[i].firstCluster = image->openFiles[i+1].firstCluster;
		image->openFiles[i].mode = image->openFiles[i+1].mode;
	}
	image->openFilesCnt--;

	// printf success message
	printf("File closed successfully: %s\n", args);

	// print open files
	printf("Open Files: ");
	for (i = 0; i < image->openFilesCnt; i++)
		printf("%s ", image->openFiles[i].filename);
	printf("\n");

	return true;

}

// read the data from a file in the current working directory with the name FILENAME. Start reading
// from the file at OFFSET bytes and stop after reading SIZE bytes. If the OFFSET+SIZE is larger than
// the size of the file, just read size-OFFSET bytes starting at OFFSET
bool read_command(char args[], Image * image, char errorMessage[]) {

	// initial variables
	DirEntry entries[150];
	int i, mode, argsCount = 0, dirEntryCount = 0;
	int offset, size, firstCluster;
	int clusters[150];
	char arguments[100][100];
	char filename[150];

	// check to make sure there is a filename given
	if((strcmp(args,"") == 0)){
		sprintf(errorMessage, "command \"read\" is missing arguments: FILENAME OFFSET SIZE\n");
		return false; 
	}

	// split arguments
	split(args, " ", &argsCount, arguments);
	if (argsCount != 3) {
		sprintf(errorMessage, "too many or too few arguments passed\n");
		return false;
	}

	// get filename
	strcpy(filename, arguments[0]);

	// convert offset and size to ints
	offset = atoi(arguments[1]);
	size = atoi(arguments[2]);

	// look for file in currently open files
	if (image->openFilesCnt == 0) {
		sprintf(errorMessage, "file is not currently open or is not readable\n");
		return false;
	}
	for (i = 0; i < image->openFilesCnt; i++) {
		if (strcmp(filename, image->openFiles[i].filename) == 0 && image->openFiles[i].mode != 1) {
			break;
		}
		else if (i == image->openFilesCnt-1) {
			sprintf(errorMessage, "file is not currently open or is not readable\n");
			return false;
		}
	}

	// read in all dir entries from current directory
	readDirEntriesInDir(image, image->currentCluster, entries, &dirEntryCount);
	
	// find specified file index in current directory
	int fileIndex = -1;
	for(i = 0; i < dirEntryCount; i++)
	{	
		char name[NameSize+1];
		hexToASCII(entries[i].Name, NameSize, name);
		removeTrailingSpace(name);
		if (entries[i].Attr[0] == 0x20 && strcmp(filename,name) == 0) {
			fileIndex = i;
			break;
		}
	}

	// handle file not found
	if (fileIndex == -1) {
		sprintf(errorMessage, "file \"%s\" does not exist\n", filename);
		return false;
	}

	// check offset and size
	int file_size = hexStringToDec(entries[fileIndex].FileSize, FileSizeSize);
	if (offset > file_size) {
		sprintf(errorMessage, "offset is larger than file size\n");
		return false;
	}

	// get starting clusters of specified file
	unsigned char firstClusNum[FstClusHISize + FstClusLOSize];
	for (i = 0; i < FstClusLOSize; i++)
		firstClusNum[i] = entries[fileIndex].FstClusLO[i];
	for (i = 0; i < FstClusHISize; i++)
		firstClusNum[i+FstClusLOSize] = entries[fileIndex].FstClusHI[i];
	firstCluster = hexStringToDec(firstClusNum, FstClusLOSize+FstClusHISize);

	// get associated clusters
	getAssociatedClusters(image, firstCluster, clusters);

	// get bytes per sector
	int bytesPerSector = hexStringToDec(image->bootSector.BytsPerSec, BytsPerSecSize);

	// loop through clusters
	unsigned char fileBuffer[size];
	int fileBufferIndex = 0;
	int tempOffset = offset;
	i = 0;
	while (clusters[i] != -1) {

		// skip cluster if offset is bigger than cluster size
		if (tempOffset >= bytesPerSector) {
			tempOffset -= bytesPerSector;
			continue;
		}

		// set cluster position
		int currentClusPos;
		if (fileBufferIndex == 0)
			currentClusPos = tempOffset;
		else
			currentClusPos = 0;

		// get buffer position of cluster
		int bufferPos = getDataStartIndex(image) + bytesPerSector * (clusters[i]-2);

		// loop through cluster and read bytes
		while ((fileBufferIndex < size) && (currentClusPos < bytesPerSector) && (offset + fileBufferIndex < file_size)) {
			fileBuffer[fileBufferIndex] = image->buffer[bufferPos+currentClusPos];
			fileBufferIndex++;
			currentClusPos++;
		}

		// if index is equal to size, then break from loop
		if (fileBufferIndex >= size)
			break;	

		i++;
	}

	// convert byteBuffer to a string
	char str[fileBufferIndex+1];
	hexToASCII(fileBuffer, fileBufferIndex, str);

	// print what was read (character by character)
	for (i = 0; i < fileBufferIndex; i++)
		printf("%c", str[i]);
	printf("\n");

	// return success
	return true;

}

// writes to a file in the current working directory with the name FILENAME. Start writing at OFFSET
// bytes and stop after writing SIZE bytes. If OFFSET+SIZE is larger than the size of the file, you will
// need to extend the length of the file to at least hold the data being written
bool write_command(char args[], Image * image, char errorMessage[]) {

	// initial variables
	DirEntry entries[150];
	int i, j, mode, argsCount = 0, dirEntryCount = 0;
	int offset, size, firstCluster;
	int clusters[100000];
	char filename[150];
	char str[10000];
	char * tempArgs;
	char arguments[100][100];
	bool edgeCase = false; 
	int clusIndex;
	int clusterPos, fatPos, oldClusters, clusBytes, bufferPos, index, currentClusPos;

	// check to make sure there is a filename given
	if((strcmp(args,"") == 0)){
		sprintf(errorMessage, "command \"write\" is missing arguments: FILENAME OFFSET SIZE STRING\n");
		return false; 
	}

	// confirm string is in command and extract it
	int quote_count = 0;
	j = 0;
	for (i = 0; i < strlen(args); i++) {
		if (args[i] == '"')
			quote_count++;
		if (quote_count == 1 && args[i] != '"')
			str[j++] = args[i];
	}
	if (quote_count != 2) {
		sprintf(errorMessage, "no string passed\n");
		return false;
	}
	str[j] = '\0';

	// trim off string from input
	tempArgs = strtok(args, "\"");
	removeTrailingSpace(tempArgs);

	// split arguments
	split(tempArgs, " ", &argsCount, arguments);
	if (argsCount != 3) {
		sprintf(errorMessage, "too many or too few arguments passed\n");
		return false;
	}

	// get filename and string
	strcpy(filename, arguments[0]);

	// convert offset and size to ints
	offset = atoi(arguments[1]);
	size = atoi(arguments[2]);

	// look for file in currently open files
	if (image->openFilesCnt == 0) {
		sprintf(errorMessage, "file is not currently open or is not writeable\n");
		return false;
	}
	for (i = 0; i < image->openFilesCnt; i++) {
		if (strcmp(filename, image->openFiles[i].filename) == 0) {
			if (image->openFiles[i].mode >= 1) { // check is writable
				break;
			}
		}
		else if (i == image->openFilesCnt-1) {
			sprintf(errorMessage, "file is not currently open or is not writeable\n");
			return false;
		}
	}

	// read in all dir entries from current directory
	readDirEntriesInDir(image, image->currentCluster, entries, &dirEntryCount);
	
	// find specified file index in current directory
	int fileIndex = -1;
	for(i = 0; i < dirEntryCount; i++)
	{	
		char name[NameSize+1];
		hexToASCII(entries[i].Name, NameSize, name);
		removeTrailingSpace(name);
		if (entries[i].Attr[0] == 0x20 && strcmp(filename,name) == 0) {
			fileIndex = i;
			break;
		}
	}

	// handle directory not found
	if (fileIndex == -1) {
		sprintf(errorMessage, "file \"%s\" does not exist\n", filename);
		return false;
	}

	// get starting clusters of specified file
	unsigned char firstClusNum[FstClusHISize + FstClusLOSize];
	for (i = 0; i < FstClusLOSize; i++)
		firstClusNum[i] = entries[fileIndex].FstClusLO[i];
	for (i = 0; i < FstClusHISize; i++)
		firstClusNum[i+FstClusLOSize] = entries[fileIndex].FstClusHI[i];
	firstCluster = hexStringToDec(firstClusNum, FstClusLOSize+FstClusHISize);

	// get bytes per sector
	int bytesPerSector = hexStringToDec(image->bootSector.BytsPerSec, BytsPerSecSize);

	// calculate the number of new clusters (if any) needed to hold new data
	int file_size = hexStringToDec(entries[fileIndex].FileSize, FileSizeSize);
	if (offset > file_size) {
		sprintf(errorMessage, "offset is larger than file size");
		return false;
	}

	// calculate the number of new clusters (if any) needed to hold new data
	int additionalBytes = (offset + size) - file_size;
	int additionalClusters = 0;
	if (additionalBytes > 0)
		additionalClusters = ((file_size + additionalBytes) / bytesPerSector) - (file_size / bytesPerSector);

	// add additional clusters to file (if needed)
	for (i = 0; i < additionalClusters; i++) {
		int check = addAdditionalCluster(image, firstCluster);
		if (check == -1) {
			sprintf(errorMessage, "no more space on disk\n");
			return false;
		}
	}

	// check for edge case
	if((offset+size) < file_size)
		edgeCase = true; 

	// get associated clusters
	getAssociatedClusters(image, firstCluster, clusters);

	// loop through clusters and write string
	int stringIndex = 0;
	int tempOffset = offset;
	i = 0;
	while (clusters[i] != -1) {

		// skip cluster if offset is bigger than cluster size
		if (tempOffset >= bytesPerSector) {
			tempOffset -= bytesPerSector;
			continue;
		}

		// set cluster position
		int currentClusPos;
		if (stringIndex == 0)
			currentClusPos = tempOffset;
		else
			currentClusPos = 0;

		// overwrite buffer with string
		while (stringIndex < size && currentClusPos < bytesPerSector) {
			bufferPos = getDataStartIndex(image);
			bufferPos += bytesPerSector * (clusters[i]-2);
			bufferPos += currentClusPos;

			if (stringIndex < strlen(str))
				image->buffer[bufferPos] = (unsigned char) str[stringIndex];
			else
				image->buffer[bufferPos] = 0x00;
			
			stringIndex++;
			currentClusPos++;
		}
		
		i++;
	}

	// handle edge case
	// remove clusters and unlink FAT table entries if use case encountered 
	if(edgeCase)
	{
		// get the clusters to remove
		int clusToRemove = ceil(1.0 * (file_size - (offset+size)) / bytesPerSector);

		// get cluster count
		int totalClusters = 0;
		while (clusters[totalClusters] != -1) totalClusters++;

		// loop through clusters
		for (i = totalClusters - 1; i > totalClusters-clusToRemove; i--) {
			clusterPos = getDataStartIndex(image) + (clusters[i] -2) * bytesPerSector;
			fatPos = getFATStartIndex(image) + (clusters[i] * 4);
			for(j = 0; j < bytesPerSector; j++)
				image->buffer[clusterPos + j] = 0x00;
			for(j = 0; j < 4; j++)
				image->buffer[fatPos + j] = 0x00;
		}

		// clear out the new "final" cluster
		int finalCluster = clusters[totalClusters-clusToRemove];
		int leftover = (offset+size) % bytesPerSector;

		// clear out the unecessary bytes in the cluster
		clusterPos = getDataStartIndex(image) + (finalCluster -2) * bytesPerSector;
		fatPos = getFATStartIndex(image) + (finalCluster * 4);
		for(j = leftover; j < bytesPerSector; j++)
			image->buffer[clusterPos + j] = 0x00;
		for(j = 0; j < 4; j++)
			image->buffer[fatPos + j] = 0xFF;

		// update file size
		file_size = offset + size;
		bufferPos = getDataStartIndex(image);
		bufferPos += (image->currentCluster - 2) * bytesPerSector;
		bufferPos += fileIndex * 64 + 32 + 28;
		for (i = 0; i < sizeof(file_size); i++)
			image->buffer[bufferPos+i] = *((unsigned char *)&file_size + i);
	}

	// update file size
	if (!edgeCase) {
		if (additionalBytes > 0) {
			unsigned char bytes[4];
			int bufferPos = getDataStartIndex(image);
			bufferPos += (image->currentCluster - 2) * bytesPerSector;
			bufferPos += fileIndex * 64 + 32 + 28;
			file_size += additionalBytes;
			intToUnsignedChar(file_size,bytes);
			for (i = 0; i < 4; i++)
				image->buffer[bufferPos+i] = bytes[i];
		}
	} else {
		getAssociatedClusters(image, image->currentCluster, clusters);
		bool found = false;
		index = 0;
		while(clusters[index] != -1)
		{
			// store the cluster
			unsigned char cluster[getClusterSize(image)];
			getCluster(image, clusters[index], cluster);

			// loop through dir entries in specific cluster
			for (i = 1; i <= 8; i++) {
				DirEntry d = readDirEntry(cluster, i);
				char dirName[NameSize+1];
				hexToASCII(d.Name, NameSize, dirName);
				removeTrailingSpace(dirName);
				if (strcmp(args,dirName) == 0) {
					found = true;
					fileIndex = i;
					break;
				}
			}
			if (found) break;

			index++;
		}

		unsigned char bytes[4];
		int bufferPos = getDataStartIndex(image) + (clusters[index]-2) * bytesPerSector + (fileIndex-1) * 64;
		file_size = offset + size;
		intToUnsignedChar(file_size,bytes);
		for (i = 0; i < 4; i++)
			image->buffer[bufferPos+i] = bytes[i];
	}

	// update image file
	if (!updateImageFile(image)) {
		printf("ERROR: image file might be corrupted");
		exit(1);
	}

	// print success message
	printf("Successfully wrote to file %s.\n", filename);
	
	// return succes
	return true;

}

// deletes the file named FILENAME from the current working directory. This needs remove the entry in
// the directory as well as reclaiming the actual file data
bool rm_command(char args[], Image * image, char errorMessage[]) {
	DirEntry entries[150];
	int clusters[150];
	int dirClusters[5];
	int dirEntryCount = 0, index = 0; 
	int i;

	// check to make sure there is a filename given
	if((strcmp(args,"") == 0)){
		sprintf(errorMessage, "no file name specified");
		return false; 
	}
	
	// check to make sure the file name does not contain spaces
	if (strchr(args, ' ') != NULL) {
		sprintf(errorMessage, "file name cannot contain spaces\n");
		return false;
	}

	// look for file in currently open files
	for (i = 0; i < image->openFilesCnt; i++) {
		if (strcmp(args, image->openFiles[i].filename) == 0){
			sprintf(errorMessage, "file is currently open\n");
			return false; 
		}
	}

	// get all directory entries in current directory
	readDirEntriesInDir(image, image->currentCluster, entries, &dirEntryCount);
	int dirIndex = -1;
	
	// find specified dirs index in current directory
	for(i=0; i<dirEntryCount; i++)
	{
		char dirName[NameSize+1];
		hexToASCII(entries[i].Name, NameSize, dirName);
		removeTrailingSpace(dirName);
		if (entries[i].Attr[0] == 0x20 && strcmp(args,dirName) == 0) {
			dirIndex = i;
			break;
		}
	}

	// handle directory not found
	if (dirIndex == -1) {
		sprintf(errorMessage, "file \"%s\" does not exist\n", args);
		return false;
	}

	// get cluster number of specified file
	unsigned char firstClusNum[FstClusHISize + FstClusLOSize];
	for (i = 0; i < FstClusLOSize; i++)
		firstClusNum[i] = entries[dirIndex].FstClusLO[i];
	for (i = 0; i < FstClusHISize; i++)
		firstClusNum[i+FstClusLOSize] = entries[dirIndex].FstClusHI[i];

	// convert cluster number to integer
	int clusNumber = hexStringToDec(firstClusNum, FstClusLOSize+FstClusHISize);

	//calculate files cluster index inside data region
	int bytesPerSector = hexStringToDec(image->bootSector.BytsPerSec, BytsPerSecSize);

	//get all clusters associated with file
	getAssociatedClusters(image, clusNumber, clusters);

	//loop through all clusters associated with file
	while(clusters[index] != -1)
	{	
		int clusterPos = getDataStartIndex(image) + (clusters[index] -2) * bytesPerSector;
		int fatPos = getFATStartIndex(image) + (clusters[index] * 4);

		//remove dirEntry from data region
		for(i = 0; i<512; i++)
	    	image->buffer[clusterPos + i] = 0x00;

	    //remove specified file from FAT region
	    for(i = 0; i < 4; i++)
	    	image->buffer[fatPos + i] = 0x00;

		index++;

	}

	//get all clusters associated with file
	getAssociatedClusters(image, image->currentCluster, clusters);

	//loop through all clusters associated with file
	bool found = false;
	index = 0;
	while(clusters[index] != -1)
	{
		// store the cluster
		unsigned char cluster[getClusterSize(image)];
		getCluster(image, clusters[index], cluster);

		// loop through dir entries in specific cluster
		for (i = 1; i <= 8; i++) {
			DirEntry d = readDirEntry(cluster, i);
			char dirName[NameSize+1];
			hexToASCII(d.Name, NameSize, dirName);
			removeTrailingSpace(dirName);
			if (strcmp(args,dirName) == 0) {
				found = true;
				dirIndex = i;
				break;
			}
		}
		if (found) break;

		index++;
	}

	// remove dir entry from parent directory
	if (found) {
	    int clusterPos = getDataStartIndex(image) + (clusters[index]-2) * bytesPerSector + (dirIndex-1) * 64;
	    for(i = 0; i<64; i++)
	    	image->buffer[clusterPos + i] = 0x00;
	}

	// update image file
	if (!updateImageFile(image)) {
		printf("ERROR: image file might be corrupted");
		exit(1);
	}

	// print success message
	printf("File \"%s\" removed successfully.\n", args);

	return true;
}

// removes a directory by the name of DIRNAME from the current working directory. Make sure to
// remove the entry from the current working directory and to remove the data DIRNAME points to
bool rmdir_command(char args[], Image * image, char errorMessage[]) {

	DirEntry entries[150];
	int clusters[150];
	int dirClusters[5];
	int dirEntryCount = 0, index = 0; 

	// check to make sure there is a directory name given
	if((strcmp(args,"") == 0)){
		sprintf(errorMessage, "no directory name specified");
		return false; 
	}
	
	// check to make sure the directory name does not contain spaces
	if (strchr(args, ' ') != NULL) {
		sprintf(errorMessage, "directory name cannot contain spaces\n");
		return false;
	}

	// make sure user isn't passing in "." or ".." as directory names
	if (strcmp(args,".") == 0 || strcmp(args,"..") == 0) {
		sprintf(errorMessage, "failed to remove \"%s\": Invalid argument\n", args);
		return false;
	}

	// get all directory entries in current directory
	readDirEntriesInDir(image, image->currentCluster, entries, &dirEntryCount);
	int i;
	int dirIndex = -1;
	
	// find specified dirs index in current directory
	for(i=0; i<dirEntryCount; i++)
	{	
		char dirName[NameSize+1];
		hexToASCII(entries[i].Name, NameSize, dirName);
		removeTrailingSpace(dirName);
		if (entries[i].Attr[0] == 0x10 && strcmp(args,dirName) == 0) {
			dirIndex = i;
			break;
		}
	}

	// handle directory not found
	if (dirIndex == -1) {
		sprintf(errorMessage, "directory \"%s\" does not exist\n", args);
		return false;
	}

	// get cluster number of specified directory
	unsigned char firstClusNum[FstClusHISize + FstClusLOSize];
	for (i = 0; i < FstClusLOSize; i++)
		firstClusNum[i] = entries[dirIndex].FstClusLO[i];
	for (i = 0; i < FstClusHISize; i++)
		firstClusNum[i+FstClusLOSize] = entries[dirIndex].FstClusHI[i];

	// convert cluster number to integer
	int dirClusNumber = hexStringToDec(firstClusNum, FstClusLOSize+FstClusHISize);

	// get all directory entries inside specified directory
	readDirEntriesInDir(image, dirClusNumber, entries, &dirEntryCount);

	// handle directory-not-empty
	if(dirEntryCount > 2)
	{
		sprintf(errorMessage, "directory \"%s\" is not empty\n", args);
		return false;
	}

	// calculate dirs cluster index inside data region
	int bytesPerSector = hexStringToDec(image->bootSector.BytsPerSec, BytsPerSecSize);
	int clusterPos = getDataStartIndex(image) + (dirClusNumber-2) * bytesPerSector;
	int fatPos = getFATStartIndex(image) + dirClusNumber * 4;

	// remove "." and ".." from data region
	for(i = 0; i<bytesPerSector; i++)
    	image->buffer[clusterPos + i] = 0x00;

    // remove specified directory from FAT region
    for(i = 0; i < 4; i++)
    	image->buffer[fatPos + i] = 0x00;

    //get all clusters associated with file
	getAssociatedClusters(image, image->currentCluster, clusters);

    //loop through all clusters associated with file
	bool found = false;
	index = 0;
	while(clusters[index] != -1)
	{
		// store the cluster
		unsigned char cluster[getClusterSize(image)];
		getCluster(image, clusters[index], cluster);

		// loop through dir entries in specific cluster
		for (i = 1; i <= 8; i++) {
			DirEntry d = readDirEntry(cluster, i);
			char dirName[NameSize+1];
			hexToASCII(d.Name, NameSize, dirName);
			removeTrailingSpace(dirName);
			if (strcmp(args,dirName) == 0) {
				found = true;
				dirIndex = i;
				break;
			}
		}
		if (found) break;

		index++;
	}

	// remove dir entry from parent directory
	if (found) {
	    int clusterPos = getDataStartIndex(image) + (clusters[index]-2) * bytesPerSector + (dirIndex-1) * 64;
	    for(i = 0; i<64; i++)
	    	image->buffer[clusterPos + i] = 0x00;
	}

	// update image file
	if (!updateImageFile(image)) {
		printf("ERROR: image file might be corrupted");
		exit(1);
	}

	// print success message
	printf("Directory \"%s\" removed successfully.\n", args);

	return true; 
}
