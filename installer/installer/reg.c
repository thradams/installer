#include <assert.h>
#include "reg.h"

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


LONG RegKey_QueryStringValue(HKEY hKey,
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
                 LPCTSTR lpszKeyName,
                 LPCTSTR pszKeyName,
                 LPCTSTR pszValue)
{
    //HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{A9E770C4-FCF1-4E52-A3B4-44D394886A3A}
    //Computador\HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{A9E770C4-FCF1-4E52-A3B4-44D394886A3A}
    //                              Software\Microsoft\Windows\CurrentVersion\Uninstall\{A9E770C4-FCF1-4E52-A3B4-44D394886A3A}
    BOOL bResult = FALSE;
    HKEY hKey = 0;
    LONG lRes = RegCreateKeyExW(hKeyParent, lpszKeyName, 0, NULL, 0, KEY_READ | KEY_WRITE, NULL, &hKey, NULL);

    if (lRes == ERROR_SUCCESS)
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
    BOOL bResult = FALSE;
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

void AddSystemVariablesPath(const wchar_t* pathToAdd)
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
        wcscat(buffer, L";");
        wcscat(buffer, pathToAdd);

        RegKey_SetStringValue(hKey, L"Path", buffer, REG_SZ);

        RegCloseKey(hKey);
    }
}


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
        //wprintf(L"LRegDeleteKey() - Key and subkey successfully deleted!\n");

        return TRUE;

    }

    else

    {

        //wprintf(L"LRegDeleteKey() - Failed to delete key and subkey!Error % d.\n, GetLastError()");

        //wprintf(L"LTrying again..\n");

    }



    lResult = RegOpenKeyEx(hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);



    if (lResult != ERROR_SUCCESS)

    {

        if (lResult == ERROR_FILE_NOT_FOUND)

        {

            //wprintf(L"RegOpenKeyEx() - Key not found!\n");

            return TRUE;

        }

        else

        {

            //wprintf(L"LRegOpenKeyEx() - Error opening key, error % d\n, GetLastError()");

            return FALSE;

        }

    }

    else

    {
        //wprintf(L"RegOpenKeyEx() - Key opened successfully!\n");
    }


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

        //wprintf(L"RegEnumKeyEx() is pretty fine!\n");

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
    {
        //wprintf(LRegEnumKeyEx() failed lol!\n);
    }



        lpEnd--;

    *lpEnd = L'\0';



    if (RegCloseKey(hKey) == ERROR_SUCCESS)
    {
        //wprintf(L"hKey key was closed successfully!\n");
    }
    else
    {
        //wprintf(L"Failed to close hKey key!\n");
    }

    // Try again to delete the key.

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS)
    {
        //  wprintf(L"RegDeleteKey() is OK!\n");
        return TRUE;
    }

    else
    {
     //   wprintf(L"RegDeleteKey() failed!\n");
    }


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
