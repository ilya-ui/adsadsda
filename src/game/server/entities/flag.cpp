/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "flag.h"
#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include "character.h"

CFlag::CFlag(CGameWorld *pGameWorld, int Team, vec2 StandPos)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_FLAG, StandPos)
{
	m_Team = Team;
	m_StandPos = StandPos;
	m_Carrier = -1;
	GameWorld()->InsertEntity(this);
}

void CFlag::Reset()
{
	m_Carrier = -1;
	m_Pos = m_StandPos;
}

void CFlag::Grab(int ClientId)
{
	m_Carrier = ClientId;
	GameServer()->CreateSound(m_Pos, SOUND_CTF_GRAB_PL);
}

void CFlag::Respawn()
{
	m_Carrier = -1;
	m_Pos = m_StandPos;
	GameServer()->CreateSound(m_Pos, SOUND_CTF_RETURN);
}

void CFlag::Tick()
{
	if (m_Carrier >= 0)
	{
		CCharacter *pChr = GameServer()->GetPlayerChar(m_Carrier);
		if (pChr && pChr->IsAlive())
		{
			m_Pos = pChr->m_Pos;
		}
		else
		{
			Respawn();
		}
	}
	else
	{
		// Check for pickup
		CCharacter *pChr = (CCharacter *)GameWorld()->ClosestCharacter(m_Pos, 20.0f, nullptr);
		if (pChr && pChr->IsAlive())
		{
			Grab(pChr->GetPlayer()->GetCid());
		}
	}
}

void CFlag::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Flag *pObj = Server()->SnapNewItem<CNetObj_Flag>(m_Team);
	if(!pObj)
		return;

	pObj->m_X = (int)m_Pos.x;
	pObj->m_Y = (int)m_Pos.y;
	pObj->m_Team = m_Team;
}
