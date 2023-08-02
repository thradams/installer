
#include "process.h"
#include "w32.h"
#include "resource.h"
#include "miniz.h"
#include <stdio.h>
#include <stdbool.h>
#include "script.h"


#define ASSERT(x)

/*HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{A9E770C4-FCF1-4E52-A3B4-44D394886A3A}
The product code is a GUID that is the principal identification of an application or product. For more information, see the ProductCode property. If significant changes are made to a product then the product code should also be changed to reflect this. It is not however a requirement that the product code be changed if the changes to the product are relatively minor.
https://docs.microsoft.com/en-us/windows/win32/msi/product-codes
HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{A9E770C4-FCF1-4E52-A3B4-44D394886A3A}
*/

#define PRODUCT_UNINST_KEY L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" PRODUCT_CODE

wchar_t INSTDIR[MAX_PATH];
HWND s_hDlg = 0;

int on_extract_entry(const char* filename, void* arg)
{
    static int i = 0;
    int n = *(int*) arg;
    char buffer[300] = {0};
    snprintf(buffer, 300, "Extracted: %s (%d of %d)\n", filename, ++i, n);

    SetDlgItemTextA(s_hDlg, IDC_MESSAGE, buffer);

    return 0;
}

bool ExtractAllFilesToDestination(DWORD idd, const wchar_t* pDestination)
{
    bool bResult = true;

    char destination[MAX_PATH];
    size_t r = wcstombs(destination, pDestination, MAX_PATH);

    HMODULE handle = GetModuleHandle(NULL);
    HRSRC rc = FindResource(handle, MAKEINTRESOURCE(idd), MAKEINTRESOURCE(256));
    if (rc != NULL)
    {
        HGLOBAL rcData = LoadResource(handle, rc);
        if (rcData != NULL)
        {
            DWORD size = SizeofResource(handle, rc);
            if (size != 0)
            {
                const char* data = LockResource(rcData);
                if (data)
                {
                    char zipPath[MAX_PATH] = {0};
                    strcat(zipPath, destination);
                    strcat(zipPath, "\\zip.zip");

                    mkdir_p(destination);

                    FILE* out = fopen(zipPath, "wb");
                    if (out)
                    {
                        fwrite(data, sizeof(char), size, out);
                        fclose(out);

                        int arg = 2;
                        int code = zip_extract(zipPath, destination, on_extract_entry, &arg);
                        if (code == 0)
                        {
                            //ok arquivo extraido com sucesso
                            bResult = true;
                        }
                        else
                        {
                            //erro ao extrair arquivo zip 
                            bResult = false;
                        }

                        //Remover o arquivo zip com sucesso ou falhar
                        DeleteFileA(zipPath);

                        if (bResult)
                        {
                            //Em caso de sucesso chamar funcao usuario
                            extern void OnFilesExtracted();
                            OnFilesExtracted();
                        }
                        else
                        {
                            //customizar erro?
                        }
                    }
                    else
                    {
                        //Falha ao abrir arquivo de escrita do zip
                        bResult = false;
                    }
                    UnlockResource(rcData);
                }
                else
                {
                    //Erro interno do instalador
                    bResult = false;
                }
            }
            else
            {
                //Erro interno do instalador
                bResult = false;
            }
        }
        else
        {
            //Erro interno do instalador
            bResult = false;
        }
    }
    else
    {
        //Erro interno do instalador
    }

    return bResult;
}


struct LicenseDlg
{
    HWND m_hDlg;
    HWND m_hParent;
};

void LicenseDlg_OnInit(struct LicenseDlg* p)
{
    HMODULE handle = GetModuleHandle(NULL);
    HRSRC rc = FindResource(handle, MAKEINTRESOURCE(IDR_RTF1),
        MAKEINTRESOURCE(257));
    HGLOBAL rcData = LoadResource(handle, rc);
    DWORD size = SizeofResource(handle, rc);
    const TCHAR* data = LockResource(rcData);
    SetTextEx(GetDlgItem(p->m_hDlg, IDC_RICHEDIT22), data, ST_DEFAULT, CP_ACP);
    UnlockResource(rcData);
}

void LicenseDlg_OnCommand(struct LicenseDlg* p, int cmd, int lparam, HWND h)
{
    if (cmd == IDCANCEL || cmd == IDOK)
    {
        EndDialog(p->m_hDlg, 1);
    }
}


LRESULT __stdcall LicenseDlg_ProcEx(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    BOOL bHandled = 0;
    struct LicenseDlg* pThis = 0;
    if (message == WM_INITDIALOG)
    {
        struct LicenseDlg* p = (struct LicenseDlg*) lParam; SetWindowLongW(hWnd, (-21), (LONG_PTR) p); p->m_hDlg = hWnd; CenterWindow(hWnd, p->m_hParent); LicenseDlg_OnInit(p); return (INT_PTR) 0;
    }
    else
    {
        pThis = (struct LicenseDlg*) GetWindowLongW(hWnd, (-21));
    }
    if (!pThis)
    {
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    switch (message)
    {
        case WM_COMMAND:
            LicenseDlg_OnCommand(pThis, ((WORD) (((DWORD_PTR) (wParam)) & 0xffff)), ((WORD) ((((DWORD_PTR) (wParam)) >> 16) & 0xffff)), (HWND) lParam);
            break;

    } return bHandled;
}

struct AboutDlg
{
    HWND m_hDlg;
    HWND m_hParent;
};

static void ShowPage(struct AboutDlg* p, int index)
{
    s_hDlg = p->m_hDlg;
    ShowWindow(GetDlgItem(p->m_hDlg, IDC_DEST_FRAME), index == 0 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(p->m_hDlg, IDC_FOLDER_BUTTON), index == 0 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(p->m_hDlg, IDC_DESTINATION), index == 0 ? SW_SHOW : SW_HIDE);

    ShowWindow(GetDlgItem(p->m_hDlg, IDC_AGREE_CHECK), index == 0 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(p->m_hDlg, IDC_LICENSE_LINK), index == 0 ? SW_SHOW : SW_HIDE);

    ShowWindow(GetDlgItem(p->m_hDlg, IDC_PROGRESS1), index == 1 ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(p->m_hDlg, IDC_MESSAGE), index != 0 ? SW_SHOW : SW_HIDE);

    //IDC_INSTALL_BUTTON
    if (index == 2)
    {
        ShowWindow(GetDlgItem(p->m_hDlg, IDC_INSTALL_BUTTON), SW_HIDE);
        SetWindowText(GetDlgItem(p->m_hDlg, IDCANCEL), L"Close");
    }
}

static void MainDlg_OnInit(struct AboutDlg* p)
{
    Button_SetElevationRequiredState(GetDlgItem(p->m_hDlg, IDC_INSTALL), TRUE);
    TCHAR pf[MAX_PATH];
    SHGetSpecialFolderPath(
        0,
        pf,
        CSIDL_PROGRAM_FILESX86,
        FALSE);

    wcscat(pf, L"\\" PRODUCT_PUBLISHER L"\\" PRODUCT_NAME);

    SetDlgItemText(p->m_hDlg, IDC_DESTINATION, pf);
    SetWindowText(p->m_hDlg, DISPLAY_NAME L" Installer");
    SetDlgItemText(p->m_hDlg, IDC_PRODUCT_NAME, DISPLAY_NAME);


    //precisa dizer que concorda
    EnableWindow(GetDlgItem(p->m_hDlg, IDC_INSTALL_BUTTON), FALSE);

    ShowPage(p, 0);
}

static bool WriteRegCommon()
{
    bool bResult = true;
    wchar_t uninst[MAX_PATH] = {0};
    wcscat(uninst, INSTDIR);
    wcscat(uninst, L"\\uninstall.exe");
    //criar GUID?

    //https://docs.microsoft.com/en-us/windows/win32/msi/uninstall-registry-key
    WriteRegStr(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY, L"DisplayName", PRODUCT_NAME);
    WriteRegStr(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY, L"UninstallString", uninst);
    WriteRegStr(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY, L"DisplayIcon", uninst);
    WriteRegStr(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY, L"DisplayVersion", PRODUCT_VERSION);
    WriteRegStr(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY, L"URLInfoAbout", PRODUCT_WEB_SITE);
    WriteRegStr(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY, L"Publisher", PRODUCT_PUBLISHER);
    WriteRegStr(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY, L"InstallLocation", INSTDIR);
    WriteRegStr(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY, L"InstallSource", INSTDIR);
    WriteRegStr(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY, L"InstallSourceFile", INSTDIR);
    WriteRegStr(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY, L"NoModify", L"1");
    WriteRegStr(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY, L"NoRepair", L"1");

    return bResult;
}

static void MainDlg_OnCommand(struct AboutDlg* p, int cmd, int lparam, HWND h)
{
    if (cmd == IDCANCEL)
    {
        EndDialog(p->m_hDlg, 1);
        PostQuitMessage(0);
    }

    else if (cmd == IDC_AGREE_CHECK)
    {
        int bChecked = Button_GetCheck(GetDlgItem(p->m_hDlg, IDC_AGREE_CHECK));
        EnableWindow(GetDlgItem(p->m_hDlg, IDC_INSTALL_BUTTON), bChecked);
    }
    else if (cmd == IDC_INSTALL_BUTTON)
    {
        GetDlgItemText(p->m_hDlg, IDC_DESTINATION, INSTDIR, MAX_PATH);

        ShowPage(p, 1);

        if (ExtractAllFilesToDestination(IDR_TXT1, INSTDIR))
        {
            if (WriteRegCommon())
            {
                ShowWindow(GetDlgItem(p->m_hDlg, IDC_DESTINATION), SW_HIDE);
                ShowWindow(GetDlgItem(p->m_hDlg, IDC_PROGRESS1), SW_SHOW);

                ShowPage(p, 2);
                SetDlgItemTextA(s_hDlg, IDC_MESSAGE, "Instalação concluída com sucesso.");
            }
            else
            {
                MessageBoxA(p->m_hDlg, "Falha ao escrever registro do windows", "Install", MB_ICONERROR | MB_OK);
            }
        }
        else
        {
            //falha ao extrair arquivos
            MessageBoxA(p->m_hDlg, "Falha ao expandir arquivos de instalação", "Install", MB_ICONERROR | MB_OK);
        }
    }
    else if (cmd == IDC_FOLDER)
    {
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

static BOOL MainDlg_OnNotify(struct AboutDlg* p, DWORD cmd, NMHDR* pNMHDR)
{
    if (cmd == IDC_LICENSE_LINK)
    {
        if (pNMHDR->code == NM_CLICK)
        {
            struct LicenseDlg dlg = {0};
            HINSTANCE hInstance = GetModuleHandle(NULL);
            INT_PTR r = ShowDialog(hInstance,
                IDD_LICENSE,
                &dlg,
                p->m_hDlg,
                LicenseDlg_ProcEx);
            return TRUE; // handled, don't pass to DefWindowProc
        }

    }
    return FALSE;
}

LRESULT __stdcall MainDlg_ProcEx(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    BOOL bHandled = 0;
    struct AboutDlg* pThis = 0;

    if (message == 0x0110)
    {
        struct AboutDlg* p = (struct AboutDlg*) lParam;
        SetWindowLongW(hWnd, (-21), (LONG_PTR) p);
        p->m_hDlg = hWnd;
        CenterWindow(hWnd, p->m_hParent); MainDlg_OnInit(p);
        return (INT_PTR) 0;
    }
    else
    {
        pThis = (struct AboutDlg*) GetWindowLongW(hWnd, (-21));
    }
    if (!pThis)
    {
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    switch (message)
    {
        case WM_COMMAND:
            MainDlg_OnCommand(pThis, ((WORD) (((DWORD_PTR) (wParam)) & 0xffff)), ((WORD) ((((DWORD_PTR) (wParam)) >> 16) & 0xffff)), (HWND) lParam);
            break;

        case WM_NOTIFY:
        {
            LPNMHDR p = (LPNMHDR) lParam;
            if (p) bHandled = MainDlg_OnNotify(pThis, wParam, p);
        }
        break;

    }
    return bHandled;
}


static BOOL InstallerShowDialog(HINSTANCE hInstance)
{
    struct AboutDlg dlg;

    INT_PTR r = ShowDialog(hInstance,
        IDD_INSTALLER_DIALOG,
        &dlg,
        NULL,
        MainDlg_ProcEx);
    return r;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    //this is necessary to use richedit controls
    LoadLibrary(TEXT("Riched20.dll"));

    InstallerShowDialog(hInstance);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}



