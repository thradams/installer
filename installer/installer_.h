





#include <direct.h>


#include <Windows.h>


#include <Shlobj.h>




void SaveFile(DWORD idd);

HRESULT CreateShortCut(LPCWSTR lpszPathObj, LPCSTR lpszPathLink, LPCWSTR lpszDesc);
int mkdir_p(const char* path);

BOOL DeleteRegValue(HKEY hKeyParent, LPCTSTR pszSubkey, LPCTSTR pszValueName);

BOOL DeleteRegKey(HKEY hKeyParent, LPCTSTR pszSubkey);

BOOL WriteRegStr(HKEY hKeyParent, LPCTSTR pszSubkey, LPCTSTR pszKeyName, LPCTSTR pszValue);

BOOL ReadRegStr(HKEY hKeyParent, LPCTSTR pszSubkey, LPCTSTR pszKeyName, LPTSTR pszValue, ULONG* pnChars);


BOOL Start(HINSTANCE hInstance);


