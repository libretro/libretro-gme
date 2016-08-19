#include "playlist.h"

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
	/*FILE *fp;
	fp = fopen(playlist->filename,"r");
	fseek (fp,0,SEEK_END);
	playlist->playlist_data_length = ftell(fp);
	rewind(fp);
	playlist->type = GME_FILE;
	playlist->num_tracks = 1;
	playlist->current_track = 0;
	playlist->playlist_data = malloc(playlist->playlist_data_length);
	fread(playlist->playlist_data,1,playlist->playlist_data_length,fp);
	fclose(fp);*/
}

void load_zip(playlist *playlist, long sample_rate)
{
	char *ext;
	unzFile uf = NULL;
	unz_global_info64 gi;
	unz_file_info64 file_info;
	char filename_inzip[256];
	int i;
	playlist_entry *entry;
	//set playlist type
	playlist->type = GME_ZIP;
	playlist->playlist_data = NULL;
	playlist->playlist_data_length = 0;
	//load zip content
	uf = unzOpen64(playlist->filename);
	unzGetGlobalInfo64(uf,&gi);
	playlist->num_tracks = gi.number_entry;
	playlist->current_track = 0;
	for(i=0;i<gi.number_entry;i++)
	{
		Music_Emu* temp_emu;
		gme_info_t* track_info;
		playlist->entries[i] = malloc(sizeof(playlist_entry));
		entry = playlist->entries[i];
		//read compressed file info
		unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
		//initialize entry
		entry->track_number = i + 1;
		entry->track_data_length = file_info.uncompressed_size;
		entry->track_data = (char*)malloc(sizeof(char)*file_info.uncompressed_size);
		unzReadCurrentFile(uf,entry->track_data,file_info.uncompressed_size);
		entry->track_length = 0;
		//get track info
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
		else //invalid extension
			entry->track_type = NULL;
		if(entry->track_type!=NULL)
		{
			temp_emu = gme_new_emu(entry->track_type,gme_info_only);
			gme_load_data(temp_emu,entry->track_data,entry->track_data_length);
			gme_track_info(temp_emu, &track_info,0);
			strncpy(entry->track_name,track_info->song,200);
			/*if ( track_info->length <= 0 )
				entry->track_length = track_info->intro_length + track_info->loop_length * 2;	
			if ( entry->track_length <= 0 )
				entry->track_length = (long) (2.5 * 60 * 1000);
			sprintf(entry->track_name, "%s",strcmp(track_info->song,"") ? track_info->song : "-");*/
			gme_free_info(track_info);
			gme_delete( temp_emu );
		}
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
		free(playlist->entries);		
	}
	if(playlist->filename!=NULL)	
		free(playlist->filename);
	if(playlist->playlist_data != NULL)
		free(playlist->playlist_data);
	free(playlist);
}