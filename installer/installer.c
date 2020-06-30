#include  "installer.h"
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "zip.h"
#include "resource.h"

int on_extract_entry(const char* filename, void* arg) {
    static int i = 0;
    int n = *(int*)arg;
    printf("Extracted: %s (%d of %d)\n", filename, ++i, n);

    return 0;
}

void SaveFile(DWORD idd)
{

    HMODULE handle = GetModuleHandle(NULL);
    HRSRC rc = FindResource(handle, MAKEINTRESOURCE(idd),
                            MAKEINTRESOURCE(256));
    HGLOBAL rcData = LoadResource(handle, rc);

    DWORD size = SizeofResource(handle, rc);

    const char* data = LockResource(rcData);


    mkdir_p("C:\\Program Files (x86)\\thiagotest\\programa1");

    FILE* out = fopen("C:\\Program Files (x86)\\thiagotest\\programa1\\setup.zip", "wb");
    if (out)
    {
        fwrite(data, sizeof(char), size, out);
        fclose(out);
        //_mkdir("tmp");
        int arg = 2;
        zip_extract("C:\\Program Files (x86)\\thiagotest\\programa1\\setup.zip", "C:\\Program Files (x86)\\thiagotest\\programa1\\", on_extract_entry, &arg);

        DeleteFileA("C:\\Program Files (x86)\\thiagotest\\programa1\\setup.zip");
        //CopyFile()
        //copiar tudo agora para destino.

        //apagar pasta temp

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
    strcpy(_path, path);

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


HKEY  OpenRegKey(HKEY hKeyParent,
                 LPCTSTR lpszKeyName,
                 REGSAM samDesired/* = KEY_READ | KEY_WRITE*/)
{
    assert(hKeyParent != NULL);
    HKEY hKey = NULL;
    LONG lRes = RegOpenKeyEx(hKeyParent, lpszKeyName, 0, samDesired, &hKey);

    if (lRes == ERROR_SUCCESS)
    {
        assert(lRes == ERROR_SUCCESS);
    }

    return hKey;
}

inline LONG RegKey_QueryStringValue(HKEY hKey,
                                    LPCTSTR pszValueName,
                                    LPTSTR pszValue,
                                    ULONG* pnChars)
{
    LONG lRes;
    DWORD dwType;
    ULONG nBytes;

    assert(hKey != NULL);
    assert(pnChars != NULL);

    nBytes = (*pnChars) * sizeof(TCHAR);
    *pnChars = 0;
    lRes = RegQueryValueEx(hKey, pszValueName, NULL, &dwType, (LPBYTE)(pszValue),
                           &nBytes);

    if (lRes != ERROR_SUCCESS)
    {
        return lRes;
    }

    if (dwType != REG_SZ && dwType != REG_EXPAND_SZ)
    {
        return ERROR_INVALID_DATA;
    }

    if (pszValue != NULL)
    {
        if (nBytes != 0)
        {

            if ((nBytes % sizeof(TCHAR) != 0) || (pszValue[nBytes / sizeof(TCHAR) - 1] != 0))
            {
                return ERROR_INVALID_DATA;
            }

        }
        else
        {
            pszValue[0] = L'\0';
        }
    }

    *pnChars = nBytes / sizeof(TCHAR);

    return ERROR_SUCCESS;
}


LONG RegKey_QueryDWORDValue(HKEY hKey, LPCTSTR pszValueName, DWORD* dwValue)
{
    LONG lRes;
    ULONG nBytes;
    DWORD dwType;

    assert(hKey != NULL);

    nBytes = sizeof(DWORD);
    lRes = RegQueryValueEx(hKey, pszValueName, NULL, &dwType, (LPBYTE)(&dwValue),
                           &nBytes);

    if (lRes != ERROR_SUCCESS)
        return lRes;

    if (dwType != REG_DWORD)
        return ERROR_INVALID_DATA;

    return ERROR_SUCCESS;
}

LONG RegKey_SetStringValue(
    HKEY hKey,
    LPCTSTR pszValueName,
    LPCTSTR pszValue,
    DWORD dwType)
{
    assert(hKey != NULL);
    assert((dwType == REG_SZ) || (dwType == REG_EXPAND_SZ));

    if (pszValue == NULL)
    {
        return ERROR_INVALID_DATA;
    }

    return RegSetValueEx(hKey,
                         pszValueName,
                         0,
                         dwType,
                         (const BYTE*)(pszValue),
                         ((DWORD)(wcslen(pszValue)) + 1) * sizeof(TCHAR));
}

BOOL DeleteRegValue(HKEY hKeyParent,
                    LPCTSTR pszSubkey,
                    LPCTSTR pszValueName)
{
    BOOL bResult = FALSE;
    HKEY hKey = OpenRegKey(hKeyParent, pszSubkey, KEY_READ | KEY_WRITE);

    if (hKey)
    {
        LSTATUS e = RegDeleteKeyValue(hKey, pszSubkey, pszValueName);
        RegCloseKey(hKey);
    }
    return bResult;
}


BOOL DeleteRegKey(HKEY hKeyParent,
                  LPCTSTR pszSubkey)
{
    BOOL bResult = FALSE;
    HKEY hKey = OpenRegKey(hKeyParent, pszSubkey, KEY_READ | KEY_WRITE);

    if (hKey)
    {
        LSTATUS e = RegDeleteKey(hKey, pszSubkey);
        RegCloseKey(hKey);
    }
    return bResult;
}


BOOL WriteRegStr(HKEY hKeyParent, 
                 LPCTSTR pszSubkey,
                 LPCTSTR pszKeyName,
                 LPCTSTR pszValue)
{
    BOOL bResult = FALSE;
    HKEY hKey = OpenRegKey(hKeyParent, pszSubkey, KEY_READ | KEY_WRITE);

    if (hKey)
    {
        RegKey_SetStringValue(hKey, pszKeyName, pszValue, REG_SZ);
        RegCloseKey(hKey);
    }
    return bResult;
}

BOOL ReadRegStr(HKEY hKeyParent,
                LPCTSTR pszSubkey,
                LPCTSTR pszKeyName,
                LPTSTR pszValue,
                ULONG* pnChars)
{
    BOOL bResult = false;
    HKEY hKey = OpenRegKey(hKeyParent, pszSubkey, KEY_READ | KEY_WRITE);


    if (hKey)
    {
        RegKey_QueryStringValue(hKey,
                                pszKeyName,
                                pszValue,
                                pnChars);
        RegCloseKey(hKey);
    }
    return bResult;
}

//ReadRegStr $R1 ${ PRODUCT_UNINST_ROOT_KEY } "Software\Microsoft\Windows\CurrentVersion\Uninstall\MobileServer" "UninstallString"

void AddSystemVariable()
{
    //https://nsis.sourceforge.io/Docs/Chapter4.html
    //root_key subkey key_name value
    //WriteRegStr HKLM "Software\My Company\My Software" "String Value" "dead beef"
    //

    //ver classe class CRegKey da ATL
    //exemplo de como adicionar uma sistem variable no path
    //HKEY_CURRENT_USER\Software\Elipse Software\E3\Studio\RecentDomains
#define PATH_REGISTRY_RECENT L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment"


    HKEY hKey = OpenRegKey(HKEY_LOCAL_MACHINE, PATH_REGISTRY_RECENT, KEY_READ | KEY_WRITE);


    if (hKey)
    {
        
        wchar_t buffer[2000];
        ULONG chars = 2000;
        RegKey_QueryStringValue(hKey,
                                L"Path",
                                buffer,
                                &chars);
        wcscat(buffer, L";C:\\Program Files(x86)\\Elipse Software\\Elipse CloudLink");
        RegKey_SetStringValue(hKey, L"Path", buffer, REG_SZ);

        RegCloseKey(hKey);
    }
}






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
        SaveFile(IDR_TXT1);
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
