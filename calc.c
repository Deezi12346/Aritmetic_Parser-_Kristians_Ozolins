// Kristians Ozolins 251RDC075
// https://github.com/Deezi12346/Aritmetic_Parser-_Kristians_Ozolins
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used: OpenAI ChatGPT ; Stack Overflow (links below)
// https://chatgpt.com/share/6ade85a9-f638-83ed-8d5f-b4642f67946e
// ChatGPT assistance documented in the shared conversation above.
// Actual prompts from that conversation (quoted):
// - "Im making an arithmetic parser and i need to add invalid character check"
// - "Could I just use string h library and do it that way?"
// - "What about the positon thought? should it be added?"
// - "Can you help me with parentheses?"
// - "The parser can handle simple expressions but when we get into ** it breaks"
// These prompts led to discussions of character validation, error positions,
// recursive descent and operator precedence. Earlier code was revised later.
// Other AI assistance discussed in subsequent messages: output files,
// multiple expressions, directory mode, floats and overflow checks.
// https://stackoverflow.com/questions/628761/convert-a-character-digit-to-the-corresponding-integer-in-c
// https://stackoverflow.com/a/64597676
// https://stackoverflow.com/questions/63854654/parsing-simple-mathematical-expressions
// https://stackoverflow.com/questions/7430248/creating-a-new-directory-in-c
// https://stackoverflow.com/questions/4293475/reading-c-file-line-by-line-using-fgetc
// https://stackoverflow.com/questions/11836064/c-creating-new-file-extensions-based-on-a-filename


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <math.h>

//builds numbers digit by digit without needing to store a whole line
long long addDigit(long long number, int charc)
{
    return number * 10 + (charc - '0');
}
//keeps integer results as integers, and decimal results as doubles
// AI assistance: discussed using both long long and double to preserve integer division.
typedef struct
{
    long long integer;
    double decimal;
    int isFloat;
} Value;

Value makeInt(long long number)
{
    Value result;
    result.integer = number;
    result.decimal = (double)number;
    result.isFloat = 0;
    return result;
}

Value makeFloat(double number)
{
    Value result;
    result.integer = 0;
    result.decimal = number;
    result.isFloat = 1;
    return result;
}

//variables used by the parser
FILE *file;
int charc;
long long position = 1;
long long errorPosition = 0; //-1 means allocation failure, not an expression error

//counts every character read, including newlines and comment text
// AI assistance: discussed recording 1-based positions for invalid input.
void nextChar(void)
{
    if (charc != EOF)
    {
        charc = fgetc(file);
        position = position + 1;
    }
}

//skip spaces, tabs and carriage returns without crossing the current line
void skipSpaces(void)
{
    while (charc == ' ' || charc == '\t' || charc == '\r')
    {
        nextChar();
    }
}

// AI assistance: discussed recursive descent to support nested parentheses.
// The parser was later extended into primary, power, term and expression.
Value expression(void);
// AI assistance: extended number reading to floats with strtod and error positions.
Value primary(void)
{
    skipSpaces();

    if (charc >= '0' && charc <= '9')
    {
        long long numberPosition = position;
        size_t length = 0, size = 32;
        char *text = malloc(size);
        int isFloat = 0;
        if (text == NULL) { errorPosition = -1; return makeInt(0); }
        while ((charc >= '0' && charc <= '9') || (charc == '.' && !isFloat))
        {
            if (length == size - 1) //doubling keeps long numbers efficient
            {
                if (size > (size_t)-1 / 2) { free(text); errorPosition = -1; return makeInt(0); }
                char *newText = realloc(text, size * 2);
                if (newText == NULL) { free(text); errorPosition = -1; return makeInt(0); }
                text = newText;
                size = size * 2;
            }
            int wasDot = charc == '.';
            long long dotPosition = position;
            text[length] = (char)charc;
            length = length + 1;
            nextChar();
            if (wasDot) isFloat = 1;
            if (wasDot && (charc < '0' || charc > '9'))
            {
                free(text); errorPosition = dotPosition; return makeInt(0);
            }
        }
        text[length] = '\0';
        //Convert decimals once with strtod; integers keep all 64 bits.
        errno = 0;
        Value number = isFloat ? makeFloat(strtod(text, NULL)) : makeInt(strtoll(text, NULL, 10));
        if ((isFloat && !isfinite(number.decimal)) || (!isFloat && errno == ERANGE))
        {
            errorPosition = numberPosition;
        }
        free(text);
        return number;
    }

    if (charc == '(')
    {
        nextChar();

        //calculate everything inside the parentheses
        Value number = expression();

        if (errorPosition != 0)
        {
            return makeInt(0);
        }

        skipSpaces();

        if (charc != ')')
        {
            //current position is where a closing parenthesis was expected
            errorPosition = position;
            return makeInt(0);
        }

        nextChar();
        return number;
    }

    errorPosition = position;
    return makeInt(0);
}
//look for a second *, then put the character back with ungetc
// AI assistance: user's ** question led to adding a power parsing level.
// Later discussion covered recognizing ** using fgetc and ungetc.
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
// AI assistance: right-associative powers, float powers and overflow checks.
Value power(void)
{
    Value number = primary();

    if (errorPosition != 0)
    {
        return makeInt(0);
    }

    skipSpaces();

    if (doubleStar())
    {
        long long operationPosition = position;
        nextChar(); //first *
        nextChar(); //second *
        skipSpaces();

        long long exponentPosition = position;
        Value exponent = power(); //right-associative: 2**3**2 is 2**(3**2)

        if (errorPosition != 0)
        {
            return makeInt(0);
        }

        //negative integer exponents are not allowed unless a float is involved
        if (!number.isFloat && !exponent.isFloat && exponent.integer < 0)
        {
            errorPosition = exponentPosition;
            return makeInt(0);
        }

        if (number.isFloat || exponent.isFloat)
        {
            double answer = pow(number.decimal, exponent.decimal);
            if (!isfinite(answer))
            {
                //NaN or infinity is an error at the ** operator
                errorPosition = operationPosition;
                return makeInt(0);
            }
            return makeFloat(answer);
        }

        //square the base to calculate large exponents without a very long loop
        long long answer = 1;
        long long base = number.integer;
        long long times = exponent.integer;
        while (times > 0)
        {
            if (times % 2 == 1)
            {
                if (__builtin_mul_overflow(answer, base, &answer))
                {
                    errorPosition = operationPosition;
                    return makeInt(0);
                }
            }

            times = times / 2;
            if (times > 0)
            {
                if (__builtin_mul_overflow(base, base, &base))
                {
                    errorPosition = operationPosition;
                    return makeInt(0);
                }
            }
        }

        return makeInt(answer);
    }

    return number;
}

//term uses power so ** is calculated before * and /
// AI assistance: mixed integer/float arithmetic, keeping integer division.
Value term(void)
{
    Value number = power();

    while (errorPosition == 0)
    {
        skipSpaces();

        if (charc != '*' && charc != '/')
        {
            break;
        }

        int operation = charc;
        long long operationPosition = position;
        nextChar();
        skipSpaces();

        long long rightPosition = position;
        Value number2 = power();

        if (errorPosition != 0)
        {
            return makeInt(0);
        }

        if (operation == '*')
        {
            if (number.isFloat || number2.isFloat)
            {
                number = makeFloat(number.decimal * number2.decimal);
                if (!isfinite(number.decimal))
                {
                    errorPosition = operationPosition;
                    return makeInt(0);
                }
            }
            else
            {
                long long answer;
                if (__builtin_mul_overflow(number.integer, number2.integer, &answer))
                {
                    errorPosition = operationPosition;
                    return makeInt(0);
                }
                number = makeInt(answer);
            }
        }
        else
        {
            if (number2.decimal == 0.0)
            {
                //division by zero is reported at the right operand
                errorPosition = rightPosition;
                return makeInt(0);
            }

            if (number.isFloat || number2.isFloat)
            {
                number = makeFloat(number.decimal / number2.decimal);
                if (!isfinite(number.decimal))
                {
                    errorPosition = operationPosition;
                    return makeInt(0);
                }
            }
            else
            {
                if (number.integer == (-9223372036854775807LL - 1) && number2.integer == -1)
                {
                    errorPosition = operationPosition;
                    return makeInt(0);
                }
                number = makeInt(number.integer / number2.integer);
            }
        }
    }

    return number;
}

//expression uses term so * and / are calculated before + and -
Value expression(void)
{
    Value number = term();

    while (errorPosition == 0)
    {
        skipSpaces();

        if (charc != '+' && charc != '-')
        {
            break;
        }

        int operation = charc;
        long long operationPosition = position;
        nextChar();

        Value number2 = term();

        if (errorPosition != 0)
        {
            return makeInt(0);
        }

        if (number.isFloat || number2.isFloat)
        {
            if (operation == '+')
            {
                number = makeFloat(number.decimal + number2.decimal);
            }
            else
            {
                number = makeFloat(number.decimal - number2.decimal);
            }

            if (!isfinite(number.decimal))
            {
                errorPosition = operationPosition;
                return makeInt(0);
            }
        }
        else
        {
            long long answer;
            int overflow;
            if (operation == '+')
            {
                overflow = __builtin_add_overflow(number.integer, number2.integer, &answer);
            }
            else
            {
                overflow = __builtin_sub_overflow(number.integer, number2.integer, &answer);
            }
            if (overflow)
            {
                errorPosition = operationPosition;
                return makeInt(0);
            }
            number = makeInt(answer);
        }
    }

    return number;
}

//same file calculation for one input file or each .txt file in a folder
// AI assistance: output files, multiline processing and first-error reporting.
int processFile(const char *inputName, const char *outputDir)
{
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

    size_t pathSize = strlen(outputDir) + nameLength + 60;
    char *outputName = malloc(pathSize);
    if (outputName == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
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
        fclose(file);
        return 1;
    }

    //nextChar also counts newlines and comments so positions do not reset
    //positions start at 1 for each new file in directory mode
    position = 1;
    errorPosition = 0;
    charc = fgetc(file);


    while (charc != EOF && errorPosition == 0)
    {
        skipSpaces();

        //a final line with only spaces is also blank
        if (charc == EOF)
        {
            break;
        }

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

        Value result = expression();
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

        //integer operations stay integer, decimal operations print up to 15 digits
        int written;
        if (result.isFloat)
        {
            written = fprintf(output, "%.15g\n", result.decimal);
        }
        else
        {
            written = fprintf(output, "%lld\n", result.integer);
        }
        if (written < 0)
        {
            fprintf(stderr, "Cannot write output file\n");
            fclose(output);
            fclose(file);
            free(outputName);
            return 1;
        }

        if (charc == '\n')
        {
            nextChar();
        }
    }

    //If any expression is invalid, remove earlier results and write only ERROR.
    if (errorPosition > 0)
    {
        if (fclose(output) != 0)
        {
            fprintf(stderr, "Cannot write output file\n");
            fclose(file);
            free(outputName);
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
            return 1;
        }
    }

    int failed = ferror(file) || errorPosition < 0;
    if (fclose(output) != 0)
    {
        failed = 1;
    }
    if (fclose(file) != 0)
    {
        failed = 1;
    }

    free(outputName);

    if (failed)
    {
        fprintf(stderr, "%s\n", errorPosition < 0 ? "Memory allocation failed" : "File reading or writing failed");
        return 1;
    }
    return 0;
}
// AI assistance: directory mode with -d/--dir and -o/--output-dir options.
int main(int argcount, char *argpass[])
{
    const char *inputName = NULL;
    const char *inputDir = NULL;
    const char *outputDir = NULL;

    //directory input was added after the single file version
    //read the options in any order
    for (int i = 1; i < argcount; i = i + 1)
    {
        if (strcmp(argpass[i], "-d") == 0 || strcmp(argpass[i], "--dir") == 0)
        {
            if (i + 1 >= argcount || inputDir != NULL)
            {
                fprintf(stderr, "Missing or repeated input directory\n");
                return 1;
            }
            i = i + 1;
            inputDir = argpass[i];
        }
        else if (strcmp(argpass[i], "-o") == 0 ||
                 strcmp(argpass[i], "--output-dir") == 0)
        {
            if (i + 1 >= argcount || outputDir != NULL)
            {
                fprintf(stderr, "Missing or repeated output directory\n");
                return 1;
            }
            i = i + 1;
            outputDir = argpass[i];
        }
        else if (argpass[i][0] == '-' || inputName != NULL)
        {
            fprintf(stderr, "Unknown option or extra input\n");
            return 1;
        }
        else
        {
            inputName = argpass[i];
        }
    }

    //show the required error when no input was given
    if (inputName == NULL && inputDir == NULL)
    {
        fprintf(stderr, "No input file\n");
        return 1;
    }

    if ((inputName == NULL && inputDir == NULL) ||
        (inputName != NULL && inputDir != NULL) ||
        (inputDir != NULL && inputDir[0] == '\0') ||
        (outputDir != NULL && outputDir[0] == '\0'))
    {
        fprintf(stderr, "Usage: calc [-d DIR] [-o OUTDIR] [input.txt]\n");
        return 1;
    }

    //take just the filename, without its folder and .txt ending
    //the folder name is used with -d
    char *defaultDir = NULL;
    if (outputDir == NULL)
    {
        const char *path = inputDir != NULL ? inputDir : inputName;
        size_t end = strlen(path);
        while (end > 1 && path[end - 1] == '/')
        {
            end = end - 1;
        }
        size_t start = end;
        while (start > 0 && path[start - 1] != '/')
        {
            start = start - 1;
        }
        size_t baseLength = end - start;
        if (inputDir == NULL && baseLength >= 4 &&
            strncmp(path + end - 4, ".txt", 4) == 0)
        {
            baseLength = baseLength - 4;
        }

        size_t size = baseLength + 40;
        defaultDir = malloc(size);
        if (defaultDir == NULL)
        {
            fprintf(stderr, "Memory allocation failed\n");
            return 1;
        }
        snprintf(defaultDir, size, "%.*s_kristians_251RDC075",
                 (int)baseLength, path + start);
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
            return 1;
        }
    }

    //do not process our own output files in directory mode
    if (inputDir != NULL)
    {
        struct stat inputInfo;
        struct stat outputInfo;
        if (stat(inputDir, &inputInfo) == 0 && stat(outputDir, &outputInfo) == 0 &&
            inputInfo.st_dev == outputInfo.st_dev && inputInfo.st_ino == outputInfo.st_ino)
        {
            fprintf(stderr, "Input and output directories must be different\n");
            free(defaultDir);
            return 1;
        }
    }

    int failed = 0;
    if (inputDir == NULL)
    {
        failed = processFile(inputName, outputDir);
    }
    else
    {
        //Only look at .txt files directly inside this folder.
        DIR *folder = opendir(inputDir);
        if (folder == NULL)
        {
            fprintf(stderr, "Cannot open input directory\n");
            free(defaultDir);
            return 1;
        }

        struct dirent *entry;
        while (1)
        {
            errno = 0;
            entry = readdir(folder);
            if (entry == NULL)
            {
                if (errno != 0)
                {
                    fprintf(stderr, "Cannot read input directory\n");
                    failed = 1;
                }
                break;
            }

            size_t length = strlen(entry->d_name);
            if (length < 5 || strcmp(entry->d_name + length - 4, ".txt") != 0)
            {
                continue;
            }

            size_t pathSize = strlen(inputDir) + length + 2;
            char *path = malloc(pathSize);
            if (path == NULL)
            {
                fprintf(stderr, "Memory allocation failed\n");
                failed = 1;
                break;
            }
            snprintf(path, pathSize, "%s/%s", inputDir, entry->d_name);

            struct stat info;
            if (stat(path, &info) == 0 && S_ISREG(info.st_mode))
            {
                if (processFile(path, outputDir) != 0)
                {
                    failed = 1;
                    free(path);
                    break;
                }
            }
            free(path);
        }
        if (closedir(folder) != 0)
        {
            fprintf(stderr, "Cannot close input directory\n");
            failed = 1;
        }
    }

    free(defaultDir);
    return failed;
}
