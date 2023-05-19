/// @file Prog4Version3.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "headerFile.h"


/// The standardized name for the program that sorts the lines by line number
#define PROCESSING_NAME "./processing"

/// The standardized name for the program that distributes data to their assigned process
#define DISTRIBUTE_NAME "./distribute"

/** 
 * reads through a data directory, and returns the number of
 * files it contains
 * @param filepath the path to the data directory
 * @returns the number of files in the directory
 **/
int fileCounter(char* filePath){
  DIR* directory = opendir(filePath);
  if (directory == NULL)
  {
    printf("data folder %s not found\n", filePath);
    exit(1);
  }
  struct dirent* source;
  int fileCounter = 0;
  while ((source = readdir(directory)) != NULL)
  {
      if (source->d_type == DT_REG)
      {
        fileCounter++;
      }
  }
  closedir(directory);
  return fileCounter;

}


/** 
 * reads through every file in the data directory and evenly splits the contents of the 
 * files into lists to be used in later data processing
 * @param directory an open directory open to folder with the data in it
 * @param dataPath the filepath to the data directory
 * @param procNum the number of processes being used
 * @param overflow the number of file lists that will have an offset number of files in them
 * @param maxFilesPerProc the highest number of files that can be in a file list. 
 * @returns the number of files in the the current process' file list
 **/
int gen1DataMaker( DIR* directory, char* dataPath, int procNum, int overflow, int maxFilesPerProc){
    

     // get the current process' to-sort file
    char* dataFileName = (char*) calloc(strlen(FIRST_OUTPUT_PREFIX)+ intToStrLen(procNum) + 4, 1);
    sprintf(dataFileName, "%s%d.txt", FIRST_OUTPUT_PREFIX, (procNum));
    FILE* currentProc = fopen(dataFileName, "a");
    int filesForProc = 0;
    int l = 0;
    if((procNum >= overflow) && (overflow != 0)){
        l++;
    }

    for( ; l < maxFilesPerProc ; l++){
        
        // initialize the dirent, start going through the directory.
        struct dirent* fileInFolder;
        if((fileInFolder = readdir(directory)) != NULL){
            char* name = fileInFolder->d_name;
                if ((name[0] != '.') && (fileInFolder->d_type == DT_REG)){
		
                    // get the current data file content
                    char* fileName = (char*) malloc(strlen(name)+strlen(dataPath) +1);
			        sprintf(fileName, "%s%s", dataPath, name);
                    FILE* Read = fopen(fileName, "r");
                    if(Read == NULL){
                        printf("%s did Not open\n", fileName);
                    }
                    char* readLine = NULL;
                    int size;
                    size_t length = 0;
                    if((size = getline(&readLine, &length, Read)) != -1){
                    } 
                    
                    // copy the file's contents to its to-sort list
                    fprintf(currentProc, "%s", readLine );
                    
                    fclose(Read);
                    filesForProc++;
                }
                else{
                    l--;
                }
        }
    }
    fclose(currentProc);
    return filesForProc;
}


/** 
 * Concatinates the entirey of the original source code contained in the data directory
 * @param totalProc the total number of child processes run by the program
 **/
void finalOutput(int totalProc){

    char* writeFileName = "Output.c";
    FILE* Write = fopen(writeFileName, "w+");
    int i = 0;
    for(i = 0; i < totalProc; i++){

        char* readFileName = (char*) calloc(strlen(THIRD_OUTPUT_PREFIX) + intToStrLen(i) + 4, 1);
        sprintf(readFileName, "%s%d.txt", THIRD_OUTPUT_PREFIX, (i));
        FILE* Read = fopen(readFileName, "r");
        char* line = NULL;
        size_t n = 0;
        ssize_t input;

        while((getline(&line, &n, Read)) != EOF ){
            fputs(line, Write);
        }  
        fclose(Read); 
    }

    fclose(Write);
    
}


int main(int argc, char *argv[]){

// check for / at back of file path
char* placeHolder = strrchr(argv[2], '/');
int niceSize = strlen(placeHolder);
if(niceSize != 1){
    strcat(argv[2], "/");
}


// counts how many files are in the directory, how many proccesses to use.
  int fileCount = fileCounter(argv[2]);
  int maxFilesPerProc = 0;
  int procCount = atoi(argv[1]);
  int overflow = fileCount % procCount;

// determines how many files per proccess.
  if(overflow != 0){
    maxFilesPerProc = ((fileCount-(overflow)) / procCount) + 1;
  }
  else{
    maxFilesPerProc = (fileCount / procCount);
  }


int i, j;

// Checks ot see if the directory is valid
DIR* directory = opendir(argv[2]);
if (directory == NULL){
    printf("data folder %s not found, again\n", argv[2]);
    exit(1);
  }


// creates files to hold each process' assigned to-sort files.
for( i = 0; i < procCount; i++){
  char* dataFileName = (char*) calloc(strlen(FIRST_OUTPUT_PREFIX)+ intToStrLen(i) + 4, 1);
  sprintf(dataFileName, "%s%d.txt", FIRST_OUTPUT_PREFIX, (i));
  FILE* currentProc = fopen(dataFileName, "w+");
  fclose(currentProc);
  free(dataFileName);
}



closedir(directory);

// writes each process' to-sort files to its premade file list. (3.1.2) 
// as far as I can tell, this isnt meant to be forked, because 
// it is just how I partitioned the files.
directory = opendir(argv[2]);
int fileAmount[fileCount];
for( i = 0; i < procCount; i++){

    fileAmount[i] = gen1DataMaker( directory, argv[2], i, overflow, maxFilesPerProc);

}

closedir(directory);

// creates files to hold each process' assigned files
for( i = 0; i < procCount; i++){
  char* dataFileName = (char*) calloc(strlen(SECOND_OUTPUT_PREFIX)+ intToStrLen(i) + 4, 1);
  sprintf(dataFileName, "%s%d.txt", SECOND_OUTPUT_PREFIX, (i));
  FILE* currentProc = fopen(dataFileName, "w+");
  fclose(currentProc);
  free(dataFileName);
}

// populates the second gen of folders with each process' assigned file contents.
// this will be the set of children that distributes.
for( i = 0; i < procCount; i++){

  char* x = (char*) calloc(intToStrLen(i)*sizeof(char) + 1, 1);
  char* y = (char*) calloc(intToStrLen(fileAmount[i])*sizeof(char) + 1, 1);
  char* z = (char*) calloc(strlen(DISTRIBUTE_NAME)*sizeof(char) + 1, 1);

  char** param = (char**) malloc(3*sizeof(char*) + 1 );
  param[1] = (char*) malloc( intToStrLen(i)*sizeof(char) + 1);
  param[2] = (char*) malloc( intToStrLen(fileAmount[i])*sizeof(char) + 1);
  param[0] = (char*) calloc(strlen(DISTRIBUTE_NAME)*sizeof(char) + 1, 1);
  param[3] = NULL;

  sprintf(x , "%d", i);
  sprintf(y , "%d", fileAmount[i]);
  z = DISTRIBUTE_NAME;

  strcpy(param[0], z);
  strcpy(param[1], x);
  strcpy(param[2], y);
  


  
  if(fork() == 0){
    int g = execv(DISTRIBUTE_NAME, param);
  }
  
}
for( i = 0; i < procCount; i++){
  wait(NULL);
}


// Looks through the folders and sorts them by line number.
for(i = 0; i < procCount; i++){
  
  char* x = (char*) calloc(intToStrLen(i)*sizeof(char) + 1, 1);
  char* z = (char*) calloc(strlen(PROCESSING_NAME)*sizeof(char) + 1, 1);

  char** param = (char**) malloc(3*sizeof(char*) + 1 );
  param[1] = (char*) malloc( intToStrLen(i)*sizeof(char) + 1);
  param[0] = (char*) calloc(strlen(PROCESSING_NAME)*sizeof(char) + 1, 1);
  param[2] = NULL;

  sprintf(x , "%d", i);
  
  z = PROCESSING_NAME;
  strcpy(param[0], z);
  strcpy(param[1], x);


  if(fork() == 0){
    int g = execv(PROCESSING_NAME, param);
  }

 }

for( i = 0; i < procCount; i++){
  wait(NULL);
}



// goes through all sorted data files and prints them to one final
// output file, which is of type .c
finalOutput(procCount);

// this deletes the files created because the clutter is a bit of a mess.
for( i = 0; i < procCount; i++){
    char* firstFileName = (char*) calloc(strlen(FIRST_OUTPUT_PREFIX) + intToStrLen(i) + 4, 1);
    sprintf(firstFileName, "%s%d.txt", FIRST_OUTPUT_PREFIX, (i));
    char* secondFileName = (char*) calloc(strlen(SECOND_OUTPUT_PREFIX) + intToStrLen(i) + 4, 1);
    sprintf(secondFileName, "%s%d.txt", SECOND_OUTPUT_PREFIX, (i));
    char* thirdFileName = (char*) calloc(strlen(THIRD_OUTPUT_PREFIX) + intToStrLen(i) + 4, 1);
    sprintf(thirdFileName, "%s%d.txt", THIRD_OUTPUT_PREFIX, (i));
    remove(firstFileName);
    remove(secondFileName);
    remove(thirdFileName);
}


}