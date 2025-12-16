#ifndef GAME_SERVER_ENTITIES_LASERPIXEL_H
#define GAME_SERVER_ENTITIES_LASERPIXEL_H

#include <game/server/entity.h>

class CLaserPixel : public CEntity
{
public:
	CLaserPixel(CGameWorld *pGameWorld, vec2 Pos, int LaserType);

	void Reset() override;
	void Tick() override;
	void Snap(int SnappingClient) override;

	void SetColor(int LaserType);
	int GetColor() const { return m_LaserType; }

private:
	int m_LaserType;
	int m_StartTick;
};

#endif // GAME_SERVER_ENTITIES_LASERPIXEL_H
