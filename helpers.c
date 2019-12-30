/*
  Project 3 for COP4610
  Function Definitions
*/

#include "proj3.h"


// function for removing trailing whitespace
void removeTrailingSpace(char * str) 
{
	// remove newline
	char * newline = strchr(str, '\n');
	if (newline != NULL)
		*newline = '\0';
	// trim trailing space
	char * end = str + strlen(str) - 1; // sent end to the end of the string
	while (end > str && isspace(*end)) end--; // loop through string and if char is a space, then subtract the pointer
	// end is pointing at character right before ending
	// so, set character after it to be null character
	end[1] = '\0';
}

// function for splitting a string into an array of strings
void split(char * str, char * delim, int * count, char arr[100][100])
{
	// initial variables
	char temp[strlen(str)+1];
	strcpy(temp, str);
	char * value = strtok(temp, delim);

	// loop through and split string
	*count = 0;
	while (value != NULL) {
		strcpy(arr[*count], value);
		value = strtok(NULL, delim);
		*count = *count + 1;
	}
}

void intToUnsignedChar(int n, unsigned char * bytes) {
	bytes[3] = (n >> 24) & 0xFF;
	bytes[2] = (n >> 16) & 0xFF;
	bytes[1] = (n >> 8) & 0xFF;
	bytes[0] = n & 0xFF;
}

// function for converting hex string (little endian) to ASCII
void hexToASCII(const unsigned char * buffer, int bufferSize, char * str) {
	int i, j;
	char k;
	for (i = 0; i < bufferSize; i++) {
		j = (int) buffer[i];
		k = (char) j;
		str[i] = k;
	}
	str[bufferSize] = '\0';
}

// function for converting hex string (little endian) to integer
int hexStringToDec(const unsigned char * buffer, int bufferSize) {

	int i, decimal = 0;
	int values[bufferSize];

	for (i = 0; i < bufferSize; i++) 
		values[i] = (int) buffer[i];
	for (i = 0; i < bufferSize; i++)
		decimal += values[i] * power(256, i);

	return decimal;
}

int power(int base, int raise) {
	if (raise == 0)
		return 1;
	else
		return (base * power(base, raise-1));
}

//function to compare two strings, used by qsort
int compareDirs(const void * p1, const void * p2)
{	
	DirEntry * d1 = (DirEntry *) p1;
	DirEntry * d2 = (DirEntry *) p2;
	return strcmp( d1 -> Name, d2 -> Name );
}
