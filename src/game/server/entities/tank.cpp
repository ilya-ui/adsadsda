/* DDNet Tank Entity */
#include "tank.h"
#include "character.h"
#include "projectile.h"

#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include <generated/protocol.h>

CTank::CTank(CGameWorld *pGameWorld, vec2 Pos, int Owner) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_TANK, Pos, 100)
{
	m_Owner = Owner;
	m_Pilot = -1;
	m_Velocity = vec2(0, 0);
	m_TurretAngle = 0;
	m_FireCooldown = 0;

	for(int i = 0; i < 12; i++)
		m_aIds[i] = Server()->SnapNewId();

	GameWorld()->InsertEntity(this);
}

void CTank::Reset()
{
	RemovePilot();

	for(int i = 0; i < 12; i++)
		Server()->SnapFreeId(m_aIds[i]);

	Destroy();
}

void CTank::SetPilot(int ClientId)
{
	m_Pilot = ClientId;
	if(ClientId >= 0 && ClientId < MAX_CLIENTS)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
		if(pPlayer)
		{
			pPlayer->m_InTank = true;
			pPlayer->m_pTank = this;
		}
	}
}

void CTank::RemovePilot()
{
	if(m_Pilot >= 0 && m_Pilot < MAX_CLIENTS)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[m_Pilot];
		if(pPlayer)
		{
			pPlayer->m_InTank = false;
			pPlayer->m_pTank = nullptr;
			
			CCharacter *pChr = pPlayer->GetCharacter();
			if(pChr)
			{
				pChr->SetPosition(m_Pos + vec2(80, -20));
				pChr->m_Pos = m_Pos + vec2(80, -20);
				pChr->m_PrevPos = m_Pos + vec2(80, -20);
			}
		}
	}
	m_Pilot = -1;
}

void CTank::Fire()
{
	if(m_FireCooldown > 0)
		return;
	
	m_FireCooldown = 30; // Half second cooldown
	
	vec2 TurretDir = vec2(cos(m_TurretAngle), sin(m_TurretAngle));
	vec2 TurretEnd = m_Pos + vec2(0, -25) + TurretDir * 60.0f;
	
	// Get mouse target position for proper explosion direction
	CNetObj_PlayerInput Input = GameServer()->GetLastPlayerInput(m_Pilot);
	vec2 MouseTarget = vec2(Input.m_TargetX, Input.m_TargetY);
	
	// Fire 10 grenades in same direction
	for(int i = 0; i < 10; i++)
	{
		new CProjectile(
			GameWorld(),
			WEAPON_GRENADE,
			m_Pilot,
			TurretEnd,
			TurretDir,
			(int)(Server()->TickSpeed() * 2.0f),
			false, true, SOUND_GRENADE_EXPLODE, MouseTarget);
	}
	
	GameServer()->CreateSound(TurretEnd, SOUND_GRENADE_FIRE);
}

void CTank::Tick()
{
	if(m_FireCooldown > 0)
		m_FireCooldown--;

	if(m_Pilot < 0)
	{
		// No pilot - just apply gravity
		m_Velocity.y += 0.5f;
		m_Velocity.x *= 0.98f;
	}
	else
	{
		CPlayer *pPilot = GameServer()->m_apPlayers[m_Pilot];
		if(!pPilot)
		{
			RemovePilot();
			return;
		}

		CCharacter *pChr = pPilot->GetCharacter();
		if(!pChr)
		{
			RemovePilot();
			return;
		}

		// Get input
		CNetObj_PlayerInput Input = GameServer()->GetLastPlayerInput(m_Pilot);
		
		// Update turret angle to point at cursor
		vec2 CursorPos = m_Pos + vec2(Input.m_TargetX, Input.m_TargetY);
		vec2 TurretBase = m_Pos + vec2(0, -25);
		vec2 Dir = CursorPos - TurretBase;
		m_TurretAngle = atan2(Dir.y, Dir.x);
		
		// Movement - left/right
		if(Input.m_Direction < 0)
			m_Velocity.x -= 0.8f;
		else if(Input.m_Direction > 0)
			m_Velocity.x += 0.8f;
		else
			m_Velocity.x *= 0.95f;
		
		// Limit speed
		if(m_Velocity.x > 8.0f) m_Velocity.x = 8.0f;
		if(m_Velocity.x < -8.0f) m_Velocity.x = -8.0f;
		
		// Gravity
		m_Velocity.y += 0.5f;
		
		// Fire on LMB
		if(Input.m_Fire & 1)
			Fire();

		// Keep pilot inside tank
		pChr->SetPosition(m_Pos + vec2(0, -10));
		pChr->m_Pos = m_Pos + vec2(0, -10);
		pChr->m_PrevPos = m_Pos + vec2(0, -10);
		pChr->ResetVelocity();
	}

	// Apply velocity with collision
	vec2 NewPos = m_Pos + m_Velocity;
	float BodyW = 100.0f;
	float BodyH = 40.0f;
	
	// Check ground collision
	bool OnGround = Collision()->CheckPoint(m_Pos + vec2(0, BodyH/2 + 1)) ||
	                Collision()->CheckPoint(m_Pos + vec2(-BodyW/3, BodyH/2 + 1)) ||
	                Collision()->CheckPoint(m_Pos + vec2(BodyW/3, BodyH/2 + 1));
	
	// Horizontal collision
	bool HitWallLeft = Collision()->CheckPoint(NewPos + vec2(-BodyW/2, 0)) ||
	                   Collision()->CheckPoint(NewPos + vec2(-BodyW/2, BodyH/3));
	bool HitWallRight = Collision()->CheckPoint(NewPos + vec2(BodyW/2, 0)) ||
	                    Collision()->CheckPoint(NewPos + vec2(BodyW/2, BodyH/3));
	
	if(HitWallLeft && m_Velocity.x < 0)
		m_Velocity.x = 0;
	if(HitWallRight && m_Velocity.x > 0)
		m_Velocity.x = 0;
	
	// Vertical collision
	bool HitCeiling = Collision()->CheckPoint(NewPos + vec2(0, -BodyH/2));
	bool HitFloor = Collision()->CheckPoint(NewPos + vec2(0, BodyH/2)) ||
	                Collision()->CheckPoint(NewPos + vec2(-BodyW/3, BodyH/2)) ||
	                Collision()->CheckPoint(NewPos + vec2(BodyW/3, BodyH/2));
	
	if(HitCeiling && m_Velocity.y < 0)
		m_Velocity.y = 0;
	if(HitFloor && m_Velocity.y > 0)
		m_Velocity.y = 0;
	
	m_Pos = m_Pos + m_Velocity;
}


void CTank::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	float BodyW = 100.0f;
	float BodyH = 30.0f;
	float TrackH = 15.0f;
	float TurretLen = 60.0f;

	int StartTick = Server()->Tick() - 2;
	CNetObj_Laser *pObj;

	// === TANK BODY ===
	vec2 BodyTL = m_Pos + vec2(-BodyW/2, -BodyH);
	vec2 BodyTR = m_Pos + vec2(BodyW/2, -BodyH);
	vec2 BodyBL = m_Pos + vec2(-BodyW/2, 0);
	vec2 BodyBR = m_Pos + vec2(BodyW/2, 0);

	// Top line
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[0]);
	if(pObj)
	{
		pObj->m_X = (int)BodyTR.x;
		pObj->m_Y = (int)BodyTR.y;
		pObj->m_FromX = (int)BodyTL.x;
		pObj->m_FromY = (int)BodyTL.y;
		pObj->m_StartTick = StartTick;
	}

	// Left line
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[1]);
	if(pObj)
	{
		pObj->m_X = (int)BodyBL.x;
		pObj->m_Y = (int)BodyBL.y;
		pObj->m_FromX = (int)BodyTL.x;
		pObj->m_FromY = (int)BodyTL.y;
		pObj->m_StartTick = StartTick;
	}

	// Right line
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[2]);
	if(pObj)
	{
		pObj->m_X = (int)BodyBR.x;
		pObj->m_Y = (int)BodyBR.y;
		pObj->m_FromX = (int)BodyTR.x;
		pObj->m_FromY = (int)BodyTR.y;
		pObj->m_StartTick = StartTick;
	}

	// === TRACKS ===
	vec2 TrackTL = m_Pos + vec2(-BodyW/2 - 5, 0);
	vec2 TrackTR = m_Pos + vec2(BodyW/2 + 5, 0);
	vec2 TrackBL = m_Pos + vec2(-BodyW/2 - 5, TrackH);
	vec2 TrackBR = m_Pos + vec2(BodyW/2 + 5, TrackH);

	// Track top
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[3]);
	if(pObj)
	{
		pObj->m_X = (int)TrackTR.x;
		pObj->m_Y = (int)TrackTR.y;
		pObj->m_FromX = (int)TrackTL.x;
		pObj->m_FromY = (int)TrackTL.y;
		pObj->m_StartTick = StartTick;
	}

	// Track bottom
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[4]);
	if(pObj)
	{
		pObj->m_X = (int)TrackBR.x;
		pObj->m_Y = (int)TrackBR.y;
		pObj->m_FromX = (int)TrackBL.x;
		pObj->m_FromY = (int)TrackBL.y;
		pObj->m_StartTick = StartTick;
	}

	// Track left
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[5]);
	if(pObj)
	{
		pObj->m_X = (int)TrackBL.x;
		pObj->m_Y = (int)TrackBL.y;
		pObj->m_FromX = (int)TrackTL.x;
		pObj->m_FromY = (int)TrackTL.y;
		pObj->m_StartTick = StartTick;
	}

	// Track right
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[6]);
	if(pObj)
	{
		pObj->m_X = (int)TrackBR.x;
		pObj->m_Y = (int)TrackBR.y;
		pObj->m_FromX = (int)TrackTR.x;
		pObj->m_FromY = (int)TrackTR.y;
		pObj->m_StartTick = StartTick;
	}

	// === TURRET ===
	vec2 TurretBase = m_Pos + vec2(0, -BodyH - 10);
	vec2 TurretDir = vec2(cos(m_TurretAngle), sin(m_TurretAngle));
	vec2 TurretEnd = TurretBase + TurretDir * TurretLen;

	// Turret barrel
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[7]);
	if(pObj)
	{
		pObj->m_X = (int)TurretEnd.x;
		pObj->m_Y = (int)TurretEnd.y;
		pObj->m_FromX = (int)TurretBase.x;
		pObj->m_FromY = (int)TurretBase.y;
		pObj->m_StartTick = StartTick;
	}

	// Turret dome (small box around base)
	float DomeSize = 15.0f;
	vec2 DomeTL = TurretBase + vec2(-DomeSize, -DomeSize);
	vec2 DomeTR = TurretBase + vec2(DomeSize, -DomeSize);
	vec2 DomeBL = TurretBase + vec2(-DomeSize, DomeSize/2);
	vec2 DomeBR = TurretBase + vec2(DomeSize, DomeSize/2);

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[8]);
	if(pObj)
	{
		pObj->m_X = (int)DomeTR.x;
		pObj->m_Y = (int)DomeTR.y;
		pObj->m_FromX = (int)DomeTL.x;
		pObj->m_FromY = (int)DomeTL.y;
		pObj->m_StartTick = StartTick;
	}

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[9]);
	if(pObj)
	{
		pObj->m_X = (int)DomeBL.x;
		pObj->m_Y = (int)DomeBL.y;
		pObj->m_FromX = (int)DomeTL.x;
		pObj->m_FromY = (int)DomeTL.y;
		pObj->m_StartTick = StartTick;
	}

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[10]);
	if(pObj)
	{
		pObj->m_X = (int)DomeBR.x;
		pObj->m_Y = (int)DomeBR.y;
		pObj->m_FromX = (int)DomeTR.x;
		pObj->m_FromY = (int)DomeTR.y;
		pObj->m_StartTick = StartTick;
	}

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[11]);
	if(pObj)
	{
		pObj->m_X = (int)DomeBR.x;
		pObj->m_Y = (int)DomeBR.y;
		pObj->m_FromX = (int)DomeBL.x;
		pObj->m_FromY = (int)DomeBL.y;
		pObj->m_StartTick = StartTick;
	}
}
