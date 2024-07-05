#pragma once
#include <Windows.h>
#include <direct.h>
#include <Windows.h>
#include <windowsx.h>
#include <Shlobj.h>
#include <Richedit.h>

HRESULT CreateShortCut(LPCWSTR lpszPathObj,
                       LPCSTR lpszPathLink,
                       LPCWSTR lpszDesc);

int _wmkdir_p(const wchar_t* path);

BOOL DeleteRegValue(HKEY hKeyParent, LPCTSTR pszSubkey, LPCTSTR pszValueName);
BOOL DeleteRegKey(HKEY hKeyParent, LPCTSTR pszSubkey);
BOOL WriteRegStr(HKEY hKeyParent, LPCTSTR pszSubkey, LPCTSTR pszKeyName, LPCTSTR pszValue);
BOOL ReadRegStr(HKEY hKeyParent, LPCTSTR pszSubkey, LPCTSTR pszKeyName, LPTSTR pszValue, ULONG* pnChars);

BOOL DialogTemplate_Load(struct DialogTemplate* pThis, HINSTANCE hInst, LPCTSTR lpDialogTemplateID);
INT_PTR ShowDialog(HINSTANCE instance, DWORD dlgID, void* p, HWND hParent, DLGPROC lpDialogFunc);
void GetSystemIconFont(wchar_t* strFontNameOut, int* nPointSize);
BOOL CenterWindow(HWND hWnd, HWND hWndCenter);



int SetTextEx(HWND hWnd,
                     LPCTSTR lpstrText,
                     DWORD dwFlags/* = ST_DEFAULT*/, UINT uCodePage/* = CP_ACP*/);
BOOL ShowSelectFolderDialog(HWND hwndOwner,
                                   LPCTSTR lpszTitle,
                                   LPCTSTR startDir,
                                   TCHAR szDir[MAX_PATH]);


int ShowOpenDialog(HWND hwnd,
                   wchar_t fileOut[MAX_PATH],
                   const wchar_t* pszInitialDir,
                   const wchar_t* lpstrFilter);

HWND Create(void* pMain,
            WNDPROC proc,
            PCWSTR lpWindowClassName,
            PCWSTR lpWindowName,
            DWORD dwStyle,
            DWORD dwExStyle,
            HWND hWndParent,
            UINT  MENUIDD,
            UINT  ICON,
            int x,
            int y,
            int nWidth,
            int nHeight);


int mkdir_p(const char* path);

int ExecuteCommand(char * cmd);
