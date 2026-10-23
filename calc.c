
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

    //stores the two numbers and checks if + was found
    long long number1 = 0;
    long long number2 = 0;
    int adding = 0;
    int operation = '+';
    //reads the numbers digit by digit
    int charc = fgetc(file);

    while (charc != EOF && charc != '\n')
    {
        //Stack Overflow converting characters into numbers
        //https://stackoverflow.com/questions/628761/convert-a-character-digit-to-the-corresponding-integer-in-c

        if (charc >= '0' && charc <= '9')
        {
             //multiply by 10 to allow numbers with more than one digit
            if (adding == 0)
            {
                number1 = number1 * 10 + (charc - '0');
            }
            else
            {
                number2 = number2 * 10 + (charc - '0');
            }
        }
        //after + the digits go into the second number
        if (charc == '+')
        {
            adding = 1;
            operation = '+';
        }

        if (charc == '-')
{
         adding = 1;
         operation = '-';
}

        charc = fgetc(file);
    }

     //checks which operation to calculate
    if (operation == '+')
    {
        printf("Result: %lld\n", number1 + number2);
    }

    if (operation == '-')
    {
        printf("Result: %lld\n", number1 - number2);
    }

    fclose(file);

    return 0;
}

