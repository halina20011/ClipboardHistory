#include <stdlib.h>
#include <unistd.h>

#include "base64.c"

int printA(int *array, unsigned int size){
    printf("Array[%u]{", size);
    for(int i = 0; i < size; i++){
        printf("%i, ", array[i]);
    }
    printf("}\n");
    return 0;
}

int fileExists(char *fileName){
    if(access(fileName, F_OK) == -1){
        FILE *fp;
        fp = fopen(fileName, "w");
        if(fp == NULL){
            printf("[%d] Error when creating new file\n", __LINE__);
        }
        fclose(fp);
    }
    return 0;
}

int writeToTmpFile(char *fileName, const char *text){
    FILE *fp;
    fp = fopen(fileName, "w");
    if(fp == NULL){
        return 1;
    }
    fprintf(fp, "%s", text);
    fclose(fp);
    return 0;
}

char *removeLeadingZeros(char *buf, unsigned int bufSize, unsigned int *numberSize){
    unsigned int numStart = 0;

    while(buf[numStart] == '0'){
        numStart += 1;
    }
     
    // printf("Num: '%s' start: %u\n", buf, numStart);

    unsigned int length = bufSize - numStart;

    char *returnChar;
    length++;           // Add null termination
    returnChar = malloc(sizeof(char) * length);

    if(returnChar == NULL){
        return NULL;
    }

    char *pos;
    pos = returnChar;

    for(int i = numStart; i < bufSize; i++){
        *pos++ = buf[i];
    }
    *pos = '\0';
    *numberSize = pos - returnChar;

    // printf("%zu\n", sizeof(returnChar));
    // printf("Return char: %s\n", returnChar);

    fflush(stdout);

    return returnChar;
}

int countChar(FILE *fp, const char find){
    int count = 0;
    char c = getc(fp);

    while(c != EOF){
        if(c == find){
            count++;
        }
        c = getc(fp);
    }

    return count;
}

// Max size of file is 65535
unsigned int sizeOfFile(FILE *fp){
    unsigned int count = 0;

    while(getc(fp) != EOF){
        count++;
    }

    return count;
}

unsigned char *getData(char *fileName, unsigned int *dataLength, unsigned int dataLengthCount, unsigned int index, size_t *decodedTextSize){
    FILE *fp;
    fp = fopen(fileName, "r");
    if(fp == NULL){
        printf("Error when reading encoded data.\n");
        return NULL;
    }

    unsigned int *pos = dataLength;

    //    7   xxxxxx
    // xxxxxx:[data]
    int dataLengthOffset = 7;
    int fileIndex = dataLengthOffset;
    for(int i = 0; i < index; i++){
        unsigned int l = *pos++;
        // printf("Data l: %u\n", l);
        //           dataLength:data\n
        fileIndex += dataLengthOffset + l + 1;
    }
    // printf("File index: %u\n", fileIndex);
   
    // de-referencing the pointer
    unsigned int dataSize = *pos;
    // printf("Encoded data size: %u\n", dataSize);
    char *data = (char*)malloc(sizeof(char) * (dataSize + 1));
    
    fseek(fp, fileIndex, SEEK_SET);
    char *rCode = fgets(data, dataSize + 1, fp);

    if(rCode == NULL){
        printf("Error occurrend when reading encoded data.\n");
        fclose(fp);
        return NULL;
    }

    fflush(stdout);
    fclose(fp);

    unsigned char *decodedData;
    size_t decodedDataSize = 0;
    decodedData = base64Decode(data, dataSize, &decodedDataSize);
    
    // printf("Encoded data: %s\n", data);
    printf("Decoded data{%zu}: %s\n", decodedDataSize, decodedData);

    *decodedTextSize = decodedDataSize;
    return decodedData;
}

// Return list of all numbers with data sizes
unsigned int *readFile(char *fileName, unsigned int *dataLengthCount){
    FILE *fp;
    fp = fopen(fileName, "r");
    if(fp == NULL){
        printf("Error when opening file on line: %d\n", __LINE__);
        return NULL;
    }
    
    unsigned int *dataLength;
    
    // Count number of newlines and colons for allocating dataLength size 
    int newLineC = countChar(fp, '\n');
    rewind(fp);
    int colonC = countChar(fp, ':');
    rewind(fp);
    // printf("New lines:  %i\n", newLineC);
    // printf("Colons:     %i\n", colonC);

    unsigned int size = 0;
    if(newLineC == colonC){
        size = newLineC;
    }
    else{
        printf("Wrong format in file\n");
        exit(1);
    }
    
    dataLength = malloc(sizeof(unsigned int) * size);
    unsigned int *pos = dataLength;

    // Size + '\0'
    char buf[8];
    char num[7];

    while(fgets(buf, sizeof(buf), fp) != NULL){
        // printf("Buf: %s\n", buf);
        if(buf[6] == ':'){
            memcpy(num, buf, 6);
            // printf("Num: %s\n", num);

            if(num == NULL){
                printf("AAAAA num is NULL!");
            }

            char *numberChar;
            unsigned int numberSize;
            numberChar = removeLeadingZeros(num, 6, &numberSize);

            // Parsing
            unsigned int number = 0;
            number = (unsigned int)strtoul(numberChar, NULL, 10);
            // printf("Status of parsing: %u", number);
            free(numberChar);
            // printf("Numer: %u\n", number);
            fflush(stdout);
            *pos++ = number;
        }
    }
    *dataLengthCount = pos - dataLength;

    // de-referencing the pointer
    unsigned int tmpLength = *dataLengthCount;
    printA(dataLength, tmpLength);

    // if(dataLengthCount != lineCount){
    //     printf("Error when parsing file on line: %d\n", __LINE__);
    //     return NULL;
    // }

    fclose(fp);
    fflush(stdout);

    return dataLength;
}

int writeToFile(char *fileName, char *result, unsigned long size){
    FILE *fp;
    fp = fopen(fileName, "r+");
    if(fp == NULL){
        printf("%s", "Error when opening file\n!");
        return 1;
    }
    // unsigned int size = countChar(fp, ':');
    // printf("Number of ':' %i", size);
    int n = countChar(fp, '\n');
    printf("Line count: %d\n", n);
    // TODO: if there are more then 10 lines remove them
    // if(10 < n){
    // 
    // }
    
    // Convert data to base64
    size_t out;
    unsigned char *base64Output;
    base64Output = base64Encode(result, size, &out);
    if(base64Output == NULL){
        printf("Error");
        return 1;
    }

    printf("Size:   %zu\n", out);
    printf("Base64: %s\n", base64Output);

    // Writing to file
    char *textSize = malloc(sizeof(char) * 6);
    sprintf(textSize, "%06u", out);
    fprintf(fp, "%s:%s\n", textSize, base64Output);

    free(textSize);
    free(base64Output);

    fflush(stdout);

    fclose(fp);
    return 0;
}

// Delete specific line from file
int deleteLine(char *fileName, int index){
    FILE *fp;
    fp = fopen(fileName, "r");
    if(fp == NULL){
        return 1;
    }
    
    // Check if lineIndex is out of range
    int lineCount = countChar(fp, '\n');
    rewind(fp);

    if(lineCount <= index){
        printf("Delete index is out of range. Number of lines: %i Index: %i\n", lineCount, index);
        return 1;
    }

    printf("Line count in file: %i", lineCount);
    if(lineCount == 0){
        return 0;
    }

    // Get size of file
    // unsigned int size = sizeOfFile();
    // printf("%u", size);
    // rewind(fp);
    
    // Get size of file after line deletion
    unsigned int fileSizeAfterDeletion = 0;
    unsigned int currentLineOffset = 0;
    unsigned int fileSplitIndex[2] = {0, 0};

    unsigned int *dataLength;
    unsigned int dataLengthCount;
    
    dataLength = readFile(fileName, &dataLengthCount);
    if(dataLength == NULL){
        return 1;
    }
    
    unsigned int *pos = dataLength;

    for(int i = 0; i < dataLengthCount; i++){
        // de-referencing the pointer
        unsigned int dataSize = *pos++;
        //               dataSize + dataLength + '\n'
        unsigned int lineSize = 7 + dataSize + 1;
        // printf("Line size: %u\n", dataSize);
        if(i != index){
            fileSizeAfterDeletion += lineSize;
            // printf("Size: %u\n", fileSizeAfterDeletion);
        }
        else{
            fileSplitIndex[0] = currentLineOffset;
            fileSplitIndex[1] = currentLineOffset + lineSize;
            // printf("Split [%u][%u]\n", fileSplitIndex[0], fileSplitIndex[1]);
        }
        currentLineOffset += lineSize;
    }

    free(dataLength);

    // Make array of file without line that should be deleted
    char *editedFileData = malloc(sizeof(char) * (fileSizeAfterDeletion + 1));
    char *editedFileDataPos = editedFileData;
    
    printf("0|---------|%u       %u|---------|%u\n", fileSplitIndex[0], fileSplitIndex[1], currentLineOffset);

    // Add before gap
    fseek(fp, 0, SEEK_SET);
    char readChar = fgetc(fp);
    unsigned int charReadIndex = 0;

    while(readChar != EOF){
        // printf("Index: %u\n", charReadIndex);
        if(charReadIndex < fileSplitIndex[0] || fileSplitIndex[1] <= charReadIndex){
            *editedFileDataPos++ = readChar;
        }
        readChar = fgetc(fp);
        charReadIndex += 1;
    }
    *editedFileDataPos = '\0';

    // printf("Edited file: >%s<\n", editedFileData);
    fclose(fp);

    // Write to file
    fp = fopen(fileName, "w");
    if(fp == NULL){
        return 1;
    }
    fprintf(fp, "%s", editedFileData);

    fclose(fp);

    free(editedFileData);

    fflush(stdout);

    return 0;
}
