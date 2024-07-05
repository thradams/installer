#include <Windows.h>
#include <WinUser.h>
#include "w32.h"


#include <shlobj_core.h>
#include <Shlobj.h>

#pragma comment(lib, "Comctl32.lib")
#define ASSERT(x)

#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif


//local struct used for implementation
#pragma pack(push, 1)
struct DLGINITSTRUCT
{
    WORD nIDC;
    WORD message;
    DWORD dwSize;
};
struct DLGTEMPLATEEX
{
    WORD dlgVer;
    WORD signature;
    DWORD helpID;
    DWORD exStyle;
    DWORD style;
    WORD cDlgItems;
    short x;
    short y;
    short cx;
    short cy;

    // Everything else in this structure is variable length,
    // and therefore must be determined dynamically

    // sz_Or_Ord menu;      // name or ordinal of a menu resource
    // sz_Or_Ord windowClass; // name or ordinal of a window class
    // WCHAR title[titleLen]; // title string of the dialog box
    // short pointsize;     // only if DS_SETFONT is set
    // short weight;      // only if DS_SETFONT is set
    // short bItalic;     // only if DS_SETFONT is set
    // WCHAR font[fontLen];   // typeface name, if DS_SETFONT is set
};
struct DLGITEMTEMPLATEEX
{
    DWORD helpID;
    DWORD exStyle;
    DWORD style;
    short x;
    short y;
    short cx;
    short cy;
    DWORD id;

    // Everything else in this structure is variable length,
    // and therefore must be determined dynamically

    // sz_Or_Ord windowClass; // name or ordinal of a window class
    // sz_Or_Ord title;     // title string or ordinal of a resource
    // WORD extraCount;     // bytes following creation data
};
#pragma pack(pop)




HINSTANCE s_hInstance;

struct DialogTemplate
{
    HGLOBAL m_hTemplate;
    DWORD m_dwTemplateSize;
    BOOL m_bSystemFont;
};

BOOL IsDialogEx(const DLGTEMPLATE* pTemplate)
{
    return ((struct DLGTEMPLATEEX*)pTemplate)->signature == 0xFFFF;
}


inline int FontAttrSize(BOOL bDialogEx)
{
    return (int)sizeof(WORD) * (bDialogEx ? 3 : 1);
}


BYTE* GetFontSizeField(const DLGTEMPLATE* pTemplate)
{
    BOOL bDialogEx = IsDialogEx(pTemplate);
    WORD* pw;

    if (bDialogEx)
        pw = (WORD*)((struct DLGTEMPLATEEX*)pTemplate + 1);
    else
        pw = (WORD*)(pTemplate + 1);

    if (*pw == (WORD)-1)      // Skip menu name string or ordinal
        pw += 2; // WORDs
    else
        while (*pw++);

    if (*pw == (WORD)-1)      // Skip class name string or ordinal
        pw += 2; // WORDs
    else
        while (*pw++);

    while (*pw++);          // Skip caption string

    return (BYTE*)pw;
}

BOOL HasFont(const DLGTEMPLATE* pTemplate)
{
    return (DS_SETFONT &
            (IsDialogEx(pTemplate) ? ((struct DLGTEMPLATEEX*)pTemplate)->style :
            pTemplate->style));
}

BOOL DialogTemplate_SetFont(struct DialogTemplate* pThis, LPCWSTR lpFaceName, WORD nFontSize)
{
    ASSERT(pThis->m_hTemplate != NULL);

    if (pThis->m_dwTemplateSize == 0)
        return FALSE;

    DLGTEMPLATE* pTemplate = (DLGTEMPLATE*)GlobalLock(pThis->m_hTemplate);
    if (pTemplate == NULL)
    {
        ASSERT(FALSE);
        return FALSE;
    }
    BOOL bDialogEx = IsDialogEx(pTemplate);
    BOOL bHasFont = HasFont(pTemplate);
    int cbFontAttr = FontAttrSize(bDialogEx);

    if (bDialogEx)
        ((struct DLGTEMPLATEEX*)pTemplate)->style |= DS_SETFONT;
    else
        pTemplate->style |= DS_SETFONT;

    size_t nFaceNameLen = wcslen(lpFaceName);

    if (nFaceNameLen >= LF_FACESIZE)
    {
        // Name too long
        return FALSE;
    }

#ifdef _UNICODE
    size_t cbNew = cbFontAttr + ((nFaceNameLen + 1) * sizeof(TCHAR));
    BYTE* pbNew = (BYTE*)lpFaceName;
#else
    WCHAR wszFaceName[LF_FACESIZE];
    int cbNew = cbFontAttr + 2 * MultiByteToWideChar(CP_ACP, 0, lpFaceName, -1, wszFaceName, LF_FACESIZE);
    BYTE* pbNew = (BYTE*)wszFaceName;
#endif

    if (cbNew < (size_t)cbFontAttr)
    {
        return FALSE;
    }

    BYTE* pb = GetFontSizeField(pTemplate);
    int cbOld = (int)(bHasFont ? cbFontAttr + 2 * (wcslen((WCHAR*)(pb + cbFontAttr)) + 1) : 0);

    BYTE* pOldControls = (BYTE*)(((DWORD_PTR)pb + cbOld + 3) & ~((DWORD_PTR)3));
    BYTE* pNewControls = (BYTE*)(((DWORD_PTR)pb + cbNew + 3) & ~((DWORD_PTR)3));

    WORD nCtrl = bDialogEx ? (WORD)((struct DLGTEMPLATEEX*)pTemplate)->cDlgItems :
        (WORD)pTemplate->cdit;

    if (cbNew != cbOld && nCtrl > 0)
    {
        size_t nBuffLeftSize = (size_t)(pThis->m_dwTemplateSize - (pOldControls - (BYTE*)pTemplate));

        if (nBuffLeftSize > pThis->m_dwTemplateSize)
        {
            return FALSE;
        }

        memmove_s(pNewControls, nBuffLeftSize, pOldControls, nBuffLeftSize);
    }

    *(WORD*)pb = nFontSize;
    memmove_s(pb + cbFontAttr, cbNew - cbFontAttr, pbNew, cbNew - cbFontAttr);

    pThis->m_dwTemplateSize += (ULONG)(pNewControls - pOldControls);

    GlobalUnlock(pThis->m_hTemplate);
    pThis->m_bSystemFont = FALSE;
    return TRUE;
}

HRESULT CreateShortCut(LPCWSTR lpszPathObj, LPCSTR lpszPathLink, LPCWSTR lpszDesc)
{
    HRESULT hres;
    IShellLink* psl;

    // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
    // has already been called.
    hres = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLink, (LPVOID*)&psl);
    if (SUCCEEDED(hres))
    {
        IPersistFile* ppf;

        // Set the path to the shortcut target and add the description. 
        psl->lpVtbl->SetPath(psl, lpszPathObj);
        psl->lpVtbl->SetDescription(psl, lpszDesc);

        // Query IShellLink for the IPersistFile interface, used for saving the 
        // shortcut in persistent storage. 
        hres = psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, (LPVOID*)&ppf);

        if (SUCCEEDED(hres))
        {
            WCHAR wsz[MAX_PATH];

            // Ensure that the string is Unicode. 
            MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1, wsz, MAX_PATH);

            // Save the link by calling IPersistFile::Save. 
            hres = ppf->lpVtbl->Save(ppf, wsz, TRUE);
            ppf->lpVtbl->Release(ppf);
        }
        psl->lpVtbl->Release(psl);
    }
    return hres;
}


extern HINSTANCE s_hInstance = 0;

INT_PTR ShowDialog(HINSTANCE hInstance,
                   DWORD dlgID,
                   void* p,
                   HWND hParent,
                   DLGPROC lpDialogFunc)
{
    struct DialogTemplate dlt = { 0 };
    //HINSTANCE hInstance = s_hInstance;
    DialogTemplate_Load(&dlt, hInstance, MAKEINTRESOURCEW(dlgID));

    wchar_t strFontName[200] = { 0 };
    int nPointSize = 0;
    GetSystemIconFont(strFontName, &nPointSize);
    DialogTemplate_SetFont(&dlt, strFontName, (WORD)nPointSize);

    // get pointer to the modified dialog template
    LPSTR pdata = (LPSTR)GlobalLock(dlt.m_hTemplate);

    if (pdata == 0)
    {
        return 0;
    }

    INT_PTR r = DialogBoxIndirectParam(
        hInstance,
        (LPCDLGTEMPLATEW)pdata,
        hParent,
        lpDialogFunc,
        (LPARAM)p);


    // unlock memory object
    GlobalUnlock(dlt.m_hTemplate);
    return r;
}

BOOL IsWindowsXPOrGreater()
{
#pragma warning( push )
#pragma warning( disable : 4996 )
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);
    return osvi.dwMajorVersion >= 5;
#pragma warning( pop )
}


void GetSystemIconFont(wchar_t* strFontNameOut, int* nPointSize)
{
    wcscpy(strFontNameOut, L"MS Shell Dlg"); // out
    *nPointSize = 8; //out

    NONCLIENTMETRICS metrics;

    if (IsWindowsXPOrGreater())
    {
        metrics.cbSize = sizeof(NONCLIENTMETRICS);
    }
    else
    {
        metrics.cbSize = sizeof(NONCLIENTMETRICS) - sizeof(int);
    }

    if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &metrics, 0) != 0)
    {
        HDC hDC = GetDC(NULL);
        int nLPixY = GetDeviceCaps(hDC, LOGPIXELSY);
        ReleaseDC(NULL, hDC);

        // copy font parameters ///nao sei bem pq 72 mas funciona?
        *nPointSize = -MulDiv(metrics.lfMessageFont.lfHeight, 72, nLPixY);
        wcscpy(strFontNameOut, metrics.lfMessageFont.lfFaceName);
    }
    else
    {
        ASSERT(0);//
    }
}

BOOL DialogTemplate_SetTemplate(struct DialogTemplate* pThis, const DLGTEMPLATE* pTemplate, UINT cb)
{
    pThis->m_dwTemplateSize = cb;
    SIZE_T nAllocSize = pThis->m_dwTemplateSize + LF_FACESIZE * 2;

    if (nAllocSize < pThis->m_dwTemplateSize)
    {
        return FALSE;
    }

    if ((pThis->m_hTemplate = GlobalAlloc(GPTR, nAllocSize)) == NULL)
    {
        return FALSE;
    }

    DLGTEMPLATE* pNew = (DLGTEMPLATE*)GlobalLock(pThis->m_hTemplate);
    memcpy_s((BYTE*)pNew, (size_t)pThis->m_dwTemplateSize, pTemplate, (size_t)pThis->m_dwTemplateSize);

    pThis->m_bSystemFont = (HasFont(pNew) == 0);

    GlobalUnlock(pThis->m_hTemplate);
    return TRUE;
}


BOOL DialogTemplate_Load(struct DialogTemplate* pThis, HINSTANCE hInst, LPCTSTR lpDialogTemplateID)
{
    if (hInst == NULL)
    {
        return FALSE;
    }

    HRSRC hRsrc = FindResource(hInst, lpDialogTemplateID, RT_DIALOG);

    if (hRsrc == NULL)
    {
        ASSERT(false);
        return FALSE;
    }

    HGLOBAL hTemplate = LoadResource(hInst, hRsrc);
    if (hTemplate == NULL)
    {
        ASSERT(false);
        return FALSE;
    }

    DLGTEMPLATE* pTemplate = (DLGTEMPLATE*)LockResource(hTemplate);


    BOOL bSet = DialogTemplate_SetTemplate(pThis, pTemplate, (UINT)SizeofResource(hInst, hRsrc));
    UnlockResource(hTemplate);
    FreeResource(hTemplate);
    return bSet;
}


DWORD GetStyle(HWND hWnd)
{
    ASSERT(IsWindow(hWnd));
    return (DWORD)GetWindowLong(hWnd, GWL_STYLE);
}

BOOL CenterWindow(HWND hWnd, HWND hWndCenter)
{
    ASSERT(IsWindow(hWnd));

    // determine owner window to center against
    DWORD dwStyle = GetStyle(hWnd);

    if (hWndCenter == NULL)
    {
        if (dwStyle & WS_CHILD)
        {
            hWndCenter = GetParent(hWnd);
        }
        else
        {
            hWndCenter = GetWindow(hWnd, GW_OWNER);
        }
    }

    // get coordinates of the window relative to its parent
    RECT rcDlg;
    GetWindowRect(hWnd, &rcDlg);
    RECT rcArea;
    RECT rcCenter;
    HWND hWndParent;

    if (!(dwStyle & WS_CHILD))
    {
        // don't center against invisible or minimized windows
        if (hWndCenter != NULL)
        {
            DWORD dwStyleCenter = GetWindowLong(hWndCenter, GWL_STYLE);

            if (!(dwStyleCenter & WS_VISIBLE) || (dwStyleCenter & WS_MINIMIZE))
            {
                hWndCenter = NULL;
            }
        }

        // center within screen coordinates
#if WINVER < 0x0500
        SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcArea, NULL);
#else
        HMONITOR hMonitor = NULL;

        if (hWndCenter != NULL)
        {
            hMonitor = MonitorFromWindow(hWndCenter,
                                         MONITOR_DEFAULTTONEAREST);
        }
        else
        {
            hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
        }

        //ATLENSURE_RETURN_VAL(hMonitor != NULL, FALSE);

        MONITORINFO minfo;
        minfo.cbSize = sizeof(MONITORINFO);
        /*BOOL bResult = */
        GetMonitorInfo(hMonitor, &minfo);
        //ATLENSURE_RETURN_VAL(bResult, FALSE);

        rcArea = minfo.rcWork;
#endif

        if (hWndCenter == NULL)
        {
            rcCenter = rcArea;
        }
        else
        {
            GetWindowRect(hWndCenter, &rcCenter);
        }
    }
    else
    {
        // center within parent client coordinates
        hWndParent = GetParent(hWnd);
        ASSERT(IsWindow(hWndParent));

        GetClientRect(hWndParent, &rcArea);
        ASSERT(IsWindow(hWndCenter));
        GetClientRect(hWndCenter, &rcCenter);
        MapWindowPoints(hWndCenter, hWndParent, (POINT*)&rcCenter, 2);
    }

    int DlgWidth = rcDlg.right - rcDlg.left;
    int DlgHeight = rcDlg.bottom - rcDlg.top;

    // find dialog's upper left based on rcCenter
    int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
    int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

    // if the dialog is outside the screen, move it inside
    if (xLeft + DlgWidth > rcArea.right)
    {
        xLeft = rcArea.right - DlgWidth;
    }

    if (xLeft < rcArea.left)
    {
        xLeft = rcArea.left;
    }

    if (yTop + DlgHeight > rcArea.bottom)
    {
        yTop = rcArea.bottom - DlgHeight;
    }

    if (yTop < rcArea.top)
    {
        yTop = rcArea.top;
    }

    // map screen coordinates to child coordinates
    return SetWindowPos(hWnd, NULL, xLeft, yTop, -1, -1,
                        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

// callback function
INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
    if (uMsg == BFFM_INITIALIZED && pData != 0) SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
    return 0;
}


BOOL ShowSelectFolderDialog(HWND hwndOwner,
                                   LPCTSTR lpszTitle,
                                   LPCTSTR startDir,
                                   TCHAR szDir[MAX_PATH])
{
    //startDir pode ser null que eh tratado
    BROWSEINFO bInfo;
    bInfo.hwndOwner = hwndOwner;
    bInfo.pidlRoot = NULL;
    bInfo.pszDisplayName = szDir; // Address of a buffer to receive the display name of the folder selected by the user
    bInfo.lpszTitle = lpszTitle;
    bInfo.ulFlags = BIF_NEWDIALOGSTYLE;
    bInfo.lpfn = BrowseCallbackProc;
    bInfo.lParam = (LPARAM)startDir;
    bInfo.iImage = -1;

    LPITEMIDLIST lpItem = SHBrowseForFolder(&bInfo);
    if (lpItem != NULL)
    {
        SHGetPathFromIDList(lpItem, szDir);
        //......
        return TRUE;
    }
    return FALSE;
}

int ShowOpenDialog(HWND hwnd,
                   wchar_t fileOut[MAX_PATH],
                   const wchar_t* pszInitialDir,
                   const wchar_t* lpstrFilter)
{
    OPENFILENAMEW ofn;
    WCHAR szFile[MAX_PATH];
    szFile[0] = L'\0';

    WCHAR szInitialDir[MAX_PATH];
    szInitialDir[0] = L'\0';
    if (pszInitialDir != NULL)
    {
        wcscpy_s(szInitialDir, MAX_PATH, pszInitialDir);
    }


    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;

    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
    ofn.lpstrFilter = lpstrFilter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = szInitialDir;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_DONTADDTORECENT;

    if (GetOpenFileNameW(&ofn) == TRUE)
    {
        fileOut = szFile;
        return IDOK;
    }

    return IDCANCEL;
}


int CALLBACK PropertySheetDialogCallback(HWND hwndDlg,
                                         UINT uMsg,
                                         LPARAM lParam)
{
    switch (uMsg)
    {
        case PSCB_BUTTONPRESSED:
            switch (lParam)
            {
                case PSBTN_OK:
                    break;
                case PSBTN_CANCEL:
                    break;
                case PSBTN_APPLYNOW:
                    break;
                case  PSBTN_FINISH:
                    break;
            }
            break;

        case PSCB_INITIALIZED:
            break;

        case PSCB_PRECREATE:
        {
            DLGTEMPLATE* pDlgTemplate = (DLGTEMPLATE*)lParam;
            struct DialogTemplate dlt;
            dlt.m_hTemplate = pDlgTemplate;

            wchar_t strFontName[200];
            int nPointSize;
            GetSystemIconFont(strFontName, &nPointSize);
            DialogTemplate_SetFont(&dlt, strFontName, (WORD)nPointSize);

            memmove((void*)lParam, dlt.m_hTemplate, dlt.m_dwTemplateSize);
        }

        break;
    }
    return 0;
}


INT_PTR ShowPropertySheet(HINSTANCE hInstance,
                          HWND hwndParent,
                          const wchar_t* pszCaption,
                          PROPSHEETPAGE** pages,
                          size_t nPages)
{
    PFNPROPSHEETCALLBACK callback = PropertySheetDialogCallback;
    if (hInstance == NULL)
    {
        hInstance = s_hInstance;
    }

    ASSERT(nPages <= 4); //basta aumentar quando precisar...
    HPROPSHEETPAGE rhpsp[4];
    struct DialogTemplate dlt[4];

    for (size_t i = 0; i < nPages; i++)
    {
        DialogTemplate_Load(&dlt[i], hInstance, pages[i]->pszTemplate);

        wchar_t strFontName[200];
        int nPointSize;
        GetSystemIconFont(strFontName, &nPointSize);
        DialogTemplate_SetFont(&dlt[i], strFontName, (WORD)nPointSize);
        LPSTR pdata = (LPSTR)GlobalLock(dlt[i].m_hTemplate);

        if (pdata == 0)
        {
            ASSERT(0);
            return 0;
        }
        pages[i]->pResource = (LPCDLGTEMPLATE)(dlt[i].m_hTemplate);
        rhpsp[i] = CreatePropertySheetPage(pages[i]);
    }

    PROPSHEETHEADER psh;
    ZeroMemory(&psh, sizeof(psh));
    psh.dwSize = sizeof(psh);
    psh.hInstance = hInstance;
    psh.hwndParent = hwndParent;
    psh.phpage = rhpsp;
    psh.dwFlags = PSH_USEICONID | PSH_USECALLBACK | PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
    psh.pszCaption = pszCaption;
    psh.pszIcon = IDI_APPLICATION;
    psh.nStartPage = 0;
    psh.nPages = (UINT)nPages;
    psh.pfnCallback = callback;
    psh.hbmWatermark = 0;// (HBITMAP)(T*)(this);
    INT_PTR r = PropertySheet(&psh);

    for (size_t i = 0; i < nPages; i++)
    {
        GlobalUnlock(dlt[i].m_hTemplate);
    }

    return r;
}


int SetTextEx(HWND hWnd,
                     LPCTSTR lpstrText,
                     DWORD dwFlags/* = ST_DEFAULT*/, UINT uCodePage/* = CP_ACP*/)
{
    SETTEXTEX ste = { 0 };
    ste.flags = dwFlags;
    ste.codepage = uCodePage;
    return (int)SendMessage(hWnd, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)lpstrText);
}


BOOL ReadRegStr(HKEY hKeyParent,
                LPCTSTR pszSubkey,
                LPCTSTR pszKeyName,
                LPTSTR pszValue,
                ULONG* pnChars);


HKEY  OpenRegKey(HKEY hKeyParent,
                 LPCTSTR lpszKeyName,
                 REGSAM samDesired/* = KEY_READ | KEY_WRITE*/)
{
    ASSERT(hKeyParent != NULL);
    HKEY hKey = NULL;
    LONG lRes = RegOpenKeyExW(hKeyParent, lpszKeyName, 0, samDesired, &hKey);

    if (lRes == ERROR_SUCCESS)
    {
        ASSERT(lRes == ERROR_SUCCESS);
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

    ASSERT(hKey != NULL);
    ASSERT(pnChars != NULL);

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

    ASSERT(hKey != NULL);

    nBytes = sizeof(DWORD);
    lRes = RegQueryValueEx(hKey, pszValueName, NULL, &dwType, (LPBYTE)(&dwValue),
                           &nBytes);

    if (lRes != ERROR_SUCCESS)
        return lRes;

    if (dwType != REG_DWORD)
        return ERROR_INVALID_DATA;

    return ERROR_SUCCESS;
}

LONG RegKey_SetStringValue(HKEY hKey,
    LPCTSTR pszValueName,
    LPCTSTR pszValue,
    DWORD dwType)
{
    ASSERT(hKey != NULL);
    ASSERT((dwType == REG_SZ) || (dwType == REG_EXPAND_SZ));

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


BOOL RegDelnode(HKEY hKeyRoot, LPTSTR lpSubKey)
{
    WCHAR szDelKey[MAX_PATH * 2];
    wcscpy(szDelKey, lpSubKey);
    // Recurse starts from root key, HKEY_CLASSES_ROOT
    return RegDelnodeRecurse(hKeyRoot, szDelKey);
}

void rmtree(const char path[])
{
#if 0
    char* full_path;
    DIR* dir;
    struct stat stat_path, stat_entry;
    struct dirent* entry;

    // stat for the path
    stat(path, &stat_path);

    // if path does not exists or is not dir - exit with status -1
    if (S_ISDIR(stat_path.st_mode) == 0) {
        fprintf(stderr, "%s: %s\n", "Is not directory", path);
        exit(-1);
    }

    // if not possible to read the directory for this user
    if ((dir = opendir(path)) == NULL) {
        fprintf(stderr, "%s: %s\n", "Can`t open directory", path);
        exit(-1);
    }

    // the length of the path
    path_len = strlen(path);

    // iteration through entries in the directory
    while ((entry = readdir(dir)) != NULL) {

        // skip entries "." and ".."
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        // determinate a full path of an entry
        full_path = calloc(path_len + strlen(entry->d_name) + 1, sizeof(char));
        strcpy(full_path, path);
        strcat(full_path, "/");
        strcat(full_path, entry->d_name);

        // stat for the entry
        stat(full_path, &stat_entry);

        // recursively remove a nested directory
        if (S_ISDIR(stat_entry.st_mode) != 0) {
            rmtree(full_path);
            continue;
        }

        // remove a file object
        if (unlink(full_path) == 0)
            printf("Removed a file: %s\n", full_path);
        else
            printf("Can`t remove a file: %s\n", full_path);
        free(full_path);
    }

    // remove the devastated directory and close the object of it
    if (rmdir(path) == 0)
        printf("Removed a directory: %s\n", path);
    else
        printf("Can`t remove a directory: %s\n", path);

    closedir(dir);
#endif
}

int mkdir_p(const char* path)
{
    /* Adapted from http://stackoverflow.com/a/2336245/119527 */
    const size_t len = strlen(path);
    char _path[MAX_PATH];
    char* p;

    errno = 0;

    /* Copy string so its mutable */
    if (len > sizeof(_path) - 1) {
        errno = ENAMETOOLONG;
        return -1;
    }
    lstrcpyA(_path, path);

    /* Iterate the string */
    for (p = _path + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            /* Temporarily truncate */
            *p = '\0';

            if (_mkdir(_path) != 0) {
                if (errno != EEXIST)
                    return -1;
            }

            *p = '/';
        }
    }

    if (mkdir(_path) != 0) {
        if (errno != EEXIST)
            return -1;
    }

    return 0;
}




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
            int nHeight)

{

    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc = proc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = 0; // LoadIcon(hInstance, MAKEINTRESOURCE(IconId));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (MENUIDD != 0)
    {
        wcex.lpszMenuName = MAKEINTRESOURCE(MENUIDD);
    }
    else
    {
        wcex.lpszMenuName = NULL;
    }

    wcex.lpszClassName = lpWindowClassName;

    if (ICON)
    {
        wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(ICON));
    }
    else
    {
        wcex.hIconSm = NULL;
    }

    RegisterClassEx(&wcex);
    HWND hWnd = CreateWindowEx(dwExStyle,
                               lpWindowClassName,
                               lpWindowName,
                               dwStyle,
                               x,
                               y,
                               nWidth,
                               nHeight,
                               hWndParent,
                               0,
                               GetModuleHandle(NULL),
                               (void*)pMain);
    return hWnd;
}



DWORD GetModuleDir(HMODULE hModule, LPTSTR   lpFilename, DWORD   nSize)
{
    if (GetModuleFileNameW(NULL, lpFilename, nSize) > 0)
    {
        //return 0;
    }

    //wchar_t path_buffer[_MAX_PATH];
    wchar_t drive[_MAX_DRIVE];
    wchar_t dir[_MAX_DIR];
    wchar_t fname[_MAX_FNAME];
    wchar_t ext[_MAX_EXT];
    _wsplitpath(lpFilename, drive, dir, fname, ext);

    lpFilename[0] = 0;
    wcscat(lpFilename, drive);
    wcscat(lpFilename, dir);

    return 1;
}

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
