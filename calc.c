
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

FILE *file = fopen(argpass[1], "r");

    //checks if file could be opened
    if (file == NULL)
    {
        fprintf(stderr, "Cannot open file\n");
        return 1;
    }

    printf("File opened\n");

   
    int charc;
    int charc = fgetc(file);

    while (charc != EOF)
    {
        putchar(charc);
        charc = fgetc(file);
    }
    fclose(file;
    
    return 0;

    long number1 = 0;
 l  long number2 = 0;
    int adding = 0;
    int charc = fgetc(file);
