/* DDNet Turret Entity - stationary laser turret */
#include "turret.h"
#include "character.h"
#include "laser.h"

#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include <generated/protocol.h>

CTurret::CTurret(CGameWorld *pGameWorld, vec2 Pos, int Owner) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER, Pos, 50)
{
	m_Owner = Owner;
	m_Pilot = -1;
	m_Angle = -3.14159f / 2.0f; // Point up initially
	m_FireCooldown = 0;

	for(int i = 0; i < 8; i++)
		m_aIds[i] = Server()->SnapNewId();

	GameWorld()->InsertEntity(this);
}

void CTurret::Reset()
{
	RemovePilot();

	for(int i = 0; i < 8; i++)
		Server()->SnapFreeId(m_aIds[i]);

	Destroy();
}

void CTurret::SetPilot(int ClientId)
{
	m_Pilot = ClientId;
	// Player is NOT frozen - can move freely while in turret
}

void CTurret::RemovePilot()
{
	m_Pilot = -1;
}

void CTurret::OnHammerHit(int ClientId)
{
	if(ClientId < 0 || ClientId >= MAX_CLIENTS)
		return;

	// If same player hits again, exit
	if(m_Pilot == ClientId)
	{
		RemovePilot();
		GameServer()->SendChatTarget(ClientId, "You left the turret");
		return;
	}

	// If someone else is using it
	if(m_Pilot >= 0)
	{
		GameServer()->SendChatTarget(ClientId, "Someone else is using this turret");
		return;
	}

	// Enter turret
	SetPilot(ClientId);
	GameServer()->SendChatTarget(ClientId, "Turret! Aim with mouse, LMB to fire. Hook or Jump to exit.");
}

void CTurret::Fire()
{
	if(m_FireCooldown > 0)
		return;
	
	m_FireCooldown = 8; // Fast fire rate
	
	vec2 Dir = vec2(cos(m_Angle), sin(m_Angle));
	vec2 BarrelEnd = m_Pos + vec2(0, -30) + Dir * 50.0f;
	
	// Create laser projectile
	new CLaser(GameWorld(), BarrelEnd, Dir, 800.0f, m_Pilot, WEAPON_LASER);
	
	GameServer()->CreateSound(BarrelEnd, SOUND_LASER_FIRE);
}

void CTurret::Tick()
{
	if(m_FireCooldown > 0)
		m_FireCooldown--;

	if(m_Pilot >= 0)
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
		
		// Exit on Hook (RMB) or Jump
		if(Input.m_Hook || Input.m_Jump)
		{
			RemovePilot();
			return;
		}
		
		// Update turret angle to point at cursor
		vec2 CursorPos = m_Pos + vec2(Input.m_TargetX, Input.m_TargetY);
		vec2 TurretBase = m_Pos + vec2(0, -30);
		vec2 Dir = CursorPos - TurretBase;
		m_Angle = atan2(Dir.y, Dir.x);
		
		// Fire on LMB
		if(Input.m_Fire & 1)
			Fire();
	}
}

void CTurret::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	int StartTick = Server()->Tick() - 2;
	CNetObj_Laser *pObj;

	// === BASE (tripod) ===
	float BaseW = 40.0f;
	float BaseH = 30.0f;
	
	vec2 BaseL = m_Pos + vec2(-BaseW/2, 0);
	vec2 BaseR = m_Pos + vec2(BaseW/2, 0);
	vec2 BaseTop = m_Pos + vec2(0, -BaseH);

	// Left leg
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[0]);
	if(pObj)
	{
		pObj->m_X = (int)BaseL.x;
		pObj->m_Y = (int)BaseL.y;
		pObj->m_FromX = (int)BaseTop.x;
		pObj->m_FromY = (int)BaseTop.y;
		pObj->m_StartTick = StartTick;
	}

	// Right leg
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[1]);
	if(pObj)
	{
		pObj->m_X = (int)BaseR.x;
		pObj->m_Y = (int)BaseR.y;
		pObj->m_FromX = (int)BaseTop.x;
		pObj->m_FromY = (int)BaseTop.y;
		pObj->m_StartTick = StartTick;
	}

	// Bottom bar
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[2]);
	if(pObj)
	{
		pObj->m_X = (int)BaseR.x;
		pObj->m_Y = (int)BaseR.y;
		pObj->m_FromX = (int)BaseL.x;
		pObj->m_FromY = (int)BaseL.y;
		pObj->m_StartTick = StartTick;
	}

	// === TURRET HEAD ===
	vec2 HeadCenter = m_Pos + vec2(0, -BaseH - 15);
	float HeadSize = 20.0f;
	
	// Head box
	vec2 HeadTL = HeadCenter + vec2(-HeadSize/2, -HeadSize/2);
	vec2 HeadTR = HeadCenter + vec2(HeadSize/2, -HeadSize/2);
	vec2 HeadBL = HeadCenter + vec2(-HeadSize/2, HeadSize/2);
	vec2 HeadBR = HeadCenter + vec2(HeadSize/2, HeadSize/2);

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

	// === BARREL ===
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
