/* *
 * This file is part of Feng
 *
 * Copyright (C) 2009 by LScube team <team@lscube.org>
 * See AUTHORS for more details
 *
 * feng is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * feng is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with feng; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * */

#include <config.h>

#include <stdbool.h>
#include <time.h>

#include <json.h>

#include "feng.h"
#include "network/rtsp.h"
#include "media/media.h"

static size_t stats_total_bytes_sent;
static size_t stats_total_bytes_read;
static time_t stats_start_time;

/**
 * @brief Initialize the statistics
 *
 * Set @ref stats_start_time to the current time.
 */

void stats_init()
{
    stats_start_time = time(NULL);
}

void stats_account_read(RTSP_Client *rtsp, size_t bytes)
{
    if ( rtsp->pair && (rtsp = rtsp->pair->rtsp_client) == NULL )
        return;

    rtsp->bytes_read += bytes;
    stats_total_bytes_sent += bytes;
}

void stats_account_sent(RTSP_Client *rtsp, size_t bytes)
{
    if ( rtsp->pair && (rtsp = rtsp->pair->rtsp_client) == NULL )
        return;

    rtsp->bytes_sent += bytes;
    stats_total_bytes_read += bytes;
}

/**
 * @brief Produce per client statistics
 *
 * @note feed to g_slist_foreach
 */

static void client_stats(gpointer c, gpointer s)
{
    RTSP_Client *client = c;
    RTSP_session *session = client->session;
    json_object *clients_stats = s;
    json_object *stats = json_object_new_object();
    // Sessionless clients are querying stats, let's ignore them.
    if (!session) return;
    json_object_object_add(stats, "resource_uri",
        json_object_new_string(session->resource_uri));
    json_object_object_add(stats, "user_agent",
        json_object_new_string("missing"/*client->stats->user_agent*/));
    json_object_object_add(stats, "remote_host",
        json_object_new_string(client->remote_host));
    json_object_object_add(stats, "remote_host",
        json_object_new_string(client->remote_host));
    json_object_object_add(stats, "bytes_sent",
        json_object_new_int(client->bytes_sent));
    json_object_object_add(stats, "bytes_read",
        json_object_new_int(client->bytes_read));
    json_object_array_add(clients_stats, stats);
}

/**
 * @brief Report instant statistics
 */

void feng_send_statistics(RTSP_Client *rtsp)
{
    RFC822_Response *response =
        rfc822_response_new(rtsp->pending_request, RTSP_Ok);
    json_object *stats = json_object_new_object();
    json_object *clients_stats = json_object_new_array();

    json_object_object_add(stats, "bytes_sent",
        json_object_new_int(stats_total_bytes_sent));

    json_object_object_add(stats, "bytes_read",
        json_object_new_int(stats_total_bytes_read));

    json_object_object_add(stats, "uptime",
        json_object_new_int(time(NULL) - stats_start_time));

    clients_each(client_stats, clients_stats);

    json_object_object_add(stats, "clients",
                           json_object_new_int(json_object_array_length(clients_stats)));

    json_object_object_add(stats, "per_client", clients_stats);

    response->body = g_string_new(json_object_to_json_string(stats));

    rfc822_headers_set(response->headers,
                       RTSP_Header_Content_Type,
                       g_strdup("application/json"));
    rfc822_headers_set(response->headers,
                       RTSP_Header_Content_Base,
                       g_strdup_printf("%s/", rtsp->pending_request->object));
    rfc822_response_send(rtsp, response);
}

