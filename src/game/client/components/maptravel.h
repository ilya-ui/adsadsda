/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_MAPTRAVEL_H
#define GAME_CLIENT_COMPONENTS_MAPTRAVEL_H

#include <base/vmath.h>
#include <engine/console.h>
#include <game/client/component.h>
#include <vector>

class CMapTravel : public CComponent
{
public:
	bool m_Active;
	vec2 m_TargetPos;
	std::vector<vec2> m_Path;
	bool m_Pathing;

	CMapTravel();
	int Sizeof() const override { return sizeof(*this); }

	void OnRender() override;
	void OnConsoleInit() override;
	void OnStateChange(int NewState, int OldState) override;
	bool OnCursorMove(float x, float y, IInput::ECursorType CursorType) override;
	bool OnInput(const IInput::CEvent &Event) override;

	void Toggle();
	void SetTarget(vec2 Pos);
	void TickPathing();

private:
	std::vector<vec2> FindPath(vec2 Start, vec2 End);
	static void ConToggle(IConsole::IResult *pResult, void *pUserData);
	
	// Input state for the current tick
	int m_InputDirection;
	bool m_InputJump;
	bool m_InputHook;
};

#endif
