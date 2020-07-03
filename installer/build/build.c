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
    //vai para diretorio do installer
    chdir("../installer/");

    char cwd[MAX_PATH];
    if (_getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working dir: %s\n", cwd);
    }
    else {
        perror("getcwd() error");
        return 1;
    }



    const char* donotexpand[] = { "config.h" };



    //monta amalgamation
    const char* files[] = {
        "zip.c",
        "WindowsDesktop.c",
        "reg.c",
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

    //////////////////////////////////////

    const char* uninstall_files[] = {
        "reg.c"
    };

    amalgamate("../uninstall/uninstall_.c",
               /*bHeaderMode*/false,
               uninstall_files,
               SIZEOFARRAY(uninstall_files),
               NULL,
               0);



    //monta amalgamation header
    const char* uninstall_headers[] = {
        "reg.h"
    };

    amalgamate("../uninstall/uninstall_.h", /*bHeaderMode*/true, uninstall_headers, SIZEOFARRAY(uninstall_headers), NULL, 0);
 }