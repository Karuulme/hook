#include <Windows.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
using namespace std;

#define FIRSTBYTECPY 5
#define TRAMPOLINEBYTE 20
#define SHORTJUMPBYTE 15
#define HOOKINGFUNC_MAXSIZE 15

typedef struct _SHORTJUMPADDRESS {
    VOID* shortJumpAddress = NULL;
}SHORTJUMPADDRESS, * PSHORTJUMPADDRESS;
typedef struct _TRAMBOLINEADDRESS {
    VOID* trambolineAddress = NULL;
}TRAMBOLINEADDRESS, * PTRAMBOLINEADDRESS;
typedef struct _VIRTUALJUMPPOINTS {
    SHORTJUMPADDRESS  shortJumpAddress;
    TRAMBOLINEADDRESS trambolineAddress;
}VIRTUALJUMPPOINTS, * PVIRTUALJUMPPOINTS;
typedef struct _INFORMATIONABOUTHOOKEDFUNCTIONS {
    VOID* oriAddress      = 0;
    VOID* hookFuncAddress = 0;
    VIRTUALJUMPPOINTS virtualAddresses;
}INFORMATIONABOUTHOOKEDFUNCTIONS, * PINFORMATIONABOUTHOOKEDFUNCTIONS;
typedef  struct _HOOKEDLIST {
private:
    INFORMATIONABOUTHOOKEDFUNCTIONS hooks[HOOKINGFUNC_MAXSIZE];
    short lenght = 0;
public:
    short size() {
        return lenght;
    }
    ~_HOOKEDLIST();
    void OneDelete(VOID* pOriginAddress);
    void Add(INFORMATIONABOUTHOOKEDFUNCTIONS informationabouthookedfunctions);
    INFORMATIONABOUTHOOKEDFUNCTIONS get(short i);

}HOOKEDLIST, * PHOOKEDLIST;
void HOOKEDLIST::OneDelete(VOID* pOriginAddress){
    for (int i = 0; i < lenght; i++) {
        if (hooks[i].oriAddress==pOriginAddress) {
            VirtualFree(get(i).virtualAddresses.shortJumpAddress.shortJumpAddress, 0, MEM_RELEASE);
            VirtualFree(get(i).virtualAddresses.trambolineAddress.trambolineAddress, 0, MEM_RELEASE);
        }
    }        
}
INFORMATIONABOUTHOOKEDFUNCTIONS HOOKEDLIST::get(short i) {
    if (i <= lenght)
        return hooks[i];
    return hooks[0];
}
HOOKEDLIST::~_HOOKEDLIST() {
    for (int i = 0; i < lenght; i++) {
        VirtualFree(get(i).virtualAddresses.shortJumpAddress.shortJumpAddress,   0, MEM_RELEASE);
        VirtualFree(get(i).virtualAddresses.trambolineAddress.trambolineAddress, 0, MEM_RELEASE);
    }
}
void HOOKEDLIST::Add(INFORMATIONABOUTHOOKEDFUNCTIONS informationabouthookedfunctions) {
    memcpy(&hooks[lenght], &informationabouthookedfunctions, sizeof(INFORMATIONABOUTHOOKEDFUNCTIONS));
    lenght++;
}
HOOKEDLIST _hookedList;
//----------------------------------------------------------------------------------------------------------
LPVOID  MK_VirtualAlloc(VOID* startAddress,SIZE_T size) {
    ULONG* requiredFunctionAddress = (ULONG*)startAddress;
    LPVOID address=NULL;
    while (address == NULL){
        address = VirtualAlloc(requiredFunctionAddress, size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        requiredFunctionAddress = requiredFunctionAddress + 0x1000;
    }
    memset(address, 0x90, size);
    DWORD testValue_1 = 0;
    if (VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &testValue_1) == 0)
        return NULL;
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
    if (VirtualProtect(memoryAddress, size, PAGE_EXECUTE_READWRITE, &testValue_1) == 0)
        return -2;
    memset(memoryAddress,0x90,size);
    return 0;
}
//----------------------------------------------------------------------------------------------------------
int MK_HookCreate(VOID* pOriginAddress, VOID* pHookFuncAddress, LPVOID* tramFuncAddress) {
    INFORMATIONABOUTHOOKEDFUNCTIONS newHookInformation = { 0 };
    newHookInformation.oriAddress      = pOriginAddress;
    newHookInformation.hookFuncAddress = pHookFuncAddress;

    newHookInformation.virtualAddresses.shortJumpAddress.shortJumpAddress= MK_VirtualAlloc(pOriginAddress, SHORTJUMPBYTE);
    if (newHookInformation.virtualAddresses.shortJumpAddress.shortJumpAddress ==NULL) {
        return -1;
    }
    newHookInformation.virtualAddresses.trambolineAddress.trambolineAddress  = MK_VirtualAlloc(pOriginAddress, TRAMPOLINEBYTE);
    if (newHookInformation.virtualAddresses.trambolineAddress.trambolineAddress ==NULL) {
        return -2;
    }
    memcpy(newHookInformation.virtualAddresses.trambolineAddress.trambolineAddress, pOriginAddress,5);
    if (MK_VirtualProtect(pOriginAddress, FIRSTBYTECPY)!=0) {
        return -3;
    }
    MK_LongJump((unsigned char*)pOriginAddress+ FIRSTBYTECPY, (unsigned char*)newHookInformation.virtualAddresses.trambolineAddress.trambolineAddress + FIRSTBYTECPY);
    *tramFuncAddress = (LPVOID*)newHookInformation.virtualAddresses.trambolineAddress.trambolineAddress;
    long long int offset =((long long int)newHookInformation.virtualAddresses.shortJumpAddress.shortJumpAddress - (long long int)pOriginAddress)-5;
    memset(pOriginAddress,0xE9, 1);
    memcpy((unsigned char*)pOriginAddress+1, &offset, 4);
    MK_LongJump(pHookFuncAddress, newHookInformation.virtualAddresses.shortJumpAddress.shortJumpAddress);
    _hookedList.Add(newHookInformation);
    return 0;
 }
//----------------------------------------------------------------------------------------------------------
typedef int (WINAPI* TB_MessageBoxA)(HWND, LPCSTR, LPCSTR, UINT);
TB_MessageBoxA TBMessageBoxA = NULL;

int WINAPI HK_MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType){
    cout << "gelen mesaj:" << lpText <<endl;
    TBMessageBoxA(NULL, "memet", "asdas", MB_OK);
    return 0;
}

typedef int (WINAPI* TB_MessageBoxW)(HWND, LPCWSTR, LPCWSTR, UINT);
TB_MessageBoxW TBMessageBoxW = NULL;

int WINAPI HK_MessageBoxW(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
    cout << "gelen mesaj:" << lpText << endl;
    TBMessageBoxW(NULL, L"memet WW", L"asdas", MB_OK);
    return 0;
}
//----------------------------------------------------------------------------------------------------------
int main() {
    MK_HookCreate(&MessageBoxA,&HK_MessageBoxA ,  reinterpret_cast<LPVOID*>(&TBMessageBoxA));
    MK_HookCreate(&MessageBoxW, &HK_MessageBoxW, reinterpret_cast<LPVOID*>(&TBMessageBoxW));
    MessageBoxA(NULL, "berk", "asdas", MB_OK);
    MessageBoxW(NULL, L"berk", L"asdas", MB_OK);
    return 0;
}
//----------------------------------------------------------------------------------------------------------