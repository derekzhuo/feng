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

#include <stdlib.h>
#include <string.h>
#include <fenice/utils.h>
#include <fenice/mediainfo.h>

int enum_media(char *object,media_entry **list)
{
	static SD_descr *SD_global_list=NULL;
	SD_descr *descr_curr,*descr_next;
	int res;
	for (descr_curr=SD_global_list; descr_curr!=NULL; descr_curr=descr_curr->next) {
		if (strcmp(descr_curr->filename,object)==0) {
			break;
		}
	}
	if (descr_curr!=NULL) {
		// .SD Found
		*list=descr_curr->me_list;
		return ERR_NOERROR;
	}
	else {	
		// .SD not found
		if (SD_global_list==NULL) {
			SD_global_list=(SD_descr*)calloc(1,sizeof(SD_descr));
			if (SD_global_list==NULL) {
				*list=NULL;
				return ERR_ALLOC;
			}			
			strcpy(SD_global_list->filename,object);
			res=parse_SD_file(object,SD_global_list);
			if (res==ERR_NOERROR) {
				*list=SD_global_list->me_list;
				return ERR_NOERROR;
			}
			else {
				*list=NULL;
				SD_global_list=NULL;
				return res;
			}
		}
		else {
			for (descr_curr=SD_global_list; descr_curr!=NULL; descr_curr=descr_curr->next) {
				descr_next=descr_curr;
			}
			descr_curr=descr_next;
			descr_next->next=(SD_descr*)calloc(1,sizeof(SD_descr));			
			if ((descr_next=descr_next->next) == NULL) {
				*list=NULL;
				return ERR_ALLOC;
			}
			strcpy(descr_next->filename,object);
			res=parse_SD_file(object,descr_next);
			if (res==ERR_NOERROR) {
				*list=descr_next->me_list;
				return ERR_NOERROR;
			}
			else {
				free(descr_curr->next);
				*list=NULL;
				return res;
			}		
		}
	}
	return 0;
}
