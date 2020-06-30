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

int main()
{


    //vai para diretorio do installer
    chdir("../installer/");

    //monta amalgamation
    const char* files[] = {
        "zip.c",
        "WindowsDesktop.c",
        "Installer.c"
    };

    amalgamate("installer_.c", /*bHeaderMode*/false, files, (sizeof(files) / sizeof(files[0])));



    //monta header publico selecionando alguns headers para incluir
    const char* headers[] = {
        "installer.h"
    };


    amalgamate("installer_.h",  /*bHeaderMode*/true, headers, (sizeof(headers) / sizeof(headers[0])));
}