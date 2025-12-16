/* DDNet Auto Cannon Entity - automatic targeting cannon with knockback for zombie event */
#include "autocannon.h"
#include "character.h"
#include "projectile.h"

#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include <game/server/zombieevent.h>
#include <generated/protocol.h>

CAutoCannon::CAutoCannon(CGameWorld *pGameWorld, vec2 Pos, int Owner) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER, Pos, 60)
{
	m_Owner = Owner;
	m_Angle = -3.14159f / 2.0f;
	m_FireCooldown = 0;
	m_EventMode = true;

	for(int i = 0; i < 10; i++)
		m_aIds[i] = Server()->SnapNewId();

	GameWorld()->InsertEntity(this);
}

void CAutoCannon::Reset()
{
	for(int i = 0; i < 10; i++)
		Server()->SnapFreeId(m_aIds[i]);
	Destroy();
}

CCharacter *CAutoCannon::FindNearestZombie()
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

void CAutoCannon::Fire()
{
	if(m_FireCooldown > 0)
		return;

	m_FireCooldown = FIRE_RATE;

	vec2 Dir = vec2(cos(m_Angle), sin(m_Angle));
	vec2 BarrelEnd = m_Pos + vec2(0, -50) + Dir * 60.0f;

	// Fire shotgun-like projectile for knockback
	new CProjectile(
		GameWorld(),
		WEAPON_SHOTGUN,
		m_Owner,
		BarrelEnd,
		Dir,
		(int)(Server()->TickSpeed() * 0.8f),
		false, // Freeze
		true,  // Explosive for knockback
		SOUND_SHOTGUN_FIRE,
		Dir);

	GameServer()->CreateSound(BarrelEnd, SOUND_SHOTGUN_FIRE);
}

void CAutoCannon::Tick()
{
	if(m_FireCooldown > 0)
		m_FireCooldown--;

	CCharacter *pTarget = FindNearestZombie();
	if(pTarget)
	{
		vec2 CannonBase = m_Pos + vec2(0, -50);
		vec2 Dir = pTarget->GetPos() - CannonBase;
		m_Angle = atan2(Dir.y, Dir.x);
		Fire();
	}
}

void CAutoCannon::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	int StartTick = Server()->Tick() - 2;
	CNetObj_Laser *pObj;

	// Cannon base (wide platform)
	float BaseW = 60.0f;
	float BaseH = 20.0f;

	vec2 BaseL = m_Pos + vec2(-BaseW / 2, 0);
	vec2 BaseR = m_Pos + vec2(BaseW / 2, 0);
	vec2 BaseTopL = m_Pos + vec2(-BaseW / 2, -BaseH);
	vec2 BaseTopR = m_Pos + vec2(BaseW / 2, -BaseH);

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[0]);
	if(pObj)
	{
		pObj->m_X = (int)BaseR.x;
		pObj->m_Y = (int)BaseR.y;
		pObj->m_FromX = (int)BaseL.x;
		pObj->m_FromY = (int)BaseL.y;
		pObj->m_StartTick = StartTick;
	}

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[1]);
	if(pObj)
	{
		pObj->m_X = (int)BaseTopR.x;
		pObj->m_Y = (int)BaseTopR.y;
		pObj->m_FromX = (int)BaseTopL.x;
		pObj->m_FromY = (int)BaseTopL.y;
		pObj->m_StartTick = StartTick;
	}

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[2]);
	if(pObj)
	{
		pObj->m_X = (int)BaseL.x;
		pObj->m_Y = (int)BaseL.y;
		pObj->m_FromX = (int)BaseTopL.x;
		pObj->m_FromY = (int)BaseTopL.y;
		pObj->m_StartTick = StartTick;
	}

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[3]);
	if(pObj)
	{
		pObj->m_X = (int)BaseR.x;
		pObj->m_Y = (int)BaseR.y;
		pObj->m_FromX = (int)BaseTopR.x;
		pObj->m_FromY = (int)BaseTopR.y;
		pObj->m_StartTick = StartTick;
	}

	// Cannon barrel mount (circle approximation)
	vec2 MountCenter = m_Pos + vec2(0, -BaseH - 30);
	float MountRadius = 25.0f;

	// Draw octagon for mount
	for(int i = 0; i < 4; i++)
	{
		float a1 = (float)i * 3.14159f / 2.0f;
		float a2 = (float)(i + 1) * 3.14159f / 2.0f;
		vec2 p1 = MountCenter + vec2(cos(a1), sin(a1)) * MountRadius;
		vec2 p2 = MountCenter + vec2(cos(a2), sin(a2)) * MountRadius;

		pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[4 + i]);
		if(pObj)
		{
			pObj->m_X = (int)p2.x;
			pObj->m_Y = (int)p2.y;
			pObj->m_FromX = (int)p1.x;
			pObj->m_FromY = (int)p1.y;
			pObj->m_StartTick = StartTick;
		}
	}

	// Barrel (thick)
	vec2 Dir = vec2(cos(m_Angle), sin(m_Angle));
	vec2 BarrelStart = MountCenter;
	vec2 BarrelEnd = MountCenter + Dir * 60.0f;

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[8]);
	if(pObj)
	{
		pObj->m_X = (int)BarrelEnd.x;
		pObj->m_Y = (int)BarrelEnd.y;
		pObj->m_FromX = (int)BarrelStart.x;
		pObj->m_FromY = (int)BarrelStart.y;
		pObj->m_StartTick = StartTick;
	}

	// Second barrel line for thickness
	vec2 Perp = vec2(-Dir.y, Dir.x) * 5.0f;
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[9]);
	if(pObj)
	{
		pObj->m_X = (int)(BarrelEnd.x + Perp.x);
		pObj->m_Y = (int)(BarrelEnd.y + Perp.y);
		pObj->m_FromX = (int)(BarrelStart.x + Perp.x);
		pObj->m_FromY = (int)(BarrelStart.y + Perp.y);
		pObj->m_StartTick = StartTick;
	}
}
