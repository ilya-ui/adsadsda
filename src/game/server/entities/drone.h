#ifndef GAME_SERVER_ENTITIES_DRONE_H
#define GAME_SERVER_ENTITIES_DRONE_H

#include <game/server/entity.h>

class CDrone : public CEntity
{
public:
	CDrone(CGameWorld *pGameWorld, vec2 Pos, int Owner);

	virtual void Reset() override;
	virtual void Tick() override;
	virtual void Snap(int SnappingClient) override;

private:
	int m_Owner;
	vec2 m_Velocity;
	int m_LastBombTick;
	int m_IDs[16];
};

#endif
