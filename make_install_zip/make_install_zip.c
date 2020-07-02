
#include "zip.h"
#include "..\installer\config.h"
/*
 Este projeto monta o foo.zip que eh o arquivo se sera descmpactado na pasta destino
*/
int main()
{
    //este programa monta um foo.zip no diretorio do installer
    //juntando 2 arquivos

    struct finfo {
        const char* from;
        const char* dest;
    } files[] = { FILES };

    //vai para diretorio do installer
    chdir("../installer/");

    struct zip_t* zip = zip_open("foo.zip", ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
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