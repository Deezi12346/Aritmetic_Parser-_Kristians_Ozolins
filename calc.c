
// Kristians Ozolins 251RDC075
// https://github.com/Deezi12346/Aritmetic_Parser-_Kristians_Ozolins
// Tools used and links:
//https://stackoverflow.com/questions/64596361/evaluate-a-simple-mathematical-expression-with-limited-tools-no-arrays-and-no/64597676#64597676
//https://stackoverflow.com/questions/63854654/parsing-simple-mathematical-expressions

#include <stdio.h>
#include <string.h>

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
    long long position = 1;
    int neednumber = 1;

    while (charc != EOF && charc != '\n')
    {
        //Stack Overflow converting characters into numbers
        //https://stackoverflow.com/questions/628761/convert-a-character-digit-to-the-corresponding-integer-in-c

        //checks if character is allowed
        //ChatGPT used here for invalid character check
        if (charc == 0 || strchr("0123456789 +-*/\t", charc) == NULL)
        {
            printf("ERROR:%lld\n", position);
            fclose(file);
            return 0;
        }
        if (charc >= '0' && charc <= '9')
        {
        neednumber = 0;
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
        // Used these two discussions with similiar code for this part
        //https://stackoverflow.com/a/64597676
        //https://stackoverflow.com/questions/63854654/parsing-simple-mathematical-expressions
        //checks for missing numbers and calculates previous operation

        if (charc == '+' || charc == '-' || charc == '*' || charc == '/')
        {
            if (neednumber == 1)
            {
                printf("ERROR:%lld\n", position);
                fclose(file);
                return 0;
            }

            if (adding == 1)
            {
                if (operation == '+')
                    number1 = number1 + number2;

                if (operation == '-')
                    number1 = number1 - number2;

                if (operation == '*')
                    number1 = number1 * number2;

                if (operation == '/')
                {
                    if (number2 == 0)
                    {
                        fprintf(stderr, "Cannot divide by zero\n");
                        fclose(file);
                        return 1;
                    }

                    number1 = number1 / number2;
                }

                number2 = 0;
            }

            neednumber = 1;
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
        if (charc == '*')
        {
            adding = 1;
            operation = '*';
        }

        if (charc == '/')
        {
            adding = 1;
            operation = '/';
        }

        position++;
        charc = fgetc(file);
    }
    if (neednumber == 1)
{
    printf("ERROR:%lld\n", position);
    fclose(file);
    return 0;
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

    if (operation == '*')
    {
        printf("Result: %lld\n", number1 * number2);
    }

    if (operation == '/')
    {
        if (number2 == 0)
        {
            fprintf(stderr, "Cannot divide by zero\n");
            fclose(file);
            return 1;
        }

        printf("Result: %lld\n", number1 / number2);
    }

    
    fclose(file);

    return 0;
}

