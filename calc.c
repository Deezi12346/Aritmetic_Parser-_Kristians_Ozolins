
// Kristians Ozolins 251RDC075
// https://github.com/Deezi12346/Aritmetic_Parser-_Kristians_Ozolins
// Tools used and links:
//https://stackoverflow.com/questions/64596361/evaluate-a-simple-mathematical-expression-with-limited-tools-no-arrays-and-no/64597676#64597676
//https://stackoverflow.com/questions/63854654/parsing-simple-mathematical-expressions

#include <stdio.h>
#include <string.h>

//converts a digit character into part of a number
long long addDigit(long long number, int charc)
{
    return number * 10 + (charc - '0');
}
//variables used by the parser
FILE *file;
int charc;
long long position = 1;
long long errorPosition = 0;

//reads the next character
void nextChar(void)
{
    if (charc != EOF)
    {
        charc = fgetc(file);
        position = position + 1;
    }
}

//skips spaces
void skipSpaces(void)
{
    while (charc == ' ' || charc == '\t')
    {
        nextChar();
    }
}

long long expression(void);

//reads numbers or parentheses
long long primary(void)
{
    skipSpaces();

    if (charc >= '0' && charc <= '9')
    {
        long long number = 0;

        while (charc >= '0' && charc <= '9')
        {
            number = addDigit(number, charc);
            nextChar();
        }

        return number;
    }

    if (charc == '(')
    {
        nextChar();

        //calculate everything inside the parentheses
        long long number = expression();

        if (errorPosition != 0)
        {
            return 0;
        }

        skipSpaces();

        if (charc != ')')
        {
            errorPosition = position;
            return 0;
        }

        nextChar();
        return number;
    }

    errorPosition = position;
    return 0;
}

//handles multiplication and division first
long long term(void)
{
    long long number = primary();

    while (errorPosition == 0)
    {
        skipSpaces();

        if (charc != '*' && charc != '/')
        {
            break;
        }

        int operation = charc;
        nextChar();
        skipSpaces();

        long long rightPosition = position;
        long long number2 = primary();

        if (errorPosition != 0)
        {
            return 0;
        }

        if (operation == '*')
        {
            number = number * number2;
        }
        else
        {
            if (number2 == 0)
            {
                errorPosition = rightPosition;
                return 0;
            }

            number = number / number2;
        }
    }

    return number;
}

//handles addition and subtraction
long long expression(void)
{
    long long number = term();

    while (errorPosition == 0)
    {
        skipSpaces();

        if (charc != '+' && charc != '-')
        {
            break;
        }

        int operation = charc;
        nextChar();

        long long number2 = term();

        if (errorPosition != 0)
        {
            return 0;
        }

        if (operation == '+')
        {
            number = number + number2;
        }
        else
        {
            number = number - number2;
        }
    }

    return number;
}

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

    file = fopen(argpass[1], "r");

    //checks if file could be opened
    if (file == NULL)
    {
        fprintf(stderr, "Cannot open file\n");
        return 1;
    }

    printf("File opened\n");

    //reworked parser because old one only worked with simple expressions
    //reads the first character
    charc = fgetc(file);

    //calculates the expression
    long long result = expression();

    skipSpaces();

    //checks for errors
    if (errorPosition == 0 && charc != '\n' && charc != EOF)
    {
        errorPosition = position;
    }

    // OLD CALCULATOR VERSION - kept for reference
    //    long long number1 = 0;
    //    long long number2 = 0;
    //    long long totalnumber = 0;
    //    int adding = 0;
    //    int operation = '+';
    //    int neednumber = 1;
    //
    //    while (charc != EOF && charc != '\n')
    //    {
    //        //Stack Overflow converting characters into numbers
    //        //https://stackoverflow.com/questions/628761/convert-a-character-digit-to-the-corresponding-integer-in-c
    //
    //        //checks if character is allowed
    //        //ChatGPT used here for invalid character check
    //        if (charc == 0 || strchr("0123456789 +-*/\t", charc) == NULL)
    //        {
    //            printf("ERROR:%lld\n", position);
    //            fclose(file);
    //            return 0;
    //        }
    //        if (charc >= '0' && charc <= '9')
    //        {
    //            neednumber = 0;
    //            //multiply by 10 to allow numbers with more than one digit
    //            if (adding == 0)
    //            {
    //                number1 = addDigit(number1, charc);
    //            }
    //            else
    //            {
    //                number2 = addDigit(number2, charc);
    //            }
    //        }
    //        // Used these two discussions with similar code for this part
    //        //https://stackoverflow.com/a/64597676
    //        //https://stackoverflow.com/questions/63854654/parsing-simple-mathematical-expressions
    //        //checks for missing numbers and calculates previous operation
    //        if (charc == '+' || charc == '-' || charc == '*' || charc == '/')
    //        {
    //            if (neednumber == 1)
    //            {
    //                printf("ERROR:%lld\n", position);
    //                fclose(file);
    //                return 0;
    //            }
    //
    //            if (adding == 1)
    //            {
    //                //save addition and subtraction for later
    //                if (operation == '+')
    //                {
    //                    totalnumber = totalnumber + number1;
    //                    number1 = number2;
    //                }
    //                if (operation == '-')
    //                {
    //                    totalnumber = totalnumber + number1;
    //                    number1 = -number2;
    //                }
    //                //calculate multiplication and division first
    //                if (operation == '*')
    //                {
    //                    number1 = number1 * number2;
    //                }
    //                if (operation == '/')
    //                {
    //                    if (number2 == 0)
    //                    {
    //                        fprintf(stderr, "Cannot divide by zero\n");
    //                        fclose(file);
    //                        return 1;
    //                    }
    //                    number1 = number1 / number2;
    //                }
    //                number2 = 0;
    //            }
    //            neednumber = 1;
    //        }
    //
    //        //after + the digits go into the second number
    //        if (charc == '+')
    //        {
    //            adding = 1;
    //            operation = '+';
    //        }
    //        if (charc == '-')
    //        {
    //            adding = 1;
    //            operation = '-';
    //        }
    //        if (charc == '*')
    //        {
    //            adding = 1;
    //            operation = '*';
    //        }
    //        if (charc == '/')
    //        {
    //            adding = 1;
    //            operation = '/';
    //        }
    //        position++;
    //        charc = fgetc(file);
    //    }
    //    if (neednumber == 1)
    //    {
    //        printf("ERROR:%lld\n", position);
    //        fclose(file);
    //        return 0;
    //    }
    //
    //    //checks which operation to calculate
    //    if (operation == '+')
    //    {
    //        printf("Result: %lld\n", totalnumber + number1 + number2);
    //    }
    //    if (operation == '-')
    //    {
    //        printf("Result: %lld\n", totalnumber + number1 - number2);
    //    }
    //    if (operation == '*')
    //    {
    //        printf("Result: %lld\n", totalnumber + number1 * number2);
    //    }
    //    if (operation == '/')
    //    {
    //        if (number2 == 0)
    //        {
    //            fprintf(stderr, "Cannot divide by zero\n");
    //            fclose(file);
    //            return 1;
    //        }
    //        printf("Result: %lld\n", totalnumber + number1 / number2);
    //    }
    // END OF OLD CALCULATOR VERSION

    //new parser result
    if (errorPosition != 0)
    {
        printf("ERROR:%lld\n", errorPosition);
    }
    else
    {
        printf("Result: %lld\n", result);
    }

    fclose(file);

    return 0;
}