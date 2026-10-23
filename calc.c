
// Kristians Ozolins 251RDC075
// https://github.com/Deezi12346/Aritmetic_Parser-_Kristians_Ozolins

#include <stdio.h>

int main(int argcount, char *argpass[])
{
    //argcount counts arguments and argpass stores them
    if (argcount < 2)
    {
        //checks whether filename was given
        fprintf(stderr, "No input file\n");
        return 1;
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

    long long number1 = 0;
    long long number2 = 0;
    int adding = 0;
    int charc = fgetc(file);

    while (charc != EOF && charc != '\n')
    {
        if (charc >= '0' && charc <= '9')
        {
            if (adding == 0)
            {
                number1 = number1 * 10 + (charc - '0');
            }
            else
            {
                number2 = number2 * 10 + (charc - '0');
            }
        }

        if (charc == '+')
        {
            adding = 1;
        }

        charc = fgetc(file);
    }
    printf("Result: %lld\n", number1 + number2);

    fclose(file);

    return 0;
}

