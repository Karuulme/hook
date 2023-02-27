#include <Windows.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
using namespace std;
BOOL WINAPI hooked_WriteFile(HWND   hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT   uType);
typedef int (WINAPI * orjinalMesajAlan)(HWND   hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT   uType);
LPVOID orjinalKodAlloc = {};
int bit;
int main() {
    HINSTANCE library = LoadLibraryA("user32.dll");
    FARPROC  orjinalKod = GetProcAddress(library, "MessageBoxA");
    if (NULL == orjinalKod) {
        cout << GetLastError();
        return -1;
    }
    unsigned char* chr_orjinalKod = (unsigned char*)orjinalKod;
    unsigned char opCode;
    for (bit = 0; bit < 100; bit++) {
        cout<< bit<<". " << hex << (int)chr_orjinalKod[bit] << endl;
        opCode = (int)chr_orjinalKod[bit];
        if (opCode == 0xc3)
        {
            // cout << "0xc3------------------------------------RET  64 bit"<<endl;
            bit++;
            break;
        }
        if (opCode == 0xc2)
        {
            //cout << "0xc2------------------------------------RET X 32bit"<<endl;
            bit += 2;
            break;
        }
    }

    cout << "----------------------------------------------------------------------------------" << endl;
    unsigned char* geciciAlan = (unsigned char*)orjinalKod;
    
    while (true) {
        orjinalKodAlloc=VirtualAlloc(geciciAlan, bit, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if (orjinalKodAlloc != NULL)
            break;
        geciciAlan = (unsigned char*)geciciAlan - 0x1000;
    }
    cout << "new alloc:" << orjinalKodAlloc<<endl;

    memset(orjinalKodAlloc,0x90,bit);//nop
    memcpy(orjinalKodAlloc, orjinalKod, bit);
    unsigned char* geciciparsel = (unsigned char*)orjinalKodAlloc;
    int len = bit;

    DWORD test_1;
    if (VirtualProtect(orjinalKod, bit, PAGE_EXECUTE_READWRITE, &test_1) == 0) {
        VirtualFree(orjinalKodAlloc, bit, MEM_RELEASE);
        cout << -1;
        return -1;
    }
    memset(chr_orjinalKod, 0x90, bit);
   // unsigned char* hookFunction = (unsigned char*)hooked_WriteFile;

    *chr_orjinalKod = (unsigned char)0x48;
    *(chr_orjinalKod + 1) = (unsigned char)0xB8;
    *((long long int*)(chr_orjinalKod + 2)) = (long long int)hooked_WriteFile;
    *((unsigned short*)(chr_orjinalKod + 10)) = 0xe0ff;

    for (int i = 0; i < bit; i++) {
        cout  << hex<<(int)chr_orjinalKod[i] << endl;
    }



    MessageBoxA(0, "meme22222222222222222222222222t", "Mesaj Başlığı", S_OK);
    return 0;
}
BOOL WINAPI hooked_WriteFile(HWND   hWnd,LPCSTR lpText,LPCSTR lpCaption,UINT   uType)
{
    cout << "Mesaj:" << lpText<<endl;
    return true;
}