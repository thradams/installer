
//vou colocar os eventos arqui

#include "installer.h"
#include "process.h"

//https://www.codeguru.com/cpp/misc/misc/tools/article.php/c3851/ExeCreator-Utility.htm
//https://www.codeproject.com/Articles/8826/Easy-Installer

void WriteRegCommon()
{
    wchar_t uninst[MAX_PATH] = { 0 };
    wcscat(uninst, INSTDIR);
    wcscat(uninst, L"\\uninst.exe");
    //criar GUID?
    
    //https://docs.microsoft.com/en-us/windows/win32/msi/uninstall-registry-key
    WriteRegStr(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY, L"DisplayName", PRODUCT_NAME);
    WriteRegStr(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY, L"UninstallString", uninst);
    WriteRegStr(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY, L"DisplayIcon", uninst);
    WriteRegStr(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY, L"DisplayVersion", PRODUCT_VERSION);
    WriteRegStr(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY, L"URLInfoAbout", PRODUCT_WEB_SITE);
    WriteRegStr(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY, L"Publisher", PRODUCT_PUBLISHER);
    WriteRegStr(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY, L"InstallLocation", INSTDIR);
    WriteRegStr(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY, L"InstallSource", INSTDIR);
    WriteRegStr(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY, L"InstallSourceFile", INSTDIR);
    WriteRegStr(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY, L"NoModify", L"1");
    WriteRegStr(HKEY_LOCAL_MACHINE, PRODUCT_UNINST_KEY, L"NoRepair", L"1");
}

void OnFilesExtracted()
{

    //Verificar se o processo esta rodando
    //struct KillProcess kp = {0};
    //KillProcess_Init(&kp);
    //DWORD dwid;
    //KillProcess_FindProcess(&kp, L"MobileServer.exe", &dwid);
    //KillProcess_KillProcess(&kp, L"MobileServer.exe");
    //KillProcess_Destroy(&kp);
    

    //agora podemos gerar atalhos
    //rodar outros instaldores etc.
    WriteRegCommon();
}