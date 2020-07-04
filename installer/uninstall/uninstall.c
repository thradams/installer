// uninstall.cpp : Defines the entry point for the application.
//

#include <stdio.h>
#include "uninstall.h"
#include "..\installer\config.h"
#include <direct.h>
#include <Windows.h>
#include <assert.h>

#include "uninstall_.h"

#define PRODUCT_UNINST_KEY L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" PRODUCT_CODE

void Finish()
{
    WCHAR value[MAX_PATH] = { 0 };
    ULONG nChars = MAX_PATH;
    if (GetModuleFileNameW(NULL, value, MAX_PATH) > 0)
    {
        //return 0;
    }

    WCHAR tempPath[MAX_PATH] = { 0 };
    DWORD dw = GetTempPathW(MAX_PATH, tempPath);
    wcscat(tempPath, L"uninstall.exe");
    CopyFile(value, tempPath, FALSE);
    SystemCreateProcess(tempPath, value);

    
    

}
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR    lpCmdLine,
                      _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    
    
    

    //quando ele passa -u quer dizer que este quer fazer a desinstaladcao
    BOOL bRemoveUninstall = wcslen(GetCommandLineW()) > 2;
    if (bRemoveUninstall)
    {
        //MessageBoxW(NULL, GetCommandLineW(), L"command line", MB_ICONINFORMATION);
        for (int i = 0; i < 100; i++)
        {
            if (remove("unistall.exe") == 0)
            {
                _rmdir("./Castle");
                break;
            }
            else
                Sleep(500);
        }
        return;
    }



    WCHAR value[MAX_PATH] = { 0 };
    ULONG nChars = MAX_PATH;

    if (GetModuleFileNameW(NULL, value, MAX_PATH) > 0)
    {
        //return 0;
    }

    //BOOL b = ReadRegStr(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY, L"InstallSource", value, &nChars);
    //if (b)
    //{
        //este eh o lugar aonde esta instalado
    //}

    //colocar neste diretorio
    _wchdir(value);
    //wcscat(value, L"/uninstall.exe");


    char cwd[MAX_PATH];
    if (_getcwd(cwd, sizeof(cwd)) != NULL) {

        MessageBoxA(NULL, cwd, "", MB_ICONINFORMATION);
    }
    else {

        return 1;
    }

    struct finfo {
        const char* from;
        const char* dest;
    } files[] = { FILES };




    for (int i = 0; i < sizeof(files) / sizeof(files[0]); i++)
    {
        if (remove(files[i].dest) != 0)
        {
            int e = errno;
        }
    }


    _rmdir("default/.vscode");
    _rmdir("default");
    RegDelnode(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY);

   

    //MSG msg;

    // Main message loop:
    //while (GetMessage(&msg, NULL, 0, 0))
    //{

      //      DispatchMessage(&msg);
//        
  //  }

    MessageBox(NULL, DISPLAY_NAME L" was successfully removed from  your computer.", DISPLAY_NAME L" Unistall", MB_ICONINFORMATION |MB_OK);
    
    Finish();

    return 0;// (int)msg.wParam;
}

