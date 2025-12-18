#ifndef GAME_SERVER_ENTITIES_VIPBLOCK_H
#define GAME_SERVER_ENTITIES_VIPBLOCK_H

#include <game/server/entity.h>

class CVipBlock : public CEntity
{
	int m_Type; // 0=V, 1=I, 2=P, 3=VIP Combined
	vec2 m_From;
	vec2 m_To;
	int m_Ids[16]; // Increased for more letters

public:
	CVipBlock(CGameWorld *pGameWorld, vec2 From, vec2 To, int Type);
	virtual void Reset() override;
	virtual void Tick() override;
	virtual void Snap(int SnappingClient) override;
};

#endif
