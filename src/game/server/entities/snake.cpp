/* Laser Snake Entity */
#include "snake.h"
#include "character.h"

#include <generated/protocol.h>
#include <game/server/gamecontext.h>
#include <game/server/player.h>

CLaserSnake::CLaserSnake(CGameWorld *pGameWorld, vec2 Pos, int Owner) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER, Pos)
{
	m_Owner = Owner;
	m_Direction = vec2(1, 0);
	m_LastMoveTick = Server()->Tick();
	m_MoveDelay = Server()->TickSpeed() / 5; // Move 5 times per second
	m_Alive = true;

	// Initialize snake with starting segments
	for(int i = 0; i < INITIAL_SEGMENTS; i++)
	{
		SnakeSegment seg;
		seg.m_Pos = Pos - m_Direction * (float)(i * 20);
		seg.m_SnapId = Server()->SnapNewId();
		m_aSegments.push_back(seg);
	}

	GameWorld()->InsertEntity(this);
}

CLaserSnake::~CLaserSnake()
{
	for(auto &seg : m_aSegments)
	{
		Server()->SnapFreeId(seg.m_SnapId);
	}
}

void CLaserSnake::Reset()
{
	m_Alive = false;
	GameWorld()->RemoveEntity(this);
}

int CLaserSnake::FindNearestPlayer()
{
	float MinDist = 99999.0f;
	int NearestId = -1;
	vec2 HeadPos = m_aSegments.empty() ? m_Pos : m_aSegments[0].m_Pos;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;
		CCharacter *pChr = pPlayer->GetCharacter();
		if(!pChr || !pChr->IsAlive())
			continue;

		float Dist = distance(HeadPos, pChr->GetPos());
		if(Dist < MinDist)
		{
			MinDist = Dist;
			NearestId = i;
		}
	}
	return NearestId;
}

void CLaserSnake::Move()
{
	if(m_aSegments.empty())
		return;

	// Find nearest player and move towards them
	int NearestId = FindNearestPlayer();
	if(NearestId >= 0)
	{
		CCharacter *pTarget = GameServer()->GetPlayerChar(NearestId);
		if(pTarget)
		{
			vec2 ToTarget = pTarget->GetPos() - m_aSegments[0].m_Pos;
			if(length(ToTarget) > 0)
				m_Direction = normalize(ToTarget);
		}
	}

	// Move head
	vec2 NewHeadPos = m_aSegments[0].m_Pos + m_Direction * 20.0f;

	// Move all segments (each follows the one in front)
	for(int i = (int)m_aSegments.size() - 1; i > 0; i--)
	{
		m_aSegments[i].m_Pos = m_aSegments[i - 1].m_Pos;
	}
	m_aSegments[0].m_Pos = NewHeadPos;
	m_Pos = NewHeadPos;
}

void CLaserSnake::EatPlayer(int ClientId)
{
	CCharacter *pChr = GameServer()->GetPlayerChar(ClientId);
	if(!pChr)
		return;

	// Kill the player
	pChr->Die(m_Owner, WEAPON_NINJA);

	// Grow the snake
	Grow();
	Grow();
	Grow();

	GameServer()->CreateSound(m_Pos, SOUND_PICKUP_NINJA, CClientMask().set());

	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "Snake ate %s! Length: %d", GameServer()->Server()->ClientName(ClientId), (int)m_aSegments.size());
	GameServer()->SendChat(-1, TEAM_ALL, aBuf);
}

void CLaserSnake::Grow()
{
	if((int)m_aSegments.size() >= MAX_SEGMENTS)
		return;

	SnakeSegment seg;
	seg.m_Pos = m_aSegments.back().m_Pos;
	seg.m_SnapId = Server()->SnapNewId();
	m_aSegments.push_back(seg);
}

void CLaserSnake::Tick()
{
	if(!m_Alive)
		return;

	// Move snake
	if(Server()->Tick() - m_LastMoveTick >= m_MoveDelay)
	{
		Move();
		m_LastMoveTick = Server()->Tick();
	}

	// Check collision with players
	if(!m_aSegments.empty())
	{
		vec2 HeadPos = m_aSegments[0].m_Pos;
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			CCharacter *pChr = GameServer()->GetPlayerChar(i);
			if(!pChr || !pChr->IsAlive())
				continue;

			if(distance(HeadPos, pChr->GetPos()) < 40.0f)
			{
				EatPlayer(i);
			}
		}
	}
}

void CLaserSnake::Snap(int SnappingClient)
{
	if(!m_Alive || m_aSegments.size() < 2)
		return;

	// Draw lasers between segments
	for(size_t i = 0; i < m_aSegments.size() - 1; i++)
	{
		if(NetworkClipped(SnappingClient, m_aSegments[i].m_Pos))
			continue;

		CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_aSegments[i].m_SnapId, sizeof(CNetObj_Laser)));
		if(!pObj)
			continue;

		pObj->m_X = (int)m_aSegments[i].m_Pos.x;
		pObj->m_Y = (int)m_aSegments[i].m_Pos.y;
		pObj->m_FromX = (int)m_aSegments[i + 1].m_Pos.x;
		pObj->m_FromY = (int)m_aSegments[i + 1].m_Pos.y;
		pObj->m_StartTick = Server()->Tick() - 2;
	}
}
