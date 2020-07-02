#pragma once


# //BEGIN_EXPORT
#include <direct.h>
#include <Windows.h>
#include <Shlobj.h>

# //END_EXPORT


#include "WindowsDesktop.h"


# //BEGIN_EXPORT

#include "config.h"

extern wchar_t INSTDIR[MAX_PATH];

HRESULT CreateShortCut(LPCWSTR lpszPathObj, LPCSTR lpszPathLink, LPCWSTR lpszDesc);
int _wmkdir_p(const wchar_t* path);

BOOL DeleteRegValue(HKEY hKeyParent, LPCTSTR pszSubkey, LPCTSTR pszValueName);

BOOL DeleteRegKey(HKEY hKeyParent, LPCTSTR pszSubkey);

BOOL WriteRegStr(HKEY hKeyParent, LPCTSTR pszSubkey, LPCTSTR pszKeyName, LPCTSTR pszValue);

BOOL ReadRegStr(HKEY hKeyParent, LPCTSTR pszSubkey, LPCTSTR pszKeyName, LPTSTR pszValue, ULONG* pnChars);


BOOL Start(HINSTANCE hInstance);
# //END_EXPORT
