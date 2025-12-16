/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_SCREEN_DRAWING_H
#define GAME_CLIENT_COMPONENTS_SCREEN_DRAWING_H

#include <base/color.h>
#include <base/vmath.h>

#include <engine/console.h>

#include <game/client/component.h>

#include <vector>

struct CDrawingPoint
{
	float m_X;
	float m_Y;
};

struct CDrawingLine
{
	std::vector<CDrawingPoint> m_vPoints;
	ColorRGBA m_Color;
	float m_Thickness;
};

class CScreenDrawing : public CComponent
{
public:
	enum
	{
		MAX_LINES = 1000,
		MAX_POINTS_PER_LINE = 10000,
		MIN_THICKNESS = 1,
		MAX_THICKNESS = 10,
	};

	CScreenDrawing();
	int Sizeof() const override { return sizeof(*this); }

	// Component interface
	void OnConsoleInit() override;
	void OnInit() override;
	void OnReset() override;
	void OnRender() override;
	bool OnInput(const IInput::CEvent &Event) override;
	bool OnCursorMove(float x, float y, IInput::ECursorType CursorType) override;

	// Drawing API
	void Toggle();
	void Clear();
	void SetColor(ColorRGBA Color);
	void SetThickness(float Thickness);
	void AddPoint(float X, float Y);
	void FinishLine();

	// Serialization
	void Serialize(char *pBuffer, int BufferSize) const;
	bool Deserialize(const char *pData);

	// State queries
	bool IsActive() const { return m_Active; }
	int GetLineCount() const { return static_cast<int>(m_vLines.size()); }
	int GetTotalPointCount() const;
	ColorRGBA GetCurrentColor() const { return m_CurrentColor; }
	float GetCurrentThickness() const { return m_CurrentThickness; }

private:
	// Console command callbacks
	static void ConToggle(IConsole::IResult *pResult, void *pUserData);
	static void ConClear(IConsole::IResult *pResult, void *pUserData);
	static void ConColor(IConsole::IResult *pResult, void *pUserData);
	static void ConThickness(IConsole::IResult *pResult, void *pUserData);

	// State
	bool m_Active;
	bool m_Drawing;
	ColorRGBA m_CurrentColor;
	float m_CurrentThickness;

	// Drawing data
	std::vector<CDrawingLine> m_vLines;
	CDrawingLine m_CurrentLine;

	// Helper to get mouse position in world coordinates
	vec2 GetMouseWorldPos() const;

	// Helper to render a line with smooth circles
	void RenderLine(const CDrawingLine &Line);
};

#endif
