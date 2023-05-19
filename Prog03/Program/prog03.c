#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>



//     pass it a file object, and 2 int's by reference ( &<int> ), and it will get 
//  the first 2 numbers from the file, and change the int's values accordingly
void getDimensions(FILE *imagePar, int *width, int *height)
{

    char W[10], H[10];
    
    fscanf(imagePar, "%s %s", W, H);
    *width = atoi(W);
    *height = atoi(H);

   return;

}




//   <program name>   <pattern file>   <image directory path>   <output directory path>

int main(int argc, char *argv[])
{
    FILE *pat;

    pat = fopen( argv[1], "r");

    int width, height, i, j;
    getDimensions(pat, &width, &height);

    char patGrid[height][width];
    char line[width];

    for( i = 0; i < width; i++)
    {
        fscanf(pat, "%s", line);
        for( j = 0; j < height; j++)
        {
            patGrid[j][i] = line[j];
        }
    }
   fclose(pat);





    // It should be very clear that from this line down to where I annotate it, I
    // have used heavy amounts of the sample code were were given. I see no reason 
    // to reinvent the wheel, and I have no intention to claim it as mine. 

    const char* rootPath = argv[2];
    DIR* directory = opendir(rootPath);
    if (directory == NULL)
    {
		printf("data folder %s not found\n", rootPath);
		exit(1);
	}

    struct dirent* imgDirSource;
	int counter = 0;

    while ((imgDirSource = readdir(directory)) != NULL)
    {
        char* name = imgDirSource->d_name;
        if ((name[0] != '.') && (imgDirSource->d_type == DT_REG))
        {
			counter++;
        }
    }
	closedir(directory);

    char** fileName = (char**) malloc(counter*sizeof(char*));

	int k = 0;
	directory = opendir(rootPath);
    while ((imgDirSource = readdir(directory)) != NULL)
    {
        char* name = imgDirSource->d_name;
        if ((name[0] != '.') && (imgDirSource->d_type == DT_REG))
        {
			fileName[k] = (char*) malloc(strlen(name)+1);
			strcpy(fileName[k], name);
            k++;

        }
    }
	closedir(directory);
   
    //
    //     This marks the end of the sample code that was used. 
	//


    //  variables for the next section. 

    FILE * image;
    FILE * output;
    char* outName;
    char* patName = strrchr(argv[1], '/');
    outName = strncat(argv[3], patName, strlen(patName)-4);
    strcat(outName, "_Matches.txt");
    output = fopen( outName, "a");
    char* test;
    char imageName[256];
    int count = 0;


//  organizes the names.
//  goes through, checks how many matches there are for each file, then goes through a second time and prints to file.


    for(k = 0; k < counter; k++)
    {
        count = 0;
        strcpy(imageName, argv[2]);

        test = strrchr(fileName[k], '.');
        if(strcmp(test, ".img") == 0)
        {
            strcat(imageName, "/");
            strcat(imageName, fileName[k]);
            image = fopen( imageName, "r");

//  

            if(image == NULL)
            {
                printf("oh, fuck, the .img file isnt an .img file\nk = %d\n", k);
                exit(1);
            }
            else
            {
                int widthTwo = 0;
                int heightTwo = 0;

                i = 0;
                j = 0;
                getDimensions(image, &widthTwo, &heightTwo);

                char imageGrid[heightTwo+4][widthTwo+4];
                char tempLine[widthTwo];

                for( i = 0; i < heightTwo; i++)
                    {
                        fscanf(image, "%s", tempLine);
                       // printf("%s , %d\n", tempLine, i);
                        for( j = 0; j < widthTwo; j++)
                            {       
                                imageGrid[j][i] = tempLine[j];
                            }
                    }

// counts the number of matches;

                for( i = 0; i < (heightTwo-2); i++)
                    {
                        for( j = 0; j < (widthTwo-2); j++)
                            {     
                                if((imageGrid[j][i] == patGrid[0][0]))
                                    {
                            
                                        if((imageGrid[j+1][i] == patGrid[1][0]) && (imageGrid[j+2][i] == patGrid[2][0]) && (imageGrid[j][i+1]==patGrid[0][1]) && (imageGrid[j+1][i+1] == patGrid[1][1]) && (imageGrid[j+2][i+1] == patGrid[2][1]) && (imageGrid[j][i+2]==patGrid[0][2]) && (imageGrid[j+1][i+2] == patGrid[1][2]) && (imageGrid[j+2][i+2] == patGrid[2][2]))
                                            {
                                                count++;

                                            }
                                    }
                            }
                    }

                if(count > 0)
                    {
                        fprintf(output, "%s\n", fileName[k]);  
                        fprintf(output, "\t%d ", count);
                    }

//  looks for the matches that have been detected and writes them to file.

                for( ; count > 0; )
                    {
                        for( i = 0; i < (heightTwo-2); i++)
                            {
                                for( j = 0; j < (widthTwo-2); j++)
                                    {
                                        if((imageGrid[j][i] == patGrid[0][0]))
                                            {
                                                if((imageGrid[j+1][i] == patGrid[1][0]) && (imageGrid[j+2][i] == patGrid[2][0]) && (imageGrid[j][i+1]==patGrid[0][1]) && (imageGrid[j+1][i+1] == patGrid[1][1]) && (imageGrid[j+2][i+1] == patGrid[2][1]) && (imageGrid[j][i+2]==patGrid[0][2]) && (imageGrid[j+1][i+2] == patGrid[1][2]) && (imageGrid[j+2][i+2] == patGrid[2][2]))
                                                    {
                                                        fprintf(output, "%d %d ", i, j );
                                                        count--;
                                                    }
                                            }
                                    }
                            }
                    }
                fprintf(output, "\n");
            }
        }

        fclose(image);  
    }
    fclose(output);
    return 0;

}