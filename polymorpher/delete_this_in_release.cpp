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

void decryptSelf(void* address, const unsigned int size, const unsigned char key)
{
__asm push eax;
__asm pop eax;
    MEMORY_BASIC_INFORMATION mbi;
    unsigned int* pJmpOffset = (unsigned int*)((unsigned char*)address + 1);
__asm push eax;
__asm pop eax;
    unsigned char* pAddr = (unsigned char*)address + *pJmpOffset + 5;
__asm push ebx;
__asm pop ebx;
    // get rwx permissions
__asm inc eax;
__asm dec eax;
    VirtualQuery((void*)pAddr, &mbi, sizeof(mbi));
    VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &mbi.Protect);
    // TODO: bruteforce
    for (int i = 0; i < size; i++) {
        *(pAddr + i) ^= key;
    }
__asm nop;
__asm inc ecx;
__asm nop;
__asm push edx;
__asm pop edx;
__asm dec ecx;
__asm inc edx;
__asm dec edx;
}

int main(int argc, char* argv[])
{
    decryptSelf((void*)&func, FUNC_SIZE_func, keyTMP);
    func();
    return 0;
__asm inc ebx;
__asm dec ebx;
}
