#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[])
    {
        //Returns an error if there are not 3 arguements, if one of the arguements is less than zero, or if one of the arguements is not an integer.
        if((argc != 4) || ((argc == 4) && ( (atoi(argv[1]) <= 0) || (atoi(argv[2]) <= 0) || (atoi(argv[3]) <= 0) ) ) )
        {
            printf("usage: %s F1 F2 n, with F2>F1>0 and n>0.\n", argv[0]);
            return 1;
        }
        // Prints an error of F2 is not strictly larger than F1.
        else if(atoi(argv[1]) >= atoi(argv[2]))
        {
            printf("error: The second argument must be strictly larger than the first.\n");
            return 2;
        }
        
        int fibLength = atoi(argv[3]);
        // Makes an array large enough for the size of the requested series.
        int* fibSet = (int*)malloc(sizeof(int) * fibLength);

        // Populates first 2 numbers in the series.
        fibSet[0] = atoi(argv[1]);
        fibSet[1] = atoi(argv[2]);

        int i;
        for(i = 2 ; i < (fibLength); i++)
        {
            // Populates the rest of the series based on the two starting values.
            fibSet[i] = fibSet[i-2] + fibSet[i-1];

        }

        // Prints first unique portion of output.
        printf("%d terms of the Fibonacci series with F1=%d and F2=%d:\n", fibLength, fibSet[0], fibSet[1]);
        for(i = 0; i < fibLength; i++)
        {
            // Prints recurring output.
            printf("F%d=%d", i+1, fibSet[i]);

            // Looks for when to print unique ending output.
            if(i != (fibLength - 1))
            {
                printf(",    ");
            }
            else
            {
                printf("\n");
            }
            
        }

        return 0;
    }