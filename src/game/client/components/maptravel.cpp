/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/keys.h>
#include <game/client/components/maptravel.h>
#include <game/client/gameclient.h>
#include <game/collision.h>
#include <game/mapitems.h>

#include <queue>
#include <map>

CMapTravel::CMapTravel()
{
	m_Active = false;
	m_Pathing = false;
	m_InputDirection = 0;
	m_InputJump = false;
	m_InputHook = false;
}

void CMapTravel::OnStateChange(int NewState, int OldState)
{
	if(NewState != IClient::STATE_ONLINE)
	{
		m_Active = false;
		m_Pathing = false;
		m_Path.clear();
	}
}

void CMapTravel::OnConsoleInit()
{
	Console()->Register("toggle_map_travel", "", CFGFLAG_CLIENT, ConToggle, this, "Toggle map travel view");
}

void CMapTravel::ConToggle(IConsole::IResult *pResult, void *pUserData)
{
	CMapTravel *pSelf = (CMapTravel *)pUserData;
	pSelf->Toggle();
}

void CMapTravel::Toggle()
{
	m_Active = !m_Active;
	if (m_Active)
	{
		Input()->MouseModeAbsolute();
	}
	else
	{
		Input()->MouseModeRelative();
	}
}

// Fix for OnInput to not block everything and use UI mouse pos
bool CMapTravel::OnCursorMove(float x, float y, IInput::ECursorType CursorType)
{
	if (!m_Active) return false;
	
	// Forward to UI to track mouse position for us
	Ui()->ConvertMouseMove(&x, &y, CursorType);
	Ui()->OnCursorMove(x, y);

	return true; // Consume input to stop camera spinning
}

bool CMapTravel::OnInput(const IInput::CEvent &Event)
{
	if (!m_Active) return false;

	// Only handle mouse click
	if (Event.m_Flags & IInput::FLAG_PRESS && Event.m_Key == KEY_MOUSE_1)
	{
		// Use UI mouse position which is updated by OnCursorMove
		float ScreenX = Ui()->MouseX();
		float ScreenY = Ui()->MouseY();

		float MapWidth = Collision()->GetWidth() * 32.0f;
		float MapHeight = Collision()->GetHeight() * 32.0f;
		vec2 CameraCenter = vec2(MapWidth / 2.0f, MapHeight / 2.0f);

		// Zoom calculation from OnRender
		float Zoom = 20.0f; 

		float ScreenW = Graphics()->ScreenWidth();
		float ScreenH = Graphics()->ScreenHeight();

		float CenteredX = ScreenX - ScreenW / 2.0f;
		float CenteredY = ScreenY - ScreenH / 2.0f;

		float WorldX = CameraCenter.x + CenteredX * Zoom;
		float WorldY = CameraCenter.y + CenteredY * Zoom;
		
		SetTarget(vec2(WorldX, WorldY));
		Toggle();
		return true;
	}
	
	// Allow other inputs (like key binds) to pass through so we can toggle off
	return false; 
}


void CMapTravel::OnRender()
{
	// Process pathing logic every frame
	if(m_Pathing)
	{
		TickPathing();
		
		if(m_InputDirection == -1) GameClient()->m_Controls.m_aInputDirectionLeft[g_Config.m_ClDummy] = 1;
		else GameClient()->m_Controls.m_aInputDirectionLeft[g_Config.m_ClDummy] = 0;
		
		if(m_InputDirection == 1) GameClient()->m_Controls.m_aInputDirectionRight[g_Config.m_ClDummy] = 1;
		else GameClient()->m_Controls.m_aInputDirectionRight[g_Config.m_ClDummy] = 0;

		GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Jump = m_InputJump;
		GameClient()->m_Controls.m_aInputData[g_Config.m_ClDummy].m_Hook = m_InputHook;
	}

	if (!m_Active) return;

	// Render full map
	float MapWidth = Collision()->GetWidth() * 32.0f;
	float MapHeight = Collision()->GetHeight() * 32.0f;
	
	// Center camera
	GameClient()->m_Camera.m_Center = vec2(MapWidth/2.0f, MapHeight/2.0f);

	// Zoom out to fit map
	GameClient()->m_Camera.m_Zoom = 20.0f; 
	GameClient()->m_Camera.m_Zooming = true;

	// Explicitly render cursor since we are in a special mode handling our own input
	RenderTools()->RenderCursor(vec2(Ui()->MouseX(), Ui()->MouseY()), 24.0f);
}

void CMapTravel::SetTarget(vec2 Pos)
{
	m_TargetPos = Pos;
	m_Path = FindPath(GameClient()->m_LocalCharacterPos, m_TargetPos);
	if(!m_Path.empty())
	{
		m_Pathing = true;
	}
	else 
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "maptravel", "No path found!");
	}
}

void CMapTravel::TickPathing()
{
	if(m_Path.empty())
	{
		m_Pathing = false;
		return;
	}

	vec2 Target = m_Path.front();
	vec2 Pos = GameClient()->m_LocalCharacterPos;

	float Dist = distance(Pos, Target);
	if(Dist < 32.0f) // Reached waypoint
	{
		m_Path.erase(m_Path.begin());
		if(m_Path.empty())
		{
			m_Pathing = false;
			m_InputDirection = 0;
			m_InputJump = 0;
			return;
		}
		Target = m_Path.front();
	}

	// Move towards target
	m_InputDirection = 0;
	if(Target.x < Pos.x - 10) m_InputDirection = -1;
	else if(Target.x > Pos.x + 10) m_InputDirection = 1;

	// Simple jump logic
	m_InputJump = 0;
	if(Target.y < Pos.y - 10) m_InputJump = 1; // Jump if target is higher
	
	// Check for wall
	if(Collision()->IsSolid(Pos.x + m_InputDirection * 40, Pos.y))
	{
		m_InputJump = 1;
	}
}

std::vector<vec2> CMapTravel::FindPath(vec2 Start, vec2 End)
{
	// Simple BFS on tiles
	int StartX = round_to_int(Start.x) / 32;
	int StartY = round_to_int(Start.y) / 32;
	int EndX = round_to_int(End.x) / 32;
	int EndY = round_to_int(End.y) / 32;

	int Width = Collision()->GetWidth();
	int Height = Collision()->GetHeight();

	if (StartX < 0 || StartX >= Width || StartY < 0 || StartY >= Height) return {};
	if (EndX < 0 || EndX >= Width || EndY < 0 || EndY >= Height) return {};

	std::queue<ivec2> Q;
	Q.push(ivec2(StartX, StartY));

	std::vector<int> CameFrom(Width * Height, -1);
	CameFrom[StartY * Width + StartX] = -2; // Start marker

	bool Found = false;

	int Directions[4][2] = {{0,1}, {0,-1}, {-1,0}, {1,0}};

	while(!Q.empty())
	{
		ivec2 Current = Q.front();
		Q.pop();

		if(Current.x == EndX && Current.y == EndY)
		{
			Found = true;
			break;
		}
		
		// Optimization: Don't search too far
		// Removed for now to ensure path finding works globally

		for(auto Dir : Directions)
		{
			int NextX = Current.x + Dir[0];
			int NextY = Current.y + Dir[1];
			// int JumpX = Current.x + Dir[0] * 2; // For jumping gaps
			
			if(NextX >= 0 && NextX < Width && NextY >= 0 && NextY < Height)
			{
				if(Collision()->IsSolid(NextX, NextY)) continue;
				
				int Index = NextY * Width + NextX;
				if(CameFrom[Index] == -1)
				{
					CameFrom[Index] = Current.y * Width + Current.x;
					Q.push(ivec2(NextX, NextY));
				}
			}
		}
	}

	if(!Found) return {};

	std::vector<vec2> Path;
	int CurrIndex = EndY * Width + EndX;
	while(CameFrom[CurrIndex] != -2)
	{
		Path.push_back(vec2((CurrIndex % Width) * 32 + 16, (CurrIndex / Width) * 32 + 16));
		CurrIndex = CameFrom[CurrIndex];
	}
	// Path.push_back(Start); // Optional
	std::reverse(Path.begin(), Path.end());

	// Simplify path??
	// For BFS, every tile is a step. That's a lot of waypoints.
	// But bot logic handles close waypoints fine.
	
	return Path;
}
