#include "vipblock.h"
#include <game/server/gamecontext.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <generated/protocol.h>

CVipBlock::CVipBlock(CGameWorld *pGameWorld, vec2 From, vec2 To, int Type) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER)
{
	m_From = From;
	m_To = To;
	m_Pos = (From + To) / 2.0f;
	m_Type = Type;
	for(int &Id : m_Ids)
		Id = Server()->SnapNewId();
	GameWorld()->InsertEntity(this);
}

void CVipBlock::Reset()
{
	for(int Id : m_Ids)
		Server()->SnapFreeId(Id);
	m_MarkedForDestroy = true;
}

void CVipBlock::Tick()
{
	for(CCharacter *pChr = (CCharacter *)GameWorld()->FindFirst(CGameWorld::ENTTYPE_CHARACTER); pChr; pChr = (CCharacter *)pChr->TypeNext())
	{
		if(pChr->GetPlayer() && pChr->GetPlayer()->m_IsVip)
			continue;

		vec2 IntersectPos;
		if(closest_point_on_line(m_From, m_To, pChr->GetPos(), IntersectPos) && distance(pChr->GetPos(), IntersectPos) < 28.0f)
		{
			pChr->SetVelocity(vec2(0, 0));
			vec2 Dir = normalize(pChr->GetPos() - IntersectPos);
			pChr->Move(Dir * 5.0f);
		}
	}
}

void CVipBlock::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	// Draw main line
	CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_Ids[0], sizeof(CNetObj_Laser)));
	if(pObj)
	{
		pObj->m_X = (int)m_To.x;
		pObj->m_Y = (int)m_To.y;
		pObj->m_FromX = (int)m_From.x;
		pObj->m_FromY = (int)m_From.y;
		pObj->m_StartTick = Server()->Tick() - 2;
	}

	float x = m_Pos.x;
	float y = m_Pos.y;

	// Letters (centered)
	auto DrawLetter = [&](int Type, vec2 Center, int StartId) -> int {
		float cx = Center.x;
		float cy = Center.y;
		int Count = 0;
		if(Type == 0) // V
		{
			vec2 aLetter[2][2] = {{vec2(cx - 8, cy - 8), vec2(cx, cy + 8)}, {vec2(cx + 8, cy - 8), vec2(cx, cy + 8)}};
			for(int i = 0; i < 2; i++) {
				CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_Ids[StartId + i], sizeof(CNetObj_Laser)));
				if(!pObj) continue;
				pObj->m_X = (int)aLetter[i][1].x; pObj->m_Y = (int)aLetter[i][1].y;
				pObj->m_FromX = (int)aLetter[i][0].x; pObj->m_FromY = (int)aLetter[i][0].y;
				pObj->m_StartTick = Server()->Tick() - 2;
			}
			Count = 2;
		}
		else if(Type == 1) // I
		{
			CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_Ids[StartId], sizeof(CNetObj_Laser)));
			if(pObj) {
				pObj->m_X = (int)cx; pObj->m_Y = (int)cy + 8;
				pObj->m_FromX = (int)cx; pObj->m_FromY = (int)cy - 8;
				pObj->m_StartTick = Server()->Tick() - 2;
			}
			Count = 1;
		}
		else if(Type == 2) // P
		{
			vec2 aLetter[4][2] = {{vec2(cx - 6, cy - 8), vec2(cx - 6, cy + 8)}, {vec2(cx - 6, cy - 8), vec2(cx + 6, cy - 8)}, {vec2(cx + 6, cy - 8), vec2(cx + 6, cy)}, {vec2(cx - 6, cy), vec2(cx + 6, cy)}};
			for(int i = 0; i < 4; i++) {
				CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_Ids[StartId + i], sizeof(CNetObj_Laser)));
				if(!pObj) continue;
				pObj->m_X = (int)aLetter[i][1].x; pObj->m_Y = (int)aLetter[i][1].y;
				pObj->m_FromX = (int)aLetter[i][0].x; pObj->m_FromY = (int)aLetter[i][0].y;
				pObj->m_StartTick = Server()->Tick() - 2;
			}
			Count = 4;
		}
		return Count;
	};

	if(m_Type >= 0 && m_Type <= 2)
	{
		DrawLetter(m_Type, m_Pos, 1);
	}
	else if(m_Type == 3) // VIP Combined
	{
		DrawLetter(0, m_Pos + vec2(-18, 0), 1); // V
		DrawLetter(1, m_Pos, 3);                // I
		DrawLetter(2, m_Pos + vec2(18, 0), 4);  // P
	}
}
