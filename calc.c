// Kristians Ozolins 251RDC075
// https://github.com/Deezi12346/Aritmetic_Parser-_Kristians_Ozolins
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used: OpenAI ChatGPT ; Stack Overflow (links below)
// https://stackoverflow.com/questions/628761/convert-a-character-digit-to-the-corresponding-integer-in-c
// https://stackoverflow.com/a/64597676
// https://stackoverflow.com/questions/63854654/parsing-simple-mathematical-expressions
// https://stackoverflow.com/questions/7430248/creating-a-new-directory-in-c?
// https://stackoverflow.com/questions/4293475/reading-c-file-line-by-line-using-fgetc?
// https://stackoverflow.com/questions/11836064/c-creating-new-file-extensions-based-on-a-filename?

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

//builds numbers digit by digit without needing to store a whole line
long long addDigit(long long number, int charc)
{
    return number * 10 + (charc - '0');
}
//variables used by the parser
FILE *file;
int charc;
long long position = 1;
long long errorPosition = 0;

//counts every character read, including newlines and comment text
void nextChar(void)
{
    if (charc != EOF)
    {
        charc = fgetc(file);
        position = position + 1;
    }
}

//skip spaces and tabs without crossing the current line
void skipSpaces(void)
{
    while (charc == ' ' || charc == '\t')
    {
        nextChar();
    }
}

long long expression(void);
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
            //current position is where a closing parenthesis was expected
            errorPosition = position;
            return 0;
        }

        nextChar();
        return number;
    }

    errorPosition = position;
    return 0;
}
//look for a second *, then put the character back with ungetc
int doubleStar(void)
{
    if (charc != '*')
    {
        return 0;
    }

    int next = fgetc(file);
    if (next != EOF)
    {
        ungetc(next, file);
    }

    return next == '*';
}

//power has higher priority than multiplication and division
long long power(void)
{
    long long number = primary();

    if (errorPosition != 0)
    {
        return 0;
    }

    skipSpaces();

    if (doubleStar())
    {
        nextChar(); //first *
        nextChar(); //second *
        skipSpaces();

        long long exponentPosition = position;
        long long exponent = power(); //right-associative: 2**3**2 is 2**(3**2)

        if (errorPosition != 0)
        {
            return 0;
        }

        if (exponent < 0)
        {
            errorPosition = exponentPosition;
            return 0;
        }

        //square the base to calculate large exponents without a very long loop
        long long answer = 1;
        while (exponent > 0)
        {
            if (exponent % 2 == 1)
            {
                answer = answer * number;
            }

            exponent = exponent / 2;
            if (exponent > 0)
            {
                number = number * number;
            }
        }

        return answer;
    }

    return number;
}

//term uses power so ** is calculated before * and /
long long term(void)
{
    long long number = power();

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
        long long number2 = power();

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
                //division by zero is reported at the right operand
                errorPosition = rightPosition;
                return 0;
            }

            number = number / number2;
        }
    }

    return number;
}

//expression uses term so * and / are calculated before + and -
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
    const char *inputName;
    const char *outputDir = NULL;

    //start with a single file and directory input (-d) will be added later
    if (argcount == 2 && argpass[1][0] != '-')
    {
        inputName = argpass[1];
    }
    else if (argcount == 4 &&
             (strcmp(argpass[1], "-o") == 0 || strcmp(argpass[1], "--output-dir") == 0) &&
             argpass[2][0] != '\0' && argpass[3][0] != '-')
    {
        outputDir = argpass[2];
        inputName = argpass[3];
    }
    else
    {
        fprintf(stderr, "Usage: calc [-o OUTDIR] input.txt\n");
        return 1;
    }

    file = fopen(inputName, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Cannot open file\n");
        return 1;
    }

    //take just the filename, without its folder and .txt ending
    const char *name = strrchr(inputName, '/');
    if (name == NULL)
    {
        name = inputName;
    }
    else
    {
        name = name + 1;
    }

    size_t nameLength = strlen(name);
    if (nameLength >= 4 && strcmp(name + nameLength - 4, ".txt") == 0)
    {
        nameLength = nameLength - 4;
    }

    char *defaultDir = NULL;
    if (outputDir == NULL)
    {
        size_t folderSize = nameLength + 40;
        defaultDir = malloc(folderSize);
        if (defaultDir == NULL)
        {
            fprintf(stderr, "Memory allocation failed\n");
            fclose(file);
            return 1;
        }

        snprintf(defaultDir, folderSize, "%.*s_kristians_251RDC075", (int)nameLength, name);
        outputDir = defaultDir;
    }

    //create the output folder but allow an existing directory too
    if (mkdir(outputDir, 0777) != 0)
    {
        struct stat info;
        if (errno != EEXIST || stat(outputDir, &info) != 0 || !S_ISDIR(info.st_mode))
        {
            fprintf(stderr, "Cannot create output folder\n");
            free(defaultDir);
            fclose(file);
            return 1;
        }
    }

    size_t pathSize = strlen(outputDir) + nameLength + 60;
    char *outputName = malloc(pathSize);
    if (outputName == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        free(defaultDir);
        fclose(file);
        return 1;
    }

    snprintf(outputName, pathSize, "%s/%.*s_kristians_ozolins_251RDC075.txt",
             outputDir, (int)nameLength, name);

    FILE *output = fopen(outputName, "w");
    if (output == NULL)
    {
        fprintf(stderr, "Cannot write output file\n");
        free(outputName);
        free(defaultDir);
        fclose(file);
        return 1;
    }

    //nextChar also counts newlines and comments so positions do not reset
    charc = fgetc(file);

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


    while (charc != EOF && errorPosition == 0)
    {
        skipSpaces();

        //blank line: no result
        if (charc == '\n')
        {
            nextChar();
            continue;
        }

        //comment: ignore characters but still count their positions
        if (charc == '#')
        {
            while (charc != EOF && charc != '\n')
            {
                nextChar();
            }

            if (charc == '\n')
            {
                nextChar();
            }
            continue;
        }

        long long result = expression();
        skipSpaces();

        //anything after the expression on its line is an error
        if (errorPosition == 0 && charc != '\n' && charc != EOF)
        {
            errorPosition = position;
        }

        if (errorPosition != 0)
        {
            break;
        }

        if (fprintf(output, "%lld\n", result) < 0)
        {
            fprintf(stderr, "Cannot write output file\n");
            fclose(output);
            fclose(file);
            free(outputName);
            free(defaultDir);
            return 1;
        }

        if (charc == '\n')
        {
            nextChar();
        }
    }

    //If any expression is invalid, remove earlier results and write only ERROR.
    if (errorPosition != 0)
    {
        if (fclose(output) != 0)
        {
            fprintf(stderr, "Cannot write output file\n");
            fclose(file);
            free(outputName);
            free(defaultDir);
            return 1;
        }
        output = fopen(outputName, "w");
        if (output == NULL || fprintf(output, "ERROR:%lld\n", errorPosition) < 0)
        {
            fprintf(stderr, "Cannot write output file\n");
            if (output != NULL)
            {
                fclose(output);
            }
            fclose(file);
            free(outputName);
            free(defaultDir);
            return 1;
        }
    }

    int failed = ferror(file);
    if (fclose(output) != 0)
    {
        failed = 1;
    }
    if (fclose(file) != 0)
    {
        failed = 1;
    }

    free(outputName);
    free(defaultDir);

    if (failed)
    {
        fprintf(stderr, "File reading or writing failed\n");
        return 1;
    }
    return 0;
}
