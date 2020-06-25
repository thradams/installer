#include "WindowsDesktop.h"
#include <shlobj_core.h>
#include <Shlobj.h>

#pragma comment(lib, "Comctl32.lib")


#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

struct DialogTemplate
{
    HGLOBAL m_hTemplate;
    DWORD m_dwTemplateSize;
    BOOL m_bSystemFont;
};

void DrawBitmap(HDC hdc, HBITMAP hbm, int Left, int Top)
{
    BOOL f;
    HDC hdcBits;
    BITMAP bm;
    hdcBits = CreateCompatibleDC(hdc);
    GetObject(hbm, sizeof(BITMAP), &bm);
    SelectObject(hdcBits, hbm);
    f = BitBlt(hdc, Left, Top, bm.bmWidth, bm.bmHeight, hdcBits, 0, 0, SRCCOPY);
    DeleteDC(hdcBits);
}


void DrawImage(HDC hdc, RECT rc, LPCWSTR pszResourceName)
{
    UINT type = LR_CREATEDIBSECTION | LR_DEFAULTCOLOR;//LR_CREATEDIBSECTION | LR_DEFAULTSIZE;
    HBITMAP hBitmap = 0;
    hBitmap = (HBITMAP)LoadImage(GetModuleHandle(0),
                                 pszResourceName,
                                 IMAGE_BITMAP,
                                 0,
                                 0,
                                 type);
    if (hBitmap)
    {
        BITMAP bm;
        GetObject(hBitmap, sizeof(BITMAP), &bm);

        DrawBitmap(hdc, hBitmap, rc.left, rc.top);
        DeleteObject(hBitmap);
    }
}

void FillSolidRect(HDC hDC, LPCRECT lpRect, COLORREF clr)
{
    COLORREF clrOld = SetBkColor(hDC, clr);

    if (clrOld != CLR_INVALID)
    {
        ExtTextOut(hDC, 0, 0, ETO_OPAQUE, lpRect, NULL, 0, NULL);
        SetBkColor(hDC, clrOld);
    }
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

static BOOL IsDialogEx(const DLGTEMPLATE* pTemplate)
{
    return ((struct DLGTEMPLATEEX*)pTemplate)->signature == 0xFFFF;
}


static inline int FontAttrSize(BOOL bDialogEx)
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

/*
BOOL HasFont(HGLOBAL m_hTemplate)
{
    if (m_hTemplate == NULL)
    {
        ASSERT(false);
        return FALSE;
    }
    DLGTEMPLATE* pTemplate = (DLGTEMPLATE*)GlobalLock(m_hTemplate);
    BOOL bHasFont = HasFont(pTemplate);
    GlobalUnlock(m_hTemplate);
    return bHasFont;
}
*/

BOOL DialogTemplate_SetFont(struct DialogTemplate* pThis, LPCWSTR lpFaceName, WORD nFontSize)
{
    ASSERT(pThis->m_hTemplate != NULL);

    if (pThis->m_dwTemplateSize == 0)
        return FALSE;

    DLGTEMPLATE* pTemplate = (DLGTEMPLATE*)GlobalLock(pThis->m_hTemplate);
    if (pTemplate == NULL)
    {
        ASSERT(false);
        return false;
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

    if (cbNew < cbFontAttr)
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

extern HINSTANCE s_hInstance = 0;
BOOL DialogTemplate_Load(struct DialogTemplate* pThis, HINSTANCE hInst, LPCTSTR lpDialogTemplateID);
void GetSystemIconFont(wchar_t* strFontNameOut, int* nPointSize);

INT_PTR ShowDialog(DWORD dlgID,
                   void* p,
                   HWND hParent,
                   DLGPROC lpDialogFunc)
{
    struct DialogTemplate dlt = { 0 };
    HINSTANCE hInstance = s_hInstance;
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



inline DWORD GetStyle(HWND hWnd)
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



BOOL ShowSelectFolderDialog(HWND hwndOwner, LPCTSTR lpszTitle, LPCTSTR startDir, TCHAR szDir[MAX_PATH])
{
    //startDir pode ser null que eh tratado
    BROWSEINFO bInfo;
    bInfo.hwndOwner = hwndOwner;
    bInfo.pidlRoot = NULL;
    bInfo.pszDisplayName = szDir; // Address of a buffer to receive the display name of the folder selected by the user
    bInfo.lpszTitle = lpszTitle;
    bInfo.ulFlags = BIF_NEWDIALOGSTYLE;
    bInfo.lpfn = BrowseCallbackProc;
    bInfo.lParam = startDir;
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
        DialogTemplate_SetFont(&dlt, strFontName, (WORD)nPointSize);
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

