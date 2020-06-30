// installer.cpp : Defines the entry point for the application.
//
#include <Windows.h>

#include "installer.h"
#include "resource.h"
#include <assert.h>
#include "zip.h"


#include "installer.h"

#define MAX_LOADSTRING 100

#define ASSERT assert 


#include <stdio.h>




struct AboutDlg
{
    HWND m_hDlg;
    HWND m_hParent;
};

void AboutDlg_OnInit(struct AboutDlg* p)
{
    Button_SetElevationRequiredState(GetDlgItem(p->m_hDlg, IDC_INSTALL), TRUE);
}

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
    else if (cmd == IDC_FOLDER) {

        wchar_t fileOut[MAX_PATH];

        TCHAR pf[MAX_PATH];
        SHGetSpecialFolderPath(
            0,
            pf,
            CSIDL_PROGRAM_FILESX86,
            FALSE);

        BOOL b = ShowSelectFolderDialog(p->m_hDlg, L"Select folder", pf, fileOut);
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

