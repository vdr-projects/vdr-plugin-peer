/*
 * setup.h: Settings
 *
 * See the README file for copyright information and how to reach the author.
 */

#ifndef _PEER_SETUP__H
#define _PEER_SETUP__H

#include <vdr/config.h>

#define MAX_PEERS 16
#define MAX_NAME_LENGTH 22
#define MAX_IP_LENGTH 16
#define MAX_WOL_LENGTH 18

struct cPeer {
	char name[MAX_NAME_LENGTH];
	char wol[MAX_WOL_LENGTH];
	char ip[MAX_IP_LENGTH];
	int port;
	bool configured;

	const char *IpPort() const { return port ? *cString::sprintf("%s:%d", ip, port) : ip; }
	const char *Name() const { return *name ? name : IpPort(); }
	bool Parse(const char *Name, const char *Value);
	cPeer& operator=(const cPeer &Setup);
	cPeer();
};

struct cPeerSetup {
	int hideMainMenuEntry;
	char name[MAX_NAME_LENGTH];
	int wol;
	char localIp[MAX_IP_LENGTH];
	int localPort;
	cPeer peers[MAX_PEERS];

	bool Parse(const char *Name, const char *Value);
	cPeerSetup& operator=(const cPeerSetup &Setup);
	cPeerSetup();
};

extern cPeerSetup PeerSetup;

class cPeerMenuSetup: public cMenuSetupPage {
	private:
		cPeerSetup	setupTmp;

		void AddPeer(int Number, cPeer& Peer);
	protected:
		virtual eOSState ProcessKey(eKeys Key);
		virtual void Store(void);
	public:
		cPeerMenuSetup();
		virtual ~cPeerMenuSetup();
};

#endif //_PEER_SETUP__H
