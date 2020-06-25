// installer.cpp : Defines the entry point for the application.
//
#include <Windows.h>
#include "framework.h"
#include "installer.h"
#include "resource.h"
#include <assert.h>
#include "zip.h"
#include <direct.h>
#include "WindowsDesktop.h"

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



#include <windows.h>
#include <stdio.h>
#include "resource.h"


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


struct AboutDlg
{
    HWND m_hDlg;
    HWND m_hParent;
};


void AboutDlg_OnCommand(struct AboutDlg* p, int cmd, int lparam, HWND h)
{
    if (cmd == IDCANCEL)
    {
        EndDialog(p->m_hDlg, 1);
        PostQuitMessage(0);
    }
    else if (cmd == IDC_INSTALL)
    {
        SaveFile();
            MessageBoxA(p->m_hDlg, "Instalação concluida", "Install", MB_ICONINFORMATION | MB_OK);
    }
}

BEGIN_DLG_PROC(AboutDlg)
ON_COMMAND(AboutDlg)
END_DLG_PROC



//dentro do recurso deste programa ja se encontra sempre o mesmo
//zip com tudo dentro.
//este zip eh decompactado em uma pasta temporia depois
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR    lpCmdLine,
                      _In_ int       nCmdShow)
{

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    s_hInstance = hInstance;

    struct AboutDlg dlg;

    INT_PTR r = ShowDialog(IDD_INSTALLER_DIALOG,
                           &dlg,
                           NULL,
                           AboutDlg_ProcEx);

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
