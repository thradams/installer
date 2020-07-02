
#include "zip.h"

/*
 Este projeto monta um zip com todos os fontes para serem usados em outro projeto
*/
int main()
{
    

    struct finfo {
        const char* from;
        const char* dest;
    } files[] = { 
        {"./installer/installer_.h", "installer/installer/installer_.h"},
        {"./installer/script.c", "installer/installer/script.c"},
        {"./installer/installer_.c", "installer/installer/installer_.c"},
        {"./installer/installer.rc", "installer/installer/installer.rc"},
        {"./installer/installer.ico", "installer/installer/installer.ico"},
        {"./installer/small.ico", "installer/installer/small.ico"},
        {"./installer/resource.h", "installer/installer/resource.h"},
        {"./installer/config.h", "installer/installer/config.h"},
        {"./installer/installer.vcxproj", "installer/installer/installer.vcxproj"},

        {"./make_install_zip/make_install_zip.c", "installer/make_install_zip/make_install_zip.c"},
        {"./make_install_zip/zip.c", "installer/make_install_zip/zip.c"},
        {"./make_install_zip/miniz.h", "installer/make_install_zip/miniz.h"},
        {"./make_install_zip/zip.h", "installer/make_install_zip/zip.h"},
        {"./make_install_zip/make_install_zip.vcxproj", "installer/make_install_zip/make_install_zip.vcxproj"},

        {"./uninstall/uninstall.c", "installer/uninstall/uninstall.c"},
        {"./uninstall/small.ico", "installer/uninstall/small.ico"},
        {"./uninstall/resource.h", "installer/uninstall/resource.h"},
        {"./uninstall/uninstall.ico", "installer/uninstall/uninstall.ico"},
        {"./uninstall/uninstall.rc", "installer/uninstall/uninstall.rc"},
        {"./uninstall/uninstall.h", "installer/uninstall/uninstall.h"},
        {"./uninstall/uninstall.vcxproj", "installer/uninstall/uninstall.vcxproj"}
        
        
    };

    //vai para diretorio do installer
    chdir("../");

    struct zip_t* zip = zip_open("installer_src.zip", ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
    {
        for (int i = 0; i < sizeof(files) / sizeof(files[0]); i++)
        {
            zip_entry_open(zip, files[i].dest);
            {
                zip_entry_fwrite(zip, files[i].from);
            }
            zip_entry_close(zip);
        }
    }
    zip_close(zip);
}