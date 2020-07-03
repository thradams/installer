




#include <Windows.h>

HKEY OpenRegKey(HKEY hKeyParent, LPCTSTR lpszKeyName, REGSAM samDesired);

LONG RegKey_QueryStringValue(HKEY hKey, LPCTSTR pszValueName, LPTSTR pszValue, ULONG* pnChars);

LONG RegKey_QueryDWORDValue(HKEY hKey, LPCTSTR pszValueName, DWORD* dwValue);

void AddSystemVariablesPath(const wchar_t* pathToAdd);

BOOL RegDelnode(HKEY hKeyRoot, LPTSTR lpSubKey);



