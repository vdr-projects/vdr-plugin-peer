/*
 * remoteosd.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <vdr/menuitems.h>
#include <vdr/i18n.h>
#include "setup.h"

cPeerSetup PeerSetup;

cPeer::cPeer()
{
	name[0] = 0;
	wol[0] = 0;
	ip[0] = 0;
	port = 0;
	configured = false;
}

cPeer& cPeer::operator=(const cPeer &Setup)
{
	strn0cpy(name, Setup.name, sizeof(name));
	strn0cpy(wol, Setup.wol, sizeof(wol));
	strn0cpy(ip, Setup.ip, sizeof(ip));
	port = Setup.port;
	configured = Setup.configured;
	return *this;
}

bool cPeer::Parse(const char *Name, const char *Value)
{
	if (!strcasecmp(Name, "_Name"))
		strn0cpy(name, Value, sizeof(name));
	else if (!strcasecmp(Name, "_WoL"))
		strn0cpy(wol, Value, sizeof(wol));
	else if (!strcasecmp(Name, "_IP"))
		strn0cpy(ip, Value, sizeof(ip));
	else if (!strcasecmp(Name, "_Port"))
		port = atoi(Value);
	else
		return false;
	configured = true;
	return true;
}

cPeerSetup::cPeerSetup()
{
	hideMainMenuEntry = 0;
	name[0] = 0;
	wol = 0;
	localIp[0] = 0;
	localPort = 6419;
}

bool cPeerSetup::Parse(const char *Name, const char *Value)
{
	if (!strcasecmp(Name, "HideMainMenuEntry"))
		hideMainMenuEntry = atoi(Value);
	else if (!strcasecmp(Name, "Name"))
		strn0cpy(name, Value, sizeof(name));
	else if (!strcasecmp(Name, "WoL"))
		wol = atoi(Value);
	else if (!strcasecmp(Name, "LocalIp"))
		strn0cpy(localIp, Value, sizeof(localIp));
	else if (!strcasecmp(Name, "LocalPort"))
		localPort = atoi(Value);
	else if (!strncasecmp(Name, "Peer", 4)) {
		char *p = NULL;
		int i = strtol(Name + 4, &p, 10) - 1;
		if (p != Name + 4 && 0 <= i && i < MAX_PEERS)
			return peers[i].Parse(p, Value);
		else
			return false;
	}
	else
		return false;
	return true;
}

cPeerSetup& cPeerSetup::operator=(const cPeerSetup &Setup)
{
	hideMainMenuEntry = Setup.hideMainMenuEntry;
	strn0cpy(name, Setup.name, sizeof(name));
	wol = Setup.wol;
	strn0cpy(localIp, Setup.localIp, sizeof(localIp));
	localPort = Setup.localPort;
	for (int i = 0; i < MAX_PEERS; ++i)
		peers[i] = Setup.peers[i];
	return *this;
}

void cPeerMenuSetup::Store()
{
	SetupStore("HideMainMenuEntry", setupTmp.hideMainMenuEntry);
	SetupStore("Name", setupTmp.name);
	SetupStore("WoL", setupTmp.wol);
	SetupStore("LocalIp", setupTmp.localIp);
	SetupStore("LocalPort", setupTmp.localPort);
	for (int i = 0; i < MAX_PEERS; ++i) {
		if (setupTmp.peers[i].configured) {
			SetupStore(cString::sprintf("Peer%d_Name", i + 1), setupTmp.peers[i].name);
			SetupStore(cString::sprintf("Peer%d_WoL", i + 1), setupTmp.peers[i].wol);
			SetupStore(cString::sprintf("Peer%d_IP", i + 1), setupTmp.peers[i].ip);
			SetupStore(cString::sprintf("Peer%d_Port", i + 1), setupTmp.peers[i].port);
		}
	}
	PeerSetup = setupTmp;
}

cPeerMenuSetup::cPeerMenuSetup()
{
	setupTmp = PeerSetup;

	Add(new cMenuEditBoolItem(tr("Hide mainmenu entry"), &setupTmp.hideMainMenuEntry));
	//Add(new cOsdItem(tr("Settings published to peers"), osUnknown, false));
	//Add(new cMenuEditStrItem(trVDR("Name"), setupTmp.name, sizeof(setupTmp.name)));
	//Add(new cMenuEditBoolItem(tr("Wake on LAN"), &setupTmp.wol));
	Add(new cOsdItem(tr("Settings for callback actions"), osUnknown, false));
	Add(new cMenuEditStrItem(tr("Local IP"), setupTmp.localIp, sizeof(setupTmp.localIp), ".1234567890"));
	Add(new cMenuEditIntItem(tr("Local SVDRP port"), &setupTmp.localPort, 1, 65535));
	Add(new cOsdItem(tr("Peers"), osUnknown, false));
	for (int i = 0; i < MAX_PEERS; ++i) {
		if (setupTmp.peers[i].configured)
			AddPeer(i + 1, setupTmp.peers[i]);
	}
	SetHelp(NULL, trVDR("Button$New"), NULL, NULL);
}

cPeerMenuSetup::~cPeerMenuSetup()
{}

void cPeerMenuSetup::AddPeer(int Number, cPeer &Peer)
{
	Add(new cMenuEditStrItem(cString::sprintf(tr("Peer %d IP"), Number), Peer.ip, sizeof(Peer.ip), ".1234567890"));
	Add(new cMenuEditIntItem(cString::sprintf(tr("Peer %d Port"), Number), &Peer.port, 0, 65535));
	Add(new cMenuEditStrItem(cString::sprintf(tr("Peer %d Name"), Number), Peer.name, sizeof(Peer.name)));
	Add(new cMenuEditStrItem(cString::sprintf(tr("Peer %d WoL"), Number), Peer.wol, sizeof(Peer.wol), ":1234567890abcdef"));
	Display();
}

eOSState cPeerMenuSetup::ProcessKey(eKeys Key)
{
	eOSState state = cMenuSetupPage::ProcessKey(Key);
	if (state == osUnknown && Key != kNone) {
		switch (Key) {
			case kGreen:
				for (int i = 0; i < MAX_PEERS; ++i) {
					if (!setupTmp.peers[i].configured) {
						AddPeer(i + 1, setupTmp.peers[i]);
						setupTmp.peers[i].configured = true;
						return osContinue;
					}
				}
				Skins.Message(mtError, "Too many peers!");
				break;
			default:
				break;
		}
	}
	return state;
}
