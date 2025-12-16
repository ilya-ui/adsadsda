/* DDNet Vodka Bottle Entity */
#ifndef GAME_SERVER_ENTITIES_VODKABOTTLE_H
#define GAME_SERVER_ENTITIES_VODKABOTTLE_H

#include <game/server/entity.h>

class CVodkaBottle : public CEntity
{
public:
	CVodkaBottle(CGameWorld *pGameWorld, vec2 Pos);

	void Reset() override;
	void Tick() override;
	void Snap(int SnappingClient) override;

private:
	int m_SnapId;
};

#endif
