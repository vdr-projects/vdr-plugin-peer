/*
 * peer.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <vdr/menu.h>
#include <vdr/device.h>
#include <vdr/plugin.h>
#include <vdr/channels.h>
#include "menu.h"
#include "setup.h"
#include "status.h"
#include "tools.h"

#define PEER_BUFSIZE KILOBYTE(1)

static const char *VERSION        = "0.0.1";
static const char *DESCRIPTION    = "Enter description for 'peer' plugin";
static const char *MAINMENUENTRY  = "VDR Peers";

//extern cNestedItemList Peers;

class cPluginPeer : public cPlugin {
private:
  cPeerStatus *status;

public:
  cPluginPeer(void);
  virtual ~cPluginPeer();
  virtual const char *Version(void) { return VERSION; }
  virtual const char *Description(void) { return DESCRIPTION; }
  virtual const char *CommandLineHelp(void);
  virtual bool ProcessArgs(int argc, char *argv[]);
  virtual bool Initialize(void);
  virtual bool Start(void);
  virtual void Stop(void);
  virtual void Housekeeping(void);
  virtual void MainThreadHook(void);
  virtual cString Active(void);
  virtual time_t WakeupTime(void);
  virtual const char *MainMenuEntry(void) { return PeerSetup.hideMainMenuEntry ? NULL : tr(MAINMENUENTRY); }
  virtual cOsdObject *MainMenuAction(void);
  virtual cMenuSetupPage *SetupMenu(void);
  virtual bool SetupParse(const char *Name, const char *Value);
  virtual bool Service(const char *Id, void *Data = NULL);
  virtual const char **SVDRPHelpPages(void);
  virtual cString SVDRPCommand(const char *Command, const char *Option, int &ReplyCode);
  };

cPluginPeer::cPluginPeer(void)
{
  status = NULL;
}

cPluginPeer::~cPluginPeer()
{
  delete status;
}

const char *cPluginPeer::CommandLineHelp(void)
{
  // Return a string that describes all known command line options.
  return NULL;
}

bool cPluginPeer::ProcessArgs(int argc, char *argv[])
{
  // Implement command line argument processing here if applicable.
  return true;
}

bool cPluginPeer::Initialize(void)
{
  //Peers.Load(AddDirectory(cPlugin::ConfigDirectory(PLUGIN_NAME_I18N), "peers.conf"));
  return true;
}

bool cPluginPeer::Start(void)
{
  status = new cPeerStatus();
  return true;
}

void cPluginPeer::Stop(void)
{
  // Stop any background activities the plugin is performing.
}

void cPluginPeer::Housekeeping(void)
{
  // Perform any cleanup or other regular tasks.
}

void cPluginPeer::MainThreadHook(void)
{
  // Perform actions in the context of the main program thread.
  // WARNING: Use with great care - see PLUGINS.html!
}

cString cPluginPeer::Active(void)
{
  // Return a message string if shutdown should be postponed
  return NULL;
}

time_t cPluginPeer::WakeupTime(void)
{
  // Return custom wakeup time for shutdown script
  return 0;
}

cOsdObject *cPluginPeer::MainMenuAction(void)
{
  return new cMenuPeer(tr(MAINMENUENTRY));
}

cMenuSetupPage *cPluginPeer::SetupMenu(void)
{
  return new cPeerMenuSetup();
}

bool cPluginPeer::SetupParse(const char *Name, const char *Value)
{
  return PeerSetup.Parse(Name, Value);
}

bool cPluginPeer::Service(const char *Id, void *Data)
{
  // Handle custom service requests from other plugins
  return false;
}

const char **cPluginPeer::SVDRPHelpPages(void)
{
  // Return help text for SVDRP commands this plugin implements
  return NULL;
}

cString cPluginPeer::SVDRPCommand(const char *Command, const char *Option, int &ReplyCode)
{
  cStringBuffer buffer;
  ReplyCode = 250;
  if (strcasecmp(Command, "STAT") == 0) {
	const char* replay = status->Replaying();
	const cList<cPeerStatusRecording> *recs = status->Recording();
	for (int i = 0; i < cDevice::NumDevices(); i++) {
		if (const cDevice *d = cDevice::GetDevice(i)) {
			buffer.Append("DEV = %d\n", d->DeviceNumber())
#if VDRVERSNUM >=10727
				.Append("NAME = %s\n", *d->DeviceName())
#endif
#if VDRVERSNUM >=10728
				.Append("TYPE = %s\n", *d->DeviceType())
#endif
				.Append("PRIO = %d\n", d->Priority());
			if (replay && d->IsPrimaryDevice()) {
				buffer.Append("PLAY = %s\n", replay);
			}
			else if (!replay && (d->IsPrimaryDevice() || d == cDevice::ActualDevice())) {
				cChannel *c = Channels.GetByNumber(cDevice::CurrentChannel());
				if (c)
					buffer.Append("LIVE = %s\n", c->Name());
			}
			for (cPeerStatusRecording * r = recs->First(); r; r = recs->Next(r)) {
				if (d == r->Device())
					buffer.Append("REC = %s\n", r->Name());

			}
		}
	}
  }
  else if (strcasecmp(Command, "NTFY") == 0) {
	if (*Option) {
		status->AddReplayNotify(Option);
		buffer.Append("Request filed. Await confirmation message on screen");
	}
	else {
		ReplyCode = 501;
		buffer.Append("Missing parameter 'IP:Port'");
	}
  }
/*
  else if (strcasecmp(Command, "CONF") == 0) {
	if (*PeerSetup.name)
		buffer.Append("Name = %s\n", PeerPluginSetup.name);
	buffer.Append("WoL = %d\n", PeerPluginSetup.wol);
  }
*/
  else
	return NULL;

  const char *reply = buffer.Export();
  if (reply) {
	return cString(reply, true);
  }
  else {
	ReplyCode = 451;
	return cString("Error while appending to buffer");
  }
}

VDRPLUGINCREATOR(cPluginPeer); // Don't touch this!
