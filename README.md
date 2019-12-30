# -Project-3-COP4610
For this project, you will design and implement a simple, user-space, shell-like utility that is capable of interpreting a FAT32 file system image. The program must understand the basic commands tomanipulate the given file system image, must not corrupt the file system image, and should be robust.You may not reuse kernel file system code and you may not copy code from other file system utilities. 

## Contents
The contents of the tar archive and a brief description of each file. 
- **main.c**: Contains the main method for using the program.  
- **proj3.c**: Contains definitions for the core command functions of the program. 
- **customs.c**: Contains the function definitions used for directly manipulating the FAT32 image file. 
- **helpers.c**: Contains function definitions for all of the helper functions. 
- **proj3.h**: The header file. Contains function prototypes for every function in the program (not just the functions defined in proj3.c).
- **README.md**: A formatted text file which contains a brief overview of the project implementation. 
- **makefile**: Utility file that compiles all of the source code. 
- **GitLog**: A text file which contains the development log for this project. We chose to put it into a text file instead of taking a screenshot from GitHub because the log is extremely long (over 90 commits).  

## How to Compile
- To compile the program, simply type ```make``` at the command line. This will create an executable named ```p3```. Execute the program by typing ```./p3``` and passing in the ```fat32.img``` file as an argument: ```./p3 fat32.img``` 

## Known Bugs
None.

## Special Considerations
None.
