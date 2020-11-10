#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <windows.h>

// xor file named "argv[1]" in range [ argv[2], argv[3] ) with random 8bit key

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        return 1;
    }

    long startAddr =    strtol(argv[2], NULL, 16);
    long endAddr =      strtol(argv[3], NULL, 16);
    unsigned char c =   0x00;
    unsigned char key = 0x00;

    srand(time(NULL));
    // key = (unsigned char)(rand() % 0x100);
    key = 0x5A; // TODO: random key

    FILE* F = NULL;
    fopen_s(&F, argv[1], "r+b");
    if (!F)
    {
        return GetLastError();
    }

    fseek(F, startAddr, SEEK_SET);

    printf("File:  %s\n", argv[1]);
    printf("start: 0x%04X\n", startAddr);
    printf("end:   0x%04X\n", endAddr);
    printf("key:   0x%02X\n", key);
    for (long i = startAddr; i < endAddr; i++, fseek(F, i, SEEK_SET))
    {
        c = fgetc(F);
        fseek(F, i, SEEK_SET);
        fputc(c ^ key, F);
    }

    fclose(F);
    return 0;
}

