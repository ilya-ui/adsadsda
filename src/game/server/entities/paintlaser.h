/* Paint laser - persistent laser for drawing */
#ifndef GAME_SERVER_ENTITIES_PAINTLASER_H
#define GAME_SERVER_ENTITIES_PAINTLASER_H

#include <game/server/entity.h>

class CPaintLaser : public CEntity
{
public:
	CPaintLaser(CGameWorld *pGameWorld, vec2 From, vec2 To, int Owner);

	void Reset() override;
	void Tick() override;
	void Snap(int SnappingClient) override;

	int GetOwnerId() const override { return m_Owner; }

private:
	vec2 m_From;
	int m_Owner;
	int m_StartTick;
};

#endif
