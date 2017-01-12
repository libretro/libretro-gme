#include <stdio.h>
#include <stdlib.h>
#include <boolean.h>
#include <string.h>

#include "playlist.h"
#include "unzip.h"
#include "log.h"

playlist *load_playlist(const char *path, long sample_rate)
{
	char *ext;
	playlist *new_playlist = NULL;
	new_playlist = malloc(sizeof(playlist));
	new_playlist->filename = malloc(strlen(path) + 1);
	new_playlist->playlist_data = NULL;
	new_playlist->num_tracks = 0;
	new_playlist->current_track = 0;
	new_playlist->playlist_data_length = 0;
	strcpy(new_playlist->filename,path);
	ext = strrchr(path,'.') +1;
	if(strcmp(ext,"zip")==0)
	{
		load_zip(new_playlist,sample_rate);
	}
	else
	{
		load_gme_file(new_playlist,sample_rate);
	}
	return new_playlist;
}

void load_gme_file(playlist *playlist, long sample_rate)
{
	FILE *fp;
	Music_Emu* temp_emu;
	gme_info_t* track_info;
	gme_type_t track_type;
	playlist_entry *entry;
	int i;
	char *ext;
	//set playlist type
	playlist->type = GME_FILE;
	ext = strrchr(playlist->filename,'.') +1;
	//check extension to determine player type
	if(strcmp(ext,"ay")==0)
		track_type = gme_ay_type;
	else if(strcmp(ext,"gbs")==0)
		track_type = gme_gbs_type;
	else if(strcmp(ext,"gym")==0)
		track_type = gme_gym_type;
	else if(strcmp(ext,"hes")==0)
		track_type = gme_hes_type;
	else if(strcmp(ext,"kss")==0)
		track_type = gme_kss_type;
	else if(strcmp(ext,"nsf")==0)
		track_type = gme_nsf_type;
	else if(strcmp(ext,"nsfe")==0)
		track_type = gme_nsfe_type;
	else if(strcmp(ext,"sap")==0)
		track_type = gme_sap_type;
	else if(strcmp(ext,"spc") == 0)
		track_type = gme_spc_type;
	else if(strcmp(ext,"vgm") == 0)
		track_type = gme_vgm_type;
	else if(strcmp(ext,"vgz") == 0)
		track_type = gme_vgz_type;
	//read file data
	fp = fopen(playlist->filename,"rb");
	fseek (fp,0,SEEK_END);
	playlist->playlist_data_length = ftell(fp);
	rewind(fp);
	playlist->playlist_data = malloc(playlist->playlist_data_length);
	fread(playlist->playlist_data,1,playlist->playlist_data_length,fp);
	fclose(fp);
	playlist->game_name = NULL;
	//get track info
	if(track_type !=NULL)
	{
		temp_emu = gme_new_emu(track_type,gme_info_only);
		gme_load_data(temp_emu,playlist->playlist_data,playlist->playlist_data_length);
		playlist->num_tracks = gme_track_count(temp_emu);
		playlist->current_track = 0;
		for(i=0;i<playlist->num_tracks;i++)
		{
			playlist->entries[i] = malloc(sizeof(playlist_entry));
			entry = playlist->entries[i];
			gme_track_info(temp_emu, &track_info,i);
			if(strcmp(track_info->game,"")==0)
			{
				char *basename = strrchr(playlist->filename,'/') +1;
				playlist->game_name = malloc((strlen(basename)+1) * sizeof(char));
				strcpy(playlist->game_name,basename);
			}
			else
			{
				playlist->game_name = malloc((strlen(track_info->game)+1) * sizeof(char));
				strcpy(playlist->game_name,track_info->game);
			}
			entry->track_number = i + 1;
			entry->track_data_length = 0;
			entry->track_data = NULL;
			entry->track_type = track_type;
			entry->track_length = track_info->length;
			if ( entry->track_length <= 0 )
				entry->track_length = track_info->intro_length + track_info->loop_length * 2;
			if ( entry->track_length <= 0 )
				entry->track_length = (long) (2.5 * 60 * 1000);
			if(strcmp(track_info->song,"") == 0)
			{
				entry->track_name = calloc(10,sizeof(char));
				sprintf(entry->track_name, "Track %i",i+1);
			}
			else
			{
				entry->track_name = calloc(strlen(track_info->song)+1,sizeof(char));
				strcpy(entry->track_name, track_info->song);
			}
			gme_free_info(track_info);
		}
		gme_delete( temp_emu );
	}
}

void load_zip(playlist *playlist, long sample_rate)
{
	char *ext;
	unzFile uf = NULL;
	unz_global_info64 gi;
	unz_file_info64 file_info;
	char filename_inzip[256];
	char err_msg[256];
	int i;
	playlist_entry *entry;
	//set playlist type
	playlist->type = GME_ZIP;
	playlist->playlist_data = NULL;
	playlist->playlist_data_length = 0;
	playlist->game_name = NULL;
	//load zip content
	uf = unzOpen64(playlist->filename);
	unzGetGlobalInfo64(uf,&gi);
	playlist->num_tracks = gi.number_entry;
	playlist->current_track = 0;
	for(i=0;i<gi.number_entry;i++)
	{
		Music_Emu* temp_emu;
		gme_info_t* track_info;
		void* buf;
		int err;
		int bytes_read;
		uInt size_buf = 8192;
		playlist->entries[i] = malloc(sizeof(playlist_entry));
		entry = playlist->entries[i];
		//read compressed file info
		err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
		if(err!=UNZ_OK)
		{
			sprintf(err_msg,"error %d with zipfile in unzGetCurrentFileInfo\n",err);
			handle_error(err_msg);
		}
		//initialize entry
		entry->track_number = i + 1;
		entry->track_data_length = file_info.uncompressed_size;
		entry->track_data = (char*)malloc(sizeof(char)*file_info.uncompressed_size);
		//check extension to determine player type
		ext = strrchr(filename_inzip,'.') +1;
		if(strcmp(ext,"ay")==0)
			entry->track_type = gme_ay_type;
		else if(strcmp(ext,"gbs")==0)
			entry->track_type = gme_gbs_type;
		else if(strcmp(ext,"gym")==0)
			entry->track_type = gme_gym_type;
		else if(strcmp(ext,"hes")==0)
			entry->track_type = gme_hes_type;
		else if(strcmp(ext,"kss")==0)
			entry->track_type = gme_kss_type;
		else if(strcmp(ext,"nsf")==0)
			entry->track_type = gme_nsf_type;
		else if(strcmp(ext,"nsfe")==0)
			entry->track_type = gme_nsfe_type;
		else if(strcmp(ext,"sap")==0)
			entry->track_type = gme_sap_type;
		else if(strcmp(ext,"spc") == 0)
			entry->track_type = gme_spc_type;
		else if(strcmp(ext,"vgm") == 0)
			entry->track_type = gme_vgm_type;
		else if(strcmp(ext,"vgz") == 0)
			entry->track_type = gme_vgz_type;
		else
			entry->track_type = NULL;
		//setup buffer
		bytes_read = 0;
		buf = (void*)malloc(size_buf);
		if (buf==NULL)
		{
				strcpy(err_msg,"error allocating memory");
				handle_error(err_msg);
		}
		//read file from zip
		err = unzOpenCurrentFilePassword(uf,NULL);
		if (err!=UNZ_OK)
		{
				sprintf(err_msg,"error %d with zipfile in unzOpenCurrentFilePassword\n",err);
				handle_error(err_msg);
		}
		//get data from zip
		do {
			err = unzReadCurrentFile(uf,buf,size_buf);
			if(err<0)
			{
				sprintf(err_msg,"error %d with zipfile in unzReadCurrentFile\n",err);
				handle_error(err_msg);
			}
			if(err>0)
			{
				memcpy(entry->track_data + bytes_read,buf,err * sizeof(char));
				bytes_read += err;
			}
		} while (err>0);
		if(buf!=NULL)
			free(buf);
		//get track info
		entry->track_length = 0;
		if(entry->track_type!=NULL)
		{
			temp_emu = gme_new_emu(entry->track_type,gme_info_only);
			gme_load_data(temp_emu,entry->track_data,entry->track_data_length);
			gme_track_info(temp_emu, &track_info,0);
			if (playlist->game_name == NULL)
			{
				if(strcmp(track_info->game,"")==0)
				{
					char *basename = strrchr(playlist->filename,'/') +1;
					playlist->game_name = malloc((strlen(basename)+1) * sizeof(char));
					strcpy(playlist->game_name,basename);
				}
				else
				{
					playlist->game_name = malloc((strlen(track_info->game)+1) * sizeof(char));
					strcpy(playlist->game_name,track_info->game);
				}
			}
			entry->track_length = track_info->length;
			if ( entry->track_length <= 0 )
				entry->track_length = track_info->intro_length + track_info->loop_length * 2;
			if ( entry->track_length <= 0 )
				entry->track_length = (long) (2.5 * 60 * 1000);
			if(strcmp(track_info->song,"") == 0)
			{
				entry->track_name = calloc(10,sizeof(char));
				sprintf(entry->track_name, "Track %i",i);
			}
			else
			{
				entry->track_name = calloc(strlen(track_info->song)+1,sizeof(char));
				strcpy(entry->track_name, track_info->song);
			}
			gme_free_info(track_info);
			gme_delete( temp_emu );
		}
		//read next file in zip
		if ((i+1)<gi.number_entry)
    {
    	unzGoToNextFile(uf);
		}
	}
	//close zip
	unzCloseCurrentFile(uf);
}

void unload_playlist(playlist *playlist)
{
	int i;
	if(playlist->entries!=NULL)
	{
		for(i=0;i<playlist->num_tracks;i++)
      {
         if(playlist->entries[i]->track_data != NULL)
            free(playlist->entries[i]->track_data);
         free(playlist->entries[i]);
      }
	}
	if(playlist->filename!=NULL)
	{
		free(playlist->filename);
	}
	if(playlist->game_name!=NULL)
	{
		free(playlist->game_name);
	}
	if(playlist->playlist_data != NULL)
	{
		free(playlist->playlist_data);
	}
	free(playlist);

}
