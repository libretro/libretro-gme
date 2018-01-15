#include <stdio.h>
#include <stdlib.h>
#include <boolean.h>
#include <string.h>
#include "file/file_path.h"

#include "playlist.h"
#include "log.h"

bool get_playlist(const char *path, playlist **pl)
{
	//local variables
	int i,j,position;
	Music_Emu *temp_emu;
	gme_file_data *gfd;
	gme_err_t err_msg;
	//init playlist
	*pl = malloc(sizeof(playlist));
	(*pl)->num_files = 0;
	(*pl)->files = NULL;
	(*pl)->num_tracks = 0;
	(*pl)->tracks = NULL;
	//load libretro content file
	if(get_playlist_gme_files(path,&((*pl)->files),&((*pl)->num_files),&((*pl)->num_tracks)))
	{
		(*pl)->tracks = malloc(sizeof(gme_track_data*) * (*pl)->num_tracks);
		//read tracks
		position = 0;
		for(i=0;i<(*pl)->num_files;i++)
		{
			gfd = (*pl)->files[i];
			temp_emu = gme_new_emu(gfd->file_type,gme_info_only);
			err_msg = gme_load_data(temp_emu,gfd->data,gfd->length);
			if(err_msg==NULL)
			{
				for(j=0;j<gfd->num_tracks;j++)
				{
					if(get_track_data(temp_emu,i,j,gfd->name,&((*pl)->tracks[position])))
					{
						position++;
					}
				}				
			}
			else
				return false;
		}
	}
	else
		return false;
	return true;
}

bool get_playlist_gme_files(const char *path,gme_file_data ***gme_files,int *num_files, int *num_tracks)
{
	bool success;
	int i;
	success = true;
	file_data **files = NULL;
	if(get_file_data(path,&files)) {
		*num_files = sizeof(files) / sizeof(file_data*);
		*gme_files = malloc(sizeof(gme_file_data*) * *num_files);
		for(i=0;i< *num_files;i++)
		{
			(*gme_files)[i] = NULL;
			if(!get_gme_file_data(files[i],&((*gme_files)[i])))
			{
				success = false;
				break;
			}
			free(files[i]);
			if((*gme_files)[i]==NULL)
			{
				success = false;
				break;
			}
			*num_tracks += (*gme_files)[i]->num_tracks;
		}
		free(files);
	}
	else
		success = false;
	return success;
}

bool get_gme_file_data(file_data *fd,gme_file_data **gfd)
{
	Music_Emu* temp_emu;
	gme_err_t err_msg;
	char *ext;
	(*gfd) = malloc(sizeof(gme_file_data));
	//set playlist type
	ext = strrchr(fd->name,'.') +1;
	//check extension to determine player type
	if(strcmp(ext,"ay")==0)
		(*gfd)->file_type = gme_ay_type;
	else if(strcmp(ext,"gbs")==0)
		(*gfd)->file_type = gme_gbs_type;
	else if(strcmp(ext,"gym")==0)
		(*gfd)->file_type = gme_gym_type;
	else if(strcmp(ext,"hes")==0)
		(*gfd)->file_type = gme_hes_type;
	else if(strcmp(ext,"kss")==0)
		(*gfd)->file_type = gme_kss_type;
	else if(strcmp(ext,"nsf")==0)
		(*gfd)->file_type = gme_nsf_type;
	else if(strcmp(ext,"nsfe")==0)
		(*gfd)->file_type = gme_nsfe_type;
	else if(strcmp(ext,"sap")==0)
		(*gfd)->file_type = gme_sap_type;
	else if(strcmp(ext,"spc") == 0)
		(*gfd)->file_type = gme_spc_type;
	else if(strcmp(ext,"vgm") == 0)
		(*gfd)->file_type = gme_vgm_type;
	else if(strcmp(ext,"vgz") == 0)
		(*gfd)->file_type = gme_vgz_type;
	else
		return false;
	temp_emu = gme_new_emu((*gfd)->file_type,gme_info_only);
	err_msg = gme_load_data(temp_emu,fd->data,fd->length);
	if(err_msg==NULL)
		(*gfd)->num_tracks = gme_track_count(temp_emu);
	else
	{
		handle_error(err_msg);
		return false;
	}
	gme_delete( temp_emu );
	//deep copy file data
	(*gfd)->name = calloc(strlen(fd->name+1),sizeof(char));
	strcpy((*gfd)->name,fd->name);
	(*gfd)->data = malloc(fd->length * sizeof(char));
	memcpy((*gfd)->data,fd->data,fd->length);
	(*gfd)->length = fd->length;
	return true;
}

bool get_track_data(Music_Emu* emu,int trackid, int fileid, const char *filename,gme_track_data **gtd)
{
	gme_info_t* ti;
	*gtd = malloc(sizeof(gme_track_data));
	(*gtd)->file_id = fileid;
	(*gtd)->track_id = trackid;
	gme_track_info(emu, &ti,trackid);
	//game name
	if(strcmp(ti->game,"")==0)
	{
		(*gtd)->game_name = calloc(strlen(filename)+1,sizeof(char));
		strcpy((*gtd)->game_name,filename);
	}
	else
	{
		(*gtd)->game_name = calloc(strlen(ti->game)+1,sizeof(char));
		strcpy((*gtd)->game_name,ti->game);
	}
	//track length
	(*gtd)->track_length = ti->length;
	if ( (*gtd)->track_length <= 0 )
		(*gtd)->track_length = ti->intro_length + ti->loop_length * 2;
	if ( (*gtd)->track_length <= 0 )
		(*gtd)->track_length = (long) (2.5 * 60 * 1000);
	//track name
	if(strcmp(ti->song,"") == 0)
	{
		(*gtd)->track_name = calloc(10,sizeof(char));
		sprintf((*gtd)->track_name, "Track %i",trackid+1);
	}
	else
	{
		(*gtd)->track_name = calloc(strlen(ti->song)+1,sizeof(char));
		strcpy((*gtd)->track_name, ti->song);
	}
	gme_free_info(ti);
	return true;
}

bool cleanup_playlist(playlist *playlist)	
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
	return true;
}
