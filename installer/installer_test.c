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
#include <Shlobj.h>

#define MAX_LOADSTRING 100

#define ASSERT assert 


#include <stdio.h>


int on_extract_entry(const char* filename, void* arg) {
    static int i = 0;
    int n = *(int*)arg;
    printf("Extracted: %s (%d of %d)\n", filename, ++i, n);

    return 0;
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

void SaveFile()
{

    HMODULE handle = GetModuleHandle(NULL);
    HRSRC rc = FindResource(handle, MAKEINTRESOURCE(IDR_TXT1),
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

