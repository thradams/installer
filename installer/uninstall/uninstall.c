// uninstall.cpp : Defines the entry point for the application.
//

#include <stdio.h>
#include "uninstall.h"
#include "..\installer\config.h"
#include <direct.h>
#include <Windows.h>
#include <assert.h>

#include "uninstall_.h"

#define PRODUCT_UNINST_KEY L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" PRODUCT_CODE

void Finish()
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
    
    
    
    MessageBoxA(NULL, GetCommandLineA(), "COMMAND LINE", MB_ICONINFORMATION);

    char* cmd = GetCommandLineA();
    //quando ele passa -u quer dizer que este quer fazer a desinstaladcao
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
                Sleep(500);
        }
        return;
    }



    WCHAR value[MAX_PATH] = { 0 };
    ULONG nChars = MAX_PATH;

    if (GetModuleFileNameW(NULL, value, MAX_PATH) > 0)
    {
        //return 0;
    }

    _wchdir(value);

    char cwd[MAX_PATH];
    if (_getcwd(cwd, sizeof(cwd)) != NULL) {

    }
    else {

        return 1;
    }

    //Vamos remover todas as chaves do registro
    RegDelnode(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY);

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
            MessageBox(NULL, L" Error removing file.", L" Unistall", MB_ICONERROR| MB_OK);
            //continua
        }
    }

    //Remover pastas
    _rmdir("default/.vscode");
    _rmdir("default");


    MessageBox(NULL, DISPLAY_NAME L" was successfully removed from  your computer.", DISPLAY_NAME L" Unistall", MB_ICONINFORMATION |MB_OK);

    //auto deletar-se
    Finish();

    return 0;
}

