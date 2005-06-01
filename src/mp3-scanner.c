/*
 * $Id$
 * Implementation file for mp3 scanner and monitor
 *
 * Ironically, this now scans file types other than mp3 files,
 * but the name is the same for historical purposes, not to mention
 * the fact that sf.net makes it virtually impossible to manage a cvs
 * root reasonably.  Perhaps one day soon they will move to subversion.
 * 
 * /me crosses his fingers
 *
 * Copyright (C) 2003 Ron Pedde (ron@pedde.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#define _POSIX_PTHREAD_SEMANTICS
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <id3tag.h>
#include <limits.h>
#include <restart.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>  /* htons and friends */
#include <sys/stat.h>
#include <dirent.h>      /* why here?  For osx 10.2, of course! */

#include "daapd.h"
#include "db-generic.h"
#include "err.h"
#include "mp3-scanner.h"
#include "ssc.h"

#ifndef HAVE_STRCASESTR
# include "strcasestr.h"
#endif

/*
 * Typedefs
 */

char *scan_winamp_genre[] = {
    "Blues",              // 0
    "Classic Rock",
    "Country",
    "Dance",
    "Disco",
    "Funk",               // 5
    "Grunge",
    "Hip-Hop",
    "Jazz",
    "Metal",
    "New Age",            // 10
    "Oldies",
    "Other",
    "Pop",
    "R&B",
    "Rap",                // 15
    "Reggae",
    "Rock",
    "Techno",
    "Industrial",
    "Alternative",        // 20
    "Ska",
    "Death Metal",
    "Pranks",
    "Soundtrack",
    "Euro-Techno",        // 25
    "Ambient",
    "Trip-Hop",
    "Vocal",
    "Jazz+Funk",
    "Fusion",             // 30
    "Trance",
    "Classical",
    "Instrumental",
    "Acid",
    "House",              // 35
    "Game",
    "Sound Clip",
    "Gospel",
    "Noise",
    "AlternRock",         // 40
    "Bass",
    "Soul",
    "Punk",
    "Space",
    "Meditative",         // 45
    "Instrumental Pop",
    "Instrumental Rock",
    "Ethnic",
    "Gothic",
    "Darkwave",           // 50
    "Techno-Industrial",
    "Electronic",
    "Pop-Folk",
    "Eurodance",
    "Dream",              // 55
    "Southern Rock",
    "Comedy",
    "Cult",
    "Gangsta",
    "Top 40",             // 60
    "Christian Rap",
    "Pop/Funk",
    "Jungle",
    "Native American",
    "Cabaret",            // 65
    "New Wave",
    "Psychadelic",
    "Rave",
    "Showtunes",
    "Trailer",            // 70
    "Lo-Fi",
    "Tribal",
    "Acid Punk",
    "Acid Jazz",
    "Polka",              // 75
    "Retro",
    "Musical",
    "Rock & Roll",
    "Hard Rock",
    "Folk",               // 80
    "Folk/Rock",
    "National folk",
    "Swing",
    "Fast-fusion",
    "Bebob",              // 85
    "Latin",
    "Revival",
    "Celtic",
    "Bluegrass",
    "Avantgarde",         // 90
    "Gothic Rock",
    "Progressive Rock",
    "Psychedelic Rock",
    "Symphonic Rock",
    "Slow Rock",          // 95
    "Big Band",
    "Chorus",
    "Easy Listening",
    "Acoustic",
    "Humour",             // 100
    "Speech",
    "Chanson",
    "Opera",
    "Chamber Music",
    "Sonata",             // 105 
    "Symphony",
    "Booty Bass",
    "Primus",
    "Porn Groove",
    "Satire",             // 110
    "Slow Jam",
    "Club",
    "Tango",
    "Samba",
    "Folklore",           // 115
    "Ballad",
    "Powder Ballad",
    "Rhythmic Soul",
    "Freestyle",
    "Duet",               // 120
    "Punk Rock",
    "Drum Solo",
    "A Capella",
    "Euro-House",
    "Dance Hall",         // 125
    "Goa",
    "Drum & Bass",
    "Club House",
    "Hardcore",
    "Terror",             // 130
    "Indie",
    "BritPop",
    "NegerPunk",
    "Polsk Punk",
    "Beat",               // 135
    "Christian Gangsta",
    "Heavy Metal",
    "Black Metal",
    "Crossover",
    "Contemporary C",     // 140
    "Christian Rock",
    "Merengue",
    "Salsa",
    "Thrash Metal",
    "Anime",              // 145
    "JPop",
    "SynthPop",
    "Unknown"
};

/* 
 * Typedefs
 */

typedef struct {
    char *suffix;
    int	(*tags)(char* file, MP3FILE* pmp3);
    int	(*files)(char* file, MP3FILE* pmp3);
    char *type;         /* daap.songformat */
    char *codectype;    /* song.codectype */
    char *description;  /* daap.songdescription */
} TAGHANDLER;

#define MAYBEFREE(a) { if((a)) free((a)); };


/*
 * Forwards
 */
static int scan_path(char *path);
static int scan_gettags(char *file, MP3FILE *pmp3);
static int scan_get_nultags(char *file, MP3FILE *pmp3) { return 0; };
static int scan_get_fileinfo(char *file, MP3FILE *pmp3);
static int scan_freetags(MP3FILE *pmp3);

static void scan_static_playlist(char *path);
static void scan_music_file(char *path, struct dirent *pde, struct stat *psb);

static TAGHANDLER *scan_gethandler(char *type);


/* EXTERNAL SCANNERS */

#ifdef OGGVORBIS
extern int scan_get_ogginfo(char *filename, MP3FILE *pmp3);
#endif

#ifdef FLAC
extern int scan_get_flacinfo(char *filename, MP3FILE *pmp3);
#endif

extern int scan_get_wmainfo(char *filename, MP3FILE *pmp3);
extern int scan_get_aacinfo(char *filename, MP3FILE *pmp3);
extern int scan_get_wavinfo(char *filename, MP3FILE *pmp3);
extern int scan_get_urlinfo(char *filename, MP3FILE *pmp3);
extern int scan_get_mp3info(char *filename, MP3FILE *pmp3);

/* playlist scanners */
int scan_xml_playlist(char *filename);

/* For known types, I'm gong to use the "official" apple
 * daap.songformat, daap.songdescription, and daap.songcodecsubtype.
 * If I we don't have "official" ones, we can make them up the
 * way we currently are:  using extension or whatver.  
 *
 * This means that you can test to see if something is, say, an un-drmed
 * aac file by just testing for ->type "m4a", rather than checking every
 * different flavor of file extension.
 * 
 * NOTE: Although they are represented here as strings, the codectype is
 * *really* an unsigned short.  So when it gets serialized, it gets 
 * serialized as a short int. If you put something other than 3 or 4 
 * characters as your codectype, you'll see strange results.
 *
 * FIXME: url != pls -- this method of dispatching handlers based on file type
 * is completely wrong.  There needs to be a separate type that gets carried 
 * around with it, at least outside the database that says where the info 
 * CAME FROM.
 *
 * This system is broken, and won't work with something like a .cue file
 */
static TAGHANDLER taghandlers[] = {
    { "aac", scan_get_nultags, scan_get_aacinfo, "m4a", "mp4a", "AAC audio file" },
    { "mp4", scan_get_nultags, scan_get_aacinfo, "m4a", "mp4a", "AAC audio file" },
    { "m4a", scan_get_nultags, scan_get_aacinfo, "m4a", "mp4a", "AAC audio file" },
    { "m4p", scan_get_nultags, scan_get_aacinfo, "m4p", "mp4a", "AAC audio file" },
    { "mp3", scan_get_nultags, scan_get_mp3info, "mp3", "mpeg", "MPEG audio file" },
    { "wav", scan_get_nultags, scan_get_wavinfo, "wav", "wav", "WAV audio file" },
    { "wma", scan_get_nultags, scan_get_wmainfo, "wma", "wma", "WMA audio file" },
    { "url", scan_get_nultags, scan_get_urlinfo, "pls", NULL, "Playlist URL" },
    { "pls", scan_get_nultags, scan_get_urlinfo, "pls", NULL, "Playlist URL" },
#ifdef OGGVORBIS
    { "ogg", scan_get_nultags, scan_get_ogginfo, "ogg", "ogg", "Ogg Vorbis audio file" },
#endif
#ifdef FLAC
    { "flac", scan_get_nultags, scan_get_flacinfo, "flac","flac", "FLAC audio file" },
    { "fla", scan_get_nultags, scan_get_flacinfo,  "flac","flac", "FLAC audio file" },
#endif
    { NULL, NULL, NULL, NULL, NULL, NULL }
};

typedef struct tag_playlistlist {
    char *path;
    struct tag_playlistlist *next;
} PLAYLISTLIST;

static PLAYLISTLIST scan_playlistlist = { NULL, NULL };

/**
 * add a playlist to the playlistlist.  The playlistlist is a
 * list of playlists that need to be processed once the current
 * scan is done.  THIS IS NOT REENTRANT, and it meant to be 
 * called only inside the rescan loop.  
 *
 * \param path path of the playlist to add
 */
void scan_add_playlistlist(char *path) {
    PLAYLISTLIST *plist;

    DPRINTF(E_DBG,L_SCAN,"Adding %s for deferred processing.\n",path);

    plist=(PLAYLISTLIST*)malloc(sizeof(PLAYLISTLIST));
    if(!plist) {
	DPRINTF(E_FATAL,L_SCAN,"Malloc error\n");
	return;
    }

    plist->path=strdup(path);
    plist->next=scan_playlistlist.next;
    scan_playlistlist.next=plist;
}

/**
 * process the playlistlist
 *
 */
void scan_process_playlistlist(void) {
    PLAYLISTLIST *pnext;
    char *ext;

    while(scan_playlistlist.next) {
	pnext=scan_playlistlist.next;

	ext=pnext->path;
	if(strrchr(pnext->path,'.')) {
	    ext = strrchr(pnext->path,'.');
	}

	if(strcasecmp(ext,".xml") == 0) {
	    scan_xml_playlist(pnext->path);
	} else if(strcasecmp(ext,".m3u") == 0) {
	    scan_static_playlist(pnext->path);
	} else {
	    DPRINTF(E_LOG,L_SCAN,"Unknown playlist type: %s\n",ext);
	}

	free(pnext->path);
	scan_playlistlist.next=pnext->next;
	free(pnext);
    }
}

/*
 * scan_init
 *
 * This assumes the database is already initialized.
 *
 * Ideally, this would check to see if the database is empty.
 * If it is, it sets the database into bulk-import mode, and scans
 * the MP3 directory.
 *
 * If not empty, it would start a background monitor thread
 * and update files on a file-by-file basis
 */

int scan_init(char *path) {
    int err=0;

    if(db_start_scan()) 
	return -1;

    DPRINTF(E_DBG,L_SCAN,"Scanning for MP3s in %s\n",path);

    scan_playlistlist.next=NULL;
    err=scan_path(path);

    if(db_end_song_scan())
	return -1;

    scan_process_playlistlist();
    
    if(db_end_scan())
	return -1;

    return err;
}

/*
 * scan_path
 *
 * Do a brute force scan of a path, finding all the MP3 files there
 */
int scan_path(char *path) {
    DIR *current_dir;
    char de[sizeof(struct dirent) + MAXNAMLEN + 1]; /* extra for solaris */
    struct dirent *pde;
    int err;
    char relative_path[PATH_MAX];
    char mp3_path[PATH_MAX];
    struct stat sb;
    int modified_time;
    char *ext;
    MP3FILE *pmp3;

    if((current_dir=opendir(path)) == NULL) {
	DPRINTF(E_WARN,L_SCAN,"opendir: %s\n",strerror(errno));
	return -1;
    }

    while(1) {
	if(config.stop) {
	    DPRINTF(E_WARN,L_SCAN,"Stop req.  Aborting scan of %s.\n",path);
	    closedir(current_dir);
	    return 0;
	}

	pde=(struct dirent *)&de;

	err=readdir_r(current_dir,(struct dirent *)de,&pde);
	if(err == -1) {
	    DPRINTF(E_DBG,L_SCAN,"Error on readdir_r: %s\n",strerror(errno));
	    err=errno;
	    closedir(current_dir);
	    errno=err;
	    return -1;
	}

	if(!pde)
	    break;
	
	if(pde->d_name[0] == '.') /* skip hidden and directories */
	    continue;

	snprintf(relative_path,PATH_MAX,"%s/%s",path,pde->d_name);
	mp3_path[0] = '\x0';
	realpath(relative_path,mp3_path);
	DPRINTF(E_DBG,L_SCAN,"Found %s\n",relative_path);
	if(stat(mp3_path,&sb)) {
	    DPRINTF(E_WARN,L_SCAN,"Error statting: %s\n",strerror(errno));
	} else {
	    if(sb.st_mode & S_IFDIR) { /* dir -- recurse */
		DPRINTF(E_DBG,L_SCAN,"Found dir %s... recursing\n",pde->d_name);
		scan_path(mp3_path);
	    } else {
		/* process the file */
		if(strlen(pde->d_name) > 4) {
		    if((strcasecmp(".m3u",(char*)&pde->d_name[strlen(pde->d_name) - 4]) == 0) &&
		       config.process_m3u){
			/* we found an m3u file */
			scan_add_playlistlist(mp3_path);
		    } else if((strcasecmp(pde->d_name,"iTunes Music Library.xml")==0)) {
			scan_add_playlistlist(mp3_path);
		    } else if (((ext = strrchr(pde->d_name, '.')) != NULL) &&
			       (strcasestr(config.extensions, ext))) {
			/* only scan if it's been changed, or empty db */
			modified_time=sb.st_mtime;
			pmp3=db_fetch_path(mp3_path);

			if((!pmp3) || (pmp3->db_timestamp < modified_time) || 
			   (pmp3->force_update)) {
			    scan_music_file(path,pde,&sb);
			} else {
			    DPRINTF(E_DBG,L_SCAN,"Skipping file... not modified\n");
			}
			db_dispose_item(pmp3);
		    }
		}
	    }
	}
    }

    closedir(current_dir);
    return 0;
}

/*
 * scan_static_playlist
 *
 * Scan a file as a static playlist
 */

void scan_static_playlist(char *path) {
    char base_path[PATH_MAX];
    char file_path[PATH_MAX];
    char real_path[PATH_MAX];
    char linebuffer[PATH_MAX];
    int fd;
    int playlistid;
    M3UFILE *pm3u;
    MP3FILE *pmp3;
    struct stat sb;
    char *current;

    DPRINTF(E_WARN,L_SCAN|L_PL,"Processing static playlist: %s\n",path);
    if(stat(path,&sb)) {
	DPRINTF(E_WARN,L_SCAN,"Error statting %s: %s\n",path,strerror(errno));
	return;
    }

    if((current=strrchr(path,'/')) == NULL) {
	current = path;
    } else {
	current++;
    }

    /* temporarily use base_path for m3u name */
    strcpy(base_path,current);
    if((current=strrchr(base_path,'.'))) {
	*current='\x0';
    }

    pm3u = db_fetch_playlist(path,0);
    if(pm3u && (pm3u->db_timestamp > sb.st_mtime)) {
	/* already up-to-date */
	db_dispose_playlist(pm3u);
	return;
    }

    if(pm3u) 
	db_delete_playlist(pm3u->id);

    fd=open(path,O_RDONLY);
    if(fd != -1) {
	if(db_add_playlist(base_path,PL_STATICFILE,NULL,path,0,&playlistid) != DB_E_SUCCESS) {
	    DPRINTF(E_LOG,L_SCAN,"Error adding m3u playlist %s\n",path);
	    db_dispose_playlist(pm3u);
	    return;
	}
	/* now get the *real* base_path */
	strcpy(base_path,path);
	if((current=strrchr(base_path,'/'))) {
	    *(current+1) = '\x0';
	} /* else something is fubar */

	DPRINTF(E_INF,L_SCAN|L_PL,"Added playlist as id %d\n",playlistid);

	memset(linebuffer,0x00,sizeof(linebuffer));
	while(readline(fd,linebuffer,sizeof(linebuffer)) > 0) {
	    while((linebuffer[strlen(linebuffer)-1] == '\n') ||
		  (linebuffer[strlen(linebuffer)-1] == '\r')) 
		linebuffer[strlen(linebuffer)-1] = '\0';

	    if((linebuffer[0] == ';') || (linebuffer[0] == '#'))
		continue;

	    // FIXME - should chomp trailing comments

	    // otherwise, assume it is a path
	    if(linebuffer[0] == '/') {
		strcpy(file_path,linebuffer);
	    } else {
		snprintf(file_path,sizeof(file_path),"%s%s",base_path,linebuffer);
	    }

	    realpath(file_path,real_path);
	    DPRINTF(E_DBG,L_SCAN|L_PL,"Checking %s\n",real_path);

	    // might be valid, might not...
	    if((pmp3=db_fetch_path(real_path))) {
		db_add_playlist_item(playlistid,pmp3->id);
		db_dispose_item(pmp3);
	    } else {
		DPRINTF(E_WARN,L_SCAN|L_PL,"Playlist entry %s bad: %s\n",
			path,strerror(errno));
	    }
	}
	close(fd);
    }

    db_dispose_playlist(pm3u);
    DPRINTF(E_WARN,L_SCAN|L_PL,"Done processing playlist\n");
}


/*
 * scan_music_file
 *
 * scan a particular file as a music file
 */
void scan_music_file(char *path, struct dirent *pde, struct stat *psb) {
    MP3FILE mp3file;
    char mp3_path[PATH_MAX];
    char *current=NULL;
    char *type;
    TAGHANDLER *ptaghandler;
    char fdescr[50];

    snprintf(mp3_path,sizeof(mp3_path),"%s/%s",path,pde->d_name);

    /* we found an mp3 file */
    DPRINTF(E_INF,L_SCAN,"Found music file: %s\n",pde->d_name);
    
    memset((void*)&mp3file,0,sizeof(mp3file));
    mp3file.path=strdup(mp3_path);
    mp3file.fname=strdup(pde->d_name);
    if(strlen(pde->d_name) > 4) {
	type = strrchr(pde->d_name, '.') + 1;
	if(type) {
	    /* see if there is "official" format and info for it */
	    ptaghandler=scan_gethandler(type);
	    if(ptaghandler) {
		/* yup, use the official format */
		mp3file.type=strdup(ptaghandler->type);
		if(ptaghandler->description)
		    mp3file.description=strdup(ptaghandler->description);

		if(ptaghandler->codectype)
		    mp3file.codectype=strdup(ptaghandler->codectype);

		DPRINTF(E_DBG,L_SCAN,"Codec type: %s\n",mp3file.codectype);
	    } else {
		/* just dummy up songformat, codectype and description */
		mp3file.type=strdup(type);

		/* upper-case types cause some problems */
		current=mp3file.type;
		while(*current) {
		    *current=tolower(*current);
		    current++;
		}
		
		sprintf(fdescr,"%s audio file",mp3file.type);
		mp3file.description = strdup(fdescr);
		/* we'll just dodge the codectype */
	    }
	}
    }
    
    /* Do the tag lookup here */
    if(!scan_gettags(mp3file.path,&mp3file) && 
       !scan_get_fileinfo(mp3file.path,&mp3file)) {
	make_composite_tags(&mp3file);
	/* fill in the time_added.  I'm not sure of the logic in this.
	   My thinking is to use time created, but what is that?  Best
	   guess would be earliest of st_mtime and st_ctime...
	*/
	mp3file.time_added=psb->st_mtime;
	if(psb->st_ctime < mp3file.time_added)
	    mp3file.time_added=psb->st_ctime;
        mp3file.time_modified=psb->st_mtime;

	DPRINTF(E_DBG,L_SCAN," Date Added: %d\n",mp3file.time_added);

	DPRINTF(E_DBG,L_SCAN," Codec: %s\n",mp3file.codectype);

	db_add(&mp3file);
    } else {
	DPRINTF(E_WARN,L_SCAN,"Skipping %s - scan_gettags failed\n",pde->d_name);
    }
    
    scan_freetags(&mp3file);
}

/**
 * fetch the taghandler for this file type
 */
TAGHANDLER *scan_gethandler(char *type) {
    TAGHANDLER *phdl = taghandlers;

    while((phdl->suffix) && (strcasecmp(phdl->suffix,type)))
	phdl++;

    if(phdl->suffix)
	return phdl;

    return NULL;
}


/**
 * Dispatch the appropriate handler to get specific tag metainfomation
 *
 * \param file file to get tag info for
 * \param pmp3 mp3 file struct to fill info into
 */
int scan_gettags(char *file, MP3FILE *pmp3) {
    TAGHANDLER *hdl;

    /* dispatch to appropriate tag handler */
    hdl = scan_gethandler(pmp3->type);
    if(hdl && hdl->tags)
	return hdl->tags(file,pmp3);

    /* otherwise, it's a file type we don't understand yet */
    return 0;
}

/*
 * scan_freetags
 *
 * Free up the tags that were dynamically allocated
 */
int scan_freetags(MP3FILE *pmp3) {
    MAYBEFREE(pmp3->path);
    MAYBEFREE(pmp3->fname);
    MAYBEFREE(pmp3->title);
    MAYBEFREE(pmp3->artist);
    MAYBEFREE(pmp3->album);
    MAYBEFREE(pmp3->genre);
    MAYBEFREE(pmp3->comment);
    MAYBEFREE(pmp3->type);
    MAYBEFREE(pmp3->composer);
    MAYBEFREE(pmp3->orchestra);
    MAYBEFREE(pmp3->conductor);
    MAYBEFREE(pmp3->grouping);
    MAYBEFREE(pmp3->description);
    MAYBEFREE(pmp3->codectype);

    return 0;
}


/**
 * Dispatch to actual file info handlers
 *
 * \param file file to read file metainfo for
 * \param pmp3 struct to stuff with info gleaned
 */
int scan_get_fileinfo(char *file, MP3FILE *pmp3) {
    FILE *infile;
    off_t file_size;

    TAGHANDLER *hdl;

    /* dispatch to appropriate tag handler */
    hdl = scan_gethandler(pmp3->type);
    if(hdl && hdl->files)
	return hdl->files(file,pmp3);

    /* a file we don't know anything about... ogg or aiff maybe */
    if(!(infile=fopen(file,"rb"))) {
	DPRINTF(E_WARN,L_SCAN,"Could not open %s for reading\n",file);
	return -1;
    }

    /* we can at least get this */
    fseek(infile,0,SEEK_END);
    file_size=ftell(infile);
    fseek(infile,0,SEEK_SET);

    pmp3->file_size=file_size;

    fclose(infile);
    return 0;
}



/**
 * Manually build tags.  Set artist to computer/orchestra
 * if there is already no artist.  Perhaps this could be 
 * done better, but I'm not sure what else to do here.
 *
 * @param song MP3FILE of the file to build composite tags for
 */
void make_composite_tags(MP3FILE *song) {
    int len;

    len=0;

    if(!song->artist && (song->orchestra || song->conductor)) {
	if(song->orchestra)
	    len += strlen(song->orchestra);
	if(song->conductor)
	    len += strlen(song->conductor);

	len += 3;

	song->artist=(char*)calloc(len, 1);
	if(song->artist) {
	    if(song->orchestra)
		strcat(song->artist,song->orchestra);

	    if(song->orchestra && song->conductor)
		strcat(song->artist," - ");

	    if(song->conductor)
		strcat(song->artist,song->conductor);
	}
    }

    if(song->url) {
	song->data_kind=1;
    } else {
	song->data_kind=0;
    }

    if(!song->title)
	song->title = strdup(song->fname);

    song->item_kind = 2; /* music, I think. */
}

