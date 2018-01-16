#include <stdio.h>
#include <stdlib.h>
#include <boolean.h>
#include <string.h>
#include "file/file_path.h"
#include "zlib.h"

#include "fileformat.h"
#include "unzip.h"
#include "log.h"

static const char *gme_allowed_exts[] = {
    "ay","AY",
    "gbs","GBS",
    "gym","GYM",
    "hes","HES",
    "kss","KSS",
    "nsf","NSF",
    "nsfe","NSFE",
    "sap","SAP",
    "spc","SPC",
    "vgm","VGM",
    "vgz","VGZ"
};

static bool is_gme_allowed_ext(char *ext)
{
    int i;
    int arr_length;
    arr_length = sizeof(gme_allowed_exts) / sizeof(char*);
    for(i=0;i<arr_length;i++)
    {
        if(strcmp(ext,gme_allowed_exts[i])==0)
        {
            return true;
        }
    }
    return false;
}

static bool uncompress_file_data(file_data** fd)
{
    int srcLen,dstLen;
    file_data* src_fd = *fd;
    file_data* dest_fd;
    srcLen = src_fd->length;
    memcpy(&dstLen,&(src_fd->data[src_fd->length-4]),4);
    dest_fd = malloc(sizeof(file_data));
    dest_fd->length = dstLen;
    dest_fd->name = calloc(strlen(src_fd->name)+1,sizeof(char));
    strcpy(dest_fd->name,src_fd->name);
    dest_fd->data = malloc(dstLen * sizeof(char));
    z_stream strm  = {0};
    strm.total_in  = strm.avail_in  = srcLen;
    strm.total_out = strm.avail_out = dstLen;
    strm.next_in   = (Bytef *) src_fd->data;
    strm.next_out  = (Bytef *) dest_fd->data;

    strm.zalloc = Z_NULL;
    strm.zfree  = Z_NULL;
    strm.opaque = Z_NULL;

    int err = -1;

    err = inflateInit2(&strm, (15 + 32)); //15 window bits, and the +32 tells zlib to to detect if using gzip or zlib
    if (err == Z_OK) {
        err = inflate(&strm, Z_FINISH);
        if (err != Z_STREAM_END) {
             inflateEnd(&strm);
             return false;
        }
    }
    else {
        inflateEnd(&strm);
        return false;
    }
    inflateEnd(&strm);
    free(src_fd->data);
    free(src_fd->name);
    free(src_fd);
    *fd = dest_fd;
    return true;
}

static bool get_files_from_zip(const char *path, file_data ***dest_files, int *dest_numfiles)
{
    unzFile uf = NULL;
    unz_global_info64 gi;
    unz_file_info64 file_info;
    int i;
    char filename_inzip[256];
    char *ext;
    file_data **files;
    int numfiles,position;
    //load zip content
    uf = unzOpen64(path);
    unzGetGlobalInfo64(uf,&gi);
    numfiles = (int)gi.number_entry;
    files = malloc(sizeof(file_data*) * numfiles);
    position = 0;
    for(i=0;i<gi.number_entry;i++)
    {
        void* buf;
        int err;
        int bytes_read;
        uInt size_buf = 8192;
        //read compressed file info
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
        if(err!=UNZ_OK)
        {
            return false;
        }
        if(filename_inzip[file_info.size_filename -1]=='/')
            ext = strrchr(filename_inzip,'/');
        else
            ext = strrchr(filename_inzip,'.') + 1;
        if(is_gme_allowed_ext(ext))
        {
            //get file name in zip
            files[position] = malloc(sizeof(file_data));
            files[position]->name = calloc(strlen(filename_inzip)+1,sizeof(char));
            strcpy(files[position]->name,filename_inzip);
            //allocate uncompressed data buffer
            files[position]->length= sizeof(char) * file_info.uncompressed_size;
            files[position]->data = (char*)malloc(files[position]->length);
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
                    memcpy(files[position]->data + bytes_read,buf,err * sizeof(char));
                    bytes_read += err;
                }
            } while (err>0);
            if(buf!=NULL)
                free(buf);

            if(strcmp(ext,"vgz")==0)
                if(!uncompress_file_data(&(files[position])))
                    return false;
            position++;
        }
        else
        {
            numfiles--;
        }
        if ((i+1)<gi.number_entry)
            unzGoToNextFile(uf);
    }
    files = realloc(files,sizeof(file_data*) * numfiles);
    *dest_files = files;
    *dest_numfiles = numfiles;
    return true;
}

bool get_file_data(const char *path,file_data ***dest_files, int *dest_numfiles)
{
    //local variables
    FILE *fp;
    const char *bname;
    char *ext;
    file_data **files;
    //get file name and extension
    bname = path_basename(path);
    ext = strrchr(path,'.') +1;
    //get file data
    if(strcmp(ext,"zip")==0)
    {
        return get_files_from_zip(path,dest_files,dest_numfiles);
    }
    else
    {
        file_data *fd;
        files = malloc(sizeof(file_data*));
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
            if(!uncompress_file_data(&fd))
                return false;
        }
        files[0] = fd;
        *dest_files = files;
        *dest_numfiles = 1;
        return true;
    }
}
