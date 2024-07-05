#include <stdio.h>
#include "..\installer\setup.h"
#include <direct.h>
#include <Windows.h>
#include <assert.h>
#include "process.h"
#include  "w32.h"

#include <assert.h>
#include <Windows.h>

#define PRODUCT_UNINST_KEY L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" PRODUCT_CODE



static void Finish()
{
    //A ultima fase do instalador eh copiar-se 
    //para uma pasta temp e depois 
    //apagar-se do lugar original
    //junto com a pasta

    //Esta funcao chama o desintalador da temp passando -d

    WCHAR value[MAX_PATH] = { 0 };
    ULONG nChars = MAX_PATH;
    if (GetModuleFileNameW(NULL, value, MAX_PATH) > 0)
    {
        //return 0;
    }

    WCHAR tempPath[MAX_PATH] = { 0 };
    DWORD dw = GetTempPathW(MAX_PATH, tempPath);
    wcscat(tempPath, L"uninstall.exe");
    CopyFile(value, tempPath, FALSE);

    WCHAR commandLine[MAX_PATH] = { 0 };
    wcscat(commandLine, L"-d ");
    if (GetModuleFileNameW(NULL, &commandLine[3], MAX_PATH - 4) > 0)
    {
    }

    SystemCreateProcess(tempPath, commandLine);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR    lpCmdLine,
                      _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);


    char* cmd = GetCommandLineA();

    //Ao passar -d significa que é a última fase e ele se auto deleta.
    BOOL bRemoveUninstall = (cmd[0] == '-' && cmd[1] == 'd');
    if (bRemoveUninstall)
    {
        for (int i = 0; i < 100; i++)
        {
            if (remove(&cmd[3]) == 0)
            {
                char* dir = &cmd[3];
                int dirlen = strlen(dir);
                dir[dirlen - sizeof("uninstall.exe")] = 0;
                _rmdir(dir);
                break;
            }
            else
            {
                //aguarda um pouco
                Sleep(500);
            }
        }
        return 0;
    }



    WCHAR exePath[MAX_PATH] = { 0 };
    ULONG nChars = MAX_PATH;

    if (GetModuleDir(NULL, exePath, MAX_PATH) > 0)
    {
        //return 0;
    }

    //Poe o current dir como sendo o path do exe
    //
    if (_wchdir(exePath) == -1)
    {
        //nao eh para acontecer isso nunca
        MessageBoxA(NULL, "Error setting current dir", "Uninstall", MB_ICONERROR | MB_OK);
        return 1;
    }



    //Vamos remover todas as chaves do registro
    if (!RegDelnode(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY))
    {
        MessageBoxA(NULL, "Error removing registry keys", "Uninstall", MB_ICONERROR | MB_OK);
    }

    //Depois remover todos os arquivos salvos
    struct finfo {
        const char* from;
        const char* dest;
    } files[] = { FILES };


    for (int i = 0; i < sizeof(files) / sizeof(files[0]); i++)
    {
        if (remove(files[i].dest) != 0)
        {
            int e = errno;
            char errorMessage[MAX_PATH + 200];
            snprintf(errorMessage, sizeof(errorMessage), "Error deleting file %s. (%d)", files[i].dest, e);
            MessageBoxA(NULL, errorMessage, "Uninstall", MB_ICONERROR | MB_OK);
        }
    }

    //Remover pastas
    _rmdir("default/.vscode");
    _rmdir("default");


    MessageBox(NULL, DISPLAY_NAME L" was successfully removed from  your computer.", DISPLAY_NAME L" Unistall", MB_ICONINFORMATION | MB_OK);

    //auto deletar-se
    Finish();

    return 0;
}

