#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <generated/protocol.h>

#include <game/client/gameclient.h>
#include <game/client/render.h>
#include <game/client/ui.h>

#include "bindwheel.h"

CBindWheelSpec::CBindWheelSpec()
{
	OnReset();
}

void CBindWheelSpec::ConBindwheelExecuteHover(IConsole::IResult *pResult, void *pUserData)
{
	CBindWheelSpec *pThis = (CBindWheelSpec *)pUserData;
	pThis->ExecuteHoveredBind();
}

void CBindWheelSpec::ConOpenBindwheel(IConsole::IResult *pResult, void *pUserData)
{
	CBindWheelSpec *pThis = (CBindWheelSpec *)pUserData;
	if(pThis->GameClient()->m_Snap.m_SpecInfo.m_SpectatorId != SPEC_FREEVIEW && pThis->GameClient()->m_Snap.m_SpecInfo.m_Active)
	{
		if(pThis->Client()->State() != IClient::STATE_DEMOPLAYBACK)
		{
			if(pThis->GameClient()->m_Emoticon.IsActive())
				pThis->m_Active = false;
			else
				pThis->m_Active = pResult->GetInteger(0) != 0;
		}
	}
	else
		pThis->GameClient()->Echo("You're not spectating the player");
}

void CBindWheelSpec::ConAddBindwheelLegacy(IConsole::IResult *pResult, void *pUserData)
{
	int BindPos = pResult->GetInteger(0);
	if(BindPos < 0 || BindPos >= BINDWHEEL_MAX_BINDS_RCLIENT)
		return;

	const char *aName = pResult->GetString(1);
	const char *aCommand = pResult->GetString(2);

	CBindWheelSpec *pThis = static_cast<CBindWheelSpec *>(pUserData);
	if(pThis->m_vBinds.size() <= (size_t)BindPos)
		pThis->m_vBinds.resize((size_t)BindPos + 1);

	str_copy(pThis->m_vBinds[BindPos].m_aName, aName);
	str_copy(pThis->m_vBinds[BindPos].m_aCommand, aCommand);
}

void CBindWheelSpec::ConAddBindwheel(IConsole::IResult *pResult, void *pUserData)
{
	const char *aName = pResult->GetString(0);
	const char *aCommand = pResult->GetString(1);

	CBindWheelSpec *pThis = static_cast<CBindWheelSpec *>(pUserData);
	pThis->AddBind(aName, aCommand);
}

void CBindWheelSpec::ConRemoveBindwheel(IConsole::IResult *pResult, void *pUserData)
{
	const char *aName = pResult->GetString(0);
	const char *aCommand = pResult->GetString(1);

	CBindWheelSpec *pThis = static_cast<CBindWheelSpec *>(pUserData);
	pThis->RemoveBind(aName, aCommand);
}

void CBindWheelSpec::ConRemoveAllBindwheelBinds(IConsole::IResult *pResult, void *pUserData)
{
	CBindWheelSpec *pThis = static_cast<CBindWheelSpec *>(pUserData);
	pThis->RemoveAllBinds();
}

void CBindWheelSpec::AddBind(const char *pName, const char *pCommand)
{
	if((pName[0] == '\0' && pCommand[0] == '\0') || m_vBinds.size() >= BINDWHEEL_MAX_BINDS_RCLIENT)
		return;

	CBind Bind;
	str_copy(Bind.m_aName, pName);
	str_copy(Bind.m_aCommand, pCommand);
	m_vBinds.push_back(Bind);
}

void CBindWheelSpec::RemoveBind(const char *pName, const char *pCommand)
{
	CBind Bind;
	str_copy(Bind.m_aName, pName);
	str_copy(Bind.m_aCommand, pCommand);
	auto it = std::find(m_vBinds.begin(), m_vBinds.end(), Bind);
	if(it != m_vBinds.end())
		m_vBinds.erase(it);
}

void CBindWheelSpec::RemoveBind(int Index)
{
	if(Index >= static_cast<int>(m_vBinds.size()) || Index < 0)
		return;
	auto Pos = m_vBinds.begin() + Index;
	m_vBinds.erase(Pos);
}

void CBindWheelSpec::RemoveAllBinds()
{
	m_vBinds.clear();
}

void CBindWheelSpec::OnConsoleInit()
{
	IConfigManager *pConfigManager = Kernel()->RequestInterface<IConfigManager>();
	if(pConfigManager)
		pConfigManager->RegisterCallback(ConfigSaveCallback, this, ConfigDomain::RCLIENT);

	Console()->Register("+bindwheel_spec", "", CFGFLAG_CLIENT, ConOpenBindwheel, this, "Open bindwheel selector");
	Console()->Register("+bindwheel_execute_hover_spec", "", CFGFLAG_CLIENT, ConBindwheelExecuteHover, this, "Execute hovered bindwheel bind");

	Console()->Register("bindwheel_spec", "i[index] s[name] s[command]", CFGFLAG_CLIENT, ConAddBindwheelLegacy, this, "DONT USE THIS! USE add_bindwheel_spec INSTEAD!");
	Console()->Register("add_bindwheel_spec", "s[name] s[command]", CFGFLAG_CLIENT, ConAddBindwheel, this, "Add a bind to the bindwheel");
	Console()->Register("remove_bindwheel_spec", "s[name] s[command]", CFGFLAG_CLIENT, ConRemoveBindwheel, this, "Remove a bind from the bindwheel");
	Console()->Register("delete_all_bindwheel_binds_spec", "", CFGFLAG_CLIENT, ConRemoveAllBindwheelBinds, this, "Removes all bindwheel binds");
}

void CBindWheelSpec::OnReset()
{
	m_WasActive = false;
	m_Active = false;
	m_SelectedBind = -1;
}

void CBindWheelSpec::OnRelease()
{
	m_Active = false;
}

bool CBindWheelSpec::OnCursorMove(float x, float y, IInput::ECursorType CursorType)
{
	if(!m_Active)
		return false;

	Ui()->ConvertMouseMove(&x, &y, CursorType);
	GameClient()->m_Emoticon.m_SelectorMouse += vec2(x, y);
	return true;
}

bool CBindWheelSpec::OnInput(const IInput::CEvent &Event)
{
	if(IsActive() && Event.m_Flags & IInput::FLAG_PRESS && Event.m_Key == KEY_ESCAPE)
	{
		OnRelease();
		return true;
	}
	return false;
}

void CBindWheelSpec::OnRender()
{
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;

	static const auto QuadEaseInOut = [](float t) -> float {
		if(t == 0.0f)
			return 0.0f;
		if(t == 1.0f)
			return 1.0f;
		return (t < 0.5f) ? (2.0f * t * t) : (1.0f - std::pow(-2.0f * t + 2.0f, 2) / 2.0f);
	};

	static const float s_InnerOuterMouseBoundaryRadius = 110.0f;
	static const float s_OuterMouseLimitRadius = 170.0f;
	static const float s_OuterItemRadius = 140.0f; // 10.0f less than emoticons for extra text space
	static const float s_OuterCircleRadius = 190.0f;
	// static const float s_InnerCircleRadius = 100.0f;
	static const float s_FontSize = 12.0f;
	static const float s_FontSizeSelected = 18.0f;

	const float AnimationTime = (float)g_Config.m_TcAnimateWheelTime / 1000.0f;
	const float ItemAnimationTime = AnimationTime / 2.0f;

	if(AnimationTime != 0.0f)
	{
		for(float &Time : m_aAnimationTimeItems)
		{
			Time -= Client()->RenderFrameTime();
			if(Time <= 0.0f)
				Time = 0.0f;
		}
	}

	if(!m_Active)
	{
		if(m_WasActive)
		{
			if(g_Config.m_TcResetBindWheelMouse)
				GameClient()->m_Emoticon.m_SelectorMouse = vec2(0.0f, 0.0f);
			if(m_SelectedBind != -1)
				ExecuteBindWithSpec(m_SelectedBind);
		}
		m_WasActive = false;

		if(AnimationTime == 0.0f)
			return;

		m_AnimationTime -= Client()->RenderFrameTime() * 3.0f; // Close animation 3x faster
		if(m_AnimationTime <= 0.0f)
		{
			m_AnimationTime = 0.0f;
			return;
		}
	}
	else
	{
		m_AnimationTime += Client()->RenderFrameTime();
		if(m_AnimationTime > AnimationTime)
			m_AnimationTime = AnimationTime;
		m_WasActive = true;
	}

	const CUIRect Screen = *Ui()->Screen();

	const bool WasTouchPressed = GameClient()->m_Emoticon.m_TouchState.m_AnyPressed;
	Ui()->UpdateTouchState(GameClient()->m_Emoticon.m_TouchState);
	if(GameClient()->m_Emoticon.m_TouchState.m_AnyPressed)
	{
		const vec2 TouchPos = (GameClient()->m_Emoticon.m_TouchState.m_PrimaryPosition - vec2(0.5f, 0.5f)) * Screen.Size();
		const float TouchCenterDistance = length(TouchPos);
		if(TouchCenterDistance <= s_OuterMouseLimitRadius)
		{
			GameClient()->m_Emoticon.m_SelectorMouse = TouchPos;
		}
		else if(TouchCenterDistance > s_OuterCircleRadius)
		{
			GameClient()->m_Emoticon.m_TouchPressedOutside = true;
		}
	}
	else if(WasTouchPressed)
	{
		m_Active = false;
	}

	std::array<float, 2> aAnimationPhase;
	if(AnimationTime == 0.0f)
	{
		aAnimationPhase.fill(1.0f);
	}
	else
	{
		aAnimationPhase[0] = QuadEaseInOut(m_AnimationTime / AnimationTime);
		aAnimationPhase[1] = aAnimationPhase[0] * aAnimationPhase[0];
	}

	if(length(GameClient()->m_Emoticon.m_SelectorMouse) > s_OuterMouseLimitRadius)
		GameClient()->m_Emoticon.m_SelectorMouse = normalize(GameClient()->m_Emoticon.m_SelectorMouse) * s_OuterMouseLimitRadius;

	int SegmentCount = m_vBinds.size();
	if(SegmentCount == 0)
	{
		m_SelectedBind = -1;
	}
	else
	{
		float SegmentAngle = 2.0f * pi / SegmentCount;
		float SelectedAngle = angle(GameClient()->m_Emoticon.m_SelectorMouse) + SegmentAngle / 2.0f;
		if(SelectedAngle < 0.0f)
			SelectedAngle += 2.0f * pi;
		if(length(GameClient()->m_Emoticon.m_SelectorMouse) > s_InnerOuterMouseBoundaryRadius)
			m_SelectedBind = (int)(SelectedAngle / (2.0f * pi) * SegmentCount);
		else
			m_SelectedBind = -1;
	}

	if(m_SelectedBind != -1)
	{
		m_aAnimationTimeItems[m_SelectedBind] += Client()->RenderFrameTime() * 2.0f; // To counteract earlier decrement
		if(m_aAnimationTimeItems[m_SelectedBind] >= ItemAnimationTime)
			m_aAnimationTimeItems[m_SelectedBind] = ItemAnimationTime;
	}

	Ui()->MapScreen();

	Graphics()->BlendNormal();
	Graphics()->TextureClear();
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.3f * aAnimationPhase[0]);
	Graphics()->DrawCircle(Screen.w / 2.0f, Screen.h / 2.0f, s_OuterCircleRadius * aAnimationPhase[0], 64);
	Graphics()->QuadsEnd();

	Graphics()->WrapClamp();
	const float Theta = pi * 2.0f / m_vBinds.size();
	for(int i = 0; i < static_cast<int>(m_vBinds.size()); i++)
	{
		const CBind &Bind = m_vBinds[i];
		const float Angle = Theta * i;
		const vec2 Pos = direction(Angle) * s_OuterItemRadius * aAnimationPhase[1];
		const float Phase = ItemAnimationTime == 0.0f ? (i == m_SelectedBind ? 1.0f : 0.0f) : QuadEaseInOut(m_aAnimationTimeItems[i] / ItemAnimationTime);
		const float FontSize = (s_FontSize + Phase * (s_FontSizeSelected - s_FontSize)) * aAnimationPhase[1];
		const char *pName = Bind.m_aName;
		if(pName[0] == '\0')
		{
			pName = "Empty";
			TextRender()->TextColor(0.7f, 0.7f, 0.7f, aAnimationPhase[1]);
		}
		else
		{
			TextRender()->TextColor(1.0f, 1.0f, 1.0f, aAnimationPhase[1]);
		}
		float Width = TextRender()->TextWidth(FontSize, pName);
		TextRender()->Text(Screen.w / 2.0f + Pos.x - Width / 2.0f, Screen.h / 2.0f + Pos.y - FontSize / 2.0f, FontSize, pName);
	}
	TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
	Graphics()->WrapNormal();

	// For future middle circle usage
	// Graphics()->TextureClear();
	// Graphics()->QuadsBegin();
	// Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.3f * AnimationPhase3);
	// DrawCircle(Screen.w / 2.0f, Screen.h / 2.0f, s_InnerCircleRadius * AnimationPhase3, 64);
	// Graphics()->QuadsEnd();

	RenderTools()->RenderCursor(GameClient()->m_Emoticon.m_SelectorMouse + vec2(Screen.w, Screen.h) / 2.0f, 24.0f, aAnimationPhase[0]);
}

void CBindWheelSpec::ExecuteBind(int Bind)
{
	Console()->ExecuteLine(m_vBinds[Bind].m_aCommand);
}
void CBindWheelSpec::ExecuteHoveredBind()
{
	if(m_SelectedBind >= 0)
		ExecuteBindWithSpec(m_SelectedBind);
}
void CBindWheelSpec::ExecuteBindWithSpec(int Bind)
{
	char PlayerName[32];
	int PlayerId = -1;
	if(GameClient()->m_Snap.m_SpecInfo.m_SpectatorId != SPEC_FREEVIEW && GameClient()->m_Snap.m_SpecInfo.m_Active)
	{
		const auto &Player = GameClient()->m_aClients[GameClient()->m_Snap.m_SpecInfo.m_SpectatorId];
		str_copy(PlayerName, Player.m_aName, sizeof(PlayerName));
		PlayerId = Player.ClientId();
	}
	else
		GameClient()->Echo("You're not spectating the player");
	if(PlayerId != -1)
	{
		std::string cmd = m_vBinds[Bind].m_aCommand;

		// Заменяем "playernickname" на PlayerName
		std::string to_replace_name = "playernickname";
		size_t pos = 0;
		while((pos = cmd.find(to_replace_name, pos)) != std::string::npos)
		{
			cmd.replace(pos, to_replace_name.length(), PlayerName);
			pos += strlen(PlayerName);
		}

		// Заменяем "playerid" на строковое представление PlayerId
		std::string to_replace_id = "playerid";
		std::string player_id_str = std::to_string(PlayerId);
		pos = 0;
		while((pos = cmd.find(to_replace_id, pos)) != std::string::npos)
		{
			cmd.replace(pos, to_replace_id.length(), player_id_str);
			pos += player_id_str.length();
		}

		dbg_msg("bindwheel", "Executing command: %s", cmd.c_str());
		Console()->ExecuteLine(cmd.c_str());
	}
}
void CBindWheelSpec::ConfigSaveCallback(IConfigManager *pConfigManager, void *pUserData)
{
	CBindWheelSpec *pThis = (CBindWheelSpec *)pUserData;

	for(CBind &Bind : pThis->m_vBinds)
	{
		char aBuf[BINDWHEEL_MAX_CMD_RCLIENT * 2] = "";
		char *pEnd = aBuf + sizeof(aBuf);
		char *pDst;
		str_append(aBuf, "add_bindwheel_spec \"");
		// Escape name
		pDst = aBuf + str_length(aBuf);
		str_escape(&pDst, Bind.m_aName, pEnd);
		str_append(aBuf, "\" \"");
		// Escape command
		pDst = aBuf + str_length(aBuf);
		str_escape(&pDst, Bind.m_aCommand, pEnd);
		str_append(aBuf, "\"");
		pConfigManager->WriteLine(aBuf, ConfigDomain::RCLIENT);
	}
}
