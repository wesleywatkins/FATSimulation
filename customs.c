/*
  Project 3 for COP4610
  Function Definitions
*/

#include "proj3.h"


// function for getting the index of the beginning of the FAT Region
// simply return the size of the Reserved region
int getFATStartIndex(const Image * image) 
{
	return getReservedSize(image);
}

// function for getting the index of beginning of the DATA Region
// return the size of the Reserved region + FAT region
int getDataStartIndex(const Image * image) 
{
	return getReservedSize(image) + getFATSize(image);
}

// get total clusters in image file
int getTotalClusters(const Image * image) 
{
	return getFATSize(image)/4;
}

// return the number of bytes in a cluster
int getClusterSize(const Image * image) 
{
	int bytesPerSector = hexStringToDec(image->bootSector.BytsPerSec, BytsPerSecSize);
	int sectorsPerCluster = hexStringToDec(image->bootSector.SecPerClus, SecPerClusSize);
	return bytesPerSector * sectorsPerCluster;
}

// get the size of the Reserved region by multiplying
// the bytes per sector by the number of reserved sectors
int getReservedSize(const Image * image) 
{
	int bytes_per_sector = hexStringToDec(image->bootSector.BytsPerSec, BytsPerSecSize);
	int reserved_sectors = hexStringToDec(image->bootSector.RsvdSecCnt, RsvdSecCntSize);
	return bytes_per_sector * reserved_sectors;
}

// get the size of the FAT region by multiplying the sectors
// per FAT Region (FATSz) by the bytes per sector (BytsPerSec)
// by the number of FAT sections (NumFATs)
int getFATSize(const Image * image) 
{
	int FATSz = hexStringToDec(image->bootSector.FATSz32, FATSz32Size);
	int NumberOfFATs = hexStringToDec(image->bootSector.NumFATs, NumFATsSize);
	int bytesPerSector = hexStringToDec(image->bootSector.BytsPerSec, BytsPerSecSize);
	return (FATSz * NumberOfFATs * bytesPerSector);
}

// get the size of the Data Region by subtracting the total
// image size by the size of the Reserved and FAT regions
int getDataSize(const Image * image) 
{
	return image->size - getReservedSize(image) - getFATSize(image);
}

// function for getting the bytes from a cluster
void getCluster(const Image * image, int clusterNumber, unsigned char * cluster) 
{
	// print tracing message
	if (dev) printf("getCluster(image pointer, %d, cluster array)\n", clusterNumber);
	// error check cluster number
	clusterNumber = (clusterNumber < 2) ? 2 : clusterNumber;
	clusterNumber = (clusterNumber > getTotalClusters(image)+2) ? getTotalClusters(image) : clusterNumber;
	// read in cluster
	int startIndex = getDataStartIndex(image) + (clusterNumber - 2) * getClusterSize(image);
	int i;
	for (i = 0; i < getClusterSize(image); i++)
		cluster[i] = image->buffer[startIndex+i];
}

// search the fat region for a free cluster
int findAvailCluster(const Image * image) 
{
	// print tracing message
	if (dev) printf("findAvailCluster(image pointer)\n");
	// variables for reading in FAT Region
	int FATRegion[getTotalClusters(image)]; // 4 bytes = cluster in FAT region
	getFATRegion(image, FATRegion); // stores FAT region into int array
	// loop through and find free cluster (= 0)
	int i;
	int root = hexStringToDec(image->bootSector.RootClus, RootClusSize);
	for (i = root; i < getTotalClusters(image); i++)
		if (FATRegion[i] == 0)
			return i;
	return -1;
}

// add new clusters to directories or files that take up
// more than a single cluster
int addAdditionalCluster(const Image * image, int firstCluster) 
{
	// print tracing message
	if (dev) printf("addAdditionalCluster(image pointer, %d)\n", firstCluster);

	// useful variables
	int FATstartIndex = getFATStartIndex(image);
	int i, pos;

	// error check firstCluster
	if (firstCluster < hexStringToDec(image->bootSector.RootClus, RootClusSize))
		return -1;
	else if (firstCluster > getTotalClusters(image))
		return -1;
	
	// store all current clusters representing file/directory
	int clusters[1000];
	getAssociatedClusters(image, firstCluster, clusters);

	// find available cluster
	int newCluster = findAvailCluster(image);
	if (newCluster == -1) return -1;

	// convert new cluster value to bytes
	unsigned char bytes[4];
	intToUnsignedChar(newCluster, bytes);

	// find last cluster
	i = 0;
	while (clusters[i] != -1) i++;

	// set old end marker to be pointed at new cluster
	pos = clusters[i-1] * 4 + FATstartIndex;

	// replace bytes
	for (i = 0; i < 4; i++) {
		image->buffer[pos+i] = bytes[i];
	}

	// change pos to represent new cluster
	pos = FATstartIndex + newCluster * 4;

	// set to end marker
	for (i = 0; i < 4; i++)
		image->buffer[pos+i] = 0xFF;

	// update image file
	if (!updateImageFile(image)) {
		printf("ERROR: image file might be corrupted");
		exit(1);
	}

	return newCluster;


}

// set value in FAT Table
void setFATTableValue(const Image * image, int index, unsigned char * value) 
{
	// print tracing message
	if (dev) printf("setFATTableValue(image pointer, %d, %x%x%x%x)\n", index, value[0], value[1], value[2], value[3]);
	// get position in buffer to change FAT table
	int FATstart = getFATStartIndex(image);
	int bufferPos = FATstart + index * 4;
	// loop through and change FAT table values
	int i;
	for (i = 0; i < 4; i++)
		image->buffer[bufferPos+i] = value[i];
}

// get array of associated characters
void getAssociatedClusters(const Image * image, int startingCluster, int * clusters) 
{
	// print tracing message
	if (dev) printf("getAssociatedClusters(image pointer, %d, clusters array)\n", startingCluster);

	// error check cluster number
	startingCluster = (startingCluster < 2) ? 2 : startingCluster;
	startingCluster = (startingCluster > getTotalClusters(image)+2) ? getTotalClusters(image) : startingCluster;
	
	// read the fat image into an array
	int FATRegion[getTotalClusters(image)]; // 4 bytes = cluster in FAT region
	getFATRegion(image, FATRegion); // stores FAT region into int array

	// traverse through clusters
	int i = 0;
	while (FATRegion[startingCluster] < 0x0FFFF8 && FATRegion[startingCluster] != 0) { // while not at ending cluster
		clusters[i++] = startingCluster; // add current cluster to array
		startingCluster = FATRegion[startingCluster]; // change starting cluster
	}
	clusters[i++] = startingCluster;

	// set end point (since we don't return a size)
	clusters[i] = -1;
}

// function for reading in the FAT region of the image file
// reading in clusters and store them in the FAT Region array
void getFATRegion(const Image * image, int * FATRegion) 
{
	// print tracing message
	if (dev) printf("getFATRegion(image pointer, int array)\n");
	// array for storing cluster value	
	unsigned char temp[4];
	// loop through clusters, convert every 4 bytes to an int
	int i, j, count = 0;
	for (i = getFATStartIndex(image); i < getDataStartIndex(image); i = i + 4) {
		for (j = 0; j < 4; j++)
			temp[j] = image->buffer[i+j];
		FATRegion[count++] = hexStringToDec(temp,4);
	}
}

// add a directory to a specific cluster
bool addDirEntryToCluster(const Image * image, DirEntry d, int cluster) 
{
	// print tracing message
	if (dev) printf("addDirEntryToCluster(image pointer, dir entry, %d", cluster);

	// initial variables
	int i, j;

	// error check cluster number
	cluster = (cluster < 2) ? 2 : cluster;
	cluster = (cluster > getTotalClusters(image)+2) ? getTotalClusters(image) : cluster;

	// get buffer position
	int bufferPos = getDataStartIndex(image) + (cluster - 2) * getClusterSize(image);

	// search for empty spot
	int clusterPos = -1;
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 64; j++) {
			if (image->buffer[bufferPos + j + (i * 64)] != 0x00)
				break;
			else if (j == 63)
				clusterPos = i;
		}
		if (clusterPos != -1) break;
	}

	// error checking
	if (clusterPos == -1) return false;

	// change buffer pos
	bufferPos += (clusterPos * 64) + 32;

	// copy values into buffer
	for (i = 0; i < NameSize; i++)
		image->buffer[bufferPos++] = d.Name[i];
	for (i = 0; i < AttrSize; i++)
		image->buffer[bufferPos++] = d.Attr[i];
	for (i = 0; i < NTresSize; i++)
		image->buffer[bufferPos++] = d.NTres[i];
	for (i = 0; i < CrtTimeTenthSize; i++)
		image->buffer[bufferPos++] = d.CrtTimeTenth[i];
	for (i = 0; i < CrtTimeSize; i++)
		image->buffer[bufferPos++] = d.CrtTime[i];
	for (i = 0; i < CrtDateSize; i++)
		image->buffer[bufferPos++] = d.CrtDate[i];
	for (i = 0; i < LstAccDateSize; i++)
		image->buffer[bufferPos++] = d.LstAccDate[i];
	for (i = 0; i < FstClusHISize; i++)
		image->buffer[bufferPos++] = d.FstClusHI[i];
	for (i = 0; i < WrtTimeSize; i++)
		image->buffer[bufferPos++] = d.WrtTime[i];
	for (i = 0; i < WrtDateSize; i++)
		image->buffer[bufferPos++] = d.WrtDate[i];
	for (i = 0; i < FstClusLOSize; i++)
		image->buffer[bufferPos++] = d.FstClusLO[i];
	for (i = 0; i < FileSizeSize; i++)
		image->buffer[bufferPos++] = d.FileSize[i];

	return true;

}

// read directory entries in the current directory into entry array
void readDirEntriesInDir(const Image * image, int dirClusNumber, DirEntry entries[], int * size) 
{
	// print tracing message
	if (dev) printf("readDirEntriesInDir(image pointer, %d, dir entry array, %d)\n", dirClusNumber, *size);

	// error check dirClusNumber
	dirClusNumber = (dirClusNumber < 2) ? 2 : dirClusNumber;
	dirClusNumber = (dirClusNumber > getTotalClusters(image)+2) ? getTotalClusters(image) : dirClusNumber;
	
	// read in clusters to search
	int clusters[150], i, j;
	getAssociatedClusters(image, dirClusNumber, clusters);

	// loop through clusters
	i = 0;
	*size = 0;
	while (clusters[i] != -1) {
		// store the cluster
		unsigned char cluster[getClusterSize(image)];
		getCluster(image, clusters[i], cluster);
		// loop through each dirEntry in the cluster
		for (j = 1; j <= 8; j++) {
			DirEntry d = readDirEntry(cluster, j);
			if (d.Attr[0] != 0x00) // if attr is 0, then the dir is empty
				entries[(*size)++] = d;
		}
		i++;
	}

}

// create a dir entry off certain parameters
DirEntry createDirEntry(const char * name, unsigned char attr, unsigned int firstCluster) 
{
	// print tracing message
	if (dev) printf("createDirEntry(%s, %x, %d)\n", name, attr, firstCluster);

	// initial variables
	DirEntry dirEntry;
	int i;

	// set name
	int size = (strlen(name) < NameSize) ? strlen(name) : NameSize;
	for (i = 0; i < size; i++)
		dirEntry.Name[i] = (unsigned char) name[i];
	for (i = size; i < NameSize; i++)
		dirEntry.Name[i] = 0x00;

	// set attr
	dirEntry.Attr[0] = attr;

	// set NTRes
	dirEntry.NTres[0] = 0x00;

	// set first cluster low and hi
	unsigned char bytes[4];
	intToUnsignedChar(firstCluster, bytes);
	dirEntry.FstClusLO[0] = bytes[0];
	dirEntry.FstClusLO[1] = bytes[1];
	dirEntry.FstClusHI[0] = bytes[2];
	dirEntry.FstClusHI[1] = bytes[3];

	// set File Size
	for (i = 0; i < FileSizeSize; i++)
		dirEntry.FileSize[i] = 0x00;

	//ignore timestamps for this project
	dirEntry.CrtTimeTenth[0] = 0x00;
	dirEntry.CrtTime[0] = 0x00;
	dirEntry.CrtTime[1] = 0x00;
	dirEntry.CrtDate[0] = 0x00;
	dirEntry.CrtDate[1] = 0x00;
	dirEntry.LstAccDate[0] = 0x00;
	dirEntry.LstAccDate[1] = 0x00;
	dirEntry.WrtTime[0] = 0x00;
	dirEntry.WrtDate[1] = 0x00;

	// return
	return dirEntry;

}

// read in directory entry given 64 bit directory entry
// cluster = cluster of data from data region
// entry = which dir entry in the cluster 1-8 to read
DirEntry readDirEntry(const unsigned char * cluster, int entry) 
{
	// print tracing message
	if (dev) printf("readDirEntry(cluster array, %d)\n", entry);

	// initial variables
	DirEntry dirEntry;

	// skip first 32 bytes
	int startIndex = (64 * --entry) + 32;

	// read in DirEntry values
	memcpy(dirEntry.Name, cluster+startIndex, NameSize); startIndex += NameSize;
	memcpy(dirEntry.Attr, cluster+startIndex, AttrSize); startIndex += AttrSize;
	memcpy(dirEntry.NTres, cluster+startIndex, NTresSize); startIndex += NTresSize;
	memcpy(dirEntry.CrtTimeTenth, cluster+startIndex, CrtTimeTenthSize); startIndex += CrtTimeTenthSize;
	memcpy(dirEntry.CrtTime, cluster+startIndex, CrtTimeSize); startIndex += CrtTimeSize;
	memcpy(dirEntry.CrtDate, cluster+startIndex, CrtDateSize); startIndex += CrtDateSize;
	memcpy(dirEntry.LstAccDate, cluster+startIndex, LstAccDateSize); startIndex += LstAccDateSize;
	memcpy(dirEntry.FstClusHI, cluster+startIndex, FstClusHISize); startIndex += FstClusHISize;
	memcpy(dirEntry.WrtTime, cluster+startIndex, WrtTimeSize); startIndex += WrtTimeSize;
	memcpy(dirEntry.WrtDate, cluster+startIndex, WrtDateSize); startIndex += WrtDateSize;
	memcpy(dirEntry.FstClusLO, cluster+startIndex, FstClusLOSize); startIndex += FstClusLOSize;
	memcpy(dirEntry.FileSize, cluster+startIndex, FileSizeSize); startIndex += FileSizeSize;

	// return directory entry after loading values
	return dirEntry;
}

// update the image file
// write buffer into image file
bool updateImageFile(const Image * image) 
{
	// print tracing message
	if (dev) printf("updateImageFile(image pointer)\n");
	// initial variables
	FILE * imageFile;
	int bytesWritten;
	// attempt to write file
	imageFile = fopen(image->filename, "wb");
	bytesWritten = fwrite(image->buffer,sizeof(unsigned char),image->size,imageFile);
	// return if it was updated successfully
	return (bytesWritten == image->size);
}

// read in image file and store it into an Image struct
// why struct? so we can save the buffer for the bytes along with
// the size of the buffer
void readInImageFile(const char * imageFileName, Image * image) {

	// initial variables
	FILE * imageFile;
	int readSize;

	// open file
	imageFile = fopen(imageFileName, "rb");
	if (!imageFile) { // check if file open correctly
		printf("ERROR: Unable to open image file\n");
		exit(1);
	}

	// get file size
	fseek(imageFile, 0, SEEK_END); // traverse file
    image->size = ftell(imageFile); // get size of file
    rewind(imageFile);			   // rewind to file start

    // read contents into buffer
    image->buffer = (unsigned char*)calloc(image->size, sizeof(unsigned char));
    readSize = fread(image->buffer, sizeof(unsigned char), image->size, imageFile);

    // confirm file was read correctly
    if (readSize != image->size) {
    	printf("ERROR: Unable to read image file\n");
    	exit(1);
    }

    // set open file count
    image->openFilesCnt = 0;

    // set current directory to root and set filename
    image->promptDepth = 0;
    strcpy(image->filename, imageFileName);
}

// read in the boot sector of the image file and save
// it into the Boot Sector struct
BootSector readBootSector(const char * imageFileName) {

	// initial variables
	BootSector bootSector;
	FILE * imageFile;

	// open file
	imageFile = fopen(imageFileName, "rb");
	if (!imageFile) {
		printf("ERROR: Unable to open image file");
		exit(1);
	}

	// read each boot sector section into its variable using its
	// predefined size
	fread(bootSector.jmpBoot, sizeof(unsigned char), jmpBootSize, imageFile);
	fread(bootSector.OEMName, sizeof(unsigned char), OEMNameSize, imageFile);
	fread(bootSector.BytsPerSec, sizeof(unsigned char), BytsPerSecSize, imageFile);
	fread(bootSector.SecPerClus, sizeof(unsigned char), SecPerClusSize, imageFile);
	fread(bootSector.RsvdSecCnt, sizeof(unsigned char), RsvdSecCntSize, imageFile);
	fread(bootSector.NumFATs, sizeof(unsigned char), NumFATsSize, imageFile);
	fread(bootSector.RootEntCnt, sizeof(unsigned char), RootEntCntSize, imageFile);
	fread(bootSector.TotSec16, sizeof(unsigned char), TotSec16Size, imageFile);
	fread(bootSector.Media, sizeof(unsigned char), MediaSize, imageFile);
	fread(bootSector.FATSz16, sizeof(unsigned char), FATSz16Size, imageFile);
	fread(bootSector.SecPerTrk, sizeof(unsigned char), SecPerTrkSize, imageFile);
	fread(bootSector.NumHeads, sizeof(unsigned char), NumHeadsSize, imageFile);
	fread(bootSector.HiddSec, sizeof(unsigned char), HiddSecSize, imageFile);
	fread(bootSector.TotSec32, sizeof(unsigned char), TotSec32Size, imageFile);
	fread(bootSector.FATSz32, sizeof(unsigned char), FATSz32Size, imageFile);
	fread(bootSector.ExtFlags, sizeof(unsigned char), ExtFlagsSize, imageFile);
	fread(bootSector.FSVer, sizeof(unsigned char), FSVerSize, imageFile);
	fread(bootSector.RootClus, sizeof(unsigned char), RootClusSize, imageFile);
	fread(bootSector.FSInfo, sizeof(unsigned char), FSInfoSize, imageFile);
	fread(bootSector.BkBootSec, sizeof(unsigned char), BkBootSecSize, imageFile);
	fread(bootSector.Reserved, sizeof(unsigned char), ReservedSize, imageFile);
	fread(bootSector.DrvNum, sizeof(unsigned char), DrvNumSize, imageFile);
	fread(bootSector.Reservedl, sizeof(unsigned char), ReservedlSize, imageFile);
	fread(bootSector.BootSig, sizeof(unsigned char), BootSigSize, imageFile);
	fread(bootSector.VolID, sizeof(unsigned char), VolIDSize, imageFile);
	fread(bootSector.VolLab, sizeof(unsigned char), VolLabSize, imageFile);
	fread(bootSector.FilSysType, sizeof(unsigned char), FilSysTypeSize, imageFile);

	return bootSector;
}

