#include <stdio.h>
#include <stdlib.h>
#include <boolean.h>
#include <string.h>
#include "file/file_path.h"

#include "playlist.h"
#include "unzip.h"
#include "log.h"

playlist* get_playlist(const char *path)
{
	//local variables
	playlist *pl = NULL;
	int i,j,position;
	Music_Emu *temp_emu;
	gme_file_data *gfd;
	gme_err_t err_msg;
	//init playlist
	pl = malloc(sizeof(playlist));
	pl->num_files = 0;
	pl->files = NULL;
	pl->num_tracks = 0;
	pl->tracks = NULL;
	//load libretro content file
	pl->files = get_playlist_gme_files(path);
	if(pl->files)
	{
		pl->num_files = sizeof(pl->files) / sizeof(gme_file_data*);
		//count tracks
		for(i=0;i<pl->num_files;i++)
		{
			pl->num_tracks += pl->files[i]->num_tracks;
		}
		pl->tracks = malloc(sizeof(gme_track_data*) * pl->num_tracks);
		//read tracks
		position = 0;
		for(i=0;i<pl->num_files;i++)
		{
			gfd = pl->files[i];
			temp_emu = gme_new_emu(gfd->file_type,gme_info_only);
			err_msg = gme_load_data(temp_emu,gfd->data,gfd->length);
			if(err_msg==NULL)
			{
				for(j=0;j<gfd->num_tracks;j++)
				{
					pl->tracks[position] = get_track_data(temp_emu,i,j,gfd->name);
					position++;
				}				
			}
		}
	}
	else
		return NULL;
	return pl;
}

gme_file_data** get_playlist_gme_files(const char *path)
{
	int num_files;
	int i;
	gme_file_data **gme_files = NULL;
	file_data **files = NULL;
	char *ext;
	ext = strrchr(path,'.') +1;
	if(strcmp(ext,"zip")==0)
	{
		//load files in zip
		files = get_files_from_zip(path);
		if(files) {
			num_files = sizeof(files) / sizeof(gme_file_data*);
			gme_files = malloc(sizeof(gme_file_data*)*num_files);
			for(i=0;i<num_files;i++)
			{
				gme_files[i] = get_gme_file_data(files[i]);
				free(files[i]);
				if(gme_files[i]==NULL)
					return NULL;
			}
			free(files);
		}
		else
			return NULL;
	}
	else
	{
		files = malloc(sizeof(file_data*));
		files[0] = get_file_data(path);
		if(files)
		{
			gme_files = malloc(sizeof(gme_file_data*));
			gme_files[0] = get_gme_file_data(files[0]);
			free(files);
		}
		else
			return NULL;
	}
	return gme_files;
}

gme_file_data* get_gme_file_data(file_data *fd)
{
	Music_Emu* temp_emu;
	gme_err_t err_msg;
	char *ext;
	gme_file_data *gfd;
	gfd = malloc(sizeof(gme_file_data));
	//set playlist type
	ext = strrchr(fd->name,'.') +1;
	//check extension to determine player type
	if(strcmp(ext,"ay")==0)
		gfd->file_type = gme_ay_type;
	else if(strcmp(ext,"gbs")==0)
		gfd->file_type = gme_gbs_type;
	else if(strcmp(ext,"gym")==0)
		gfd->file_type = gme_gym_type;
	else if(strcmp(ext,"hes")==0)
		gfd->file_type = gme_hes_type;
	else if(strcmp(ext,"kss")==0)
		gfd->file_type = gme_kss_type;
	else if(strcmp(ext,"nsf")==0)
		gfd->file_type = gme_nsf_type;
	else if(strcmp(ext,"nsfe")==0)
		gfd->file_type = gme_nsfe_type;
	else if(strcmp(ext,"sap")==0)
		gfd->file_type = gme_sap_type;
	else if(strcmp(ext,"spc") == 0)
		gfd->file_type = gme_spc_type;
	else if(strcmp(ext,"vgm") == 0)
		gfd->file_type = gme_vgm_type;
	else if(strcmp(ext,"vgz") == 0)
		gfd->file_type = gme_vgz_type;
	temp_emu = gme_new_emu(gfd->file_type,gme_info_only);
	err_msg = gme_load_data(temp_emu,fd->data,fd->length);
	if(err_msg==NULL)
		gfd->num_tracks = gme_track_count(temp_emu);
	else
	{
		handle_error(err_msg);
		return NULL;
	}
	gme_delete( temp_emu );
	//deep copy file data
	gfd->name = calloc(strlen(fd->name+1),sizeof(char));
	strcpy(gfd->name,fd->name);
	gfd->data = malloc(fd->length * sizeof(char));
	memcpy(gfd->data,fd->data,fd->length);
	gfd->length = fd->length;
	return gfd;
}

file_data** get_files_from_zip(const char *path)
{
    unzFile uf = NULL;
    unz_global_info64 gi;
    unz_file_info64 file_info;
    int i;
    file_data **filedatas;
    char filename_inzip[256];
    //load zip content
    uf = unzOpen64(path);
    unzGetGlobalInfo64(uf,&gi);
    filedatas = malloc(sizeof(file_data*) * gi.number_entry);

    for(i=0;i<gi.number_entry;i++)
    {
        void* buf;
        int err;
        int bytes_read;
        uInt size_buf = 8192;
        filedatas[i] = malloc(sizeof(file_data));
        //read compressed file info
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
        if(err!=UNZ_OK)
        {
            return NULL;
        }
        //get file name in zip
        filedatas[i]->name = calloc(strlen(filename_inzip)+1,sizeof(char));
        strcpy(filedatas[i]->name,filename_inzip);
        //allocate uncompressed data buffer
        filedatas[i]->length= sizeof(char) * file_info.uncompressed_size;
        filedatas[i]->data = (char*)malloc(filedatas[i]->length);
        //setup buffer
        bytes_read = 0;
        buf = (void*)malloc(size_buf);
        if (buf==NULL)
            return NULL;
        //read file from zip
        err = unzOpenCurrentFilePassword(uf,NULL);
        if (err!=UNZ_OK)
            return NULL;
        //get data from zip
        do 
        {
            err = unzReadCurrentFile(uf,buf,size_buf);
            if(err<0)
                return NULL;
            if(err>0)
            {
                memcpy(filedatas[i]->data + bytes_read,buf,err * sizeof(char));
                bytes_read += err;
            }
        } while (err>0);
        if(buf!=NULL)
            free(buf);
    }
    return filedatas;
}

file_data* get_file_data(const char *path)
{
	//local variables
	file_data *fd;
	FILE *fp;	
	const char *bname;
	//get file name
	bname = path_basename(path);
	//get file data
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
	return fd;
}

gme_track_data* get_track_data(Music_Emu* emu,int trackid, int fileid, const char *filename)
{
	gme_info_t* ti;
	gme_track_data* gtd;
	gtd = malloc(sizeof(gme_track_data));
	gtd->file_id = fileid;
	gtd->track_id = trackid;
	gme_track_info(emu, &ti,trackid);
	//game name
	if(strcmp(ti->game,"")==0)
	{
		gtd->game_name = calloc(strlen(filename)+1,sizeof(char));
		strcpy(gtd->game_name,filename);
	}
	else
	{
		gtd->game_name = calloc(strlen(ti->game)+1,sizeof(char));
		strcpy(gtd->game_name,ti->game);
	}
	//track length
	gtd->track_length = ti->length;
	if ( gtd->track_length <= 0 )
		gtd->track_length = ti->intro_length + ti->loop_length * 2;
	if ( gtd->track_length <= 0 )
		gtd->track_length = (long) (2.5 * 60 * 1000);
	//track name
	if(strcmp(ti->song,"") == 0)
	{
		gtd->track_name = calloc(10,sizeof(char));
		sprintf(gtd->track_name, "Track %i",trackid+1);
	}
	else
	{
		gtd->track_name = calloc(strlen(ti->song)+1,sizeof(char));
		strcpy(gtd->track_name, ti->song);
	}
	gme_free_info(ti);
	return gtd;
}
void cleanup_playlist(playlist *playlist)
{
	int i;
	if(playlist->tracks!=NULL)
	{
		for(i=0;i<playlist->num_tracks;i++)
      	{
         	if(playlist->tracks[i] != NULL)
         	{
				if(playlist->tracks[i]->game_name != NULL)
	            	free(playlist->tracks[i]->game_name);
				if(playlist->tracks[i]->track_name != NULL)
	            	free(playlist->tracks[i]->track_name);
	         	free(playlist->tracks[i]);
         	}
      	}
      	free(playlist->tracks);
	}
	if(playlist->files!=NULL)
	{
		for(i=0;i<playlist->num_files;i++)
		{
			if(playlist->files[i] != NULL)
			{
				if(playlist->files[i]->data != NULL)
					free(playlist->files[i]->data);
				if(playlist->files[i]->name != NULL)
					free(playlist->files[i]->name);
				free(playlist->files[i]);
			}
		}
		free(playlist->files);
	}
	free(playlist);
}
