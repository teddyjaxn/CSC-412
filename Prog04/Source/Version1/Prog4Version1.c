#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "headerFile.h"



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


//  transcribes one file to another
void Distribute( int procNum, int fileCount ){

    int i;
    // sets up Read file
    char* readFileName = (char*) calloc(strlen(FIRST_OUTPUT_PREFIX) + intToStrLen(procNum) + 4, 1);
    sprintf(readFileName, "%s%d.txt", FIRST_OUTPUT_PREFIX, (procNum));
    FILE* Read = fopen(readFileName, "r");
    for(i = 0; i < fileCount; i++ ){

        // reads the files assigned process
        char c[10];
        int fileToProc;
        fscanf(Read, "%s ", c);
        fileToProc = atoi(c);

        // sets up the assigned Write file
        char* writeFileName = (char*) calloc(strlen(SECOND_OUTPUT_PREFIX) + intToStrLen(fileToProc) + 4, 1);
        sprintf(writeFileName, "%s%d.txt", SECOND_OUTPUT_PREFIX, (fileToProc));
        FILE* Write = fopen(writeFileName, "a");
        
        // transcribes from one file to the next
        char temp = fgetc(Read);
        while(temp != '\n'){
            putc(temp, Write);
            temp = fgetc(Read);
        }
        if(temp == '\n'){
            putc(temp, Write);
        }

        // iterates to the next list position and closes files.
        
        fclose(Write);
    }
    fclose(Read);
}

// takes the distributed file contents, transcribes them to an array, and sort them.
// prints the output to a new file.
void SortContents( int procNum, int fileCount){

    // first round of variables
    int lineCount = 0;
    char** fileLines = (char**) malloc(lineCount*sizeof(char*)+1);
    int i;
    int* sideLine = (int*) malloc((1+ lineCount)*sizeof(int)+1);

    
    // sets up Read file
    char* readFileName = (char*) calloc(strlen(SECOND_OUTPUT_PREFIX) + intToStrLen(procNum) + 4, 1);
    sprintf(readFileName, "%s%d.txt", SECOND_OUTPUT_PREFIX, (procNum));
    FILE* Read = fopen(readFileName, "r");

    // second round of variables
    char ln[10] ;
    int LN = 0;
    char* line = NULL;
    size_t n = 0;
    ssize_t input;

    // reads the first character from the file while also checking if its valid.
    // the line number is stored in an int array taht will be used later to sort 
    // the line. I didnt use qsort, because sometimes its faster to climb the 
    // hill you know, rather than take the road you dont. Trust me, I tried.
    i = 0;
    int j = 0;
    while((fscanf(Read, "%s", ln)) != EOF ){
        LN = atoi(ln);
        sideLine[i] = LN;
        input = getline(&line, &n, Read);
        lineCount++;
        fileLines = (char**)realloc(fileLines, (lineCount * sizeof(char*)) + 1);
        sideLine = (int*) realloc(sideLine, ((1 + lineCount)*sizeof(int))+1);

        // makes space for the current line in the array, then assigns it.
        
        fileLines[i] = (char*) malloc(input + 1);
        strcpy(fileLines[i], line);
        i++;
        
    }

    // Once we have the list of lines, as well as the parallel list of line numbers,
    // we are going to use those to output the lines in the right order. 
    int lowest;
    int lowestIndex = 0;

    // creates and opens the file we will be writing to.
    char* writeFileName = (char*) calloc(strlen(THIRD_OUTPUT_PREFIX) + intToStrLen(procNum) + 4, 1);
    sprintf(writeFileName, "%s%d.txt", THIRD_OUTPUT_PREFIX, (procNum));
    FILE* Write = fopen(writeFileName, "w+");

    // populates the output file with the lines in order. the sorting is done
    // to the array of line numbers, the printing is done with the parallel list 
    // of line contents. 
    for(i = 0; i < lineCount; i++){
        lowest = 99999;
        for(j = 0; j < lineCount; j++){
            if((sideLine[j] < lowest) && (sideLine[j] >= 0)){
                lowest = sideLine[j];
                lowestIndex = j;
            }
        }
        fputs(fileLines[lowestIndex], Write);
        sideLine[lowestIndex] = -1;
    }
    

    fclose(Read);
    fclose(Write);

}

// partitions the data into more or less equal parts for the child processes.
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
                        //printf("File contents are as follows\n\t%s\n", readFile);
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
// as far as I can tell, this will not be forked in the future, but rather
// is just how I partition data for the first children.

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
     Distribute(i, fileAmount[i]);
 }


// Looks through the folders and sorts them by line number.
for(i = 0; i < procCount; i++){
    SortContents(i, maxFilesPerProc);
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