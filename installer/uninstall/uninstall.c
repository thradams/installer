// uninstall.cpp : Defines the entry point for the application.
//

#include <stdio.h>
#include "uninstall.h"
#include "..\installer\config.h"
#include <direct.h>
#include <Windows.h>
#include <assert.h>



//*************************************************************
//  RegDelnodeRecurse()
//  Purpose:    Deletes a registry key and all it's subkeys / values.
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//*************************************************************

BOOL RegDelnodeRecurse(HKEY hKeyRoot, LPTSTR lpSubKey)
{
    LPTSTR lpEnd;
    LONG lResult;
    DWORD dwSize;
    WCHAR szName[MAX_PATH];
    HKEY hKey;
    FILETIME ftWrite;

    // First, see if we can delete the key without having to recurse
    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS)
    {
        wprintf(L"LRegDeleteKey() - Key and subkey successfully deleted!\n");

        return TRUE;

    }

    else

    {

        wprintf(L"LRegDeleteKey() - Failed to delete key and subkey!Error % d.\n, GetLastError()");

        wprintf(L"LTrying again..\n");

    }



    lResult = RegOpenKeyEx(hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);



    if (lResult != ERROR_SUCCESS)

    {

        if (lResult == ERROR_FILE_NOT_FOUND)

        {

            wprintf(L"RegOpenKeyEx() - Key not found!\n");

            return TRUE;

        }

        else

        {

            wprintf(L"LRegOpenKeyEx() - Error opening key, error % d\n, GetLastError()");

            return FALSE;

        }

    }

    else

        wprintf(L"RegOpenKeyEx() - Key opened successfully!\n");



    // Check for an ending slash and add one if it is missing

    lpEnd = lpSubKey + lstrlen(lpSubKey);



    if (*(lpEnd - 1) != L'\\')

    {

        *lpEnd = L'\\';

        lpEnd++;

        *lpEnd = L'\0';

    }



    // Enumerate the keys

    dwSize = MAX_PATH;

    lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL, NULL, NULL, &ftWrite);



    if (lResult == ERROR_SUCCESS)

    {

        wprintf(L"RegEnumKeyEx() is pretty fine!\n");

        do {

            wcscpy_s(lpEnd, MAX_PATH * 2, szName);



            if (!RegDelnodeRecurse(hKeyRoot, lpSubKey))

            {

                break;

            }



            dwSize = MAX_PATH;

            lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL, NULL, NULL, &ftWrite);



        }
        while (lResult == ERROR_SUCCESS);

    }

    else

        //wprintf(LRegEnumKeyEx() failed lol!\n);



        lpEnd--;

    *lpEnd = L'\0';



    if (RegCloseKey(hKey) == ERROR_SUCCESS)

        wprintf(L"hKey key was closed successfully!\n");

    else

        wprintf(L"Failed to close hKey key!\n");



    // Try again to delete the key.

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);



    if (lResult == ERROR_SUCCESS)

    {

        wprintf(L"RegDeleteKey() is OK!\n");

        return TRUE;

    }

    else

        wprintf(L"RegDeleteKey() failed!\n");



    return FALSE;

}



//*************************************************************

//  RegDelnode()
//  Purpose:    Deletes a registry key and all it's subkeys / values.
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//*************************************************************
BOOL RegDelnode(HKEY hKeyRoot, LPTSTR lpSubKey)
{
    WCHAR szDelKey[MAX_PATH * 2];
    wcscpy(szDelKey, lpSubKey);
    // Recurse starts from root key, HKEY_CLASSES_ROOT
    return RegDelnodeRecurse(hKeyRoot, szDelKey);
}
/*
 este programa representa o desintalador que fica junto da instalacao
*/

HKEY  OpenRegKey(HKEY hKeyParent,
                 LPCTSTR lpszKeyName,
                 REGSAM samDesired/* = KEY_READ | KEY_WRITE*/)
{
    assert(hKeyParent != NULL);
    HKEY hKey = NULL;
    LONG lRes = RegOpenKeyExW(hKeyParent, lpszKeyName, 0, samDesired, &hKey);

    if (lRes == ERROR_SUCCESS)
    {
        assert(lRes == ERROR_SUCCESS);
    }

    return hKey;
}

BOOL DeleteRegKey(HKEY hKeyParent,
                  LPCTSTR pszSubkey)
{
    BOOL bResult = FALSE;
    //HKEY hKey = OpenRegKey(hKeyParent, pszSubkey, KEY_READ | KEY_WRITE | KEY_);

    //if (hKey)
    //{
    LSTATUS e = RegDeleteKey(hKeyParent, pszSubkey);
    // RegCloseKey(hKey);
 //}
    return bResult;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR    lpCmdLine,
                      _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);



    struct finfo {
        const char* from;
        const char* dest;
    } files[] = { FILES };




    for (int i = 0; i < sizeof(files) / sizeof(files[0]); i++)
    {
        if (remove(files[i].dest) != 0)
        {
            int e = errno;
        }
    }


    _rmdir("default/.vscode");
    _rmdir("default");
    _rmdir("../Castle");
#define PRODUCT_UNINST_KEY L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" PRODUCT_CODE
    RegDelnode(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY);
    //MSG msg;

    // Main message loop:
    //while (GetMessage(&msg, NULL, 0, 0))
    //{

      //      DispatchMessage(&msg);
//        
  //  }

    MessageBox(NULL, L"Unistalled", L"Unistall", MB_OK);
    return 0;// (int)msg.wParam;
}

