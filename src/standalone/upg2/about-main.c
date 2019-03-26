 #include <stdio.h>
#include <string.h>

int getLength(char*);

int main(int argc, char* argv[])
{
    char* parameter = argv[0];
    int parameterCount = 0;
    int totalLength = 0;

    while(parameter != NULL) {
        int stringLength = getLength(parameter);
        totalLength += stringLength;
        printf("%-21s %4d\n", parameter, stringLength);
        parameterCount++;
        parameter = argv[parameterCount];
    }

    printf("%-21s %4d\n", "Total length", totalLength);
    printf("%-21s %2.2f\n", "Average length", (float) totalLength/argc);
    return totalLength;
}


int getLength(char* str) {
    char *c = str;
    int stringLength = 0;
    while (*c != '\0') {
        stringLength++;
        c++;
    }
    return stringLength;
}