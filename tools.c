/*
 * Copyright (C) 2013 Frank Schmirler <vdr@schmirler.de>
 *
 * This file is part of VDR Plugin peer.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 * Or, point your browser to http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

#include "tools.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

cStringBuffer::cStringBuffer(int Size)
{
	length = 0;
	size = Size;
	buffer = Size > 0 ? (char *) malloc(sizeof(char) * Size) : NULL;
}

cStringBuffer::~cStringBuffer()
{
	free(buffer);
}

cStringBuffer& cStringBuffer::Append(const char *Fmt, ...)
{
	va_list ap;

	while (buffer) {
		va_start(ap, Fmt);
		int n = vsnprintf(buffer + length, size - length, Fmt, ap);
		va_end(ap);

		if (n < size - length) {
			length +=n;
			break;
		}
		// overflow: realloc and try again
		size *= 2;
		char *tmp = (char *) realloc(buffer, sizeof(char) * size);
		if (!tmp)
			free(buffer);
		buffer = tmp;
	}
	return *this;
}

char* cStringBuffer::Export()
{
	char *s = buffer;
	buffer = NULL;
	return s;
};

bool Parse(char *s, cString& Key, cString& Value)
{
	char *p = strchr(s, '=');
	if (p) {
		*p = 0;
		Key = compactspace(s);
		Value = compactspace(p + 1);
		return true;
	}
	return false;
}

bool Parse(const char *s, cString& Key, cString& Value)
{
	char *t = strdup(s);
	bool result = Parse(t, Key, Value);
	free(t);
	return result;
}

void ParseIpPort(const char *s, cString& Ip, unsigned short& Port)
{
	char *t = strdup(s);
	char *p = strchr(t, ':');
	if (p) {
		*p = 0;
		Port = (unsigned short) atoi(p + 1);
	}
	else
		Port = 0;
	Ip = cString(t, true);
}

bool WakeOnLAN(const char *Mac)
{
	int fd;
	int optval = 1;
	unsigned char buffer[102];
	unsigned char mac[6];
	const char *p;
	char *n = NULL;

	p = Mac;
	for (int i = 0; i < 6; i++) {
		long l = strtol(p, &n, 16);
dsyslog("%ld %p %p %d %c", l, p, n, i, *n);
		if (p == n || l < 0 || l > 255 || (i == 5 && *n != 0) || (i != 5 && *n != ':')) {
			esyslog("peer: invalid mac address '%s'", Mac);
			return false;
		}
		mac[i] = (unsigned char) l;
		p = n + 1;
	}

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd != -1 && setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) != -1) {
		struct sockaddr_in sin;
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = inet_addr("255.255.255.255");
		sin.sin_port = htons(9);

		for (int i = 0; i < 6; i++)
			buffer[i] = 0xFF;

		for (int i = 1; i <= 16; i++)
			memcpy(&buffer[i * 6], &mac, sizeof(mac));

		if (sendto(fd, &buffer, sizeof(buffer), 0, (struct sockaddr*)&sin, sizeof(sin)) != -1 ) {
			close(fd);
			return true;
		}
	}
	close(fd);
	esyslog("peer: Failed to send WoL packet: %m");
	return false;
}
 
