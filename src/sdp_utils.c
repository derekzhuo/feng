/* *
 *
 * Copyright (C) 2007 by LScube team <team@streaming.polito.it>
 * See AUTHORS for more details
 *
 * Feng is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Feng is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Feng; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * */

#include <fenice/utils.h>

// Ripped from ffmpeg, see sdp.c

static void digit_to_char(char *dst, uint8_t src)
{
    if (src < 10) {
        *dst = '0' + src;
    } else {
        *dst = 'A' + src - 10;
    }
}

static char *data_to_hex(char *buff, const uint8_t *src, int s)
{
    int i;

    for(i = 0; i < s; i++) {
        digit_to_char(buff + 2 * i, src[i] >> 4);
        digit_to_char(buff + 2 * i + 1, src[i] & 0xF);
    }

    return buff;
}

char *extradata2config(const uint8_t *extradata, int extradata_size)
{
    char *config = malloc(extradata_size * 2 + 1);

    if (config == NULL) {
        return NULL;
    }

    data_to_hex(config, extradata, extradata_size);

    config[extradata_size * 2] = '\0';

    return config;
}