#ifndef GAME_SERVER_ENTITIES_BOTFIGHTER_H
#define GAME_SERVER_ENTITIES_BOTFIGHTER_H

#include <game/server/entity.h>

class CBotFighter : public CEntity
{
public:
	CBotFighter(CGameWorld *pGameWorld, vec2 Pos);

	virtual void Reset() override;
	virtual void Tick() override;
	virtual void Snap(int SnappingClient) override;
    virtual void Destroy() override;

private:
	vec2 m_Velocity;
    int m_JumpTick;
    int m_AttackTick;
    int m_MoveTick;
    int m_Dir;
    
	int m_Ids[16]; // IDs for lasers
    
    class CCharacter *FindNearestTarget();
    void FireHammer();
};

#endif
