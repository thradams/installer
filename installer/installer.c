// installer.cpp : Defines the entry point for the application.
//
#include <Windows.h>
#include "framework.h"
#include "installer.h"
#include "resource.h"
#include <assert.h>
#include <Shldisp.h>

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
#include "zlib.h"

const char* LoadFileInResource(int name, int type, DWORD* size)
{
    HMODULE handle = GetModuleHandle(NULL);
    HRSRC rc = FindResource(handle, MAKEINTRESOURCE(name),
                            MAKEINTRESOURCE(type));
    HGLOBAL rcData = LoadResource(handle, rc);
    *size = SizeofResource(handle, rc);
    return  (const char*)(LockResource(rcData));
}


HRESULT Unzip2Folder(BSTR lpZipFile, BSTR lpFolder)
{
    IShellDispatch* pISD;

    Folder* pZippedFile = 0L;
    Folder* pDestination = 0L;

    long FilesCount = 0;
    IDispatch* pItem = 0L;
    FolderItems* pFilesInside = 0L;

    VARIANT Options, OutFolder, InZipFile, Item;
    CoInitialize(NULL);

    if (CoCreateInstance(&CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, &IID_IShellDispatch, (void**)&pISD) != S_OK)
        return E_FAIL;

    InZipFile.vt = VT_BSTR;
    InZipFile.bstrVal = lpZipFile;
    pISD->lpVtbl->NameSpace(pISD, InZipFile, &pZippedFile);
    if (!pZippedFile)
    {
        pISD->lpVtbl->Release(pISD);
        return E_FAIL;
    }

    OutFolder.vt = VT_BSTR;
    OutFolder.bstrVal = lpFolder;
    pISD->lpVtbl->NameSpace(pISD, OutFolder, &pDestination);
    if (!pDestination)
    {
        pZippedFile->lpVtbl->Release(pZippedFile);
        pISD->lpVtbl->Release(pISD);
        return E_FAIL;
    }

    pZippedFile->lpVtbl->Items(pZippedFile, &pFilesInside);
    if (!pFilesInside)
    {
        pDestination->lpVtbl->Release(pDestination);
        pZippedFile->lpVtbl->Release(pZippedFile);
        pISD->lpVtbl->Release(pISD);
        return 1;
    }

    pFilesInside->lpVtbl->get_Count(pFilesInside, &FilesCount);
    if (FilesCount < 1)
    {
        pFilesInside->lpVtbl->Release(pFilesInside);
        pDestination->lpVtbl->Release(pDestination);
        pZippedFile->lpVtbl->Release(pZippedFile);
        pISD->lpVtbl->Release(pISD);
        return 0;
    }

    pFilesInside->lpVtbl->QueryInterface(pFilesInside, &IID_IDispatch, (void**)&pItem);

    Item.vt = VT_DISPATCH;
    Item.pdispVal = pItem;

    Options.vt = VT_I4;
    Options.lVal = 1024 | 512 | 16 | 4;//http://msdn.microsoft.com/en-us/library/bb787866(VS.85).aspx

    HRESULT retval = pDestination->lpVtbl->CopyHere(pDestination, Item, Options) == S_OK;

    pItem->lpVtbl->Release(pItem); pItem = 0L;
    pFilesInside->lpVtbl->Release(pFilesInside); pFilesInside = 0L;
    pDestination->lpVtbl->Release(pDestination); pDestination = 0L;
    pZippedFile->lpVtbl->Release(pZippedFile); pZippedFile = 0L;
    pISD->lpVtbl->Release(pISD); pISD = 0L;


    CoUninitialize();

    return retval;
}

void UnZipFile(const char* strSrc, const char* strDest)
{
    BSTR source = SysAllocString(L"installer.zip");
    BSTR dest = SysAllocString(L"folder");

    HRESULT hResult = S_FALSE;
    IShellDispatch* pIShellDispatch = NULL;
    Folder* pToFolder = NULL;
    VARIANT variantDir, variantFile, variantOpt;

    CoInitialize(NULL);

    hResult = CoCreateInstance(&CLSID_Shell, NULL, CLSCTX_INPROC_SERVER,
                               &IID_IShellDispatch, (void**)&pIShellDispatch);
    if (SUCCEEDED(hResult))
    {
        VariantInit(&variantDir);
        variantDir.vt = VT_BSTR;
        variantDir.bstrVal = dest;
        hResult = pIShellDispatch->lpVtbl->NameSpace(pIShellDispatch, variantDir, &pToFolder);

        if (SUCCEEDED(hResult))
        {
            Folder* pFromFolder = NULL;
            VariantInit(&variantFile);
            variantFile.vt = VT_BSTR;
            variantFile.bstrVal = source;
            hResult = pIShellDispatch->lpVtbl->NameSpace(pIShellDispatch, variantFile, &pFromFolder);

            FolderItems* fi = NULL;
            pFromFolder->lpVtbl->Items(pFromFolder, &fi);

            VariantInit(&variantOpt);
            variantOpt.vt = VT_I4;
            variantOpt.lVal = FOF_NO_UI;

            VARIANT newV;
            VariantInit(&newV);
            newV.vt = VT_DISPATCH;
            newV.pdispVal = fi;
            hResult = pToFolder->lpVtbl->CopyHere(pToFolder, newV, variantOpt);
            Sleep(1000);
            pFromFolder->lpVtbl->Release(pFromFolder);
            pToFolder->lpVtbl->Release(pToFolder);
        }
        pIShellDispatch->lpVtbl->Release(pIShellDispatch);
    }

    CoUninitialize();
}

void SaveFile()
{


    HMODULE handle = GetModuleHandle(NULL);
    HRSRC rc = FindResource(handle, MAKEINTRESOURCE(IDR_TXT1),
                            MAKEINTRESOURCE(256));
    HGLOBAL rcData = LoadResource(handle, rc);

    DWORD size = SizeofResource(handle, rc);

    const char* data = LockResource(rcData);


    FILE* out = fopen("c:\\installer.zip", "wb");
    if (out)
    {
        fwrite(data, sizeof(char), size, out);
        fclose(out);

        Unzip2Folder(L"c:\\installer.zip", L"c:\\folder");
    }
    UnlockResource(rcData);


}


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
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                PostQuitMessage(0);
                SaveFile();
                return (INT_PTR)TRUE;
            }
            break;
    }
    return (INT_PTR)FALSE;
}
