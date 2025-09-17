#ifndef GAME_CLIENT_COMPONENTS_TCLIENT_BINDCHAT_H
#define GAME_CLIENT_COMPONENTS_TCLIENT_BINDCHAT_H

#include <game/client/component.h>
#include <game/client/lineinput.h>

#include <engine/console.h>

class IConfigManager;

enum
{
	BINDCHAT_MAX_NAME = 64,
	BINDCHAT_MAX_PARAMS = 64,
	BINDCHAT_MAX_HELP = 128,
	BINDCHAT_MAX_CMD = 1024,
	BINDCHAT_MAX_BINDS = 256,
};

class CBindChat : public CComponent
{
public:
	class CBind
	{
	public:
		bool m_IsEx = false;
		char m_aName[BINDCHAT_MAX_NAME];
		char m_aParams[BINDCHAT_MAX_PARAMS];
		char m_aHelp[BINDCHAT_MAX_HELP];
		char m_aCommand[BINDCHAT_MAX_CMD];
		CBind() = default;
		CBind(const char *pName, const char *pCommand);
		CBind(const char *pName, const char *pParams, const char *pHelp, const char *pCommand);
		bool CompContent(const CBind &Other) const;
	};
	class CBindDefault
	{
	public:
		const char *m_pTitle;
		CBind m_Bind;
		CLineInput m_LineInput;
	};
	static std::vector<std::pair<const char *, std::vector<CBindDefault>>> BIND_DEFAULTS;
	class CBindRclient
	{
	public:
		const char *m_pTitle;
		CBind m_Bind;
		CLineInput m_LineInput;
		CBindRclient(const char *pTitle, const char *pName, const char *pCommand)
		{
			m_pTitle = pTitle;
			m_Bind = CBind(pName, pCommand);
		}
	};

private:
	static void ConAddBindchat(IConsole::IResult *pResult, void *pUserData);
	static void ConAddBindchatEx(IConsole::IResult *pResult, void *pUserData);
	static void ConBindchats(IConsole::IResult *pResult, void *pUserData);
	static void ConRemoveBindchat(IConsole::IResult *pResult, void *pUserData);
	static void ConRemoveBindchatAll(IConsole::IResult *pResult, void *pUserData);
	static void ConBindchatDefaults(IConsole::IResult *pResult, void *pUserData);

	static void ConfigSaveCallback(IConfigManager *pConfigManager, void *pUserData);

	static void ExecuteBindExCallback(IConsole::IResult *pResult, void *pUserData);
	void ExecuteBind(const CBind &Bind, const char *pArgs);

public:
	std::vector<CBind> m_vBinds;

	CBindChat();
	int Sizeof() const override { return sizeof(*this); }
	void OnConsoleInit() override;

	void AddBind(const char *pName, const char *pParams, const char *pHelp, const char *pCommand, bool IsEx);
	void AddBind(const CBind &Bind);

	void RemoveBindCommand(const char *pCommand);
	bool RemoveBind(const char *pName);
	void RemoveAllBinds();

	CBind *GetBind(const char *pCommand);

	bool CheckBindChat(const char *pText);
	bool ChatDoBinds(const char *pText);
	bool ChatDoAutocomplete(bool ShiftPressed);
};

static CBindChat::CBindRclient s_aDefaultBindChatRclientFindSkin[] = {
	{"Find skin", ".findskin", "find_skin"},
	{"Copy skin", ".copyskin", "copy_skin"},
	{"Find player", ".findplayer", "find_player"},
	{"Copy player", ".copyplayer", "copy_player"},
	{"Copy color", ".copycolor", "copy_color"},
	{"Find skin from ddstats", ".findskinddstats", "ri_find_skin_from_ddstats"},
	{"Copy skin from ddstats", ".copyskinddstats", "ri_copy_skin_from_ddstats"},
	{"Find player from ddstats", ".findplayerddstats", "ri_find_player_from_ddstats"},
	{"Copy player from ddstats", ".copyplayerddstats", "ri_copy_player_from_ddstats"},
	{"Backup profile after copy", ".backupprofile", "ri_backup_player_profile"},
};
static CBindChat::CBindRclient s_aDefaultBindChatRclientChat[] = {
	{"Find hours", ".findhours", "find_hours"},
	{"Add a word to the censor list", ".addcensor", "add_censor_list"},
	{"Find time on other map", ".findtime", "ri_find_time_on_map"},
	{"Search map info", ".mapinfo", "ri_search_map_info"},
	{"Add player to the white list", ".addwhitelist", "add_white_list"},
};
static CBindChat::CBindRclient s_aDefaultBindChatRclientTracker[] = {
	{"Tracker player", ".track", "tracker"},
	{"Tracker player reset", ".trackres", "tracker_reset"},
	{"Tracker player remove", ".trackrem", "tracker_remove"},
};
static CBindChat::CBindRclient s_aDefaultBindChatRclientTrackerHistory[] = {
	{"Tracker player", ".trackplayer", "target_player_pos"},
	{"Tracker player reset", ".trackplayerres", "target_player_pos_reset"},
};
#endif
