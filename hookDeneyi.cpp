#include <Windows.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
using namespace std;

#define FIRSTBYTECPY 5
#define TRAMPOLINEBYTE 40
#define SHORTJUMPBYTE 15
typedef struct _INFORMATIONABOUTHOOKEDFUNCTIONS {
    ULONG oriAddress;
    ULONG hookFuncAddress;
    VOID* shortJumpAddress = NULL;
    VOID* trambolineAddress  = NULL;
}INFORMATIONABOUTHOOKEDFUNCTIONS, *PINFORMATIONABOUTHOOKEDFUNCTIONS;
vector<pair<ULONG, INFORMATIONABOUTHOOKEDFUNCTIONS>> _HookedFunction;


//----------------------------------------------------------------------------------------------------------
LPVOID  MK_VirtualAlloc(VOID* startAddress,SIZE_T size) {
    ULONG* requiredFunctionAddress = (ULONG*)startAddress;
    LPVOID address=NULL;
    while (address == NULL)
    {
        address = VirtualAlloc(requiredFunctionAddress, size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        requiredFunctionAddress = requiredFunctionAddress + 0x1000;
    }
    memset(address, 0x90, size);
    DWORD testValue_1 = 0;
    if (VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &testValue_1) == 0) {
        cout << -1;
        return 0;
    }
    return address;

}
VOID MK_LongJump(VOID * gotoAddress,VOID * setAddress) {

    unsigned char* uc_shortJumpAddress = (unsigned char*)setAddress;
    *uc_shortJumpAddress = (unsigned char)0x48;
    *(uc_shortJumpAddress + 1) = (unsigned char)0xB8;
    *((long long int*)(uc_shortJumpAddress + 2)) = (long long int)gotoAddress;
    *((unsigned short*)(uc_shortJumpAddress + 10)) = 0xe0ff;
}
//----------------------------------------------------------------------------------------------------------
int MK_VirtualProtect(VOID* memoryAddress,SIZE_T size) {
    DWORD testValue_1 = 0;
    if (VirtualProtect(memoryAddress, size, PAGE_EXECUTE_READWRITE, &testValue_1) == 0) {
        cout << -2;
        return -2;
    }
    memset(memoryAddress,0x90,size);
}

//----------------------------------------------------------------------------------------------------------
typedef int (WINAPI* TB_MessageBoxA)(HWND, LPCSTR, LPCSTR, UINT);
TB_MessageBoxA TBMessageBoxA = NULL;
int WINAPI TEST(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
    cout << "TEST VERISI" << endl;
    return 0;
}
int MK_HookCreate(VOID* pOriginAddress, VOID* pHookFuncAddress, LPVOID* tramFuncAddress) {
    //LPVOID* ppOriginal = reinterpret_cast<LPVOID*>(tramFuncAddress);
    INFORMATIONABOUTHOOKEDFUNCTIONS newHookInformation = { 0 };
    ULONG* requiredFunctionAddress = (ULONG*)pOriginAddress;

    
    newHookInformation.shortJumpAddress = MK_VirtualAlloc(pOriginAddress, SHORTJUMPBYTE);
    newHookInformation.trambolineAddress = MK_VirtualAlloc(pOriginAddress, TRAMPOLINEBYTE);
    memcpy(newHookInformation.trambolineAddress, pOriginAddress,5);
    MK_VirtualProtect(pOriginAddress, FIRSTBYTECPY);
 
    unsigned char* deneme9 = (unsigned char*)newHookInformation.trambolineAddress;
    for (int i = 0; i < 20; i++) {
        cout << hex << (int)deneme9[i] << endl;
    }
    cout << "-------------------------------------" << endl;
    //---------------------------->>>>>>>>>>>>>>
    MK_LongJump((unsigned char*)pOriginAddress+ FIRSTBYTECPY, (unsigned char*)newHookInformation.trambolineAddress + FIRSTBYTECPY);
    *tramFuncAddress = (LPVOID*)newHookInformation.trambolineAddress;
    for (int i = 0; i < 20; i++) {
        cout << hex << (int)deneme9[i] << endl;
    }
    //----------------------------<<<<<<<<<<<<<<

    long long int offset =((long long int)newHookInformation.shortJumpAddress- (long long int)pOriginAddress)-5;
    memset(pOriginAddress,0xE9, 1);
    memcpy((unsigned char*)pOriginAddress+1, &offset, 4);
    MK_LongJump(pHookFuncAddress, newHookInformation.shortJumpAddress);

     return 0;
 }
//----------------------------------------------------------------------------------------------------------

int WINAPI HK_MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
    cout << "BISELER OLMAYA BASLADI" << endl;
    TBMessageBoxA(NULL, "TUGÇEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "asdas", MB_OK);
    return 0;
}

//----------------------------------------------------------------------------------------------------------
void clear() {
    for (int i = 0; i < _HookedFunction.size(); i++) {
      // VirtualFree(_HookedFunction.at(i).second.trambolineAddress,);
    }
}
//----------------------------------------------------------------------------------------------------------

int main() {
    MK_HookCreate(&MessageBoxA,&HK_MessageBoxA, reinterpret_cast<LPVOID*>(&TBMessageBoxA));
    MessageBoxA(NULL, "TUGÇE", "asdas", MB_OK);
//    return 0;
}
//----------------------------------------------------------------------------------------------------------