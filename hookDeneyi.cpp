#include <Windows.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
using namespace std;

#define FIRSTBYTECPY 5
#define TRAMPOLINBYTE 40
typedef struct _INFORMATIONABOUTHOOKEDFUNCTIONS {
    ULONG nameAddress;
    ULONG oriAddress;
    ULONG hookFuncAddress;
    VOID* tramFuncAddress;
    VOID* tramAddress = NULL;
    ;
}INFORMATIONABOUTHOOKEDFUNCTIONS, * PINFORMATIONABOUTHOOKEDFUNCTIONS;

vector<pair<ULONG, INFORMATIONABOUTHOOKEDFUNCTIONS>> _HOOKEDFUNCTION;
//----------------------------------------------------------------------------------------------------------
int HOOK_CREATE(VOID* pOriginAddress, VOID* pHookFuncAddress, VOID* tramFuncAddress) {
    INFORMATIONABOUTHOOKEDFUNCTIONS newHookInformation = { 0 };

    DWORD testValue_1 = 0;
    testValue_1 = 0;
    if (VirtualProtect(pOriginAddress, FIRSTBYTECPY, PAGE_EXECUTE_READWRITE, &testValue_1) == 0) {
        cout << -1;
        VirtualFree(tramFuncAddress, FIRSTBYTECPY, MEM_RELEASE);
        return -1;
    }     memset(pOriginAddress, 0x90, FIRSTBYTECPY);
    *((unsigned char*)pOriginAddress) = (unsigned char)0x48;
    *((unsigned char*)pOriginAddress + 1) = (unsigned char)0xB8;
    *((long long int*)((unsigned char*)pOriginAddress + 2)) = (long long int)pHookFuncAddress;
    *((unsigned short*)((unsigned char*)pOriginAddress + 10)) = 0xe0ff;

    ULONG* requiredFunctionAddress = (ULONG*)pOriginAddress;
    while (newHookInformation.tramAddress == NULL)
    {
        newHookInformation.tramAddress = VirtualAlloc(requiredFunctionAddress, TRAMPOLINBYTE, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        requiredFunctionAddress = requiredFunctionAddress + 0x1000;
    }
    cout << "pOriginAddress:               :" << pOriginAddress << endl;
    cout << "newHookInformation.tramAddress:" << newHookInformation.tramAddress << endl;

    ULONG* offset = (ULONG*)newHookInformation.tramAddress - (ULONG)pOriginAddress;
    memset(newHookInformation.tramAddress, 0x90, TRAMPOLINBYTE);
    memcpy(newHookInformation.tramAddress, pOriginAddress, FIRSTBYTECPY);

    *((unsigned char*)newHookInformation.tramAddress + FIRSTBYTECPY) = (unsigned char)0x48;
    *((unsigned char*)newHookInformation.tramAddress + 1 + FIRSTBYTECPY) = (unsigned char)0xB8;
    *((long long int*)((unsigned char*)newHookInformation.tramAddress + 2 + FIRSTBYTECPY)) = (long long int)offset + FIRSTBYTECPY;
    *((unsigned short*)((unsigned char*)newHookInformation.tramAddress + 10 + FIRSTBYTECPY)) = 0xe0ff;

    tramFuncAddress = newHookInformation.tramAddress;;
    unsigned char* deneme1 = (unsigned char*)newHookInformation.tramAddress;
    for (int i = 0; i < TRAMPOLINBYTE; i++)
        cout << hex << (int)deneme1[i] << endl;


    newHookInformation.nameAddress = (ULONG)pOriginAddress;
    newHookInformation.oriAddress = (ULONG)pOriginAddress;
    newHookInformation.hookFuncAddress = (ULONG)pHookFuncAddress;
    newHookInformation.tramFuncAddress = tramFuncAddress;
    _HOOKEDFUNCTION.push_back(make_pair((ULONG)pOriginAddress, newHookInformation));

    return 0;
}
//----------------------------------------------------------------------------------------------------------
typedef int (WINAPI* TB_MessageBoxA)(HWND, LPCSTR, LPCSTR, UINT);
TB_MessageBoxA TBMessageBoxA = NULL;

int WINAPI HK_MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
    TBMessageBoxA(NULL, "TUGÇE", "asdas", MB_OK);
    cout << "BISELER OLMAYA BASLADI" << endl;
    return 0;
}
//----------------------------------------------------------------------------------------------------------
void clear() {
    for (int i = 0; i < _HOOKEDFUNCTION.size(); i++) {
        //free(_HOOKEDFUNCTION.at(i).second.tramFuncAddress);
    }
}
//----------------------------------------------------------------------------------------------------------
int main() {
    HOOK_CREATE(&MessageBoxA, &HK_MessageBoxA, &TBMessageBoxA);
    MessageBoxA(NULL, "TUGÇE", "asdas", MB_OK);
    clear();
}
//----------------------------------------------------------------------------------------------------------