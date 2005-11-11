/*
 * $Id$
 * Header info for daapd server
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

/**
 * \file daapd.h
 *
 * header file for main.c.  Why it isn't main.h, I don't know.
 * In fact...
 *
 * \todo make daapd.h into main.h
 */

#ifndef _DAAPD_H_
#define _DAAPD_H_

#include "webserver.h"

/** Simple struct for holding stat info.
 * \todo wire up the tag_stats#bytes_served stuff into r_write() in restart.c
 */
typedef struct tag_stats {
    time_t start_time;          /**< When the server was started */
    int songs_served;           /**< How many songs have been served */
 
    unsigned int gb_served;     /**< How many gigs of data have been served (unused) */
    unsigned int bytes_served;  /**< How many bytes of data served (unused) */
} STATS;

/** Global config struct */
typedef struct tag_config {
    int use_mdns;         /**< Should we do rendezvous advertisements? */
    int stop;             /**< Time to exit? */
    int reload;           /**< Time to reload and/or rescan the database? */
    char *configfile;     /**< path to config file */
    char *web_root;       /**< path to the dir containing the web files */
    char *iface;          /**< interface to advertise on */
    int port;             /**< port to listen on */
    int rescan_interval;  /**< How often to do a background fs rescan */
    int always_scan;      /**< 0 to minimize disk usage (embedded devices) */
    int process_m3u;      /**< Should we process m3u files? */
    int scan_type;        /**< Method for finding playtime. see scan-mp3.c */
    int compress;         /**< Should we compress? */
    int pid;              /**< pid that will accept INT to terminate */
    int latin1_tags;      /**< interpret all tags as latin1 rather than utf8 */
    char *adminpassword;  /**< Password to web management pages */
    char *readpassword;   /**< iTunes password */
    char *mp3dir;         /**< root directory of the mp3 files */
    char *servername;     /**< Name advertised via rendezvous */
    char *playlist;       /**< Path to the playlist file */
    char *runas;          /**< Who to drop privs to (if run as root) */
    char *dbdir;          /**< Where to put the db file */
    char *extensions;     /**< What music file extentions to process */
    char *ssc_codectypes; /**< What codectypes are converted in server */
    char *ssc_prog;       /**< Server side music format converter prog */
    char *artfilename;    /**< What filename to merge coverart with */
    char *logfile;        /**< What file to use as a logfile */
    char *compdirs;       /**< Compilations directories */
    char **complist;      /**< list of compilation directories */
    STATS stats;          /**< Stats structure (see above) */
    WSHANDLE server;      /**< webserver handle */    
} CONFIG;

extern CONFIG config;

/* Forwards */
extern int drop_privs(char *user);

#endif /* _DAAPD_H_ */
