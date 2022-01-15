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

#include "menu.h"
#include "svdrp.h"
#include "tools.h"
#include "setup.h"
#include "include/remoteosd.h"
#include "include/remotetimers.h"
#include <vdr/i18n.h>
#include <vdr/plugin.h>
#include <vdr/remote.h>
#include <vdr/tools.h>

// -------------- cMenuPeerItem ----------------
class cMenuPeerItem : public cOsdItem {
public:
	cPeer &peer;
public:
	const cPeer& Peer() { return peer; };
	cMenuPeerItem(cPeer &Peer);
};

cMenuPeerItem::cMenuPeerItem(cPeer &Peer): peer(Peer) {
	SetText(cString::sprintf("%s\t%s", Peer.Name(), Peer.IpPort()));
}

// -------------- cMenuPeerMessage ----------------
class cMenuPeerMessage : public cOsdMenu
{
private:
	const cPeer &peer;
	char message[128];
protected:
	virtual eOSState ProcessKey(eKeys Key);
public:
	cMenuPeerMessage(const cPeer &Peer);
};

cMenuPeerMessage::cMenuPeerMessage(const cPeer &Peer): cOsdMenu(cString::sprintf(tr("Message to '%s'"), Peer.Name()), 15), peer(Peer)
{
	*message = 0;
#if VDRVERSNUM >= 10728
	SetMenuCategory(mcPlugin);
#endif
	cMenuEditStrItem *item = new cMenuEditStrItem(tr("Send message"), message, sizeof(message));
	Add(item);
	// open in edit mode
	cRemote::Put(kRight);
}

eOSState cMenuPeerMessage::ProcessKey(eKeys Key)
{
	eOSState state = cOsdMenu::ProcessKey(Key);
	if (state == osUnknown) {
		switch (Key) {
			case kOk:
				if (*message) {
					cSvdrp *svdrp = cSvdrp::GetInstance();
					if (svdrp->Connect(peer.ip, peer.port) && svdrp->CmdMESG(message)) {
						Skins.Message(mtInfo, tr("Message sent"));
						state = osBack;
					}
					else {
						Skins.Message(mtError, tr("Failed to send message"));
						state = osContinue;
					}
					svdrp->Disconnect();
				}
				break;
			default:
				break;
		}
	}
	return state;
}

// -------------- cMenuPeerDetails ----------------
class cMenuPeerDetails : public cOsdMenu
{
private:
	const cPeer &peer;
	void Set();
protected:
	virtual eOSState ProcessKey(eKeys Key);
public:
	cMenuPeerDetails(const cPeer &Peer);
};

cMenuPeerDetails::cMenuPeerDetails(const cPeer &Peer): cOsdMenu(Peer.Name(), MAX_NAME_LENGTH), peer(Peer)
{
#if VDRVERSNUM >= 10728
	SetMenuCategory(mcPlugin);
#endif
	Set();
	SetHelp(tr("Message"), *PeerSetup.localIp ? tr("Notify") : NULL);
}

void cMenuPeerDetails::Set()
{
	cSvdrp *svdrp = cSvdrp::GetInstance();
	const cList<cLine>* reply = svdrp->CmdPEER_STAT();
	if (reply) {

		int dev = -1;
		int prio = IDLEPRIORITY;
		cString name(""), type("");
		Clear();
		for (cLine *line = reply->First(); line; line = reply->Next(line)) {
			cString key, value;
			if (Parse(line->Text(), key, value)) {
				if (strcasecmp("DEV", *key) == 0) {
					dev = atoi(*value);
				}
				else if (strcasecmp("NAME", *key) == 0) {
					name = value;
				}
				else if (strcasecmp("TYPE", *key) == 0) {
					type = value;
				}
				else if (strcasecmp("PRIO", *key) == 0) {
					prio = atoi(*value);
					Add(new cOsdItem(cString::sprintf("%d: %s\t%s %s: %s", dev + 1, *type, *name, trVDR("Priority"), (prio == IDLEPRIORITY ? tr("Idle") : *itoa(prio))), osUnknown, false));
				}
				else if (strcasecmp("PLAY", *key) == 0) {
					Add(new cOsdItem(cString::sprintf("%s\t%s", tr("Replaying"), *value)));
				}
				else if (strcasecmp("LIVE", *key) == 0) {
					Add(new cOsdItem(cString::sprintf("%s\t%s", tr("Watching"), *value)));
				}
				else if (strcasecmp("REC", *key) == 0) {
					Add(new cOsdItem(cString::sprintf("%s\t%s", tr("Recording"), *value)));
				}
				else {
					dsyslog("peer: Unknown PLUG peer STAT value '%s = %s'", *key, *value);
				}
			}
		}
		Display();
	}
}

eOSState cMenuPeerDetails::ProcessKey(eKeys Key)
{
	eOSState state = cOsdMenu::ProcessKey(Key);
	if (state == osUnknown) {
		switch (Key) {
			case kRed:
				return AddSubMenu(new cMenuPeerMessage(peer));
				break;
			case kGreen: {
					cSvdrp *svdrp = cSvdrp::GetInstance();
					if (svdrp->Connect(peer.ip, peer.port)) {
						if (!svdrp->CmdPEER_NTFY(cString::sprintf("%s:%d", PeerSetup.localIp, PeerSetup.localPort)))
							Skins.Message(mtError, tr("Failed to register notification"));
						svdrp->Disconnect();
					}
					else
						Skins.Message(mtError, tr("Connection to peer failed"));
					return osEnd;
				}
				break;
			default:
				break;
		}
	}
	return state;
}

// -------------- cMenuPeer ----------------
cMenuPeer::cMenuPeer(const char *Title): cOsdMenu(Title, MAX_NAME_LENGTH) {
	helpKeys = -1;
	hasRemoteOsd = cPluginManager::CallFirstService("RemoteOsd::Menu-v1.0", NULL);
	hasRemoteTimers = cPluginManager::CallFirstService("RemoteTimers::Menu-v1.0", NULL);
#if VDRVERSNUM >= 10728
	SetMenuCategory(mcPlugin);
#endif
	for (int i = 0; i < MAX_PEERS; ++i) {
		if (*PeerSetup.peers[i].ip)
			Add(new cMenuPeerItem(PeerSetup.peers[i]));
	}
	SetCurrent(First());
	SetHelpKeys();
}

cMenuPeer::~cMenuPeer()
{}

void cMenuPeer::SetHelpKeys()
{
	int newHelpKeys = 0;
	cMenuPeerItem *current = (cMenuPeerItem *) Get(Current());
	if (current && *current->Peer().wol)
		newHelpKeys = 1;
	if (newHelpKeys != helpKeys) {
		helpKeys = newHelpKeys;
		SetHelp(hasRemoteTimers ? trVDR("Button$Timer") : NULL, hasRemoteTimers ? trVDR("Button$Schedule") : NULL, hasRemoteOsd ? trVDR("Button$Menu") : NULL, newHelpKeys ? tr("Wakeup") : NULL);
	}
}

eOSState cMenuPeer::ProcessKey(eKeys Key)
{
	eOSState state = cOsdMenu::ProcessKey(Key);
	cMenuPeerItem *current = (cMenuPeerItem*) Get(Current());
	if (state == osUnknown && current) {
		const cPeer &peer = current->Peer();
		switch (Key) {
			case kRed:
				if (hasRemoteTimers) {
					struct RemoteTimers_Menu_v1_0 rt;
					rt.serverIp = peer.ip;
					rt.serverPort = peer.port;
					rt.state = osTimers;
					cPluginManager::CallFirstService("RemoteTimers::Menu-v1.0", &rt);
					if (rt.menu)
						return AddSubMenu(rt.menu);
					return osContinue;
				}
				break;
			case kGreen:
				if (hasRemoteTimers) {
					struct RemoteTimers_Menu_v1_0 rt;
					rt.serverIp = peer.ip;
					rt.serverPort = peer.port;
					rt.state = osSchedule;
					cPluginManager::CallFirstService("RemoteTimers::Menu-v1.0", &rt);
					if (rt.menu)
						return AddSubMenu(rt.menu);
					return osContinue;
				}
				break;
			case kYellow: 
				if (hasRemoteOsd) {
					struct RemoteOsd_Menu_v1_0 ro;
					ro.serverIp = peer.ip;
					ro.serverPort = peer.port;
					ro.key = "MENU";
					cPluginManager::CallFirstService("RemoteOsd::Menu-v1.0", &ro);
					if (ro.menu)
						return AddSubMenu(ro.menu);
					Display();
					return osContinue;
				}
				break;
			case kBlue: 
				if (*peer.wol) {
					if (WakeOnLAN(peer.wol))
						Skins.Message(mtInfo, tr("WoL packet sent"));
					else
						Skins.Message(mtError, tr("Failed to send WoL packet"));
				}
				break;
			case kOk: {
					cSvdrp *svdrp = cSvdrp::GetInstance();
					if (svdrp->Connect(peer.ip, peer.port)) {
						state = AddSubMenu(new cMenuPeerDetails(peer));
						svdrp->Disconnect();
					}
					else {
						Skins.Message(mtError, tr("Connection to peer failed"));
						state = osContinue;
					}
					return state;
				}
				break;
			default:
				break;
		}
	}
	if (Key != kNone)
		SetHelpKeys();
	return state;
}

