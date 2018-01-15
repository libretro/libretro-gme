    #include <stdio.h>
#include <stdlib.h>
#include <boolean.h>
#include <string.h>
#include "file/file_path.h"
#include "zlib.h"

#include "fileformat.h"
#include "unzip.h"
#include "log.h"

static bool get_files_from_zip(const char *path, file_data** *files)
{
    unzFile uf = NULL;
    unz_global_info64 gi;
    unz_file_info64 file_info;
    int i;
    char filename_inzip[256];
    //load zip content
    uf = unzOpen64(path);
    unzGetGlobalInfo64(uf,&gi);
    *files = malloc(sizeof(file_data*) * gi.number_entry);

    for(i=0;i<gi.number_entry;i++)
    {
        void* buf;
        int err;
        int bytes_read;
        uInt size_buf = 8192;
        (*files)[i] = malloc(sizeof(file_data));
        //read compressed file info
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
        if(err!=UNZ_OK)
        {
            return false;
        }
        //get file name in zip
        (*files)[i]->name = calloc(strlen(filename_inzip)+1,sizeof(char));
        strcpy((*files)[i]->name,filename_inzip);
        //allocate uncompressed data buffer
        (*files)[i]->length= sizeof(char) * file_info.uncompressed_size;
        (*files)[i]->data = (char*)malloc((*files)[i]->length);
        //setup buffer
        bytes_read = 0;
        buf = (void*)malloc(size_buf);
        if (buf==NULL)
            return false;
        //read file from zip
        err = unzOpenCurrentFilePassword(uf,NULL);
        if (err!=UNZ_OK)
            return false;
        //get data from zip
        do 
        {
            err = unzReadCurrentFile(uf,buf,size_buf);
            if(err<0)
                return false;
            if(err>0)
            {
                memcpy((*files)[i]->data + bytes_read,buf,err * sizeof(char));
                bytes_read += err;
            }
        } while (err>0);
        if(buf!=NULL)
            free(buf);
    }
    return true;
}

static bool get_gz_data(const char *path,int length, char **data)
{
    //local variables
    gzFile gfp;
    gfp = gzopen(path,"rb");
    if(!gfp)
        return false;
    gzread(gfp,*data,length);
    gzclose(gfp);
    return true;
}

bool get_file_data(const char *path,file_data ***files)
{
    //local variables
    FILE *fp;
    const char *bname;
    char *ext;
    unsigned long uncomp_length;
    //get file name and extension
    bname = path_basename(path);
    ext = strrchr(path,'.') +1;
    //get file data
    if(strcmp(ext,"zip")==0)
    {
        return get_files_from_zip(path,files);
    }
    else
    {
        file_data *fd;
        *files = malloc(sizeof(file_data*));
        fd = malloc(sizeof(file_data));
        fp = fopen(path,"rb");
        //get file length
        fseek (fp,0,SEEK_END);
        fd->length = ftell(fp);
        rewind(fp);
        //get file data
        fd->data = malloc(sizeof(char)*fd->length);
        fread(fd->data,1,fd->length,fp);
        fclose(fp);
        fd->name = calloc(strlen(bname)+1,sizeof(char));
        strcpy(fd->name,bname);
        if(strcmp(ext,"vgz")==0)
        {
            memcpy(&uncomp_length,&(fd->data[fd->length-4]),4);
            fd->length = (int)uncomp_length;
            free(fd->data);
            fd->data = malloc(sizeof(char)*fd->length);
            if(!get_gz_data(path,fd->length,&(fd->data)))
                return false;
        }
        (*files)[0] = fd;
        return true;
    }
}
