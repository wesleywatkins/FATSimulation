/*
  Project 3 for COP4610
  Header File
*/

#ifndef PROJ3_H
#define PROJ3_H

// includes
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <math.h>

// typedefs and global definitions
typedef int bool;
#define true 1
#define false 0
#define dev 0

// colored output
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"

// global variables for Boot Sector
#define jmpBootSize 3
#define OEMNameSize 8
#define BytsPerSecSize 2
#define SecPerClusSize 1
#define RsvdSecCntSize 2
#define NumFATsSize 1
#define RootEntCntSize 2
#define TotSec16Size 2
#define MediaSize 1
#define FATSz16Size 2
#define SecPerTrkSize 2
#define NumHeadsSize 2
#define HiddSecSize 4
#define TotSec32Size 4
#define FATSz32Size 4
#define ExtFlagsSize 2
#define FSVerSize 2
#define RootClusSize 4
#define FSInfoSize 2
#define BkBootSecSize 2
#define ReservedSize 12
#define DrvNumSize 1
#define ReservedlSize 1
#define BootSigSize 1
#define VolIDSize 4
#define VolLabSize 11
#define FilSysTypeSize 8

// global variables for directory entry
#define NameSize 11
#define AttrSize 1
#define NTresSize 1
#define CrtTimeTenthSize 1
#define CrtTimeSize 2
#define CrtDateSize 2
#define LstAccDateSize 2
#define FstClusHISize 2
#define WrtTimeSize 2
#define WrtDateSize 2
#define FstClusLOSize 2
#define FileSizeSize 4

// struct for Dir Entry
typedef struct {
	unsigned char Name[NameSize];
	unsigned char Attr[AttrSize];
	unsigned char NTres[NTresSize];
	unsigned char CrtTimeTenth[CrtTimeTenthSize];
	unsigned char CrtTime[CrtTimeSize];
	unsigned char CrtDate[CrtDateSize];
	unsigned char LstAccDate[LstAccDateSize];
	unsigned char FstClusHI[FstClusHISize];
	unsigned char WrtTime[WrtTimeSize];
	unsigned char WrtDate[WrtDateSize];
	unsigned char FstClusLO[FstClusLOSize];
	unsigned char FileSize[FileSizeSize];
} DirEntry;

// struct for boot sector
typedef struct {
	unsigned char jmpBoot[jmpBootSize];
	unsigned char OEMName[OEMNameSize];
	unsigned char BytsPerSec[BytsPerSecSize];
	unsigned char SecPerClus[SecPerClusSize];
	unsigned char RsvdSecCnt[RsvdSecCntSize];
	unsigned char NumFATs[NumFATsSize];
	unsigned char RootEntCnt[RootEntCntSize];
	unsigned char TotSec16[TotSec16Size];
	unsigned char Media[MediaSize];
	unsigned char FATSz16[FATSz16Size];
	unsigned char SecPerTrk[SecPerTrkSize];
	unsigned char NumHeads[NumHeadsSize];
	unsigned char HiddSec[HiddSecSize];
	unsigned char TotSec32[TotSec32Size];
	unsigned char FATSz32[FATSz32Size];
	unsigned char ExtFlags[ExtFlagsSize];
	unsigned char FSVer[FSVerSize];
	unsigned char RootClus[RootClusSize];
	unsigned char FSInfo[FSInfoSize];
	unsigned char BkBootSec[BkBootSecSize];
	unsigned char Reserved[ReservedSize];
	unsigned char DrvNum[DrvNumSize];
	unsigned char Reservedl[ReservedlSize];
	unsigned char BootSig[BootSigSize];
	unsigned char VolID[VolIDSize];
	unsigned char VolLab[VolLabSize];
	unsigned char FilSysType[FilSysTypeSize];
} BootSector;

// struct for currently open files
typedef struct {
	char filename[150];
	int firstCluster;
	int mode; // 0 = r, 1 = w, 2 = rw, 3 = wr
} OpenFile;

// struct for image file
typedef struct {
	OpenFile openFiles[150];
	int openFilesCnt;
	BootSector bootSector;
	unsigned char * buffer;
	int size;
	int currentCluster;
	char filename[150];
	char cwd[150][150];
	int promptDepth;
} Image;

// main functions
void printPrompt(const Image * image);
void parseInput(char * line, Image * image);

// custom functions
int getClusterSize(const Image * image);
int getReservedSize(const Image * image);
int getFATSize(const Image * image);
int getDataSize(const Image * image);
int getFATStartIndex(const Image * image);
int getDataStartIndex(const Image * image);
// FAT Region
int getTotalClusters(const Image * image);
int findAvailCluster(const Image * image);
int addAdditionalCluster(const Image * image, int firstCluster);
void setFATTableValue(const Image * image, int index, unsigned char * value);
void getAssociatedClusters(const Image * image, int startingCluster, int * clusters);
void getFATRegion(const Image * image, int * FATRegion);
// Data Region
bool addDirEntryToCluster(const Image * image, DirEntry d, int cluster);
void getCluster(const Image * image, int clusterNumber, unsigned char * cluster);
void readDirEntriesInDir(const Image * image, int dirClusNumber, DirEntry entries[], int * size);
DirEntry readDirEntry(const unsigned char * cluster, int entry);
// Creation
DirEntry createDirEntry(const char * name, unsigned char attr, unsigned int firstCluster);
// General
bool updateImageFile(const Image * image);
void readInImageFile(const char * imageFileName, Image * image);
BootSector readBootSector(const char * imageFileName);


// project functions
// had to include _command to prevent conflicting file names w/ #include files
void exit_command(Image * image);
void info_command(const Image * image);
bool ls_command(char args[], const Image * image, char errorMessage[]);
bool cd_command(char args[], Image * image, char errorMessage[]);
bool size_command(char args[], const Image * image, char errorMessage[]); // return -1 if filename does not exist
bool creat_command(char args[], Image * image, char errorMessage[]);
bool mkdir_command(char args[], Image * image, char errorMessage[]);
bool open_command(char args[], Image * image, char errorMessage[]);
bool close_command(char args[], Image * image, char errorMessage[]);
bool read_command(char args[], Image * image, char errorMessage[]);
bool write_command(char args[], Image * image, char errorMessage[]);
bool rm_command(char args[], Image * image, char errorMessage[]);
bool rmdir_command(char args[], Image * image, char errorMessage[]);

// helper functions
void removeTrailingSpace(char * str);
void split(char * str, char * delim, int * count, char arr[100][100]);
void intToUnsignedChar(int n, unsigned char bytes[]);
void hexToASCII(const unsigned char * buffer, int bufferSize, char * str);
int hexStringToDec(const unsigned char * buffer, int bufferSize);
int power(int base, int raise);
int compareDirs(const void * p1, const void * p2);

#endif