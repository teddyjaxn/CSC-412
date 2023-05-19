/// @file Prog5Version1.cpp
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>


/**
 *  takes in the name of a process from the job file and returns aa corresponding code.
 * @param commandName the string taken from the task file that states the command to be executed.
 * @returns a code corresponding to the task.
 * */
int taskCode(std::string commandName){

std::string commandList[6] = { "flipH", "flipV", "gray", "crop", "rotate", "end"};
int taskCodeIndex = 0;
while(taskCodeIndex!= 6){

    if( commandName == commandList[taskCodeIndex]){

        return taskCodeIndex+1;
    }
    taskCodeIndex++;
}
std::cout<<"Invalid Task Encountered"<<"\n";
return 7;
}



// takes in:
//  
//          a List of tasks, which is going to be a file.
//          Data Folder, where to find the images
//          Output Folder, where to output the new images.

// reads through the task list and runs appropriate fork and exec commands for each task.

/**
 * reads and executes a list of image manipulation tasks supplied by a .job file. 
 * Searches for the images named in the list of tasks in the data folder assigned to is via parameter, 
 * and sends the edited images to the directory that is passed to it as an output folder.
 * @param jobList the file path to a .job file which contains a list of tasks sto perform.
 * @param DatFolder the file path to a folder where images to be edited will be searched for.
 * @param output the folder path to a directory where edited images will be placed.
 * @returns 0 is it encounters an end of file, or exits with parameter 0 if it encounters an "end" command in the jobList file.
 * */
int main(int argc, char const *argv[]){


    bool run = true;
    std::ifstream jobFile;
    jobFile.open( argv[1], std::ios::in );
    int i = -1;
    std::string command;
    while(jobFile>>command){
        i++;

        //std::string command;
        //jobFile >> command;
        //std::cout<<command<<"\n";
        int code = taskCode(command);
        //std::cout<<"Above command has code number: "<<code<<"\n";

        // if the code is between 1 and 3, then it only needs an image name as a parameter, followed by an output path.

        // if the code is 4, then it has image name, and 4 numbers, 5 parameters in total.

        // if the code is 5, then it has directional rotation, and image name, 2 parameters in total.

        // if the code is 6, the program exits.

        
        char *subProg = &(command[0]);    
        //std::cout<<"lauching program named: "<<subProg<<"\n";
        
        if(code >= 6){
           
            return 0;
        }
        else if(code <= 3){

            
            std::string imageName;
            jobFile>>imageName;
            imageName = "/" + imageName;
            if(fork() == 0){
                
                std::string progPath = "./ImageLibrary/applications/" + command;
                std::string imagePath = argv[2] + imageName;
                std::string output = argv[3];
                
                char** param = (char**) malloc(3*sizeof(char*) + 1 );

                param[0] = &(progPath[0]);
                param[1] = &(imagePath[0]);
                param[2] = &(output[0]);

                // std::cout<<
                // imagePath<<"\t"<<param[1]<<"\n"<<
                // progPath<<"\t"<<param[0]<<"\n"<<
                // output<<"\t"<<param[2]<<"\n";
            
            
                // std::cout<<"forked, launching progarm: <"<<&(progPath[0])<<">\n";
                
                execv(&(progPath[0]), param);
                
            }

        }
        else if(code == 4){

            std::string imageName;
            jobFile>>imageName;
            imageName = "/" + imageName;
            std::string x, y, w, h;
            jobFile>>x>>y>>w>>h;
            if(fork() == 0){

                std::string progPath = "./ImageLibrary/applications/" + command;
                std::string imagePath = argv[2] + imageName;
                std::string output = argv[3];
                
                char** param = (char**) malloc(7*sizeof(char*) + 1 );

                param[0] = &(progPath[0]);
                param[1] = &(imagePath[0]);
                param[2] = &(output[0]);
                param[3] = &(x[0]);
                param[4] = &(y[0]);
                param[5] = &(w[0]);
                param[6] = &(h[0]);

                execv(&(progPath[0]), param);


            }
        }
        else if(code == 5){
            
            std::string rotation;
            jobFile>>rotation;
            //std::cout<<rotation<<"\n";

            
            if(rotation.at(0) != '-'){
                rotation = "-" + rotation;
                //std::cout<<"rotation had - added, now reads"<< rotation <<"\n";
            }
            
            
            std::string imageName;
            jobFile>>imageName;
            imageName = "/" + imageName;
            
            if(fork() == 0){
                for(int k = 1; k < rotation.length(); k++){
                    if( (rotation.at(k) != 'L') && (rotation.at(k) != 'l') && (rotation.at(k) != 'R') && (rotation.at(k) != 'r') ){
                        exit(0);
                    }
                }   


                std::string progPath = "./ImageLibrary/applications/" + command;
                std::string imagePath = argv[2] + imageName;
                std::string output = argv[3];
                
                char** param = (char**) malloc(7*sizeof(char*) + 1 );

                //std::cout<<"rotation code for: <"<<imagePath<<"> is <"<<rotation<<">\n";

                param[0] = &(progPath[0]);
                param[1] = &(rotation[0]);  
                param[2] = &(imagePath[0]);
                param[3] = &(output[0]);


                execv(&(progPath[0]), param);


            }
        }
          
    }
    for(int j = 0; j < i; j++){
                
        wait(NULL);
    }

    return 0;
}