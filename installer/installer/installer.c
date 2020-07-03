#include "config.h"
#include  "installer.h"
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "zip.h"
#include "resource.h"
#include "reg.h"

/*HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{A9E770C4-FCF1-4E52-A3B4-44D394886A3A}
The product code is a GUID that is the principal identification of an application or product. For more information, see the ProductCode property. If significant changes are made to a product then the product code should also be changed to reflect this. It is not however a requirement that the product code be changed if the changes to the product are relatively minor.
https://docs.microsoft.com/en-us/windows/win32/msi/product-codes
HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{A9E770C4-FCF1-4E52-A3B4-44D394886A3A}
*/

//#define PRODUCT_UNINST_KEY L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" PRODUCT_CODE
#define PRODUCT_UNINST_KEY L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" PRODUCT_CODE



wchar_t INSTDIR[MAX_PATH];

int mkdir_p(const char* path);

int on_extract_entry(const char* filename, void* arg) {
    static int i = 0;
    int n = *(int*)arg;
    printf("Extracted: %s (%d of %d)\n", filename, ++i, n);

    return 0;
}

void ExtractAllFilesToDestination(DWORD idd, const wchar_t*  pDestination)
{
    char destination[MAX_PATH];
    size_t r = wcstombs(destination, pDestination, MAX_PATH);

    HMODULE handle = GetModuleHandle(NULL);
    HRSRC rc = FindResource(handle, MAKEINTRESOURCE(idd),
                            MAKEINTRESOURCE(256));
    HGLOBAL rcData = LoadResource(handle, rc);

    DWORD size = SizeofResource(handle, rc);

    const char* data = LockResource(rcData);
       

    char zipPath[MAX_PATH] = { 0 };
    strcat(zipPath, destination);
    strcat(zipPath, "\\zip.zip");

    mkdir_p(destination);

    FILE* out = fopen(zipPath, "wb");
    if (out)
    {
        fwrite(data, sizeof(char), size, out);
        fclose(out);
        //_mkdir("tmp");

        int arg = 2;
        zip_extract(zipPath, destination, on_extract_entry, &arg);

        DeleteFileA(zipPath);
        //OK TODOS ARQUIVOS EXTRAIDEOS

        extern void OnFilesExtracted();

        OnFilesExtracted();
    }
    UnlockResource(rcData);


}


//#include "stdafx.h"
#include "windows.h"
#include "winnls.h"
#include "shobjidl.h"
#include "objbase.h"
#include "objidl.h"
#include "shlguid.h"

HRESULT CreateShortCut(LPCWSTR lpszPathObj, LPCSTR lpszPathLink, LPCWSTR lpszDesc)
{
    HRESULT hres;
    IShellLink* psl;

    // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
    // has already been called.
    hres = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLink, (LPVOID*)&psl);
    if (SUCCEEDED(hres))
    {
        IPersistFile* ppf;

        // Set the path to the shortcut target and add the description. 
        psl->lpVtbl->SetPath(psl, lpszPathObj);
        psl->lpVtbl->SetDescription(psl, lpszDesc);

        // Query IShellLink for the IPersistFile interface, used for saving the 
        // shortcut in persistent storage. 
        hres = psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, (LPVOID*)&ppf);

        if (SUCCEEDED(hres))
        {
            WCHAR wsz[MAX_PATH];

            // Ensure that the string is Unicode. 
            MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1, wsz, MAX_PATH);

            // Save the link by calling IPersistFile::Save. 
            hres = ppf->lpVtbl->Save(ppf,wsz, TRUE);
            ppf->lpVtbl->Release(ppf);
        }
        psl->lpVtbl->Release(psl);
    }
    return hres;
}


void rmtree(const char path[])
{
#if 0
    char* full_path;
    DIR* dir;
    struct stat stat_path, stat_entry;
    struct dirent* entry;

    // stat for the path
    stat(path, &stat_path);

    // if path does not exists or is not dir - exit with status -1
    if (S_ISDIR(stat_path.st_mode) == 0) {
        fprintf(stderr, "%s: %s\n", "Is not directory", path);
        exit(-1);
    }

    // if not possible to read the directory for this user
    if ((dir = opendir(path)) == NULL) {
        fprintf(stderr, "%s: %s\n", "Can`t open directory", path);
        exit(-1);
    }

    // the length of the path
    path_len = strlen(path);

    // iteration through entries in the directory
    while ((entry = readdir(dir)) != NULL) {

        // skip entries "." and ".."
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        // determinate a full path of an entry
        full_path = calloc(path_len + strlen(entry->d_name) + 1, sizeof(char));
        strcpy(full_path, path);
        strcat(full_path, "/");
        strcat(full_path, entry->d_name);

        // stat for the entry
        stat(full_path, &stat_entry);

        // recursively remove a nested directory
        if (S_ISDIR(stat_entry.st_mode) != 0) {
            rmtree(full_path);
            continue;
        }

        // remove a file object
        if (unlink(full_path) == 0)
            printf("Removed a file: %s\n", full_path);
        else
            printf("Can`t remove a file: %s\n", full_path);
        free(full_path);
    }

    // remove the devastated directory and close the object of it
    if (rmdir(path) == 0)
        printf("Removed a directory: %s\n", path);
    else
        printf("Can`t remove a directory: %s\n", path);

    closedir(dir);
#endif
}

int mkdir_p(const char* path)
{
    /* Adapted from http://stackoverflow.com/a/2336245/119527 */
    const size_t len = strlen(path);
    char _path[MAX_PATH];
    char* p;

    errno = 0;

    /* Copy string so its mutable */
    if (len > sizeof(_path) - 1) {
        errno = ENAMETOOLONG;
        return -1;
    }
    lstrcpyA(_path, path);

    /* Iterate the string */
    for (p = _path + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            /* Temporarily truncate */
            *p = '\0';

            if (mkdir(_path) != 0) {
                if (errno != EEXIST)
                    return -1;
            }

            *p = '/';
        }
    }

    if (mkdir(_path) != 0) {
        if (errno != EEXIST)
            return -1;
    }

    return 0;
}







struct AboutDlg
{
    HWND m_hDlg;
    HWND m_hParent;
};

void AboutDlg_OnInit(struct AboutDlg* p)
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
    ShowWindow(GetDlgItem(p->m_hDlg, IDC_PROGRESS1), SW_HIDE);
}

void WriteRegCommon()
{
    wchar_t uninst[MAX_PATH] = { 0 };
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
        
        GetDlgItemText(p->m_hDlg, IDC_DESTINATION, INSTDIR, MAX_PATH);
        
        

        ExtractAllFilesToDestination(IDR_TXT1, INSTDIR);

        WriteRegCommon();

        ShowWindow(GetDlgItem(p->m_hDlg, IDC_DESTINATION), SW_HIDE);
        ShowWindow(GetDlgItem(p->m_hDlg, IDC_PROGRESS1), SW_SHOW);


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


BOOL Start(HINSTANCE hInstance)
{
    s_hInstance = hInstance;
    struct AboutDlg dlg;

    INT_PTR r = ShowDialog(IDD_INSTALLER_DIALOG,
                           &dlg,
                           NULL,
                           AboutDlg_ProcEx);
    return r;
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR    lpCmdLine,
                      _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    Start(hInstance);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}


