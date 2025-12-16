/* DDNet Auto Tank Entity - automatic targeting tank for zombie event */
#include "autotank.h"
#include "character.h"
#include "projectile.h"

#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include <game/server/zombieevent.h>
#include <generated/protocol.h>

CAutoTank::CAutoTank(CGameWorld *pGameWorld, vec2 Pos, int Owner) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER, Pos, 80)
{
	m_Owner = Owner;
	m_Angle = -3.14159f / 2.0f;
	m_FireCooldown = 0;
	m_EventMode = true;

	for(int i = 0; i < 12; i++)
		m_aIds[i] = Server()->SnapNewId();

	GameWorld()->InsertEntity(this);
}

void CAutoTank::Reset()
{
	for(int i = 0; i < 12; i++)
		Server()->SnapFreeId(m_aIds[i]);
	Destroy();
}

CCharacter *CAutoTank::FindNearestZombie()
{
	CCharacter *pClosest = nullptr;
	float ClosestDist = RANGE * RANGE;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

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

void CAutoTank::Fire()
{
	if(m_FireCooldown > 0)
		return;

	m_FireCooldown = FIRE_RATE;

	vec2 Dir = vec2(cos(m_Angle), sin(m_Angle));
	vec2 BarrelEnd = m_Pos + vec2(0, -40) + Dir * 70.0f;

	// Fire grenade projectile for high damage
	new CProjectile(
		GameWorld(),
		WEAPON_GRENADE,
		m_Owner,
		BarrelEnd,
		Dir,
		(int)(Server()->TickSpeed() * 1.5f),
		false, // Freeze
		true,  // Explosive
		SOUND_GRENADE_EXPLODE,
		Dir);

	GameServer()->CreateSound(BarrelEnd, SOUND_GRENADE_FIRE);
}

void CAutoTank::Tick()
{
	if(m_FireCooldown > 0)
		m_FireCooldown--;

	CCharacter *pTarget = FindNearestZombie();
	if(pTarget)
	{
		vec2 TurretBase = m_Pos + vec2(0, -40);
		vec2 Dir = pTarget->GetPos() - TurretBase;
		m_Angle = atan2(Dir.y, Dir.x);
		Fire();
	}
}

void CAutoTank::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	int StartTick = Server()->Tick() - 2;
	CNetObj_Laser *pObj;

	// Tank body (larger rectangle)
	float BodyW = 80.0f;
	float BodyH = 40.0f;

	vec2 BodyTL = m_Pos + vec2(-BodyW / 2, -BodyH);
	vec2 BodyTR = m_Pos + vec2(BodyW / 2, -BodyH);
	vec2 BodyBL = m_Pos + vec2(-BodyW / 2, 0);
	vec2 BodyBR = m_Pos + vec2(BodyW / 2, 0);

	// Body outline
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[0]);
	if(pObj)
	{
		pObj->m_X = (int)BodyTR.x;
		pObj->m_Y = (int)BodyTR.y;
		pObj->m_FromX = (int)BodyTL.x;
		pObj->m_FromY = (int)BodyTL.y;
		pObj->m_StartTick = StartTick;
	}

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[1]);
	if(pObj)
	{
		pObj->m_X = (int)BodyBR.x;
		pObj->m_Y = (int)BodyBR.y;
		pObj->m_FromX = (int)BodyBL.x;
		pObj->m_FromY = (int)BodyBL.y;
		pObj->m_StartTick = StartTick;
	}

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[2]);
	if(pObj)
	{
		pObj->m_X = (int)BodyBL.x;
		pObj->m_Y = (int)BodyBL.y;
		pObj->m_FromX = (int)BodyTL.x;
		pObj->m_FromY = (int)BodyTL.y;
		pObj->m_StartTick = StartTick;
	}

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[3]);
	if(pObj)
	{
		pObj->m_X = (int)BodyBR.x;
		pObj->m_Y = (int)BodyBR.y;
		pObj->m_FromX = (int)BodyTR.x;
		pObj->m_FromY = (int)BodyTR.y;
		pObj->m_StartTick = StartTick;
	}

	// Turret (smaller box on top)
	vec2 TurretCenter = m_Pos + vec2(0, -BodyH - 20);
	float TurretSize = 30.0f;

	vec2 TurretTL = TurretCenter + vec2(-TurretSize / 2, -TurretSize / 2);
	vec2 TurretTR = TurretCenter + vec2(TurretSize / 2, -TurretSize / 2);
	vec2 TurretBL = TurretCenter + vec2(-TurretSize / 2, TurretSize / 2);
	vec2 TurretBR = TurretCenter + vec2(TurretSize / 2, TurretSize / 2);

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[4]);
	if(pObj)
	{
		pObj->m_X = (int)TurretTR.x;
		pObj->m_Y = (int)TurretTR.y;
		pObj->m_FromX = (int)TurretTL.x;
		pObj->m_FromY = (int)TurretTL.y;
		pObj->m_StartTick = StartTick;
	}

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[5]);
	if(pObj)
	{
		pObj->m_X = (int)TurretBR.x;
		pObj->m_Y = (int)TurretBR.y;
		pObj->m_FromX = (int)TurretBL.x;
		pObj->m_FromY = (int)TurretBL.y;
		pObj->m_StartTick = StartTick;
	}

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[6]);
	if(pObj)
	{
		pObj->m_X = (int)TurretBL.x;
		pObj->m_Y = (int)TurretBL.y;
		pObj->m_FromX = (int)TurretTL.x;
		pObj->m_FromY = (int)TurretTL.y;
		pObj->m_StartTick = StartTick;
	}

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[7]);
	if(pObj)
	{
		pObj->m_X = (int)TurretBR.x;
		pObj->m_Y = (int)TurretBR.y;
		pObj->m_FromX = (int)TurretTR.x;
		pObj->m_FromY = (int)TurretTR.y;
		pObj->m_StartTick = StartTick;
	}

	// Barrel
	vec2 Dir = vec2(cos(m_Angle), sin(m_Angle));
	vec2 BarrelStart = TurretCenter;
	vec2 BarrelEnd = TurretCenter + Dir * 70.0f;

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[8]);
	if(pObj)
	{
		pObj->m_X = (int)BarrelEnd.x;
		pObj->m_Y = (int)BarrelEnd.y;
		pObj->m_FromX = (int)BarrelStart.x;
		pObj->m_FromY = (int)BarrelStart.y;
		pObj->m_StartTick = StartTick;
	}
}
