// uninstall.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "uninstall.h"



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);


  
    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
       
            DispatchMessage(&msg);
        
    }

    return (int) msg.wParam;
}

