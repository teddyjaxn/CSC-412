#include <stdio.h>

int main(int argc, char *argv[])
{
    if(argc == 3)
    {
        printf("The executable %s was launched with two arguments:\n", argv[0]);
        printf("    The first arguement is: %s,\n",argv[1]);
        printf("    The second arguement is: %s.\n",argv[2]);
        return 0;
    }

    else if(argc < 3)
    {
        printf("usage:   %s <argument1> <argument2>\n", argv[0]);
        return 1;
    }

    else
    {
        printf("usage:   %s <argument1> <argument2>\n", argv[0]);
        return 2;
    }

  return 3;

}
