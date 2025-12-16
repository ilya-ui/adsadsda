/* DDNet Auto Turret Entity - automatic targeting turret for zombie event */
#include "autoturret.h"
#include "character.h"
#include "laser.h"

#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include <game/server/zombieevent.h>
#include <generated/protocol.h>

CAutoTurret::CAutoTurret(CGameWorld *pGameWorld, vec2 Pos, int Owner) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER, Pos, 50)
{
	m_Owner = Owner;
	m_Angle = -3.14159f / 2.0f;
	m_FireCooldown = 0;
	m_EventMode = true;

	for(int i = 0; i < 8; i++)
		m_aIds[i] = Server()->SnapNewId();

	GameWorld()->InsertEntity(this);
}

void CAutoTurret::Reset()
{
	for(int i = 0; i < 8; i++)
		Server()->SnapFreeId(m_aIds[i]);
	Destroy();
}

CCharacter *CAutoTurret::FindNearestZombie()
{
	CCharacter *pClosest = nullptr;
	float ClosestDist = RANGE * RANGE;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

		// Only target zombies in event mode
		if(m_EventMode && !GameServer()->m_ZombieEvent.IsZombie(i))
			continue;

		CCharacter *pChr = pPlayer->GetCharacter();
		if(!pChr)
			continue;

		vec2 Diff = m_Pos - pChr->GetPos();
		float Dist = Diff.x * Diff.x + Diff.y * Diff.y;
		if(Dist < ClosestDist)
		{
			ClosestDist = Dist;
			pClosest = pChr;
		}
	}

	return pClosest;
}

void CAutoTurret::Fire()
{
	if(m_FireCooldown > 0)
		return;

	m_FireCooldown = FIRE_RATE;

	vec2 Dir = vec2(cos(m_Angle), sin(m_Angle));
	vec2 BarrelEnd = m_Pos + vec2(0, -30) + Dir * 50.0f;

	new CLaser(GameWorld(), BarrelEnd, Dir, 800.0f, m_Owner, WEAPON_LASER);
	GameServer()->CreateSound(BarrelEnd, SOUND_LASER_FIRE);
}

void CAutoTurret::Tick()
{
	if(m_FireCooldown > 0)
		m_FireCooldown--;

	// Auto-targeting
	CCharacter *pTarget = FindNearestZombie();
	if(pTarget)
	{
		vec2 TurretBase = m_Pos + vec2(0, -30);
		vec2 Dir = pTarget->GetPos() - TurretBase;
		m_Angle = atan2(Dir.y, Dir.x);
		Fire();
	}
}


void CAutoTurret::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	int StartTick = Server()->Tick() - 2;
	CNetObj_Laser *pObj;

	// Base
	float BaseW = 40.0f;
	float BaseH = 30.0f;

	vec2 BaseL = m_Pos + vec2(-BaseW / 2, 0);
	vec2 BaseR = m_Pos + vec2(BaseW / 2, 0);
	vec2 BaseTop = m_Pos + vec2(0, -BaseH);

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[0]);
	if(pObj)
	{
		pObj->m_X = (int)BaseL.x;
		pObj->m_Y = (int)BaseL.y;
		pObj->m_FromX = (int)BaseTop.x;
		pObj->m_FromY = (int)BaseTop.y;
		pObj->m_StartTick = StartTick;
	}

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[1]);
	if(pObj)
	{
		pObj->m_X = (int)BaseR.x;
		pObj->m_Y = (int)BaseR.y;
		pObj->m_FromX = (int)BaseTop.x;
		pObj->m_FromY = (int)BaseTop.y;
		pObj->m_StartTick = StartTick;
	}

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[2]);
	if(pObj)
	{
		pObj->m_X = (int)BaseR.x;
		pObj->m_Y = (int)BaseR.y;
		pObj->m_FromX = (int)BaseL.x;
		pObj->m_FromY = (int)BaseL.y;
		pObj->m_StartTick = StartTick;
	}

	// Head
	vec2 HeadCenter = m_Pos + vec2(0, -BaseH - 15);
	float HeadSize = 20.0f;

	vec2 HeadTL = HeadCenter + vec2(-HeadSize / 2, -HeadSize / 2);
	vec2 HeadTR = HeadCenter + vec2(HeadSize / 2, -HeadSize / 2);
	vec2 HeadBL = HeadCenter + vec2(-HeadSize / 2, HeadSize / 2);
	vec2 HeadBR = HeadCenter + vec2(HeadSize / 2, HeadSize / 2);

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[3]);
	if(pObj)
	{
		pObj->m_X = (int)HeadTR.x;
		pObj->m_Y = (int)HeadTR.y;
		pObj->m_FromX = (int)HeadTL.x;
		pObj->m_FromY = (int)HeadTL.y;
		pObj->m_StartTick = StartTick;
	}

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[4]);
	if(pObj)
	{
		pObj->m_X = (int)HeadBR.x;
		pObj->m_Y = (int)HeadBR.y;
		pObj->m_FromX = (int)HeadBL.x;
		pObj->m_FromY = (int)HeadBL.y;
		pObj->m_StartTick = StartTick;
	}

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[5]);
	if(pObj)
	{
		pObj->m_X = (int)HeadBL.x;
		pObj->m_Y = (int)HeadBL.y;
		pObj->m_FromX = (int)HeadTL.x;
		pObj->m_FromY = (int)HeadTL.y;
		pObj->m_StartTick = StartTick;
	}

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[6]);
	if(pObj)
	{
		pObj->m_X = (int)HeadBR.x;
		pObj->m_Y = (int)HeadBR.y;
		pObj->m_FromX = (int)HeadTR.x;
		pObj->m_FromY = (int)HeadTR.y;
		pObj->m_StartTick = StartTick;
	}

	// Barrel
	vec2 Dir = vec2(cos(m_Angle), sin(m_Angle));
	vec2 BarrelStart = HeadCenter;
	vec2 BarrelEnd = HeadCenter + Dir * 50.0f;

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[7]);
	if(pObj)
	{
		pObj->m_X = (int)BarrelEnd.x;
		pObj->m_Y = (int)BarrelEnd.y;
		pObj->m_FromX = (int)BarrelStart.x;
		pObj->m_FromY = (int)BarrelStart.y;
		pObj->m_StartTick = StartTick;
	}
}
