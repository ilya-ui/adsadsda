/* DDNet Vodka Bottle Entity */
#include "vodkabottle.h"
#include "character.h"

#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include <generated/protocol.h>

CVodkaBottle::CVodkaBottle(CGameWorld *pGameWorld, vec2 Pos) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_PICKUP, Pos, 20)
{
	m_SnapId = Server()->SnapNewId();
	GameWorld()->InsertEntity(this);
}

void CVodkaBottle::Reset()
{
	Server()->SnapFreeId(m_SnapId);
	Destroy();
}

void CVodkaBottle::Tick()
{
	// Check for players nearby to pick up
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		CCharacter *pChr = GameServer()->GetPlayerChar(i);
		if(!pChr)
			continue;

		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

		// Already has vodka
		if(pPlayer->m_HasVodka)
			continue;

		// Check distance
		float Dist = distance(pChr->GetPos(), m_Pos);
		if(Dist < 30.0f)
		{
			// Pick up!
			pPlayer->m_HasVodka = true;
			GameServer()->SendChatTarget(i, "You picked up vodka! Press LMB to drink, F3 to drop.");
			Reset();
			return;
		}
	}
}

void CVodkaBottle::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	// Draw as a pickup (heart)
	CNetObj_Pickup *pPickup = Server()->SnapNewItem<CNetObj_Pickup>(m_SnapId);
	if(pPickup)
	{
		pPickup->m_X = (int)m_Pos.x;
		pPickup->m_Y = (int)m_Pos.y;
		pPickup->m_Type = POWERUP_HEALTH; // Heart icon for vodka
	}
}
