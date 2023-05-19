#include <stdio.h>
#include <stdlib.h>


void buildSmallSet(int* masterSet, int masterLength, int F1, int F2, int size)
    {
        int* smallSet = (int*)calloc(size, sizeof(int) * size);

        smallSet[0] = F1;
        smallSet[1] = F2;

        int j = 0;
        int k = 0;
        for(j = 2 ; j < size; j++)
            {
                // Populates the rest of the series based on the two starting values.
                smallSet[j] = smallSet[j-2] + smallSet[j-1];
            };
        
        // Populates the master set with the contents of the smaller set, as long as the integers aren't present else where in the master set.
        for( j = 0; j <= size; j++)
        {
            for( k = 0; k <= masterLength; k ++)
            {
                if(masterSet[k] == 0)
                {
                    masterSet[k] = smallSet[j];
                    k = masterLength;
                }
                else if(masterSet[k] == smallSet[j])
                {
                    k = masterLength;
                }
            }

        }
        printf("%d terms of the Fibonacci series with F1=%d and F2=%d:\n     ", size, F1, F2);
        for(j = 0; j < size; j++)
        {
            // Prints recurring output.
            printf("F%d=%d", j+1, smallSet[j]);

            // Looks for when to print unique ending output.
            if(j != (size - 1))
            {
                printf(", ");
            }
            else
            {
                printf("\n");
            }
        }

    free(smallSet);

    };



int main(int argc, char *argv[])
    {
        int i;

        // Returns an error if an incomplete set of numbers is detected.
        if((argc - 1) % 3 != 0)
        {
            printf("usage: %s [F1 F2 n]+, with F2>F1>0 and n>0\n", argv[0]);
            return 1;
        }

        // Returns an error if one of the values isnt greater than 0.
        for( i = 1; i < argc; i++)
        {
            if( atoi(argv[i]) <= 0)
            {
                printf("usage: %s [F1 F2 n]+, with F2>F1>0 and n>0\n", argv[0]);
                return 2;
            }
        }

        // Prints/returns an error if any F2 is not strictly larger than its F1.
        for( i = 1; i < argc - 1; i += 3)
        {
            if(atoi(argv[i]) >= atoi(argv[i+1]))
            {
                printf("usage: %s [F1 F2 n]+, with F2>F1>0 and n>0\n", argv[0]);
                return 3;
            }
        }
            


        // Creates an array that is big enough to fit the maximum possible number
        int masterLength = 0;
        for( i = 3; i < argc; i+=3)
        {
            masterLength = masterLength + atoi(argv[i]);
        }


        // Makes an array large enough for the size of the requested series.
        int* masterSet = (int*)calloc(masterLength, sizeof(int) * masterLength);

        for( i = 1; i < argc; i+=3)
        {
            buildSmallSet(masterSet, masterLength, atoi(argv[i]), atoi(argv[i+1]), atoi(argv[i+2]) );
        }

        //output
        printf("The values encountered in the sequences are\n     {");
        for( i = 0; i < masterLength; i ++)
        {
            if(masterSet[i] != 0)
            {
                printf("%d", masterSet[i]);

                if(masterSet[i+1] != 0)
                {
                    printf(", ");
                }

            }
            


        }
        printf("}\n");


        return 0; 
    }