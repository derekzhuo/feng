/* * 
 *  $Id$
 *  
 *  This file is part of Fenice
 *
 *  Fenice -- Open Media Server
 *
 *  Copyright (C) 2004 by
 *  	
 *	- Giampaolo Mancini	<giampaolo.mancini@polito.it>
 *	- Francesco Varano	<francesco.varano@polito.it>
 *	- Marco Penno		<marco.penno@polito.it>
 *	- Federico Ridolfo	<federico.ridolfo@polito.it>
 *	- Eugenio Menegatti 	<m.eu@libero.it>
 *	- Stefano Cau
 *	- Giuliano Emma
 *	- Stefano Oldrini
 * 
 *  Fenice is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Fenice is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Fenice; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *  
 * */

#include <stdio.h>
#include <string.h>

#include <fenice/rtsp.h>
#include <fenice/utils.h>
#include <fenice/prefs.h>
#include <fenice/sdp.h>

/*
 	****************************************************************
 	*			DESCRIBE METHOD HANDLING
 	****************************************************************
*/
int RTSP_describe(RTSP_buffer * rtsp)
{
	int valid_url, res;
	char object[255], server[255], trash[255];
	char *p;
	unsigned short port;
	char url[255];
	media_entry media, req;
	description_format descr_format = df_SDP_format; // shawill put to some default
	char descr[MAX_DESCR_LENGTH];

	printf("DESCRIBE request received.\n");

	/* Extract la URL */
	if (!sscanf(rtsp->in_buffer, " %*s %254s ", url)) {
		printf("DESCRIBE request is missing object (path/file) parameter.\n");
		send_reply(400, 0, rtsp);	/* bad request */
		return ERR_NOERROR;
	}
	/* Validate the URL */
	if (!parse_url(url, server, &port, object)) {
		printf("Mangled URL in DESCRIBE.\n");
		send_reply(400, 0, rtsp);	/* bad request */
		return ERR_NOERROR;
	}
	if (strcmp(server, prefs_get_hostname()) != 0) {	/* Currently this feature is disabled. */
		/* wrong server name */
		//      printf("DESCRIBE request specified an unknown server name.\n");
		//      send_reply(404, 0 , rtsp);  /* Not Found */
		//      return ERR_NOERROR;
	}
	if (strstr(object, "../")) {
		/* disallow relative paths outside of current directory. */
		printf("DESCRIBE request specified an object parameter with a path that is not allowed. '../' not permitted in path.\n");
		send_reply(403, 0, rtsp);	/* Forbidden */
		return ERR_NOERROR;
	}
	if (strstr(object, "./")) {
		/* Disallow ./ */
		printf
		    ("DESCRIBE request specified an object parameter with a path that is not allowed. './' not permitted in path.\n");
		send_reply(403, 0, rtsp);	/* Forbidden */
		return ERR_NOERROR;
	}
	p = strrchr(object, '.');
	valid_url = 0;
	if (p == NULL) {
		printf("DESCRIBE request specified an object (path/file) parameter that is not valid.\n");
		send_reply(415, 0, rtsp);	/* Unsupported media type */
		return ERR_NOERROR;
	} else {
		valid_url = is_supported_url(p);
	}
	if (!valid_url) {
		printf("DESCRIBE request specified an unsupported media type.\n");
		send_reply(415, 0, rtsp);	/* Unsupported media type */
		return ERR_NOERROR;
	}
	// Disallow Header REQUIRE
	if (strstr(rtsp->in_buffer, HDR_REQUIRE)) {
		printf("DESCRIBE request specified an unsupported 'require' header.\n");
		send_reply(551, 0, rtsp);	/* Option not supported */
		return ERR_NOERROR;
	}

	/* Get the description format. SDP is recomended */
	if (strstr(rtsp->in_buffer, HDR_ACCEPT) != NULL) {
		if (strstr(rtsp->in_buffer, "application/sdp") != NULL) {
			descr_format = df_SDP_format;
		} else {
			// Add here new description formats
			printf("DESCRIBE request specified an unsupported description format.\n");
			send_reply(551, 0, rtsp);	/* Option not supported */
			return ERR_NOERROR;
		}
	}
	// Get the CSeq 
	if ((p = strstr(rtsp->in_buffer, HDR_CSEQ)) == NULL) {
		printf("DESCRIBE request didn't specify a CSeq header.\n");
		send_reply(400, 0, rtsp);	/* Bad Request */
		return ERR_NOERROR;
	} else {
		if (sscanf(p, "%254s %d", trash, &(rtsp->rtsp_cseq)) != 2) {
			printf("DESCRIBE request didn't specify a CSeq number.\n");
			send_reply(400, 0, rtsp);	/* Bad Request */
			return ERR_NOERROR;
		}
	}

        
	memset(&media, 0, sizeof(media));
	memset(&req, 0, sizeof(req));
	req.flags = ME_DESCR_FORMAT;
	req.descr_format = descr_format;
	res = get_media_descr(object, &req, &media, descr);
	if (res == ERR_NOT_FOUND) {
		printf("DESCRIBE request specified an object which can't be found.\n");
		send_reply(404, 0, rtsp);	// Not found
		return ERR_NOERROR;
	}
	if (res == ERR_PARSE || res == ERR_GENERIC || res == ERR_ALLOC) {
		if (res == ERR_PARSE)
			printf("DESCRIBE request specified an object file which can be damaged.\n");
			
		if (res == ERR_GENERIC)
			printf("DESCRIBE request generated a generic server error.\n");
			
		if (res == ERR_ALLOC)
			printf("DESCRIBE request generated a memory allocation server error.\n");
			
		send_reply(500, 0, rtsp);	// Internal server error
		return ERR_NOERROR;
	}
	send_describe_reply(rtsp, object, descr_format, descr);
	return ERR_NOERROR;
}
