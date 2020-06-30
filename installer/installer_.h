





#include <direct.h>


#include <Windows.h>


#include <Shlobj.h>






#include <stdbool.h>


#include <assert.h>


#include <commctrl.h>

#define ASSERT assert

extern HINSTANCE s_hInstance;

void FillSolidRect(HDC hDC, LPCRECT lpRect, COLORREF clr);

BOOL CenterWindow(HWND hWnd, HWND hWndCenter);

INT_PTR ShowDialog(UINT idd,
                   void* p,
                   HWND hParent,
                   DLGPROC lpDialogFunc);

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

//MACROS

#define JOIN(A, B) A ## B

#define BEGIN_WIN_PROC(N) \
LRESULT CALLBACK JOIN(N, _ProcEx)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)\
{\
  struct MainWindow* pThis = 0;\
  if (message == WM_NCCREATE)\
  {\
    CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;\
    pThis = (struct MainWindow*)pCreate->lpCreateParams;\
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);\
    pThis->m_hWnd = hWnd;\
  }\
  else\
  {\
    pThis = (struct MainWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);\
  }\
 if (!pThis)\
 {\
  return DefWindowProc(hWnd, message, wParam, lParam); \
  }\
  \
  switch (message)\
  {




#define END_WIN_PROC\
  }\
  return DefWindowProc(hWnd, message, wParam, lParam);\
}



#define BEGIN_DLG_PROC(DIALOGNAME) \
LRESULT CALLBACK JOIN(DIALOGNAME, _ProcEx)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)\
{\
  struct DIALOGNAME* pThis = 0;\
  if (message == WM_INITDIALOG)\
  {\
      struct DIALOGNAME* p = (struct DIALOGNAME*)lParam;\
      SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)p);\
      p->m_hDlg = hWnd;\
      CenterWindow(hWnd, p->m_hParent);\
      JOIN(DIALOGNAME, _OnInit)(p);\
      return (INT_PTR)FALSE; \
  }\
  else\
  {\
    pThis = (struct DIALOGNAME*)GetWindowLongPtr(hWnd, GWLP_USERDATA);\
  }\
 if (!pThis)\
 {\
  return DefWindowProc(hWnd, message, wParam, lParam); \
  }\
  \
  switch (message)\
  {




#define END_DLG_PROC\
  }\
  return (INT_PTR)FALSE;\
}



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


/*WORD& DlgTemplateItemCount(_In_ DLGTEMPLATE* pTemplate)
{
    if (IsDialogEx(pTemplate))
        return (DLGTEMPLATEEX*)(pTemplate)->cDlgItems;
    else
        return pTemplate->cdit;
}*/

/*inline static const WORD& DlgTemplateItemCount(_In_ const DLGTEMPLATE* pTemplate)
{
    if (IsDialogEx(pTemplate))
        return (struct DLGTEMPLATEEX*)(pTemplate)->cDlgItems;
    else
        return pTemplate->cdit;
}
*/












////////////////////////////////////////////////////////////////////////////////////////////////

//Desenha em um DC de memoria
#define ONPAINTMEM(N)\
   case WM_PAINT:\
{\
 PAINTSTRUCT ps;\
 HDC m_hDCOriginal = BeginPaint(hWnd, &ps);\
 HDC m_hDC = CreateCompatibleDC(m_hDCOriginal);\
 RECT m_rcPaint = ps.rcPaint;\
 HBITMAP m_hBitmap = CreateCompatibleBitmap(m_hDCOriginal,\
                                           m_rcPaint.right - m_rcPaint.left,\
                                           m_rcPaint.bottom - m_rcPaint.top);\
 HBITMAP m_hBmpOld = (HBITMAP)SelectObject(m_hDC, m_hBitmap);\
 SetViewportOrgEx(m_hDC, -m_rcPaint.left, -m_rcPaint.top, 0);\
 JOIN(N, _OnPaint)(pThis, m_hDC, &ps.rcPaint);\
 BitBlt(m_hDCOriginal, m_rcPaint.left, m_rcPaint.top,\
        m_rcPaint.right - m_rcPaint.left, m_rcPaint.bottom - m_rcPaint.top,\
        m_hDC, m_rcPaint.left, m_rcPaint.top, SRCCOPY);\
 SelectObject(m_hDC, m_hBmpOld);\
 DeleteObject(m_hBitmap);\
 DeleteObject(m_hDC);\
 EndPaint(hWnd, &ps);\
 }\
 break;

//Desenha direto no DC
#define ONPAINT(N)\
    case WM_PAINT:\
    {\
      PAINTSTRUCT ps; \
      HDC hdc = BeginPaint(hWnd, &ps); \
        JOIN(N, _OnPaint)(pThis, hdc, &ps.rcPaint);\
        EndPaint(hWnd, &ps);\
        }\
        break;

#define ONCLOSE(N)\
         case WM_CLOSE:\
           JOIN(N, _OnClose)(pThis); \
         break;

#define ONTIMER(N)\
    case WM_TIMER:\
        JOIN(N, _OnTimer)(pThis, (DWORD)wParam);\
    break;

#define ONCREATE(N)\
    case WM_CREATE:\
        JOIN(N, _OnCreate)(pThis);\
    break;


#define ONDESTROY(N)\
    case WM_DESTROY:\
        JOIN(N, _OnDestroy)(pThis);\
    break;

#define ON_COMMAND(N)\
  case WM_COMMAND:\
      JOIN(N, _OnCommand)(pThis, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); \
 break;

BOOL ShowSelectFolderDialog(HWND hwndOwner, LPCTSTR lpszTitle, LPCTSTR startDir, TCHAR szDir[MAX_PATH]);




void SaveFile(DWORD idd);

HRESULT CreateShortCut(LPCWSTR lpszPathObj, LPCSTR lpszPathLink, LPCWSTR lpszDesc);
int mkdir_p(const char* path);

BOOL DeleteRegValue(HKEY hKeyParent, LPCTSTR pszSubkey, LPCTSTR pszValueName);

BOOL DeleteRegKey(HKEY hKeyParent, LPCTSTR pszSubkey);

BOOL WriteRegStr(HKEY hKeyParent, LPCTSTR pszSubkey, LPCTSTR pszKeyName, LPCTSTR pszValue);

BOOL ReadRegStr(HKEY hKeyParent, LPCTSTR pszSubkey, LPCTSTR pszKeyName, LPTSTR pszValue, ULONG* pnChars);



