/* DDNet Helicopter Entity */
#include "helicopter.h"
#include "character.h"

#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include <generated/protocol.h>

CHelicopter::CHelicopter(CGameWorld *pGameWorld, vec2 Pos, int Owner) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_HELICOPTER, Pos, 100)
{
	m_Owner = Owner;
	m_Pilot = -1;
	m_Velocity = vec2(0, 0);
	m_RotorTick = 0;

	for(int i = 0; i < 8; i++)
		m_aIds[i] = Server()->SnapNewId();

	GameWorld()->InsertEntity(this);
}

void CHelicopter::Reset()
{
	RemovePilot();

	for(int i = 0; i < 8; i++)
		Server()->SnapFreeId(m_aIds[i]);

	Destroy();
}

void CHelicopter::SetPilot(int ClientId)
{
	m_Pilot = ClientId;
	if(ClientId >= 0 && ClientId < MAX_CLIENTS)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
		if(pPlayer)
		{
			pPlayer->m_InHelicopter = true;
			pPlayer->m_pHelicopter = this;
		}
	}
}

void CHelicopter::RemovePilot()
{
	if(m_Pilot >= 0 && m_Pilot < MAX_CLIENTS)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[m_Pilot];
		if(pPlayer)
		{
			pPlayer->m_InHelicopter = false;
			pPlayer->m_pHelicopter = nullptr;
			
			CCharacter *pChr = pPlayer->GetCharacter();
			if(pChr)
			{
				pChr->SetPosition(m_Pos + vec2(60, 0));
				pChr->m_Pos = m_Pos + vec2(60, 0);
				pChr->m_PrevPos = m_Pos + vec2(60, 0);
			}
		}
	}
	m_Pilot = -1;
}

void CHelicopter::Tick()
{
	m_RotorTick++;

	// Gravity when no pilot
	if(m_Pilot < 0)
	{
		m_Velocity.y += 0.3f; // Fall down
		
		// Apply velocity with collision
		vec2 NewPos = m_Pos + m_Velocity;
		float BodyH = 40.0f;
		float BodyW = 80.0f;
		
		bool Collided = Collision()->CheckPoint(NewPos + vec2(0, BodyH/2)) ||
		                Collision()->CheckPoint(NewPos + vec2(-BodyW/2, 0)) ||
		                Collision()->CheckPoint(NewPos + vec2(BodyW/2, 0)) ||
		                Collision()->CheckPoint(NewPos + vec2(0, -BodyH/2));

		if(!Collided)
			m_Pos = NewPos;
		else
			m_Velocity = vec2(0, 0);
		return;
	}
	else
	{
		// Get pilot
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

		// Get cursor position from player input
		CNetObj_PlayerInput Input = GameServer()->GetLastPlayerInput(m_Pilot);
		vec2 CursorPos = pChr->GetPos() + vec2(Input.m_TargetX, Input.m_TargetY);

		// Move helicopter towards cursor
		vec2 Dir = CursorPos - m_Pos;
		float Dist = length(Dir);
		
		if(Dist > 10.0f)
		{
			Dir = normalize(Dir);
			float Speed = minimum(Dist * 0.1f, 15.0f);
			m_Velocity = Dir * Speed;
		}
		else
		{
			m_Velocity *= 0.8f;
		}

		// Keep pilot inside helicopter
		pChr->SetPosition(m_Pos);
		pChr->m_Pos = m_Pos;
		pChr->m_PrevPos = m_Pos;
		pChr->ResetVelocity();
	}

	// Apply velocity with collision
	vec2 NewPos = m_Pos + m_Velocity;
	float BodyH = 40.0f;
	float BodyW = 80.0f;
	
	bool Collided = Collision()->CheckPoint(NewPos + vec2(0, BodyH/2)) ||
	                Collision()->CheckPoint(NewPos + vec2(-BodyW/2, 0)) ||
	                Collision()->CheckPoint(NewPos + vec2(BodyW/2, 0)) ||
	                Collision()->CheckPoint(NewPos + vec2(0, -BodyH/2));

	if(!Collided)
		m_Pos = NewPos;
	else
		m_Velocity = vec2(0, 0);
}


void CHelicopter::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	// Draw helicopter using lasers
	float BodyW = 80.0f;
	float BodyH = 40.0f;
	
	vec2 BodyTL = m_Pos + vec2(-BodyW/2, -BodyH/2);
	vec2 BodyTR = m_Pos + vec2(BodyW/2, -BodyH/2);
	vec2 BodyBL = m_Pos + vec2(-BodyW/2, BodyH/2);
	vec2 BodyBR = m_Pos + vec2(BodyW/2, BodyH/2);

	// Rotor (spinning line on top)
	float RotorLen = 100.0f;
	float RotorAngle = (m_RotorTick * 0.3f);
	vec2 RotorDir = vec2(cos(RotorAngle), sin(RotorAngle) * 0.3f);
	vec2 RotorCenter = m_Pos + vec2(0, -BodyH/2 - 5);
	vec2 RotorL = RotorCenter - RotorDir * RotorLen/2;
	vec2 RotorR = RotorCenter + RotorDir * RotorLen/2;

	// Tail
	vec2 TailEnd = m_Pos + vec2(-BodyW/2 - 60, 0);

	int StartTick = Server()->Tick() - 2;
	CNetObj_Laser *pObj;

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

	// Bottom line
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[1]);
	if(pObj)
	{
		pObj->m_X = (int)BodyBR.x;
		pObj->m_Y = (int)BodyBR.y;
		pObj->m_FromX = (int)BodyBL.x;
		pObj->m_FromY = (int)BodyBL.y;
		pObj->m_StartTick = StartTick;
	}

	// Left line
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[2]);
	if(pObj)
	{
		pObj->m_X = (int)BodyBL.x;
		pObj->m_Y = (int)BodyBL.y;
		pObj->m_FromX = (int)BodyTL.x;
		pObj->m_FromY = (int)BodyTL.y;
		pObj->m_StartTick = StartTick;
	}

	// Right line
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[3]);
	if(pObj)
	{
		pObj->m_X = (int)BodyBR.x;
		pObj->m_Y = (int)BodyBR.y;
		pObj->m_FromX = (int)BodyTR.x;
		pObj->m_FromY = (int)BodyTR.y;
		pObj->m_StartTick = StartTick;
	}

	// Rotor
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[4]);
	if(pObj)
	{
		pObj->m_X = (int)RotorR.x;
		pObj->m_Y = (int)RotorR.y;
		pObj->m_FromX = (int)RotorL.x;
		pObj->m_FromY = (int)RotorL.y;
		pObj->m_StartTick = StartTick;
	}

	// Rotor pole
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[5]);
	if(pObj)
	{
		pObj->m_X = (int)RotorCenter.x;
		pObj->m_Y = (int)RotorCenter.y;
		pObj->m_FromX = (int)(m_Pos.x);
		pObj->m_FromY = (int)(m_Pos.y - BodyH/2);
		pObj->m_StartTick = StartTick;
	}

	// Tail
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[6]);
	if(pObj)
	{
		pObj->m_X = (int)TailEnd.x;
		pObj->m_Y = (int)TailEnd.y;
		pObj->m_FromX = (int)BodyTL.x;
		pObj->m_FromY = (int)BodyTL.y;
		pObj->m_StartTick = StartTick;
	}

	// Tail rotor
	float TailRotorAngle = m_RotorTick * 0.5f;
	float TailRotorLen = 30.0f;
	vec2 TailRotorT = TailEnd + vec2(0, -TailRotorLen/2) * cos(TailRotorAngle);
	vec2 TailRotorB = TailEnd + vec2(0, TailRotorLen/2) * cos(TailRotorAngle);

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[7]);
	if(pObj)
	{
		pObj->m_X = (int)TailRotorB.x;
		pObj->m_Y = (int)TailRotorB.y;
		pObj->m_FromX = (int)TailRotorT.x;
		pObj->m_FromY = (int)TailRotorT.y;
		pObj->m_StartTick = StartTick;
	}
}
