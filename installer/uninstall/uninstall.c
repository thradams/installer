// uninstall.cpp : Defines the entry point for the application.
//

#include <stdio.h>
#include "uninstall.h"
#include "..\installer\config.h"
#include <direct.h>
#include <Windows.h>
#include <assert.h>

#include "uninstall_.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR    lpCmdLine,
                      _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);



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
    _rmdir("./Castle");
#define PRODUCT_UNINST_KEY L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" PRODUCT_CODE
    RegDelnode(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY);
    //MSG msg;

    // Main message loop:
    //while (GetMessage(&msg, NULL, 0, 0))
    //{

      //      DispatchMessage(&msg);
//        
  //  }

    MessageBox(NULL, DISPLAY_NAME L" unistalled", L"Unistall", MB_ICONINFORMATION |MB_OK);
    return 0;// (int)msg.wParam;
}

