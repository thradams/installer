// installer.cpp : Defines the entry point for the application.
//
#include <Windows.h>
#include "framework.h"
#include "installer.h"
#include "resource.h"
#include <assert.h>
#include "zip.h"
#include <direct.h>

#define MAX_LOADSTRING 100
#pragma comment(lib, "Comctl32.lib")
#define ASSERT assert 

#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

DWORD GetStyle(HWND hWnd)
{
    ASSERT(IsWindow(hWnd));
    return (DWORD)GetWindowLong(hWnd, GWL_STYLE);
}

BOOL CenterWindow(HWND hWnd, HWND hWndCenter)
{
    ASSERT(IsWindow(hWnd));

    // determine owner window to center against
    DWORD dwStyle = GetStyle(hWnd);

    if (hWndCenter == NULL)
    {
        if (dwStyle & WS_CHILD)
        {
            hWndCenter = GetParent(hWnd);
        }
        else
        {
            hWndCenter = GetWindow(hWnd, GW_OWNER);
        }
    }

    // get coordinates of the window relative to its parent
    RECT rcDlg;
    GetWindowRect(hWnd, &rcDlg);
    RECT rcArea;
    RECT rcCenter;
    HWND hWndParent;

    if (!(dwStyle & WS_CHILD))
    {
        // don't center against invisible or minimized windows
        if (hWndCenter != NULL)
        {
            DWORD dwStyleCenter = GetWindowLong(hWndCenter, GWL_STYLE);

            if (!(dwStyleCenter & WS_VISIBLE) || (dwStyleCenter & WS_MINIMIZE))
            {
                hWndCenter = NULL;
            }
        }

        // center within screen coordinates
#if WINVER < 0x0500
        SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcArea, NULL);
#else
        HMONITOR hMonitor = NULL;

        if (hWndCenter != NULL)
        {
            hMonitor = MonitorFromWindow(hWndCenter,
                                         MONITOR_DEFAULTTONEAREST);
        }
        else
        {
            hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
        }

        //ATLENSURE_RETURN_VAL(hMonitor != NULL, FALSE);

        MONITORINFO minfo;
        minfo.cbSize = sizeof(MONITORINFO);
        /*BOOL bResult = */
        GetMonitorInfo(hMonitor, &minfo);
        //ATLENSURE_RETURN_VAL(bResult, FALSE);

        rcArea = minfo.rcWork;
#endif

        if (hWndCenter == NULL)
        {
            rcCenter = rcArea;
        }
        else
        {
            GetWindowRect(hWndCenter, &rcCenter);
        }
    }
    else
    {
        // center within parent client coordinates
        hWndParent = GetParent(hWnd);
        ASSERT(IsWindow(hWndParent));

        GetClientRect(hWndParent, &rcArea);
        ASSERT(IsWindow(hWndCenter));
        GetClientRect(hWndCenter, &rcCenter);
        MapWindowPoints(hWndCenter, hWndParent, (POINT*)&rcCenter, 2);
    }

    int DlgWidth = rcDlg.right - rcDlg.left;
    int DlgHeight = rcDlg.bottom - rcDlg.top;

    // find dialog's upper left based on rcCenter
    int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
    int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

    // if the dialog is outside the screen, move it inside
    if (xLeft + DlgWidth > rcArea.right)
    {
        xLeft = rcArea.right - DlgWidth;
    }

    if (xLeft < rcArea.left)
    {
        xLeft = rcArea.left;
    }

    if (yTop + DlgHeight > rcArea.bottom)
    {
        yTop = rcArea.bottom - DlgHeight;
    }

    if (yTop < rcArea.top)
    {
        yTop = rcArea.top;
    }

    // map screen coordinates to child coordinates
    return SetWindowPos(hWnd, NULL, xLeft, yTop, -1, -1,
                        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name


INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

#include <windows.h>
#include <stdio.h>
#include "resource.h"
//#include "zlib.h"

const char* LoadFileInResource(int name, int type, DWORD* size)
{
    HMODULE handle = GetModuleHandle(NULL);
    HRSRC rc = FindResource(handle, MAKEINTRESOURCE(name),
                            MAKEINTRESOURCE(type));
    HGLOBAL rcData = LoadResource(handle, rc);
    *size = SizeofResource(handle, rc);
    return  (const char*)(LockResource(rcData));
}




int on_extract_entry(const char* filename, void* arg) {
    static int i = 0;
    int n = *(int*)arg;
    printf("Extracted: %s (%d of %d)\n", filename, ++i, n);

    return 0;
}

void SaveFile()
{


    HMODULE handle = GetModuleHandle(NULL);
    HRSRC rc = FindResource(handle, MAKEINTRESOURCE(IDR_TXT1),
                            MAKEINTRESOURCE(256));
    HGLOBAL rcData = LoadResource(handle, rc);

    DWORD size = SizeofResource(handle, rc);

    const char* data = LockResource(rcData);


    FILE* out = fopen("c:\\foo.zip", "wb");
    if (out)
    {
        fwrite(data, sizeof(char), size, out);
        fclose(out);
        //_mkdir("tmp");
        int arg = 2;
        zip_extract("c:\\foo.zip", "c:\\tmp", on_extract_entry, &arg);

        //copiar tudo agora para destino.

        //apagar pasta temp
        
    }
    UnlockResource(rcData);


}

//dentro do recurso deste programa ja se encontra sempre o mesmo
//zip com tudo dentro.
//este zip eh decompactado em uma pasta temporia depois
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR    lpCmdLine,
                      _In_ int       nCmdShow)
{
    DWORD size = 0;
    const char* data = NULL;

    data = LoadFileInResource(IDR_TXT1, 256, &size);
    /* Access bytes in data - here's a simple example involving text output*/
    // The text stored in the resource might not be NULL terminated.
    char* buffer = (char*)malloc(sizeof(char) * (size + 1));
    memcpy(buffer, data, size);
    buffer[size] = 0; // NULL terminator
    printf("Contents of text file: %s\n", buffer); // Print as ASCII text
    free(buffer);


    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_INSTALLER, szWindowClass, MAX_LOADSTRING);


    DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), NULL, About);

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        case WM_INITDIALOG:
            CenterWindow(hDlg, NULL);
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDC_INSTALL)
            {
                SaveFile();
                MessageBoxA(hDlg, "Instalação compluída", "Install", MB_ICONINFORMATION | MB_OK);
            }
            else if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                PostQuitMessage(0);
                
                return (INT_PTR)TRUE;
            }
            break;
    }
    return (INT_PTR)FALSE;
}
