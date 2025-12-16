#include "laserdoor.h"
#include <engine/server.h>
#include <game/server/gamecontext.h>
#include <game/server/entities/character.h>
#include <generated/protocol.h>

CLaserDoor::CLaserDoor(CGameWorld *pGameWorld, vec2 From, vec2 To, int Owner) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER)
{
	m_From = From;
	m_To = To;
	m_Pos = (From + To) * 0.5f;
	m_Owner = Owner;
	m_Open = false;
	GameWorld()->InsertEntity(this);
}

void CLaserDoor::Reset() { m_MarkedForDestroy = true; }

void CLaserDoor::Tick()
{
	if(m_Open) return;
	for(CCharacter *pChr = (CCharacter *)GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); pChr; pChr = (CCharacter *)pChr->TypeNext())
	{
		vec2 IntersectPos;
		if(closest_point_on_line(m_From, m_To, pChr->GetPos(), IntersectPos) && distance(pChr->GetPos(), IntersectPos) < 28.0f)
		{
			pChr->SetVelocity(vec2(0, 0));
			vec2 Dir = normalize(pChr->GetPos() - IntersectPos);
			pChr->Move(Dir * 5.0f);
		}
	}
}

void CLaserDoor::Toggle(int ClientId)
{
	if(ClientId != m_Owner) return;
	m_Open = !m_Open;
	GameServer()->SendChat(-1, TEAM_ALL, m_Open ? "Door opened!" : "Door closed!");
}

void CLaserDoor::Snap(int SnappingClient)
{
	if(m_Open || NetworkClipped(SnappingClient)) return;
	CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, GetId(), sizeof(CNetObj_Laser)));
	if(!pObj) return;
	pObj->m_X = (int)m_To.x;
	pObj->m_Y = (int)m_To.y;
	pObj->m_FromX = (int)m_From.x;
	pObj->m_FromY = (int)m_From.y;
	pObj->m_StartTick = Server()->Tick() - 2;
}
