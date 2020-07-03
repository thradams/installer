#include <stdio.h>
#include <direct.h>
#include "zip.h"
#include "..\installer\config.h"

/*
 Este projeto monta o files.zip que eh o arquivo se sera descmpactado na pasta destino
*/
int main()
{
    //roda no diretorio mais acima de todos, da solution
    chdir("../../");

    char cwd[MAX_PATH];
    if (_getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working dir: %s\n", cwd);
    }
    else {
        perror("getcwd() error");
        return 1;
    }


    //este programa monta um files.zip no diretorio do installer
    //juntando 2 arquivos

    struct finfo {
        const char* from;
        const char* dest;
    } files[] = { FILES };


    //joga o arquivo dentro do projeto do instalador
    struct zip_t* zip = zip_open("./installer/installer/files.zip", ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
    {
        for (int i = 0; i < sizeof(files) / sizeof(files[0]); i++)
        {
            zip_entry_open(zip, files[i].dest);
            {
                if (zip_entry_fwrite(zip, files[i].from) == -1)
                {
                    printf("ERROR %s\n", files[i].from);
                    break;
                }
            }
            zip_entry_close(zip);
        }        
    }
    zip_close(zip);

    printf("files.zip GENERATED\n");
}