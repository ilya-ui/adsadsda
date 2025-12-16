/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "screen_drawing.h"

#include <algorithm>

#include <engine/graphics.h>
#include <engine/input.h>
#include <engine/keys.h>
#include <engine/shared/config.h>

#include <game/client/gameclient.h>

#include <cstdio>
#include <cstring>

CScreenDrawing::CScreenDrawing()
{
	m_Active = false;
	m_Drawing = false;
	m_CurrentColor = ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f);
	m_CurrentThickness = 2.0f;
}

void CScreenDrawing::OnInit()
{
	// Default values already set in constructor
}

void CScreenDrawing::OnReset()
{
	m_Active = false;
	m_Drawing = false;
	m_vLines.clear();
	m_CurrentLine.m_vPoints.clear();
}

void CScreenDrawing::OnConsoleInit()
{
	Console()->Register("draw_toggle", "", CFGFLAG_CLIENT, ConToggle, this, "Toggle drawing mode");
	Console()->Register("draw_clear", "", CFGFLAG_CLIENT, ConClear, this, "Clear all drawings");
	Console()->Register("draw_color", "i[r] i[g] i[b] ?i[a]", CFGFLAG_CLIENT, ConColor, this, "Set drawing color (0-255)");
	Console()->Register("draw_thickness", "i[thickness]", CFGFLAG_CLIENT, ConThickness, this, "Set line thickness (1-10)");
}

void CScreenDrawing::Toggle()
{
	m_Active = !m_Active;
	if(!m_Active && m_Drawing)
	{
		FinishLine();
		m_Drawing = false;
	}
}

void CScreenDrawing::Clear()
{
	m_vLines.clear();
	m_CurrentLine.m_vPoints.clear();
}


void CScreenDrawing::SetColor(ColorRGBA Color)
{
	m_CurrentColor = Color;
}

void CScreenDrawing::SetThickness(float Thickness)
{
	m_CurrentThickness = std::clamp(Thickness, (float)MIN_THICKNESS, (float)MAX_THICKNESS);
}

void CScreenDrawing::AddPoint(float X, float Y)
{
	if(!m_Active)
		return;

	if(m_CurrentLine.m_vPoints.size() >= MAX_POINTS_PER_LINE)
	{
		FinishLine();
		m_CurrentLine.m_Color = m_CurrentColor;
		m_CurrentLine.m_Thickness = m_CurrentThickness;
	}

	CDrawingPoint Point;
	Point.m_X = X;
	Point.m_Y = Y;
	m_CurrentLine.m_vPoints.push_back(Point);
}

void CScreenDrawing::FinishLine()
{
	if(m_CurrentLine.m_vPoints.size() >= 2)
	{
		if(m_vLines.size() < MAX_LINES)
		{
			m_vLines.push_back(m_CurrentLine);
		}
	}
	m_CurrentLine.m_vPoints.clear();
}

int CScreenDrawing::GetTotalPointCount() const
{
	int Total = 0;
	for(const auto &Line : m_vLines)
	{
		Total += static_cast<int>(Line.m_vPoints.size());
	}
	Total += static_cast<int>(m_CurrentLine.m_vPoints.size());
	return Total;
}

vec2 CScreenDrawing::GetMouseWorldPos() const
{
	// Get mouse position relative to camera center from Controls component
	vec2 MousePos = GameClient()->m_Controls.m_aMousePos[g_Config.m_ClDummy];

	// Get camera center
	vec2 CameraCenter = GameClient()->m_Camera.m_Center;

	// Mouse position is relative to camera center, so add them together
	return CameraCenter + MousePos;
}

bool CScreenDrawing::OnInput(const IInput::CEvent &Event)
{
	if(!m_Active)
		return false;

	if(Event.m_Key == KEY_MOUSE_1)
	{
		if(Event.m_Flags & IInput::FLAG_PRESS)
		{
			m_Drawing = true;
			m_CurrentLine.m_Color = m_CurrentColor;
			m_CurrentLine.m_Thickness = m_CurrentThickness;
			vec2 WorldPos = GetMouseWorldPos();
			AddPoint(WorldPos.x, WorldPos.y);
		}
		else if(Event.m_Flags & IInput::FLAG_RELEASE)
		{
			m_Drawing = false;
			FinishLine();
		}
		return true;
	}

	return false;
}

bool CScreenDrawing::OnCursorMove(float x, float y, IInput::ECursorType CursorType)
{
	if(m_Active && m_Drawing)
	{
		vec2 WorldPos = GetMouseWorldPos();
		AddPoint(WorldPos.x, WorldPos.y);
	}

	return false;
}

void CScreenDrawing::RenderLine(const CDrawingLine &Line)
{
	if(Line.m_vPoints.empty())
		return;

	Graphics()->TextureClear();
	Graphics()->QuadsBegin();
	Graphics()->SetColor(Line.m_Color);

	float Radius = Line.m_Thickness;

	// Draw circles at each point for smooth rounded appearance
	for(const auto &Point : Line.m_vPoints)
	{
		Graphics()->DrawCircle(Point.m_X, Point.m_Y, Radius, 16);
	}

	// Draw quads between points to fill gaps
	if(Line.m_vPoints.size() >= 2)
	{
		for(size_t i = 0; i < Line.m_vPoints.size() - 1; i++)
		{
			float X1 = Line.m_vPoints[i].m_X;
			float Y1 = Line.m_vPoints[i].m_Y;
			float X2 = Line.m_vPoints[i + 1].m_X;
			float Y2 = Line.m_vPoints[i + 1].m_Y;

			// Calculate perpendicular direction for line thickness
			float DirX = X2 - X1;
			float DirY = Y2 - Y1;
			float Len = sqrtf(DirX * DirX + DirY * DirY);
			if(Len > 0.0f)
			{
				float NormX = -DirY / Len * Radius;
				float NormY = DirX / Len * Radius;

				IGraphics::CFreeformItem Freeform(
					X1 + NormX, Y1 + NormY,
					X1 - NormX, Y1 - NormY,
					X2 + NormX, Y2 + NormY,
					X2 - NormX, Y2 - NormY);
				Graphics()->QuadsDrawFreeform(&Freeform, 1);
			}
		}
	}

	Graphics()->QuadsEnd();
}

void CScreenDrawing::OnRender()
{
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;

	// Get camera info for world-space rendering
	float CameraX = GameClient()->m_Camera.m_Center.x;
	float CameraY = GameClient()->m_Camera.m_Center.y;
	float Zoom = GameClient()->m_Camera.m_Zoom;

	// Calculate view dimensions and map screen to world coordinates
	float Width, Height;
	Graphics()->CalcScreenParams(Graphics()->ScreenAspect(), Zoom, &Width, &Height);
	Graphics()->MapScreen(CameraX - Width / 2.0f, CameraY - Height / 2.0f, CameraX + Width / 2.0f, CameraY + Height / 2.0f);

	// Render all completed lines in world coordinates
	for(const auto &Line : m_vLines)
	{
		RenderLine(Line);
	}

	// Render current line being drawn
	if(m_Drawing)
	{
		RenderLine(m_CurrentLine);
	}

	// Render mode indicators in screen space
	Graphics()->MapScreen(0.0f, 0.0f, Graphics()->ScreenWidth(), Graphics()->ScreenHeight());

	float YOffset = 10.0f;

	if(m_Active)
	{
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
		TextRender()->Text(10.0f, YOffset, 12.0f, "DRAW MODE");
		YOffset += 15.0f;
	}

	// Render practice mode indicator
	if(GameClient()->m_PracticeActive)
	{
		TextRender()->TextColor(0.0f, 1.0f, 0.0f, 1.0f);
		TextRender()->Text(10.0f, YOffset, 12.0f, "PRACTICE MODE");
	}
}


// Console command callbacks
void CScreenDrawing::ConToggle(IConsole::IResult *pResult, void *pUserData)
{
	CScreenDrawing *pSelf = static_cast<CScreenDrawing *>(pUserData);
	pSelf->Toggle();
}

void CScreenDrawing::ConClear(IConsole::IResult *pResult, void *pUserData)
{
	CScreenDrawing *pSelf = static_cast<CScreenDrawing *>(pUserData);
	pSelf->Clear();
}

void CScreenDrawing::ConColor(IConsole::IResult *pResult, void *pUserData)
{
	CScreenDrawing *pSelf = static_cast<CScreenDrawing *>(pUserData);
	int R = std::clamp(pResult->GetInteger(0), 0, 255);
	int G = std::clamp(pResult->GetInteger(1), 0, 255);
	int B = std::clamp(pResult->GetInteger(2), 0, 255);
	int A = pResult->NumArguments() > 3 ? std::clamp(pResult->GetInteger(3), 0, 255) : 255;
	pSelf->SetColor(ColorRGBA(R / 255.0f, G / 255.0f, B / 255.0f, A / 255.0f));
}

void CScreenDrawing::ConThickness(IConsole::IResult *pResult, void *pUserData)
{
	CScreenDrawing *pSelf = static_cast<CScreenDrawing *>(pUserData);
	pSelf->SetThickness(static_cast<float>(pResult->GetInteger(0)));
}

void CScreenDrawing::Serialize(char *pBuffer, int BufferSize) const
{
	int Offset = 0;
	Offset += snprintf(pBuffer + Offset, BufferSize - Offset, "DRAW_V1\n");
	Offset += snprintf(pBuffer + Offset, BufferSize - Offset, "%d\n", static_cast<int>(m_vLines.size()));

	for(const auto &Line : m_vLines)
	{
		Offset += snprintf(pBuffer + Offset, BufferSize - Offset, "%f %f %f %f %f %d\n",
			Line.m_Color.r, Line.m_Color.g, Line.m_Color.b, Line.m_Color.a,
			Line.m_Thickness, static_cast<int>(Line.m_vPoints.size()));

		for(const auto &Point : Line.m_vPoints)
		{
			Offset += snprintf(pBuffer + Offset, BufferSize - Offset, "%f %f\n", Point.m_X, Point.m_Y);
		}
	}
}

bool CScreenDrawing::Deserialize(const char *pData)
{
	char Header[16];
	int LineCount = 0;
	int Offset = 0;

	if(sscanf(pData + Offset, "%15s\n%n", Header, &Offset) < 1)
		return false;

	if(strcmp(Header, "DRAW_V1") != 0)
		return false;

	int TempOffset = 0;
	if(sscanf(pData + Offset, "%d\n%n", &LineCount, &TempOffset) < 1)
		return false;
	Offset += TempOffset;

	std::vector<CDrawingLine> NewLines;
	for(int i = 0; i < LineCount && i < MAX_LINES; i++)
	{
		CDrawingLine Line;
		float R, G, B, A;
		int PointCount = 0;

		if(sscanf(pData + Offset, "%f %f %f %f %f %d\n%n", &R, &G, &B, &A, &Line.m_Thickness, &PointCount, &TempOffset) < 6)
			return false;
		Offset += TempOffset;

		Line.m_Color = ColorRGBA(R, G, B, A);

		for(int j = 0; j < PointCount && j < MAX_POINTS_PER_LINE; j++)
		{
			CDrawingPoint Point;
			if(sscanf(pData + Offset, "%f %f\n%n", &Point.m_X, &Point.m_Y, &TempOffset) < 2)
				return false;
			Offset += TempOffset;
			Line.m_vPoints.push_back(Point);
		}

		NewLines.push_back(Line);
	}

	m_vLines = NewLines;
	return true;
}
