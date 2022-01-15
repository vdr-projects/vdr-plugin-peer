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

#include "svdrp.h"
#include "tools.h"
#include <vdr/config.h>
#include <vdr/osdbase.h>
#include <vdr/tools.h>
#include <stdarg.h>

cSvdrp* cSvdrp::svdrp = NULL;

cSvdrp* cSvdrp::GetInstance() {
	if (!svdrp)
		svdrp = new cSvdrp(cPluginManager::GetPlugin("svdrpservice"));
	return svdrp;
}

void cSvdrp::DeleteInstance() {
	DELETENULL(svdrp);
}

cSvdrp::cSvdrp(cPlugin *Service) {
	service = Service;
	conn.handle = -1;
}

cSvdrp::~cSvdrp() {
	if (conn.handle >= 0)
		service->Service("SvdrpConnection-v1.0", &conn);
}

bool cSvdrp::Connect(const char* ServerIp, unsigned short ServerPort) {
	if (!service)
		esyslog("peer: Plugin svdrpservice not available.");
	else if (conn.handle < 0) {
		conn.serverIp = ServerIp;
		conn.serverPort = ServerPort;
		conn.shared = true;
		service->Service("SvdrpConnection-v1.0", &conn);
	}
	return conn.handle >= 0;
}

void cSvdrp::Disconnect() {
	if (conn.handle >= 0) {
		service->Service("SvdrpConnection-v1.0", &conn);
		conn.handle = -1;
	}
}

unsigned short cSvdrp::Send(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
#if APIVERSNUM >= 10728
	cmd.command = cString::vsprintf(fmt, ap);
#else
	cmd.command = cString::sprintf(fmt, ap);
#endif
	cmd.handle = conn.handle;
	service->Service("SvdrpCommand-v1.0", &cmd);
	va_end(ap);
	return cmd.responseCode;
}

bool cSvdrp::CmdMESG(const char *Peer, const char *Message) {
	SvdrpConnection_v1_0 conn;
	SvdrpCommand_v1_0    cmd;
	cmd.responseCode = 0;
	if (service) {
		ParseIpPort(Peer, conn.serverIp, conn.serverPort);
		conn.shared = true;
		conn.handle = -1;
		service->Service("SvdrpConnection-v1.0", &conn);
		if (conn.handle >= 0) {
			cmd.command = cString::sprintf("MESG %s\r\n", Message);
			cmd.handle = conn.handle;
			service->Service("SvdrpCommand-v1.0", &cmd);
			service->Service("SvdrpConnection-v1.0", &conn);
		}
		else
			esyslog("peer: failed to send message to '%s': %m", Peer);
	}
	return cmd.responseCode == 250;
}

bool cSvdrp::CmdMESG(const char *Message) {
	return Send("MESG %s\r\n", Message) == 250;
}

/*
unsigned short cSvdrp::CmdPEER_CONF() {
	if (Send("PLUG peer CONF\r\n") == 250) {
		for (cLine *line = cmd.reply.First(); line; line = cmd.reply.Next(line)) {
			cString key, value;
			if (Parse(line->Text(), key, value)) {
				if (strcasecmp("Name", *key) == 0) {
				}
				else if (strcasecmp("WoL", *key) == 0) {
				}
				else {
					dsyslog("peer: Unknown PLUG peer CONF option '%s = %s'", *key, *value);
				}
			}
		}
	}
	return cmd.responseCode;
}
*/

const cList<cLine>* cSvdrp::CmdPEER_STAT() {
	if (Send("PLUG peer STAT\r\n") == 250)
		return &cmd.reply;
	else
		isyslog("peer plugin not installed on peer - no details available");
	return NULL;
}

bool cSvdrp::CmdPEER_NTFY(const char* IpPort) {
	if (Send("PLUG peer NTFY %s\r\n", IpPort) != 250)
		isyslog("peer plugin not installed on peer - notify not available");
	return cmd.responseCode == 250;
}


