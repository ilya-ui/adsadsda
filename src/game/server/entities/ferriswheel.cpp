/* DDNet Ferris Wheel Entity */
#include "ferriswheel.h"
#include "character.h"

#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include <generated/protocol.h>

CFerrisWheel::CFerrisWheel(CGameWorld *pGameWorld, vec2 Pos) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_PICKUP, Pos, WHEEL_RADIUS + 50)
{
	// Move center up so the leg stands on spawn position
	float LegHeight = WHEEL_RADIUS + 80.0f;
	m_Pos = Pos - vec2(0, LegHeight);
	
	m_Angle = 0;
	
	for(int i = 0; i < NUM_SEATS; i++)
		m_aSeatedPlayers[i] = -1;

	for(int i = 0; i < 40; i++)
		m_aIds[i] = Server()->SnapNewId();

	GameWorld()->InsertEntity(this);
}

void CFerrisWheel::Reset()
{
	// Release all seated players
	for(int i = 0; i < NUM_SEATS; i++)
	{
		if(m_aSeatedPlayers[i] >= 0)
			ExitSeat(i);
	}

	for(int i = 0; i < 40; i++)
		Server()->SnapFreeId(m_aIds[i]);

	Destroy();
}

bool CFerrisWheel::TryEnterSeat(int ClientId, vec2 HammerPos)
{
	// Check if player is already seated
	for(int i = 0; i < NUM_SEATS; i++)
	{
		if(m_aSeatedPlayers[i] == ClientId)
		{
			// Exit seat
			ExitSeat(i);
			return true;
		}
	}
	
	// Find closest empty seat to hammer position
	int BestSeat = -1;
	float BestDist = 60.0f; // Max hammer range
	
	for(int i = 0; i < NUM_SEATS; i++)
	{
		if(m_aSeatedPlayers[i] >= 0)
			continue;
			
		float SeatAngle = m_Angle + (float)i / NUM_SEATS * 2.0f * pi;
		vec2 SeatPos = m_Pos + vec2(cos(SeatAngle), sin(SeatAngle)) * WHEEL_RADIUS;
		
		float Dist = distance(HammerPos, SeatPos);
		if(Dist < BestDist)
		{
			BestDist = Dist;
			BestSeat = i;
		}
	}
	
	if(BestSeat >= 0)
	{
		m_aSeatedPlayers[BestSeat] = ClientId;
		GameServer()->SendChatTarget(ClientId, "Seated on ferris wheel! Hammer again to exit.");
		return true;
	}
	
	return false;
}

void CFerrisWheel::ExitSeat(int SeatIndex)
{
	if(SeatIndex < 0 || SeatIndex >= NUM_SEATS)
		return;
		
	int ClientId = m_aSeatedPlayers[SeatIndex];
	if(ClientId < 0)
		return;
	
	m_aSeatedPlayers[SeatIndex] = -1;
	
	CCharacter *pChr = GameServer()->GetPlayerChar(ClientId);
	if(pChr)
	{
		// Give small upward velocity when exiting
		pChr->SetVelocity(vec2(0, -5));
	}
	
	GameServer()->SendChatTarget(ClientId, "Exited ferris wheel!");
}

void CFerrisWheel::Tick()
{
	// Rotate wheel
	m_Angle += ROTATION_SPEED;
	if(m_Angle > 2.0f * pi)
		m_Angle -= 2.0f * pi;
	
	// Update seated players positions
	for(int i = 0; i < NUM_SEATS; i++)
	{
		if(m_aSeatedPlayers[i] < 0)
			continue;
			
		CCharacter *pChr = GameServer()->GetPlayerChar(m_aSeatedPlayers[i]);
		if(!pChr)
		{
			m_aSeatedPlayers[i] = -1;
			continue;
		}
		
		float SeatAngle = m_Angle + (float)i / NUM_SEATS * 2.0f * pi;
		vec2 SeatPos = m_Pos + vec2(cos(SeatAngle), sin(SeatAngle)) * WHEEL_RADIUS;
		
		pChr->SetPosition(SeatPos);
		pChr->m_Pos = SeatPos;
		pChr->m_PrevPos = SeatPos;
		pChr->ResetVelocity();
	}
}

void CFerrisWheel::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	int StartTick = Server()->Tick() - 2;
	CNetObj_Laser *pObj;
	int idx = 0;

	// Draw support leg (ножка)
	float LegHeight = WHEEL_RADIUS + 80.0f;
	vec2 LegBottom = m_Pos + vec2(0, LegHeight);
	vec2 LegLeft = m_Pos + vec2(-60, LegHeight);
	vec2 LegRight = m_Pos + vec2(60, LegHeight);
	
	// Left leg
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[idx++]);
	if(pObj)
	{
		pObj->m_X = (int)LegLeft.x;
		pObj->m_Y = (int)LegLeft.y;
		pObj->m_FromX = (int)m_Pos.x;
		pObj->m_FromY = (int)m_Pos.y;
		pObj->m_StartTick = StartTick;
	}
	
	// Right leg
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[idx++]);
	if(pObj)
	{
		pObj->m_X = (int)LegRight.x;
		pObj->m_Y = (int)LegRight.y;
		pObj->m_FromX = (int)m_Pos.x;
		pObj->m_FromY = (int)m_Pos.y;
		pObj->m_StartTick = StartTick;
	}
	
	// Bottom bar connecting legs
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[idx++]);
	if(pObj)
	{
		pObj->m_X = (int)LegRight.x;
		pObj->m_Y = (int)LegRight.y;
		pObj->m_FromX = (int)LegLeft.x;
		pObj->m_FromY = (int)LegLeft.y;
		pObj->m_StartTick = StartTick;
	}

	// Draw wheel spokes
	for(int i = 0; i < NUM_SEATS && idx < 40; i++)
	{
		float SeatAngle = m_Angle + (float)i / NUM_SEATS * 2.0f * pi;
		vec2 SeatPos = m_Pos + vec2(cos(SeatAngle), sin(SeatAngle)) * WHEEL_RADIUS;
		
		// Spoke from center to seat
		pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[idx++]);
		if(pObj)
		{
			pObj->m_X = (int)SeatPos.x;
			pObj->m_Y = (int)SeatPos.y;
			pObj->m_FromX = (int)m_Pos.x;
			pObj->m_FromY = (int)m_Pos.y;
			pObj->m_StartTick = StartTick;
		}
	}
	
	// Draw outer rim (connect seats)
	for(int i = 0; i < NUM_SEATS && idx < 40; i++)
	{
		float Angle1 = m_Angle + (float)i / NUM_SEATS * 2.0f * pi;
		float Angle2 = m_Angle + (float)((i + 1) % NUM_SEATS) / NUM_SEATS * 2.0f * pi;
		
		vec2 Pos1 = m_Pos + vec2(cos(Angle1), sin(Angle1)) * WHEEL_RADIUS;
		vec2 Pos2 = m_Pos + vec2(cos(Angle2), sin(Angle2)) * WHEEL_RADIUS;
		
		pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[idx++]);
		if(pObj)
		{
			pObj->m_X = (int)Pos2.x;
			pObj->m_Y = (int)Pos2.y;
			pObj->m_FromX = (int)Pos1.x;
			pObj->m_FromY = (int)Pos1.y;
			pObj->m_StartTick = StartTick;
		}
	}
	
	// Draw seat boxes
	for(int i = 0; i < NUM_SEATS && idx < 38; i++)
	{
		float SeatAngle = m_Angle + (float)i / NUM_SEATS * 2.0f * pi;
		vec2 SeatPos = m_Pos + vec2(cos(SeatAngle), sin(SeatAngle)) * WHEEL_RADIUS;
		
		// Small box for seat
		float BoxSize = 15.0f;
		vec2 TL = SeatPos + vec2(-BoxSize, -BoxSize);
		vec2 TR = SeatPos + vec2(BoxSize, -BoxSize);
		vec2 BL = SeatPos + vec2(-BoxSize, BoxSize);
		vec2 BR = SeatPos + vec2(BoxSize, BoxSize);
		
		// Top
		pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[idx++]);
		if(pObj)
		{
			pObj->m_X = (int)TR.x;
			pObj->m_Y = (int)TR.y;
			pObj->m_FromX = (int)TL.x;
			pObj->m_FromY = (int)TL.y;
			pObj->m_StartTick = StartTick;
		}
		
		// Bottom
		if(idx < 40)
		{
			pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[idx++]);
			if(pObj)
			{
				pObj->m_X = (int)BR.x;
				pObj->m_Y = (int)BR.y;
				pObj->m_FromX = (int)BL.x;
				pObj->m_FromY = (int)BL.y;
				pObj->m_StartTick = StartTick;
			}
		}
	}
}
