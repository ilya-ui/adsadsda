#include "laserpixel.h"

#include <engine/server.h>
#include <generated/protocol.h>
#include <game/server/gamecontext.h>

CLaserPixel::CLaserPixel(CGameWorld *pGameWorld, vec2 Pos, int LaserType) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER)
{
	m_Pos = Pos;
	m_LaserType = LaserType;
	m_StartTick = Server()->Tick();
	GameWorld()->InsertEntity(this);
}

void CLaserPixel::Reset()
{
	m_MarkedForDestroy = true;
}

void CLaserPixel::Tick()
{
	// Static pixel, no tick logic needed
}

void CLaserPixel::SetColor(int LaserType)
{
	m_LaserType = LaserType;
	m_StartTick = Server()->Tick(); // Update tick so client sees the change
}

void CLaserPixel::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	int SnappingClientVersion = GameServer()->GetClientVersion(SnappingClient);

	// Create a very short laser (point) at this position
	// From and To are the same to create a dot
	vec2 From = m_Pos + vec2(4.0f, 0.0f); // Small offset to make it visible

	GameServer()->SnapLaserObject(
		CSnapContext(SnappingClientVersion, Server()->IsSixup(SnappingClient), SnappingClient),
		GetId(),
		m_Pos,
		From,
		m_StartTick,
		-1,
		m_LaserType,
		0,
		0);
}
