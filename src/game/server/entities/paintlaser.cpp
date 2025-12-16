/* Paint laser - persistent laser for drawing */
#include "paintlaser.h"

#include <game/server/gamecontext.h>

CPaintLaser::CPaintLaser(CGameWorld *pGameWorld, vec2 From, vec2 To, int Owner) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER)
{
	m_From = From;
	m_Pos = To;
	m_Owner = Owner;
	m_StartTick = Server()->Tick();

	GameWorld()->InsertEntity(this);
}

void CPaintLaser::Reset()
{
	m_MarkedForDestroy = true;
}

void CPaintLaser::Tick()
{
	// Paint lasers are persistent - they don't disappear
	// Only remove if owner disconnected
	if(m_Owner >= 0 && !GameServer()->m_apPlayers[m_Owner])
	{
		Reset();
	}
}

void CPaintLaser::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient) && NetworkClipped(SnappingClient, m_From))
		return;

	int SnappingClientVersion = GameServer()->GetClientVersion(SnappingClient);

	GameServer()->SnapLaserObject(CSnapContext(SnappingClientVersion, Server()->IsSixup(SnappingClient), SnappingClient), GetId(),
		m_Pos, m_From, m_StartTick, m_Owner, LASERTYPE_RIFLE, 0, 0);
}
