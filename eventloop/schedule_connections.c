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
#include <stdlib.h>
#include <unistd.h>

#include <fenice/socket.h>
#include <fenice/eventloop.h>
#include <fenice/utils.h>
#include <fenice/rtsp.h>
#include <fenice/schedule.h>
#include <fenice/bufferpool.h>
int stop_schedule = 0;

void schedule_connections(RTSP_buffer **rtsp_list, int *conn_count)
{
	int res;
	RTSP_buffer *p,*pp;
	RTP_session *r,*t;
	
	pp=NULL;
	p=*rtsp_list;
	while (p!=NULL)
	{
		if ((res=rtsp_server(p))!=ERR_NOERROR)
		{			
        		if (res==ERR_CONNECTION_CLOSE || res==ERR_GENERIC)
			{
        			// The connection is closed
				if (res==ERR_CONNECTION_CLOSE)
					printf("RTSP connection closed by client.\n");
				else
					printf("RTSP connection closed by server.\n");

				close(p->fd);
            			--*conn_count;        	
            			if (p->session_list!=NULL) //if client truncated RTSP connection before sending TEARDOWN: error
				{
					printf("WARNING! RTSP connection truncated before ending operations.\n");
                			r=p->session_list->rtp_session;
                			// Release all RTP sessions
                			while (r!=NULL)
					{
						
						if (r->current_media->pkt_buffer);
						//Release buffer
						OMSbuff_unref(r->current_media->pkt_buffer);
        	        			// Release the scheduler entry
        	        			schedule_remove(r->sched_id);
        					// Close connections        		
                				close(r->rtp_fd);
            					close(r->rtcp_fd_in);
            					close(r->rtcp_fd_out);        		
            					// Release ports
        	        			RTP_release_port_pair(&(r->ser_ports));
        	        			t=r->next;
        	        			// Deallocate memory
        					while (schedule_semaphore (r->sched_id) == red);
        	        			free(r);
        	        			r=t;
                			}
        				// Close connection        		
					//close(p->session_list->fd);
                			// Release the RTSP session
                			free(p->session_list);
                		}
            			// Release the RSTP_buffer
            			if (p==*rtsp_list)
				{
            				*rtsp_list=p->next;
            				free(p);
            				p=*rtsp_list;
            			}
            			else
				{
            				pp->next=p->next;	                		
            				free(p);
            				p=pp->next;
            			}
            			// Release the scheduler if necessary
            			if (p==NULL && *conn_count<0)
				{
            				printf("Fermo il thread\n");
    					stop_schedule=1;
            			}
            		}
            		else
			{
				p=p->next;
            		}
		}
		else
		{
			pp=p;
			p=p->next;						
		}
	}
}
