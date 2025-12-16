/* DDNet UFO Entity */
#include "ufo.h"
#include "character.h"

#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include <generated/protocol.h>

CUfo::CUfo(CGameWorld *pGameWorld, vec2 Pos, int Owner) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_UFO, Pos, 100)
{
	m_Owner = Owner;
	m_Pilot = -1;
	m_Victim = -1;
	m_Velocity = vec2(0, 0);
	m_AnimTick = 0;
	m_BeamPhase = 0;

	for(int i = 0; i < 12; i++)
		m_aIds[i] = Server()->SnapNewId();

	GameWorld()->InsertEntity(this);
}

void CUfo::Reset()
{
	ReleaseVictim();
	RemovePilot();

	for(int i = 0; i < 12; i++)
		Server()->SnapFreeId(m_aIds[i]);

	Destroy();
}

void CUfo::SetPilot(int ClientId)
{
	m_Pilot = ClientId;
	if(ClientId >= 0 && ClientId < MAX_CLIENTS)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
		if(pPlayer)
		{
			pPlayer->m_InUfo = true;
			pPlayer->m_pUfo = this;
		}
	}
}

void CUfo::RemovePilot()
{
	if(m_Pilot >= 0)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[m_Pilot];
		if(pPlayer)
		{
			pPlayer->m_InUfo = false;
			pPlayer->m_pUfo = nullptr;
			
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


void CUfo::SetVictim(int ClientId)
{
	m_Victim = ClientId;
	if(ClientId >= 0 && ClientId < MAX_CLIENTS)
	{
		CPlayer *pVictim = GameServer()->m_apPlayers[ClientId];
		if(pVictim)
		{
			pVictim->m_InUfo = true;
			pVictim->m_pUfo = this;
		}
	}
}

void CUfo::ReleaseVictim()
{
	if(m_Victim >= 0 && m_Victim < MAX_CLIENTS)
	{
		CPlayer *pVictim = GameServer()->m_apPlayers[m_Victim];
		if(pVictim)
		{
			pVictim->m_InUfo = false;
			pVictim->m_pUfo = nullptr;
		}
	}
	m_Victim = -1;
}

void CUfo::Tick()
{
	m_AnimTick++;
	m_BeamPhase += 0.15f;

	// Gravity when no pilot
	if(m_Pilot < 0)
	{
		m_Velocity.y += 0.3f; // Fall down
		
		// Apply velocity with collision
		vec2 NewPos = m_Pos + m_Velocity;
		float BodyW = 120.0f;
		float BodyH = 40.0f;
		
		bool Collided = Collision()->CheckPoint(NewPos + vec2(0, BodyH/2)) ||
		                Collision()->CheckPoint(NewPos + vec2(-BodyW/2, 0)) ||
		                Collision()->CheckPoint(NewPos + vec2(BodyW/2, 0));

		if(!Collided)
			m_Pos = NewPos;
		else
			m_Velocity = vec2(0, 0);
		return;
	}
	else
	{
		// Get pilot input
		CPlayer *pPilot = GameServer()->m_apPlayers[m_Pilot];
		if(!pPilot)
		{
			RemovePilot();
			return;
		}

		CCharacter *pPilotChr = pPilot->GetCharacter();
		if(!pPilotChr)
		{
			RemovePilot();
			return;
		}

		// Move UFO with cursor
		CNetObj_PlayerInput Input = GameServer()->GetLastPlayerInput(m_Pilot);
		vec2 CursorPos = pPilotChr->GetPos() + vec2(Input.m_TargetX, Input.m_TargetY);

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

		// LMB (Fire) to abduct player below
		if((Input.m_Fire & 1) && !HasVictim())
		{
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(i == m_Pilot)
					continue;
					
				CCharacter *pChr = GameServer()->GetPlayerChar(i);
				if(pChr)
				{
					vec2 PlayerPos = pChr->GetPos();
					// Check if player is below UFO (wide area)
					if(abs(PlayerPos.x - m_Pos.x) < 100.0f && 
					   PlayerPos.y > m_Pos.y && 
					   PlayerPos.y < m_Pos.y + 300.0f)
					{
						SetVictim(i);
						GameServer()->SendChatTarget(m_Pilot, "Player abducted!");
						GameServer()->SendChatTarget(i, "You've been abducted! Press F4 to escape.");
						break;
					}
				}
			}
		}

		// Keep pilot inside UFO
		pPilotChr->SetPosition(m_Pos);
		pPilotChr->m_Pos = m_Pos;
		pPilotChr->m_PrevPos = m_Pos;
		pPilotChr->ResetVelocity();
	}

	// Apply velocity with collision
	vec2 NewPos = m_Pos + m_Velocity;
	float BodyW = 120.0f;
	float BodyH = 40.0f;
	
	bool Collided = Collision()->CheckPoint(NewPos + vec2(0, BodyH/2)) ||
	                Collision()->CheckPoint(NewPos + vec2(-BodyW/2, 0)) ||
	                Collision()->CheckPoint(NewPos + vec2(BodyW/2, 0));

	if(!Collided)
		m_Pos = NewPos;
	else
		m_Velocity = vec2(0, 0);

	// Drag victim
	if(HasVictim())
	{
		CCharacter *pVictimChr = GameServer()->GetPlayerChar(m_Victim);
		if(pVictimChr)
		{
			vec2 TargetPos = m_Pos + vec2(0, 50);
			pVictimChr->SetPosition(TargetPos);
			pVictimChr->m_Pos = TargetPos;
			pVictimChr->m_PrevPos = TargetPos;
			pVictimChr->ResetVelocity();
		}
		else
		{
			ReleaseVictim();
		}
	}
}


void CUfo::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	// Big UFO dimensions
	float BodyW = 120.0f;
	float BodyH = 25.0f;
	float DomeW = 50.0f;
	float DomeH = 35.0f;
	
	// Hovering animation
	float Hover = sin(m_AnimTick * 0.04f) * 4.0f;
	vec2 Center = m_Pos + vec2(0, Hover);

	int StartTick = Server()->Tick() - 2;
	CNetObj_Laser *pObj;

	// === DOME (glass top) ===
	vec2 DomeL = Center + vec2(-DomeW/2, -BodyH/2);
	vec2 DomeR = Center + vec2(DomeW/2, -BodyH/2);
	vec2 DomeTop = Center + vec2(0, -BodyH/2 - DomeH);
	vec2 DomeMidL = Center + vec2(-DomeW/3, -BodyH/2 - DomeH*0.7f);
	vec2 DomeMidR = Center + vec2(DomeW/3, -BodyH/2 - DomeH*0.7f);

	// Dome left curve
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[0]);
	if(pObj)
	{
		pObj->m_X = (int)DomeMidL.x;
		pObj->m_Y = (int)DomeMidL.y;
		pObj->m_FromX = (int)DomeL.x;
		pObj->m_FromY = (int)DomeL.y;
		pObj->m_StartTick = StartTick;
	}

	// Dome top left
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[1]);
	if(pObj)
	{
		pObj->m_X = (int)DomeTop.x;
		pObj->m_Y = (int)DomeTop.y;
		pObj->m_FromX = (int)DomeMidL.x;
		pObj->m_FromY = (int)DomeMidL.y;
		pObj->m_StartTick = StartTick;
	}

	// Dome top right
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[2]);
	if(pObj)
	{
		pObj->m_X = (int)DomeTop.x;
		pObj->m_Y = (int)DomeTop.y;
		pObj->m_FromX = (int)DomeMidR.x;
		pObj->m_FromY = (int)DomeMidR.y;
		pObj->m_StartTick = StartTick;
	}

	// Dome right curve
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[3]);
	if(pObj)
	{
		pObj->m_X = (int)DomeMidR.x;
		pObj->m_Y = (int)DomeMidR.y;
		pObj->m_FromX = (int)DomeR.x;
		pObj->m_FromY = (int)DomeR.y;
		pObj->m_StartTick = StartTick;
	}

	// === SAUCER BODY ===
	vec2 BodyL = Center + vec2(-BodyW/2, 0);
	vec2 BodyR = Center + vec2(BodyW/2, 0);
	vec2 BodyBL = Center + vec2(-BodyW/3, BodyH);
	vec2 BodyBR = Center + vec2(BodyW/3, BodyH);

	// Body top left (from dome to edge)
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[4]);
	if(pObj)
	{
		pObj->m_X = (int)DomeL.x;
		pObj->m_Y = (int)DomeL.y;
		pObj->m_FromX = (int)BodyL.x;
		pObj->m_FromY = (int)BodyL.y;
		pObj->m_StartTick = StartTick;
	}

	// Body top right
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[5]);
	if(pObj)
	{
		pObj->m_X = (int)DomeR.x;
		pObj->m_Y = (int)DomeR.y;
		pObj->m_FromX = (int)BodyR.x;
		pObj->m_FromY = (int)BodyR.y;
		pObj->m_StartTick = StartTick;
	}

	// Body bottom left
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[6]);
	if(pObj)
	{
		pObj->m_X = (int)BodyL.x;
		pObj->m_Y = (int)BodyL.y;
		pObj->m_FromX = (int)BodyBL.x;
		pObj->m_FromY = (int)BodyBL.y;
		pObj->m_StartTick = StartTick;
	}

	// Body bottom right
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[7]);
	if(pObj)
	{
		pObj->m_X = (int)BodyR.x;
		pObj->m_Y = (int)BodyR.y;
		pObj->m_FromX = (int)BodyBR.x;
		pObj->m_FromY = (int)BodyBR.y;
		pObj->m_StartTick = StartTick;
	}

	// Body bottom center
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[8]);
	if(pObj)
	{
		pObj->m_X = (int)BodyBR.x;
		pObj->m_Y = (int)BodyBR.y;
		pObj->m_FromX = (int)BodyBL.x;
		pObj->m_FromY = (int)BodyBL.y;
		pObj->m_StartTick = StartTick;
	}

	// === TRACTOR BEAM (when has victim or pilot is firing) ===
	CPlayer *pPilot = m_Pilot >= 0 ? GameServer()->m_apPlayers[m_Pilot] : nullptr;
	bool ShowBeam = HasVictim();
	if(pPilot && pPilot->GetCharacter())
	{
		CNetObj_PlayerInput Input = GameServer()->GetLastPlayerInput(m_Pilot);
		if(Input.m_Fire & 1)
			ShowBeam = true;
	}

	if(ShowBeam)
	{
		float BeamPulse = sin(m_BeamPhase) * 0.3f + 1.0f;
		float BeamTopW = 20.0f * BeamPulse;
		float BeamBotW = 60.0f * BeamPulse;
		float BeamLen = 120.0f;
		
		vec2 BeamTL = Center + vec2(-BeamTopW/2, BodyH);
		vec2 BeamTR = Center + vec2(BeamTopW/2, BodyH);
		vec2 BeamBL = Center + vec2(-BeamBotW/2, BodyH + BeamLen);
		vec2 BeamBR = Center + vec2(BeamBotW/2, BodyH + BeamLen);

		// Beam left
		pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[9]);
		if(pObj)
		{
			pObj->m_X = (int)BeamBL.x;
			pObj->m_Y = (int)BeamBL.y;
			pObj->m_FromX = (int)BeamTL.x;
			pObj->m_FromY = (int)BeamTL.y;
			pObj->m_StartTick = StartTick;
		}

		// Beam right
		pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[10]);
		if(pObj)
		{
			pObj->m_X = (int)BeamBR.x;
			pObj->m_Y = (int)BeamBR.y;
			pObj->m_FromX = (int)BeamTR.x;
			pObj->m_FromY = (int)BeamTR.y;
			pObj->m_StartTick = StartTick;
		}

		// Beam bottom
		pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[11]);
		if(pObj)
		{
			pObj->m_X = (int)BeamBR.x;
			pObj->m_Y = (int)BeamBR.y;
			pObj->m_FromX = (int)BeamBL.x;
			pObj->m_FromY = (int)BeamBL.y;
			pObj->m_StartTick = StartTick;
		}
	}
}
