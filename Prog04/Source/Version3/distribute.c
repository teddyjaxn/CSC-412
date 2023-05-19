
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headerFile.h"

/** 
 * @file distribute.c
 * reads through lines of code, determines which process they are 
 * assigned to, then writes that line of code to the corresponding 
 * process' file.
 * @param procNum the number of the process
 * @param fileCount the number of files assigned to that process
 **/
int main(int argc, char* argv[]){
    int procNum = atoi(argv[1]);
    int fileCount = atoi(argv[2]);
    int i;

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

    exit(0);
}