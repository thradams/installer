#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
#define chdir _chdir
#else
#endif

#include "build.h"

//
// Este projeto gera a versao almagamate do installer
//
//

#define SIZEOFARRAY(a) (sizeof(a) / sizeof(a[0]))
int main()
{
    const char* donotexpand[] = { "config.h" };

    //vai para diretorio do installer
    chdir("../installer/");

    //monta amalgamation
    const char* files[] = {
        "zip.c",
        "WindowsDesktop.c",
        "Process.c",
        "Installer.c"
    };

    amalgamate("installer_.c",
               /*bHeaderMode*/false, 
               files, 
               SIZEOFARRAY(files),
               donotexpand,
               SIZEOFARRAY(donotexpand));

    //monta amalgamation header
    const char* headers[] = {
        "installer.h"        
    };

    amalgamate("installer_.h", /*bHeaderMode*/true, headers, (sizeof(headers) / sizeof(headers[0])), NULL , 0);

 }