
#include "zip.h"

int main()
{
    //este programa monta um foo.zip no diretorio do installer
    //juntando 2 arquivos

    struct zip_t* zip = zip_open("..\\installer\\foo.zip", ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
    {
        //zip_entry_open(zip, "folder");
        {
            zip_entry_open(zip, "folder/makezip.c");
            {
                zip_entry_fwrite(zip, "makezip.c");
            }
            zip_entry_close(zip);

            zip_entry_open(zip, "folder/zip.h");
            {
                zip_entry_fwrite(zip, "zip.h");
            }
            zip_entry_close(zip);

        }
      //  zip_entry_close(zip);

        

      
    }
    zip_close(zip);
}