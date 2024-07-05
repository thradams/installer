#include <stdio.h>
#include <Windows.h>
#include <strsafe.h>

#include "..\installer\setup.h"
#include <direct.h>
#include <Windows.h>
#include <assert.h>
#include "process.h"
#include  "w32.h"

#include <assert.h>
#include <Windows.h>

#define PRODUCT_UNINST_KEY L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" PRODUCT_CODE



char * dirname(char * path)
{
    int last = -1;
    for (int i = 0; path[i]; i++)
    {
        if (path[i] == '\\' || path[i] == '/')
            last = i;
    }

    if (last != -1)
    {
        path[last] = 0;
    }
    return path;
}

void CreateTempUninstall()
{
    char currentDir[200];
    DWORD r = GetModuleFileNameA(NULL, currentDir, sizeof currentDir);


    char buffer[200];
    r = GetTempPath2A(sizeof buffer, buffer);

    char cmd[200] = { 0 };
    StringCbCatA(cmd, sizeof cmd, buffer);
    StringCbCatA(cmd, sizeof cmd, "un.exe");

    r = CopyFileA(currentDir, cmd, FALSE);
    //r = MoveFileExA(cmd, NULL, MOVEFILE_DELAY_UNTIL_REBOOT | MOVEFILE_REPLACE_EXISTING);
    if (!r)
    {
        r = GetLastError();
        MessageBoxA(NULL, "falha cipia", currentDir, MB_ICONERROR);
        return;
    }

    char * d = dirname(currentDir);

    cmd[0] = '\0';
    StringCbCatA(cmd, sizeof cmd, buffer);
    StringCbCatA(cmd, sizeof cmd, "un.exe ");
    StringCbCatA(cmd, sizeof cmd, currentDir);

    if (ExecuteCommand(cmd) != 0)
    {
        MessageBoxA(NULL, "Não foi possível executar o programa de desintalação", "currentDir", MB_ICONERROR);
    }
}

void DoProgramUninstall(const char * lpCmdLine)
{


    //Vamos remover todas as chaves do registro
    if (!RegDelnode(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY))
    {
        MessageBoxA(NULL, "Error removing registry keys", "Uninstall", MB_ICONERROR | MB_OK);
    }

    //Depois remover todos os arquivos salvos
    struct finfo {
        const char * from;
        const char * dest;
    } files[] = { FILES };

    char filename[200] = { 0 };


    for (int i = 0; i < sizeof(files) / sizeof(files[0]); i++)
    {
        filename[0] = '\0';
        StringCbCatA(filename, sizeof filename, lpCmdLine);
        StringCbCatA(filename, sizeof filename, "/");
        StringCbCatA(filename, sizeof filename, files[i].dest);

        if (DeleteFileA(filename) == 0)
        {
            int e = GetLastError();
            char errorMessage[MAX_PATH + 200];
            snprintf(errorMessage, sizeof(errorMessage), "Error deleting file '%s'. (%d)", filename, e);
            MessageBoxA(NULL, errorMessage, "Uninstall", MB_ICONERROR | MB_OK);
        }
    }
}
void DoUninstall(const char * lpCmdLine)
{
    int r = MessageBoxA(NULL, "Tem certeza que deseja desintalar?", "Uninstall", MB_ICONQUESTION | MB_YESNO);
    if (r != IDYES)
    {
        return;
    }


    DoProgramUninstall(lpCmdLine);




    //////////////////////////////////////////////////////////////////////////
    char cmd[200] = { 0 };

    cmd[0] = '\0';
    StringCbCatA(cmd, sizeof cmd, lpCmdLine);
    StringCbCatA(cmd, sizeof cmd, "/uninstall.exe");

    int i = 0;
    for (; i < 5; i++)
    {
        if (DeleteFileA(cmd))
            break;
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            break;
        }
        Sleep(500);
    }
    if (i == 5)
    {
        //MessageBoxA(NULL, cmd, "", MB_OK);
    }
    else
    {
        //MessageBoxA(NULL, "sucesso", "", MB_OK);
    }

    i = 0;
    for (; i < 5; i++)
    {
        if (RemoveDirectoryA(lpCmdLine))
        {
            MessageBoxA(NULL, lpCmdLine, "REMOVIDO", MB_OK);
            break;
        }
        else
        {
            cmd[0] = '\0';
            StringCbCatA(cmd, sizeof cmd, "'");
            StringCbCatA(cmd, sizeof cmd, lpCmdLine);
            StringCbCatA(cmd, sizeof cmd, "'");

            int er = GetLastError();
            char numberstr[20];
            itoa(er, numberstr, 10);

            StringCbCatA(cmd, sizeof cmd, " code=");
            StringCbCatA(cmd, sizeof cmd, numberstr);

            MessageBoxA(NULL, cmd, "FALHA", MB_OK);
        }
    }

    MessageBox(NULL, DISPLAY_NAME L" was successfully removed from  your computer.", DISPLAY_NAME L" Unistall", MB_ICONINFORMATION | MB_OK);

}

int APIENTRY WinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    //this is necessary to use richedit controls
    //LoadLibrary(TEXT("Riched20.dll"));

    if (strlen(lpCmdLine) == 0)
    {
        CreateTempUninstall();
    }
    else
    {
        DoUninstall(lpCmdLine);
    }

    return 0;
}

