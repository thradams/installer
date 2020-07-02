
#define DISPLAY_NAME L"Biscoito Display"
#define PRODUCT_NAME L"Biscoito"
#define PRODUCT_VERSION   L"1.2.3"
#define PRODUCT_PUBLISHER L"thradams"
#define PRODUCT_WEB_SITE L"https://github.com/thradams/"

#define PRODUCT_CODE L"{A9E770C4-FCF1-4E52-A3B4-44D394886A3A}"


/*HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{A9E770C4-FCF1-4E52-A3B4-44D394886A3A}
The product code is a GUID that is the principal identification of an application or product. For more information, see the ProductCode property. If significant changes are made to a product then the product code should also be changed to reflect this. It is not however a requirement that the product code be changed if the changes to the product are relatively minor.
https://docs.microsoft.com/en-us/windows/win32/msi/product-codes
HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{A9E770C4-FCF1-4E52-A3B4-44D394886A3A}
*/

//#define PRODUCT_UNINST_KEY L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" PRODUCT_CODE
#define PRODUCT_UNINST_KEY L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" PRODUCT_CODE



#define FILES\
    {"small.ico", "small.ico"},\
    {"installer.ico", "folder/installer.ico"},\
    {"debug/uninstall.exe", "uninstall.exe" }



