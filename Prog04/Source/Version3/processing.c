#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "headerFile.h"

/** 
 * @file processing.c
 * reads through the file info associated with the current process, and 
 * puts them in order by line number. once sorted, they are written to 
 * a file specific to that process
 * @param procNum the number of the process
 **/
int main(int argc, char* argv[]){

    int procNum = atoi(argv[1]);
    //int fileCount = atoi(argv[2]);

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
    exit(0);
}
