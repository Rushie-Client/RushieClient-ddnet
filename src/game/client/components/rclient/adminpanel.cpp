#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <generated/protocol.h>
#include <game/client/gameclient.h>
#include <game/client/render.h>
#include <game/client/ui.h>

#include <game/client/components/camera.h>
#include <game/client/components/chat.h>
#include <game/client/components/console.h>
#include <game/client/components/menus.h>
#include <game/client/components/controls.h>

#include <game/client/components/emoticon.h>
#include "adminpanel.h"

#include "game/localization.h"

#include <vulkan/vulkan_core.h>

struct SPopupProperties
{
	static constexpr float ms_Width = 200.0f;

	static constexpr float ms_HeadlineFontSize = 8.0f;
	static constexpr float ms_FontSize = 8.0f;
	static constexpr float ms_IconFontSize = 11.0f;
	static constexpr float ms_Padding = 6.0f;
	static constexpr float ms_Rounding = 3.0f;

	static constexpr float ms_ItemSpacing = 2.0f;
	static constexpr float ms_GroupSpacing = 5.0f;

	static constexpr float ms_RconActionHight = 25.0f;
	static constexpr float ms_RconActionWidth = 75.0f;
	static constexpr float ms_ReadyButtonsWidth = 80.0f;
	static constexpr float ms_RconTimersWidth = 50.0f;
	static constexpr float ms_ButtonHeight = 12.0f;

	static constexpr float ms_PlayerBtnWidth = 60.0f;

	static ColorRGBA WindowColor() { return ColorRGBA(0.451f, 0.451f, 0.451f, 0.9f); };
	static ColorRGBA WindowColorDark() { return ColorRGBA(0.2f, 0.2f, 0.2f, 0.9f); };
	static ColorRGBA GeneralButtonColor() { return ColorRGBA(0.541f, 0.561f, 0.48f, 0.8f); };
	static ColorRGBA GeneralActiveButtonColor() { return ColorRGBA(0.53f, 0.78f, 0.53f, 0.8f); };

	static ColorRGBA ActionGeneralButtonColor() { return ColorRGBA(0.541f, 0.561f, 0.48f, 0.8f); };
	static ColorRGBA ActionActiveButtonColor() { return ColorRGBA(0.53f, 0.78f, 0.53f, 0.8f); };
	static ColorRGBA ActionAltActiveButtonColor() { return ColorRGBA(1.0f, 0.42f, 0.42f, 0.8f); };

	static ColorRGBA TeamsGeneralButtonColor() { return ColorRGBA(0.32f, 0.32f, 0.72f, 0.8f); };
	static ColorRGBA TeamsActiveButtonColor() { return ColorRGBA(0.31f, 0.52f, 0.78f, 0.8f); };

	static ColorRGBA ActionSpecButtonColor() { return ColorRGBA(0.2f, 1.0f, 0.2f, 0.8f); }; // Bright green color for spec

	static ColorRGBA ActionBanAltButtonColor() { return ColorRGBA(0.7f, 0.1f, 0.1f, 0.8f); };
	static ColorRGBA ActionBanButtonColor() { return ColorRGBA(1.0f, 0.24f, 0.24f, 0.8f); };
	static ColorRGBA ActionKillAltButtonColor() { return ColorRGBA(0.5f, 0.45f, 0.0f, 0.8f); };
	static ColorRGBA ActionKillButtonColor() { return ColorRGBA(1.0f, 0.902f, 0.0f, 0.8f); };
	static ColorRGBA ActionKickAltButtonColor() { return ColorRGBA(0.5f, 0.27f, 0.0f, 0.8f); };
	static ColorRGBA ActionKickButtonColor() { return ColorRGBA(1.0f, 0.549f, 0.0f, 0.8f); };
	static ColorRGBA ActionMuteAltButtonColor() { return ColorRGBA(0.25f, 0.0f, 0.25f, 0.8f); };
	static ColorRGBA ActionMuteButtonColor() { return ColorRGBA(0.502f, 0.0f, 0.502f, 0.8f); };
};

void CAdminPanel::DoIconLabeledButton(CUIRect *pRect, const char *pTitle, const char *pIcon, float TextSize, float Height, ColorRGBA IconColor) const
{
	CUIRect Label;
	pRect->VSplitLeft(Height, &Label, pRect);
	DoIconButton(&Label, pIcon, TextSize, IconColor);
	Ui()->DoLabel(pRect, pTitle, TextSize, TEXTALIGN_MC);
}

void CAdminPanel::DoIconLabeledButtonDown(CUIRect *pRect, const char *pTitle, const char *pIcon, float IconSize, float TextSize, float Height, float Dif, ColorRGBA IconColor) const
{
	CUIRect Icon, Label;
	pRect->HSplitTop(Height, &Icon, &Label);
	DoIconButton(&Icon, pIcon, IconSize, IconColor);
	Label.HSplitTop(Dif, nullptr, &Label);
	Label.HSplitTop(Label.h/2, &Label, nullptr);
	Ui()->DoLabel(&Label, pTitle, TextSize, TEXTALIGN_MC);
}

void CAdminPanel::DoLabelLabeledButtonDown(CUIRect *pRect, const char *pTitleDown, const char *pTitle, float TextSize, float TextSizeDown, float Height, float Dif) const
{
	CUIRect Label, LabelDown;
	pRect->HSplitTop(Height, &Label, &LabelDown);
	Ui()->DoLabel(&Label, pTitle, TextSize, TEXTALIGN_MC);
	LabelDown.HSplitTop(Dif, nullptr, &LabelDown);
	LabelDown.HSplitTop(LabelDown.h/2, &LabelDown, nullptr);
	Ui()->DoLabel(&LabelDown, pTitleDown, TextSizeDown, TEXTALIGN_MC);
}

void CAdminPanel::DoIconButton(CUIRect *pRect, const char *pIcon, float TextSize, ColorRGBA IconColor) const
{
	TextRender()->SetFontPreset(EFontPreset::ICON_FONT);
	TextRender()->SetRenderFlags(ETextRenderFlags::TEXT_RENDER_FLAG_ONLY_ADVANCE_WIDTH | ETextRenderFlags::TEXT_RENDER_FLAG_NO_X_BEARING | ETextRenderFlags::TEXT_RENDER_FLAG_NO_Y_BEARING | ETextRenderFlags::TEXT_RENDER_FLAG_NO_OVERSIZE);
	TextRender()->TextColor(IconColor);
	Ui()->DoLabel(pRect, pIcon, TextSize, TEXTALIGN_MC);
	TextRender()->TextColor(TextRender()->DefaultTextColor());
	TextRender()->SetRenderFlags(0);
	TextRender()->SetFontPreset(EFontPreset::DEFAULT_FONT);
}

CAdminPanel::CAdminPanel()
{
	OnReset();
	m_AdminInput.SetBuffer(m_Input, sizeof(m_Input)); // for reason
	m_AdminTimers.SetBuffer(m_InputCharTimers, sizeof(m_InputCharTimers));// for time
}

void CAdminPanel::OnConsoleInit()
{
	Console()->Register("toggle_adminpanel", "", CFGFLAG_CLIENT, ConToggleAdminPanel, this, "Toggle admin panel");
}

void CAdminPanel::ConToggleAdminPanel(IConsole::IResult *pResult, void *pUserData)
{
	CAdminPanel *pSelf = (CAdminPanel *)pUserData;
	if(pSelf->Client()->RconAuthed())
		pSelf->SetActive(!pSelf->IsActive());
	else
	{
		pSelf->GameClient()->Echo("THIS FUNCTION IS NOT FOR YOU, MORTAL. ☠");
		pSelf->GameClient()->Echo("You've gone too far. We know who you are.");
		pSelf->GameClient()->Echo("I see you.");
	}
}

void CAdminPanel::SetActive(bool Active)
{
	if(m_Active == Active)
		return;

	m_Active = Active;
	if(m_Active)
	{
		m_Mouse.m_Unlocked = true;
		Console()->ExecuteLine("say /spec");
	}
	else
	{
		OnReset();
		Console()->ExecuteLine("say /spec");
	}
}

void CAdminPanel::OnReset()
{
	m_Mouse.reset();
	m_Popup.reset();
	RIReset();
	SetActive(false);
}

void CAdminPanel::OnRelease()
{
	m_Mouse.reset();
	m_Popup.reset();
	RIReset();
	SetActive(false);
}

bool CAdminPanel::OnCursorMove(float x, float y, IInput::ECursorType CursorType)
{
	if(!IsActive() || !m_Mouse.m_Unlocked)
		return false;

	if(!GameClient()->m_Snap.m_SpecInfo.m_Active)
		return false;

	Ui()->ConvertMouseMove(&x, &y, CursorType);


	vec2 OldPos = m_Mouse.m_Position;
	m_Mouse.m_Position.x += x / 3.6;
	m_Mouse.m_Position.y += y / 3.6;

	// Check if we're dragging
	if(m_Mouse.m_MouseInput && !m_Mouse.m_IsDragging)
	{
	    float DragThreshold = 5.0f;
	    if(distance(OldPos, m_Mouse.m_Position) > DragThreshold)
	    {
	        m_Mouse.m_IsDragging = true;
	        m_Mouse.m_DragStart = OldPos;
	    }
	}

	const float ScreenWidth = 100.0f * 3.0f * Graphics()->ScreenAspect();
	const float ScreenHeight = 100.0f * 3.0f;
	m_Mouse.clampPosition(ScreenWidth, ScreenHeight);

	return true;
}


bool CAdminPanel::OnInput(const IInput::CEvent &Event)
{
	if(!IsActive())
		return false;

	if(GameClient()->m_GameConsole.IsActive() || GameClient()->m_Menus.IsActive() || GameClient()->m_Chat.IsActive() || GameClient()->m_Emoticon.IsActive())
		return false;

	if(!GameClient()->m_Snap.m_SpecInfo.m_Active)
		return false;

	if(m_pActiveItem == &m_AdminInput || m_pActiveItem == &m_AdminTimers)
	{
		if(Event.m_Flags & IInput::FLAG_PRESS && Event.m_Key == KEY_ESCAPE)
		{
			if(m_pActiveItem == &m_AdminInput)
				m_AdminInput.Deactivate();
			else if(m_pActiveItem == &m_AdminTimers)
				m_AdminTimers.Deactivate();
			m_pActiveItem = nullptr;
			return true;
		}

		if(m_pActiveItem == &m_AdminInput && m_AdminInput.ProcessInput(Event))
			return true;

		if(m_pActiveItem == &m_AdminTimers && m_AdminTimers.ProcessInput(Event))
			return true;

		// Поглощаем все нажатия клавиш, пока поле ввода активно
		if(Event.m_Flags & IInput::FLAG_PRESS)
			return true;
	}


	if(Event.m_Flags & IInput::FLAG_PRESS && Event.m_Key == KEY_MOUSE_2)
	{
		// find closest player to mouse
		if(!m_Popup.m_Visible)
		{
			if(m_HoveredPlayerId != -1)
			{
				m_Popup.m_PlayerId = m_HoveredPlayerId;
				m_Popup.m_Visible = true;

				return true;
			}
		}
		else
		{
			m_Popup.m_Visible = !m_Popup.m_Visible;
			RIReset();
		}
	}


	if(Event.m_Flags & IInput::FLAG_PRESS && Event.m_Key == KEY_ESCAPE)
	{
		OnReset();
		return true;
	}

	// if(GameClient()->m_Snap.m_SpecInfo.m_Active && GameClient()->m_Snap.m_SpecInfo.m_SpectatorId == SPEC_FREEVIEW)
	// {
	// 	float Speed = 96.0f;
	// 	if(Input()->KeyIsPressed(KEY_W))
	// 		GameClient()->m_Controls.m_aMousePos[g_Config.m_ClDummy].y -= Speed;
	// 	if(Input()->KeyIsPressed(KEY_S))
	// 		GameClient()->m_Controls.m_aMousePos[g_Config.m_ClDummy].y += Speed;
	// 	if(Input()->KeyIsPressed(KEY_A))
	// 		GameClient()->m_Controls.m_aMousePos[g_Config.m_ClDummy].x -= Speed;
	// 	if(Input()->KeyIsPressed(KEY_D))
	// 		GameClient()->m_Controls.m_aMousePos[g_Config.m_ClDummy].x += Speed;
	// }

	return m_Mouse.m_Clicked;
}

void CAdminPanel::OnRender()
{
	if(!IsActive())
		return;

	if(!GameClient()->m_Snap.m_SpecInfo.m_Active)
		return;

	if(GameClient()->m_Snap.m_SpecInfo.m_Active && GameClient()->m_Snap.m_SpecInfo.m_SpectatorId == SPEC_FREEVIEW && m_pActiveItem != &m_AdminInput && m_pActiveItem != &m_AdminTimers)
	{
		float Speed = 2500.0f * (GameClient()->m_Camera.m_Zoom * 6 / g_Config.m_ClDefaultZoom); // Adjusted for frame-time independence
		float FrameTime = Client()->RenderFrameTime();
		if(Input()->KeyIsPressed(KEY_W))
			GameClient()->m_Controls.m_aMousePos[g_Config.m_ClDummy].y -= Speed * FrameTime;
		if(Input()->KeyIsPressed(KEY_S))
			GameClient()->m_Controls.m_aMousePos[g_Config.m_ClDummy].y += Speed * FrameTime;
		if(Input()->KeyIsPressed(KEY_A))
			GameClient()->m_Controls.m_aMousePos[g_Config.m_ClDummy].x -= Speed * FrameTime;
		if(Input()->KeyIsPressed(KEY_D))
			GameClient()->m_Controls.m_aMousePos[g_Config.m_ClDummy].x += Speed * FrameTime;
	}

	m_Mouse.m_LastMouseInput = m_Mouse.m_MouseInput;
	m_Mouse.m_MouseInput = Input()->KeyIsPressed(KEY_MOUSE_1);
	m_Mouse.m_Clicked = !m_Mouse.m_LastMouseInput && m_Mouse.m_MouseInput;
	// find closest player to mouse for highlighting
	float ClosestDist = 15.0f;
	m_HoveredPlayerId = -1;
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(!GameClient()->m_Snap.m_apPlayerInfos[i])
			continue;

		vec2 PlayerPos = GameClient()->m_aClients[i].m_RenderPos;
		if(PlayerPos.x == 0 && PlayerPos.y == 0)
			continue;
		float ScreenWidth = 100.0f * 3.0f * Graphics()->ScreenAspect();
		float ScreenHeight = 100.0f * 3.0f;
		m_PlayerScreenPos = vec2(
			ScreenWidth / 2 + (PlayerPos.x - GameClient()->m_Camera.m_Center.x) / 2.68f / GameClient()->m_Camera.m_Zoom,
			ScreenHeight / 2 + (PlayerPos.y - GameClient()->m_Camera.m_Center.y) / 2.68f / GameClient()->m_Camera.m_Zoom
		);




		float Dist = distance(vec2(m_Mouse.m_Position.x, m_Mouse.m_Position.y), m_PlayerScreenPos);
		if(Dist < ClosestDist)
		{
			ClosestDist = Dist;
			m_HoveredPlayerId = i;
			m_ClosestScreenPlayerPos = m_PlayerScreenPos;
		}
	}

	if(m_HoveredPlayerId != -1)
	{
		// Draw a highlight circle
		Graphics()->TextureClear();
		Graphics()->QuadsBegin();
		Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.5f);
		Graphics()->DrawCircle(m_ClosestScreenPlayerPos.x, m_ClosestScreenPlayerPos.y, 10.0f, 32);
		Graphics()->QuadsEnd();
	}

	if(m_Popup.m_Visible)
	{
		RenderPlayerPanelPopUp();
	}
	else
	{
		RenderPlayerPanelPlayersList();
	}
	if(m_Mouse.m_Unlocked)
	{
		RenderTools()->RenderCursor(m_Mouse.m_Position, 12.0f);
	}
}

void CAdminPanel::RenderPlayerPanelPopUp()
{
	const char *pPlayerName = GameClient()->m_aClients[m_Popup.m_PlayerId].m_aName;

	CUIRect Base, Label, ButtonToggle, UpperButton, LowerButton;

	if(m_LastConfirm == 0)
	{
		Base.h = 100.0f * 3.0f/1.5;
		Base.w = 100.0f * 3.0f * Graphics()->ScreenAspect()/1.5 + SPopupProperties::ms_ButtonHeight;
	}
	else
	{
		Base.h = 100.0f * 3.0f/4;
		Base.w = 100.0f * 3.0f * Graphics()->ScreenAspect()/3;
	}
	Base.x = 100.0f * 3.0f * Graphics()->ScreenAspect()/2 - Base.w/2 + (m_LastConfirm == 0 ? SPopupProperties::ms_ButtonHeight/2 : 0);
	Base.y = (100.0f * 3.0f)/2 - Base.h/2;


	vec2 ScreenTL, ScreenBR;
	Graphics()->GetScreen(&ScreenTL.x, &ScreenTL.y, &ScreenBR.x, &ScreenBR.y);

	if(Base.y + Base.h > ScreenBR.y)
	{
		Base.y -= Base.y + Base.h - ScreenBR.y;
	}
	if(Base.x + Base.w > ScreenBR.x)
	{
		Base.x -= Base.x + Base.w - ScreenBR.x;
	}

	m_Popup.m_Rect = Base;

	Base.VSplitRight(SPopupProperties::ms_ButtonHeight, &Base, &ButtonToggle);
	if(m_LastConfirm == 0 )
	{
		ButtonToggle.HSplitTop(ButtonToggle.h/2-20.0f, nullptr, &ButtonToggle);
		ButtonToggle.HSplitTop(40.0f, &ButtonToggle, nullptr);
		ButtonToggle.HSplitMid(&UpperButton, &LowerButton);
		if(Hovered(&UpperButton))
			UpperButton.Draw(SPopupProperties::WindowColor(), IGraphics::CORNER_TR, SPopupProperties::ms_Rounding);
		else
			UpperButton.Draw(SPopupProperties::WindowColor(), IGraphics::CORNER_TR, SPopupProperties::ms_Rounding);
		DoIconButton(&UpperButton, !m_PlayerList.m_Active ? FontIcons::FONT_ICON_CHEVRON_RIGHT : FontIcons::FONT_ICON_CHEVRON_LEFT, SPopupProperties::ms_IconFontSize * (Hovered(&UpperButton) ? 1.2 : 1), TextRender()->DefaultTextColor());
		if(DoButtonLogic(&UpperButton))
		{
			m_ReadyButtons = 0;
		}

		if(Hovered(&LowerButton))
			LowerButton.Draw(SPopupProperties::WindowColorDark(), IGraphics::CORNER_BR, SPopupProperties::ms_Rounding);
		else
			LowerButton.Draw(SPopupProperties::WindowColorDark(), IGraphics::CORNER_BR, SPopupProperties::ms_Rounding);
		DoIconButton(&LowerButton, !m_PlayerList.m_Active ? FontIcons::FONT_ICON_CHEVRON_RIGHT : FontIcons::FONT_ICON_CHEVRON_LEFT, SPopupProperties::ms_IconFontSize * (Hovered(&LowerButton) ? 1.2 : 1), TextRender()->DefaultTextColor());
		if(DoButtonLogic(&LowerButton))
		{
			m_ReadyButtons = 1;
		}
	}

	Base.Draw(m_ReadyButtons == 0 ? SPopupProperties::WindowColor() : SPopupProperties::WindowColorDark(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
	Base.Margin(SPopupProperties::ms_Padding, &Base);

	Base.HSplitTop(SPopupProperties::ms_HeadlineFontSize + 5, &Label, &Base);
	char aBuf[64];
	str_format(aBuf, sizeof(aBuf), "Nick: %s, ID: %i", pPlayerName, m_Popup.m_PlayerId);
	Ui()->DoLabel(&Label, aBuf, 12, TEXTALIGN_MC);
	Base.HSplitTop(5, nullptr, &Base);

	if(m_LastConfirm == 0)
	{
		if(m_ReadyButtons == 0)
		{
			RenderPlayerPanelPopUpActionButtons(&Base);
			RenderPlayerPanelPopUpTimers(&Base);
		}
		else
			RenderPlayerPanelPopUpReadyButtons(&Base);
		RenderPlayerPanelPopUpInputs(&Base);
		RenderPlayerPanelPopUpCommand(&Base);
	}
	else
		RenderPlayerPanelPopUpLastConfirm(&Base);
}

void CAdminPanel::RenderPlayerPanelPopUpActionButtons(CUIRect *pBase)
{
    CUIRect Container, Action;

	pBase->HSplitTop(SPopupProperties::ms_ItemSpacing, nullptr, pBase);
	pBase->HSplitTop(SPopupProperties::ms_RconActionHight, &Container, pBase);

	float ActionSpacing = (pBase->w - (4 * SPopupProperties::ms_RconActionWidth)) / 3;

	Container.VSplitLeft(SPopupProperties::ms_RconActionWidth, &Action, &Container);

	// Kill
	if(Hovered(&Action))
	{
		Action.Draw(m_ChosenActionButton == 1 ? SPopupProperties::ActionKillAltButtonColor() : SPopupProperties::ActionKillButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
		DoIconLabeledButtonDown(&Action, "kill", FontIcons::FONT_ICON_SCULL, SPopupProperties::ms_IconFontSize, SPopupProperties::ms_FontSize,18.0f ,0.0f, TextRender()->DefaultTextColor());
	}
	else
	{
		Action.Draw(m_ChosenActionButton == 1 ? SPopupProperties::ActionKillButtonColor() : SPopupProperties::ActionKillAltButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
		DoIconLabeledButtonDown(&Action, "kill", FontIcons::FONT_ICON_SCULL, SPopupProperties::ms_IconFontSize, SPopupProperties::ms_FontSize,18.0f ,0.0f, TextRender()->DefaultTextColor());
	}
	if(DoButtonLogic(&Action))
	{
		if(m_ChosenActionButton == 1)
			m_ChosenActionButton = 0;
		else
			m_ChosenActionButton = 1;
	}

	Container.VSplitLeft(ActionSpacing, nullptr, &Container);
	Container.VSplitLeft(SPopupProperties::ms_RconActionWidth, &Action, &Container);

	// kick
	if(Hovered(&Action))
	{
		Action.Draw(m_ChosenActionButton == 2 ? SPopupProperties::ActionKickAltButtonColor() : SPopupProperties::ActionKickButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
		DoIconLabeledButtonDown(&Action, "kick", FontIcons::FONT_ICON_DOOR_OPEN, SPopupProperties::ms_IconFontSize, SPopupProperties::ms_FontSize,18.0f ,0.0f, TextRender()->DefaultTextColor());
	}
	else
	{
		Action.Draw(m_ChosenActionButton == 2 ? SPopupProperties::ActionKickButtonColor() : SPopupProperties::ActionKickAltButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
		DoIconLabeledButtonDown(&Action, "kick", FontIcons::FONT_ICON_DOOR_OPEN, SPopupProperties::ms_IconFontSize, SPopupProperties::ms_FontSize,18.0f ,0.0f, TextRender()->DefaultTextColor());
	}
	if(DoButtonLogic(&Action))
	{
		if(m_ChosenActionButton == 2)
			m_ChosenActionButton = 0;
		else
			m_ChosenActionButton = 2;
	}

	Container.VSplitLeft(ActionSpacing, nullptr, &Container);
	Container.VSplitLeft(SPopupProperties::ms_RconActionWidth, &Action, &Container);

	// mute
	if(Hovered(&Action))
	{
		Action.Draw(m_ChosenActionButton ==  3 ? SPopupProperties::ActionMuteAltButtonColor() : SPopupProperties::ActionMuteButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
		DoIconLabeledButtonDown(&Action, "mute", FontIcons::FONT_ICON_COMMENT_SLASH, SPopupProperties::ms_IconFontSize, SPopupProperties::ms_FontSize,18.0f ,0.0f, TextRender()->DefaultTextColor());
	}
	else
	{
		Action.Draw(m_ChosenActionButton ==  3 ? SPopupProperties::ActionMuteButtonColor() : SPopupProperties::ActionMuteAltButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
		DoIconLabeledButtonDown(&Action, "mute", FontIcons::FONT_ICON_COMMENT_SLASH, SPopupProperties::ms_IconFontSize, SPopupProperties::ms_FontSize,18.0f ,0.0f, TextRender()->DefaultTextColor());
	}
	if(DoButtonLogic(&Action))
	{
		if(m_ChosenActionButton ==  3)
			m_ChosenActionButton = 0;
		else
			m_ChosenActionButton =  3;


	}

	// ban
	Container.VSplitLeft(ActionSpacing, nullptr, &Container);
	Container.VSplitLeft(SPopupProperties::ms_RconActionWidth, &Action, &Container);

	if(Hovered(&Action))
	{
		Action.Draw(m_ChosenActionButton == 4? SPopupProperties::ActionBanAltButtonColor() : SPopupProperties::ActionBanButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
		DoIconLabeledButtonDown(&Action, "local ban", FontIcons::FONT_ICON_GAVEL, SPopupProperties::ms_IconFontSize, SPopupProperties::ms_FontSize,18.0f,0.0f, TextRender()->DefaultTextColor());
	}
	else
	{
		Action.Draw(m_ChosenActionButton == 4 ? SPopupProperties::ActionBanButtonColor() : SPopupProperties::ActionBanAltButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
		DoIconLabeledButtonDown(&Action, "local ban", FontIcons::FONT_ICON_GAVEL, SPopupProperties::ms_IconFontSize, SPopupProperties::ms_FontSize,18.0f ,0.0f, TextRender()->DefaultTextColor());
	}
	if(DoButtonLogic(&Action))
	{
		if(m_ChosenActionButton == 4)
		{
			m_ChosenActionButton = 0;
		}
		else
		{
			m_ChosenActionButton = 4;
		}
	}
}

void CAdminPanel::RenderPlayerPanelPopUpTimers(CUIRect *pBase)
{
	CUIRect Container, Button;

	static const struct { const char *pTime; int Minutes; } s_aElems[] = {
		{"1m", 1}, {"5m", 5}, {"10m", 10}, {"15m", 15}, {"30m", 30}, {"45m", 45},
		{"1h", 60}, {"3h", 180}, {"12h", 720}, {"1d", 1440}, {"3d", 4320}, {"5d", 7200},
		{"1w", 10080}, {"2w", 20160}, {"3w", 30240}, {"1mo", 43200}, {"2mo", 86400}, {"3m", 129600},
		{"6m", 259200}, {"9m", 388800}, {"1y", 518400}, {"2y", 1036800}
	};

	float ItemSpacingW = (pBase->w - (6 * SPopupProperties::ms_RconTimersWidth)) / 5;

	pBase->HSplitTop(SPopupProperties::ms_ItemSpacing, nullptr, pBase);
	pBase->HSplitTop(SPopupProperties::ms_ButtonHeight, &Container, pBase);
	Ui()->DoLabel(&Container, ("Parameters"), SPopupProperties::ms_FontSize, TEXTALIGN_MC);
	pBase->HSplitTop(SPopupProperties::ms_ButtonHeight, &Container, pBase);
	for(int i = 0; i < 6; i++)
	{
		Container.VSplitLeft(SPopupProperties::ms_RconTimersWidth, &Button, &Container);
		if(Hovered(&Button))
			Button.Draw(m_InputTimers == s_aElems[i].Minutes ? SPopupProperties::GeneralButtonColor() : SPopupProperties::GeneralActiveButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
		else
			Button.Draw(m_InputTimers == s_aElems[i].Minutes ? SPopupProperties::GeneralActiveButtonColor() : SPopupProperties::GeneralButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
		Ui()->DoLabel(&Button, RCLocalize(s_aElems[i].pTime), SPopupProperties::ms_FontSize, TEXTALIGN_MC);
		if(DoButtonLogic(&Button))
		{
			if(m_InputTimers == s_aElems[i].Minutes)
			{
				m_ChosenTimerButton = -1;
				str_copy(m_InputCharTimers, "0", sizeof(m_InputCharTimers));
			}
			else
			{
				str_format(m_InputCharTimers, sizeof(m_InputCharTimers), "%i", s_aElems[i].Minutes);
				m_ChosenTimerButton = i;
			}
		}
		Container.VSplitLeft(ItemSpacingW, nullptr, &Container);
	}
	pBase->HSplitTop(SPopupProperties::ms_ItemSpacing, nullptr, pBase);
	pBase->HSplitTop(SPopupProperties::ms_ButtonHeight, &Container, pBase);
	for(int i = 6; i < 12; i++)
	{
		Container.VSplitLeft(SPopupProperties::ms_RconTimersWidth, &Button, &Container);
		if(Hovered(&Button))
			Button.Draw(m_InputTimers == s_aElems[i].Minutes ? SPopupProperties::GeneralButtonColor() : SPopupProperties::GeneralActiveButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
		else
			Button.Draw(m_InputTimers == s_aElems[i].Minutes ? SPopupProperties::GeneralActiveButtonColor() : SPopupProperties::GeneralButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
		Ui()->DoLabel(&Button, RCLocalize(s_aElems[i].pTime), SPopupProperties::ms_FontSize, TEXTALIGN_MC);
		if(DoButtonLogic(&Button))
		{
			if(m_InputTimers == s_aElems[i].Minutes)
			{
				m_ChosenTimerButton = -1;
				str_copy(m_InputCharTimers, "0", sizeof(m_InputCharTimers));
			}
			else
			{
				str_format(m_InputCharTimers, sizeof(m_InputCharTimers), "%i", s_aElems[i].Minutes);
				m_ChosenTimerButton = i;
			}
		}
		Container.VSplitLeft(ItemSpacingW, nullptr, &Container);
	}
	pBase->HSplitTop(SPopupProperties::ms_ItemSpacing, nullptr, pBase);
	pBase->HSplitTop(SPopupProperties::ms_ButtonHeight, &Container, pBase);
	for(int i = 12; i < 18; i++)
	{
		Container.VSplitLeft(SPopupProperties::ms_RconTimersWidth, &Button, &Container);
		if(Hovered(&Button))
			Button.Draw(m_InputTimers == s_aElems[i].Minutes ? SPopupProperties::GeneralButtonColor() : SPopupProperties::GeneralActiveButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
		else
			Button.Draw(m_InputTimers == s_aElems[i].Minutes ? SPopupProperties::GeneralActiveButtonColor() : SPopupProperties::GeneralButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
		Ui()->DoLabel(&Button, RCLocalize(s_aElems[i].pTime), SPopupProperties::ms_FontSize, TEXTALIGN_MC);
		if(DoButtonLogic(&Button))
		{
			if(m_InputTimers == s_aElems[i].Minutes)
			{
				m_ChosenTimerButton = -1;
				str_copy(m_InputCharTimers, "0", sizeof(m_InputCharTimers));
			}
			else
			{
				str_format(m_InputCharTimers, sizeof(m_InputCharTimers), "%i", s_aElems[i].Minutes);
				m_ChosenTimerButton = i;
			}
		}
		Container.VSplitLeft(ItemSpacingW, nullptr, &Container);
	}
	pBase->HSplitTop(SPopupProperties::ms_ItemSpacing, nullptr, pBase);
	pBase->HSplitTop(SPopupProperties::ms_ButtonHeight, &Container, pBase);
	Container.VSplitLeft(SPopupProperties::ms_RconTimersWidth, &Button, &Container);
	Container.VSplitLeft(ItemSpacingW, nullptr, &Container);
	for(int i = 18; i < 22; i++)
	{
		Container.VSplitLeft(SPopupProperties::ms_RconTimersWidth, &Button, &Container);
		if(Hovered(&Button))
			Button.Draw(m_InputTimers == s_aElems[i].Minutes ? SPopupProperties::GeneralButtonColor() : SPopupProperties::GeneralActiveButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
		else
			Button.Draw(m_InputTimers == s_aElems[i].Minutes ? SPopupProperties::GeneralActiveButtonColor() : SPopupProperties::GeneralButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
		Ui()->DoLabel(&Button, (s_aElems[i].pTime), SPopupProperties::ms_FontSize, TEXTALIGN_MC);
		if(DoButtonLogic(&Button))
		{
			if(m_InputTimers == s_aElems[i].Minutes)
			{
				m_ChosenTimerButton = -1;
				str_copy(m_InputCharTimers, "0", sizeof(m_InputCharTimers));
			}
			else
			{
				str_format(m_InputCharTimers, sizeof(m_InputCharTimers), "%i", s_aElems[i].Minutes);
				m_ChosenTimerButton = i;
			}
		}
		Container.VSplitLeft(ItemSpacingW, nullptr, &Container);
	}

}

void CAdminPanel::RenderPlayerPanelPopUpReadyButtons(CUIRect *pBase)
{
	CUIRect Label, Button, Column;
	static const struct { const char *Reason; int Minutes;} s_aReadyMute[] = {
		{"Insult", 15}, {"Spam", 5}, {"Advertising", 15}
	};
	static const struct { const char *Reason; int Minutes;} s_aReadyBan[] = {
		{"Block", 60}, {"Bot Client", 2660}, {"Behaviour Inappropriate", 120}, {"Advertising bot client", 2660}
	};
	static const struct { const char *Reason;} s_aReadyKick[] = {
		{"Block"}, {"Spam"}
	};

	// Calculate the maximum height needed for the columns
	char aBuf[128];
	float MaxHeight = SPopupProperties::ms_ButtonHeight + (3 * (SPopupProperties::ms_RconActionHight + SPopupProperties::ms_ItemSpacing));
	CUIRect ReadyButtonsArea;
	// Reserve a horizontal slice for this entire section
	pBase->HSplitTop(MaxHeight, &ReadyButtonsArea, pBase);
	// Add some spacing after this section
	pBase->HSplitTop(SPopupProperties::ms_ItemSpacing, nullptr, pBase);

	float AreaWithoutBan = ReadyButtonsArea.w - (SPopupProperties::ms_ReadyButtonsWidth * 2);

	float ItemSpacingW = (AreaWithoutBan - (2 * SPopupProperties::ms_ReadyButtonsWidth)) / 2;

	ReadyButtonsArea.VSplitLeft(SPopupProperties::ms_ReadyButtonsWidth, &Column, &ReadyButtonsArea);
	Column.HSplitTop(SPopupProperties::ms_ButtonHeight, &Label, &Column);
	Ui()->DoLabel(&Label, ("Kick"), SPopupProperties::ms_FontSize, TEXTALIGN_MC);
	for(unsigned i = 0; i < std::size(s_aReadyKick); i++)
	{
		Column.HSplitTop(SPopupProperties::ms_ItemSpacing, nullptr, &Column);
		Column.HSplitTop(SPopupProperties::ms_RconActionHight, &Button, &Column);

		if(Hovered(&Button))
		{
			Button.Draw(m_ChosenActionButton == 2 && !str_comp(m_Input, s_aReadyKick[i].Reason) ? SPopupProperties::ActionKickAltButtonColor() : SPopupProperties::ActionKickButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
			DoLabelLabeledButtonDown(&Button, "", s_aReadyKick[i].Reason, SPopupProperties::ms_IconFontSize, SPopupProperties::ms_FontSize,18.0f ,0.0f);
		}
		else
		{
			Button.Draw(m_ChosenActionButton == 2  && !str_comp(m_Input, s_aReadyKick[i].Reason) ? SPopupProperties::ActionKickButtonColor() : SPopupProperties::ActionKickAltButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
			DoLabelLabeledButtonDown(&Button, "", s_aReadyKick[i].Reason, SPopupProperties::ms_IconFontSize, SPopupProperties::ms_FontSize,18.0f ,0.0f);
		}
		if(DoButtonLogic(&Button))
		{
			if(m_ChosenActionButton == 2  && !str_comp(m_Input, s_aReadyKick[i].Reason))
			{
				m_ChosenActionButton = 0;
				m_Input[0] = '\0';
				str_copy(m_InputCharTimers, "0", sizeof(m_InputCharTimers));
				m_InputTimers = 0;
			}
			else
			{
				m_ChosenActionButton = 2;
				str_copy(m_Input, s_aReadyKick[i].Reason, sizeof(m_Input));
				str_copy(m_InputCharTimers, "0", sizeof(m_InputCharTimers));
				m_InputTimers = 0;
			}
		}
	}

	ReadyButtonsArea.VSplitLeft(ItemSpacingW, nullptr, &ReadyButtonsArea);
	ReadyButtonsArea.VSplitLeft(SPopupProperties::ms_ReadyButtonsWidth, &Column, &ReadyButtonsArea);
	Column.HSplitTop(SPopupProperties::ms_ButtonHeight, &Label, &Column);
	Ui()->DoLabel(&Label, ("Mute"), SPopupProperties::ms_FontSize, TEXTALIGN_MC);
	for(unsigned i = 0; i < std::size(s_aReadyMute); i++)
	{
		Column.HSplitTop(SPopupProperties::ms_ItemSpacing, nullptr, &Column);
		Column.HSplitTop(SPopupProperties::ms_RconActionHight, &Button, &Column);
		str_format(aBuf, sizeof(aBuf), "Minutes: %i", s_aReadyMute[i].Minutes);
		if(Hovered(&Button))
		{
			Button.Draw(m_ChosenActionButton == 3 && !str_comp(m_Input, s_aReadyMute[i].Reason) ? SPopupProperties::ActionMuteAltButtonColor() : SPopupProperties::ActionMuteButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
			DoLabelLabeledButtonDown(&Button, aBuf, s_aReadyMute[i].Reason, SPopupProperties::ms_IconFontSize, SPopupProperties::ms_FontSize,18.0f ,0.0f);
		}
		else
		{
			Button.Draw(m_ChosenActionButton == 3  && !str_comp(m_Input, s_aReadyMute[i].Reason) ? SPopupProperties::ActionMuteButtonColor() : SPopupProperties::ActionMuteAltButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
			DoLabelLabeledButtonDown(&Button, aBuf, s_aReadyMute[i].Reason, SPopupProperties::ms_IconFontSize, SPopupProperties::ms_FontSize,18.0f ,0.0f);
		}
		if(DoButtonLogic(&Button))
		{
			if(m_ChosenActionButton == 3  && !str_comp(m_Input, s_aReadyMute[i].Reason))
			{
				m_ChosenActionButton = 0;
				m_Input[0] = '\0';
				str_copy(m_InputCharTimers, "0", sizeof(m_InputCharTimers));
				m_InputTimers = 0;
			}
			else
			{
				m_ChosenActionButton = 3;
				str_copy(m_Input, s_aReadyMute[i].Reason, sizeof(m_Input));
				str_format(m_InputCharTimers, sizeof(m_InputCharTimers), "%i", s_aReadyMute[i].Minutes);
				m_InputTimers = s_aReadyMute[i].Minutes;
			}
		}
	}

	CUIRect LeftView, RightView;
	ReadyButtonsArea.VSplitRight(SPopupProperties::ms_ReadyButtonsWidth * 2, &ReadyButtonsArea, &Column);
	Column.HSplitTop(SPopupProperties::ms_ButtonHeight, &Label, &Column);
	Ui()->DoLabel(&Label, ("Ban"), SPopupProperties::ms_FontSize, TEXTALIGN_MC);
	Column.VSplitMid(&LeftView, &RightView, SPopupProperties::ms_ItemSpacing);
	int IsLeft = 1;
	for(unsigned i = 0; i < std::size(s_aReadyBan); i++)
	{

		str_format(aBuf, sizeof(aBuf), "Minutes: %i", s_aReadyBan[i].Minutes);

		if(IsLeft == 1)
		{
			LeftView.HSplitTop(SPopupProperties::ms_ItemSpacing, nullptr, &LeftView);
			LeftView.HSplitTop(SPopupProperties::ms_RconActionHight, &Button, &LeftView);
		}
		else
		{
			RightView.HSplitTop(SPopupProperties::ms_ItemSpacing, nullptr, &RightView);
			RightView.HSplitTop(SPopupProperties::ms_RconActionHight, &Button, &RightView);
		}

		if(Hovered(&Button))
		{
			Button.Draw(m_ChosenActionButton == 4 && !str_comp(m_Input, s_aReadyBan[i].Reason) ? SPopupProperties::ActionBanAltButtonColor() : SPopupProperties::ActionBanButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
			DoLabelLabeledButtonDown(&Button, aBuf, s_aReadyBan[i].Reason, SPopupProperties::ms_IconFontSize, SPopupProperties::ms_FontSize,18.0f ,0.0f);
		}
		else
		{
			Button.Draw(m_ChosenActionButton == 4  && !str_comp(m_Input, s_aReadyBan[i].Reason) ? SPopupProperties::ActionBanButtonColor() : SPopupProperties::ActionBanAltButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
			DoLabelLabeledButtonDown(&Button, aBuf, s_aReadyBan[i].Reason, SPopupProperties::ms_IconFontSize, SPopupProperties::ms_FontSize,18.0f ,0.0f);
		}
		if(DoButtonLogic(&Button))
		{
			if(m_ChosenActionButton == 4  && !str_comp(m_Input, s_aReadyBan[i].Reason))
			{
				m_ChosenActionButton = 0;
				m_Input[0] = '\0';
				str_copy(m_InputCharTimers, "0", sizeof(m_InputCharTimers));
				m_InputTimers = 0;
			}
			else
			{
				m_ChosenActionButton = 4;
				str_copy(m_Input, s_aReadyBan[i].Reason, sizeof(m_Input));
				str_format(m_InputCharTimers, sizeof(m_InputCharTimers), "%i", s_aReadyBan[i].Minutes);
				m_InputTimers = s_aReadyBan[i].Minutes;
			}
		}

		if(IsLeft == 1)
			IsLeft = 0;
		else
			IsLeft = 1;
	}
}


void CAdminPanel::RenderPlayerPanelPopUpInputs(CUIRect *pBase)
{
	CUIRect Container, Input, Label;

	pBase->HSplitTop(SPopupProperties::ms_ItemSpacing, nullptr, pBase);
	pBase->HSplitTop(SPopupProperties::ms_ButtonHeight, &Container, pBase);
	Container.VSplitLeft(35, &Label, &Input);
	Ui()->DoLabel(&Label, ("Minutes:"), SPopupProperties::ms_FontSize, TEXTALIGN_MC);
	Input.VSplitLeft(SPopupProperties::ms_ItemSpacing, nullptr, &Input);
	DoEditBox(&m_AdminTimers, &Input, SPopupProperties::ms_FontSize, SPopupProperties::GeneralActiveButtonColor(), SPopupProperties::GeneralButtonColor());
	m_InputTimers = m_AdminTimers.GetInteger();

	pBase->HSplitTop(SPopupProperties::ms_ItemSpacing, nullptr, pBase);
	pBase->HSplitTop(SPopupProperties::ms_ButtonHeight, &Container, pBase);
	Container.VSplitLeft(35, &Label, &Input);
	Ui()->DoLabel(&Label, ("Reason:"), SPopupProperties::ms_FontSize, TEXTALIGN_MC);
	Input.VSplitLeft(SPopupProperties::ms_ItemSpacing, nullptr, &Input);
	DoEditBox(&m_AdminInput, &Input, SPopupProperties::ms_FontSize, SPopupProperties::ActionBanButtonColor(), SPopupProperties::ActionBanAltButtonColor());
}

void CAdminPanel::RenderPlayerPanelPopUpCommand(CUIRect *pBase)
{
	CUIRect Container;

	pBase->HSplitTop(SPopupProperties::ms_ItemSpacing, nullptr, pBase);
	pBase->HSplitTop(SPopupProperties::ms_ButtonHeight, &Container, pBase);
	Ui()->DoLabel(&Container, ("bye bye"), SPopupProperties::ms_FontSize, TEXTALIGN_MC);
	pBase->HSplitTop(SPopupProperties::ms_ItemSpacing, nullptr, pBase);
	pBase->HSplitTop(SPopupProperties::ms_ButtonHeight, &Container, pBase);
	char aBuf[256];
	char aCommand[256];
	switch(m_ChosenActionButton)
	{
	case 0:
		str_copy(aBuf, "Click buttons pls", sizeof(aBuf));
		break;
	case 1:
		str_format(aBuf, sizeof(aBuf), "rcon kill_pl %i", m_Popup.m_PlayerId);
		break;
	case 2:
		str_format(aBuf, sizeof(aBuf), "rcon kick %i %s", m_Popup.m_PlayerId, m_Input);
		break;
	case 3:
		str_format(aBuf, sizeof(aBuf), "rcon muteid %i %i %s", m_Popup.m_PlayerId, m_InputTimers * 60, m_Input);
		break;
	case 4:
		str_format(aBuf, sizeof(aBuf), "rcon ban %i %i %s", m_Popup.m_PlayerId, m_InputTimers, m_Input);
		break;
	default:
		GameClient()->Echo("Something gone wrong sorry");
	}
	str_format(aCommand, sizeof(aCommand), "Command: %s", aBuf);
	Ui()->DoLabel(&Container, aCommand, SPopupProperties::ms_FontSize, TEXTALIGN_MC);
	pBase->HSplitTop(SPopupProperties::ms_ItemSpacing, nullptr, pBase);
	pBase->HSplitTop(SPopupProperties::ms_ButtonHeight, &Container, pBase);
	if(Hovered(&Container))
		Container.Draw(SPopupProperties::ActionBanButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
	else
		Container.Draw(SPopupProperties::ActionBanAltButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
	Ui()->DoLabel(&Container, ("Execute"), SPopupProperties::ms_FontSize, TEXTALIGN_MC);
	if(DoButtonLogic(&Container))
	{
		if(m_ChosenActionButton != 0)
			m_LastConfirm = 1;
		else
			GameClient()->Echo("Click buttons pls");
	}
}

void CAdminPanel::RenderPlayerPanelPopUpLastConfirm(CUIRect *pBase)
{
	CUIRect Container, Button;

	char aBuf[256];
	char aCommand[256];
	switch(m_ChosenActionButton)
	{
	case 0:
		str_copy(aBuf, "Click buttons pls", sizeof(aBuf));
		break;
	case 1:
		str_format(aBuf, sizeof(aBuf), "rcon kill_pl %i", m_Popup.m_PlayerId);
		break;
	case 2:
		str_format(aBuf, sizeof(aBuf), "rcon kick %i %s", m_Popup.m_PlayerId, m_Input);
		break;
	case 3:
		str_format(aBuf, sizeof(aBuf), "rcon muteid %i %i %s", m_Popup.m_PlayerId, m_InputTimers * 60, m_Input);
		break;
	case 4:
		str_format(aBuf, sizeof(aBuf), "rcon ban %i %i %s", m_Popup.m_PlayerId, m_InputTimers, m_Input);
		break;
	default:
		GameClient()->Echo("Something gone wrong sorry");
	}
	str_format(aCommand, sizeof(aCommand), "Command: %s", aBuf);

	pBase->HSplitTop(SPopupProperties::ms_ItemSpacing, nullptr, pBase);
	pBase->HSplitTop(SPopupProperties::ms_ButtonHeight, &Container, pBase);
	Ui()->DoLabel(&Container, ("Do you want do this?"), SPopupProperties::ms_FontSize, TEXTALIGN_MC);
	pBase->HSplitTop(SPopupProperties::ms_ItemSpacing, nullptr, pBase);
	pBase->HSplitTop(SPopupProperties::ms_ButtonHeight, &Container, pBase);
	Ui()->DoLabel(&Container, aCommand, SPopupProperties::ms_FontSize, TEXTALIGN_MC);
	pBase->HSplitTop(SPopupProperties::ms_ItemSpacing, nullptr, pBase);
	pBase->HSplitTop(SPopupProperties::ms_ButtonHeight, &Container, pBase);
	Container.VSplitLeft(pBase->w/2-5, &Button, &Container);
	if(Hovered(&Button))
		Button.Draw(SPopupProperties::ActionBanButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
	else
		Button.Draw(SPopupProperties::ActionBanAltButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
	Ui()->DoLabel(&Button, ("No"), SPopupProperties::ms_FontSize, TEXTALIGN_MC);
	if(DoButtonLogic(&Button))
	{
		m_LastConfirm = 0;
	}
	Container.VSplitLeft(10, nullptr, &Container);
	Container.VSplitLeft(pBase->w/2-5, &Button, &Container);
	if(Hovered(&Button))
		Button.Draw(SPopupProperties::ActionBanButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
	else
		Button.Draw(SPopupProperties::ActionBanAltButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
	Ui()->DoLabel(&Button, ("Yes"), SPopupProperties::ms_FontSize, TEXTALIGN_MC);
	if(DoButtonLogic(&Button))
	{
		GameClient()->Console()->ExecuteLine(aBuf);
		if(g_Config.m_RiPlaySounds)
			GameClient()->m_Sounds.Play(CSounds::CHN_GUI, SOUND_GRENADE_EXPLODE, 1.0f);
		OnReset();
	}
}

void CAdminPanel::RenderPlayerPanelPlayersList()
{
	CUIRect Base, Label, OneButton;

	Base.h = 100.0f * 3.0f/1.5 - 10.0f;
	Base.w = 20.0f;
	if(!m_PlayerList.m_Active)
		Base.x = -Base.w + 10;
	else
	{
		Base.w = (SPopupProperties::ms_PlayerBtnWidth + SPopupProperties::ms_ItemSpacing) * m_PlayerList.m_ColumnCount + 20.0f;
		Base.x = 0;

	}
	Base.y = (100.0f * 3.0f)/2 - Base.h/2;


	vec2 ScreenTL, ScreenBR;
	Graphics()->GetScreen(&ScreenTL.x, &ScreenTL.y, &ScreenBR.x, &ScreenBR.y);

	if(Base.y + Base.h > ScreenBR.y)
	{
		Base.y -= Base.y + Base.h - ScreenBR.y;
	}
	if(Base.x + Base.w > ScreenBR.x)
	{
		Base.x -= Base.x + Base.w - ScreenBR.x;
	}

	m_Popup.m_Rect = Base;

	Base.VSplitRight(SPopupProperties::ms_ButtonHeight, &Base, &OneButton);
	OneButton.HSplitTop(OneButton.h/2-10.0f, nullptr, &OneButton);
	OneButton.HSplitTop(20.0f, &OneButton, nullptr);
	if(Hovered(&OneButton))
		OneButton.Draw(SPopupProperties::WindowColorDark(), IGraphics::CORNER_R, SPopupProperties::ms_Rounding);
	else
		OneButton.Draw(SPopupProperties::WindowColorDark(), IGraphics::CORNER_R, SPopupProperties::ms_Rounding);
	DoIconButton(&OneButton, !m_PlayerList.m_Active ? FontIcons::FONT_ICON_CHEVRON_RIGHT : FontIcons::FONT_ICON_CHEVRON_LEFT, SPopupProperties::ms_IconFontSize * (Hovered(&OneButton) ? 1.2 : 1), TextRender()->DefaultTextColor());
	if(DoButtonLogic(&OneButton))
	{
		m_PlayerList.m_Active = !m_PlayerList.m_Active;
	}

	Base.Draw(SPopupProperties::WindowColorDark(), IGraphics::CORNER_R, SPopupProperties::ms_Rounding);
	Base.Margin(SPopupProperties::ms_Padding, &Base);

	if(IsActivePlrList())
	{
		if(GameClient()->m_Snap.m_SpecInfo.m_SpectatorId != SPEC_FREEVIEW)
			GameClient()->m_Spectator.Spectate(SPEC_FREEVIEW);

		int PlayersSum = GameClient()->m_Snap.m_NumPlayers;

		Base.HSplitTop(SPopupProperties::ms_HeadlineFontSize + 5, &Label, &Base);
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "Players Count: %i", PlayersSum);
		Ui()->DoLabel(&Label, aBuf, 12, TEXTALIGN_MC);
		Base.HSplitTop(5, nullptr, &Base);

		CUIRect PlayerRow, Player;

		int IsNeedNextLayer = 1;
		m_PlayerList.m_ColumnCount = PlayersSum / 16 + (PlayersSum % 16 != 0 ? 1 : 0);

		for(int i = 0; i < MAX_CLIENTS; ++i)
		{
			if (!GameClient()->m_Snap.m_apPlayerInfos[i])
				continue;

			str_format(aBuf, sizeof(aBuf), "%i: %s", i, GameClient()->m_aClients[i].m_aName);

			if(IsNeedNextLayer == 1)
			{
				Base.VSplitLeft(SPopupProperties::ms_PlayerBtnWidth, &PlayerRow, &Base);
				Base.VSplitLeft(SPopupProperties::ms_ItemSpacing, nullptr, &Base);

			}

			PlayerRow.HSplitTop(SPopupProperties::ms_ItemSpacing, nullptr, &PlayerRow );
			PlayerRow.HSplitTop(SPopupProperties::ms_FontSize, &Player, &PlayerRow );


			if(Hovered(&Player))
				Player.Draw(SPopupProperties::ActionActiveButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
			else
				Player.Draw(SPopupProperties::ActionGeneralButtonColor(), IGraphics::CORNER_ALL, SPopupProperties::ms_Rounding);
			Ui()->DoLabel(&Player, aBuf, SPopupProperties::ms_FontSize, TEXTALIGN_MC);
			if(DoButtonLogic(&Player))
			{
				GameClient()->m_Spectator.Spectate(i);
			}
			if(IsNeedNextLayer == 16)
				IsNeedNextLayer = 1;
			else
				IsNeedNextLayer++;
		}
	}
}

bool CAdminPanel::DoEditBox(CLineInput *pLineInput, const CUIRect *pRect, float FontSize, ColorRGBA ColorHov, ColorRGBA ColorElse)
{
	const bool Inside = pRect->Inside(m_Mouse.m_Position);
	bool Active = m_pActiveItem == pLineInput;

	if(m_Mouse.m_Clicked && Inside)
	{
		if(!Active)
		{
			if(m_pActiveItem == &m_AdminInput)
				m_AdminInput.Deactivate();
			else if(m_pActiveItem == &m_AdminTimers)
				m_AdminTimers.Deactivate();
			pLineInput->Activate(EInputPriority::UI);
			m_pActiveItem = pLineInput;
			Active = true;
		}
	}
	else if(m_Mouse.m_Clicked && !Inside && Active)
	{
		pLineInput->Deactivate();
		m_pActiveItem = nullptr;
		Active = false;
	}

	// Mouse selection logic
	CLineInput::SMouseSelection *pMouseSelection = pLineInput->GetMouseSelection();
	if(Inside)
	{
		if(!pMouseSelection->m_Selecting && m_Mouse.m_Clicked)
		{
			pMouseSelection->m_Selecting = true;
			pMouseSelection->m_PressMouse = m_Mouse.m_Position;
			pMouseSelection->m_Offset.x = pLineInput->GetScrollOffset();
		}
	}
	if(pMouseSelection->m_Selecting)
	{
		pMouseSelection->m_ReleaseMouse = m_Mouse.m_Position;
		if(!m_Mouse.m_MouseInput)
		{
			pMouseSelection->m_Selecting = false;
		}
	}

	// Render
	if(Active || Inside)
		pRect->Draw(ColorHov, IGraphics::CORNER_ALL, 3.0f);
	else
		pRect->Draw(ColorElse, IGraphics::CORNER_ALL, 3.0f);
	CUIRect Textbox;
	pRect->VMargin(2.0f, &Textbox);

	// Scrolling logic
	float ScrollOffset = pLineInput->GetScrollOffset();
	float ScrollOffsetChange = pLineInput->GetScrollOffsetChange();
	Textbox.x -= ScrollOffset;

	const STextBoundingBox BoundingBox = pLineInput->Render(&Textbox, FontSize, TEXTALIGN_ML, pLineInput->WasChanged() || pLineInput->WasCursorChanged(), -1.0f, 0.0f, {});

	Ui()->DoSmoothScrollLogic(&ScrollOffset, &ScrollOffsetChange, Textbox.w, BoundingBox.m_W, true);

	pLineInput->SetScrollOffset(ScrollOffset);
	pLineInput->SetScrollOffsetChange(ScrollOffsetChange);

	return pLineInput->WasChanged();
}

bool CAdminPanel::IsActive() const
{
	if(m_Active)
		return true;

	return false;
}


bool CAdminPanel::IsActivePopup() const
{
	if(m_Popup.m_Visible)
		return true;

	return false;
}

bool CAdminPanel::IsActivePlrList() const
{
	if(m_PlayerList.m_Active)
		return true;

	return false;
}