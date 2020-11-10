#include <windows.h>
#include <memoryapi.h>
#include <stdio.h>

#define keyTMP 0xA5 // todo random key

#define FUNC_SIZE_func 0xEE // size of function func() in bytes
void func()
{
    const unsigned int signature = 0xDEADC0DE;

    char str[] = "my_test_string";
    printf("~~");
    for (unsigned int i = 0; i < strlen(str); i++)
    {
        printf("%c+%i-", str[i], i);
    }
    printf("==\n");

}

void getRWX(void* startAddr, void* endAddr)
{
    MEMORY_BASIC_INFORMATION mbi;
    
    for (unsigned char* p = (unsigned char*)startAddr; p < (unsigned char*)endAddr; p += mbi.RegionSize)
    {
        // get rwx permissions
        VirtualQuery((void*)p, &mbi, sizeof(mbi));
        VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &mbi.Protect);
    }
}

void decryptSelf(void* address, const unsigned int size, const unsigned char key)
{
    unsigned int* pJmpOffset = (unsigned int*)((unsigned char*)address + 1);
    unsigned char* pAddr = (unsigned char*)address + *pJmpOffset + 5;

    getRWX(pAddr, pAddr + size);

    // TODO: bruteforce
    for (int i = 0; i < size; i++) {
        *(pAddr + i) ^= key;
    }
}

int main(int argc, char* argv[])
{
    decryptSelf((void*)&func, FUNC_SIZE_func, keyTMP);
    func();
    return 0;
}
