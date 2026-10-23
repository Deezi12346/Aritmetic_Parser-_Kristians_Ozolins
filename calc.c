
// Kristians Ozolins 251RDC075
// https://github.com/Deezi12346/Aritmetic_Parser-_Kristians_Ozolins
#include <stdio.h>

int main(int argcount, char *argpass[])
{
    //argcount is for argument count and argpass is how many arguments passed
    if (argcount < 2)
    {
        //checks whether filename was given 
        fprintf(stderr, "No input file\n");
        return 1;
        // prints error if no filename was given
    }

    printf("File: %s\n", argpass[1]);

    return 0;
}
