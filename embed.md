This code convert binary files in a sequence of intergers for each byte.
This can be useful in the future for multiplatform install not using windows resource.s

```c
#include <stdio.h>

int embed(const char* filename)
{
    char file_out_name[200] = { 0 };
    if (snprintf(file_out_name, sizeof file_out_name, "%s.include", filename) >= sizeof file_out_name)
        return 0;

    FILE* file_out = fopen(file_out_name, "w");
    if (file_out == NULL)
        return 0;

    FILE* file = fopen(filename, "rb");
    if (file == NULL)
    {
        fclose(file_out);
        return 0;
    }

    int count = 0;
    unsigned char ch;
    while (fread(&ch, 1, 1, file))
    {
        if (count % 25 == 0)
            fprintf(file_out, "\n");

        if (count > 0)
            fprintf(file_out, ",");

        fprintf(file_out, "%d", (int)ch);
        count++;        
    }

    fclose(file);
    fclose(file_out);
    return count;
}


int main(int argc, char** argv)
{
    unsigned char b[] = {
#include "Image-1.jpg.include"
    };

    for (int i = 1; i < argc; i++)
    {
        int bytes = embed(argv[i]);
        if (bytes == 0)
        {
            printf("error generating file %s\n", argv[i]);
        }
    }
}
```


```c


/*
  cl  /D_CRT_SECURE_NO_WARNINGS /DWIN32 /D_DEBUG /D_WINDOWS  /D_UNICODE /DUNICODE uninstall.c 
      kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib 
      advapi32.lib shell32.lib ole32.lib oleaut32.lib 
      uuid.lib odbc32.lib
      /link /MANIFEST:EMBED /MANIFESTUAC:"level='requireAdministrator' uiAccess='false'"
*/

#include <Windows.h>
#include <strsafe.h>

int ExecuteCommand(char * cmd)
{
    STARTUPINFOA si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    si.cb = sizeof(si);

    // Create the new process
    if (!CreateProcessA(
        NULL,   // Application name
        cmd,              // Command line arguments
        NULL,              // Process handle not inheritable
        NULL,              // Thread handle not inheritable
        FALSE,             // Set handle inheritance to FALSE
        0,                 // No creation flags
        NULL,              // Use parent's environment block
        NULL,              // Use parent's starting directory 
        &si,               // Pointer to STARTUPINFO structure
        &pi)               // Pointer to PROCESS_INFORMATION structure
        )
    {
        return GetLastError();
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}

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
    StringCbCatA(cmd, sizeof cmd, "sun.exe");

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

    ExecuteCommand(cmd);
}

void DoUninstall(const char * lpCmdLine)
{
    int r = MessageBoxA(NULL, "Tem certeza que deseja desintalar?", "Uninstall", MB_ICONQUESTION | MB_YESNO);
    if (r != IDYES)
    {
        return;
    }

    char cmd[200] = {0};
    
    cmd[0] = '\0';
    StringCbCatA(cmd, sizeof cmd, lpCmdLine);
    StringCbCatA(cmd, sizeof cmd, "/uninstall.pdb");

    if (DeleteFileA(cmd) == 0)
    {
        MessageBoxA(NULL, cmd, "", MB_OK);
        return;
    }

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
        MessageBoxA(NULL, cmd, "", MB_OK);
    }
    else
    {
        MessageBoxA(NULL, "sucesso", "", MB_OK);
    }

    if (RemoveDirectoryA(lpCmdLine))
    {
        MessageBoxA(NULL, "sucesso", "", MB_OK);
    }
    else
    {
        MessageBoxA(NULL, "falha remover dir", "", MB_OK);
    }
}

int APIENTRY WinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

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

```
