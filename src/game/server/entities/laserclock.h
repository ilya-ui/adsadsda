#ifndef GAME_SERVER_ENTITIES_LASERCLOCK_H
#define GAME_SERVER_ENTITIES_LASERCLOCK_H

#include <game/server/entity.h>
#include <vector>

class CLaserClock : public CEntity
{
public:
	CLaserClock(CGameWorld *pGameWorld, vec2 Pos, int Owner);
	~CLaserClock();

	void Reset() override;
	void Tick() override;
	void Snap(int SnappingClient) override;

private:
	void UpdateClock();
	void DrawCircle(vec2 Center, float Radius, int Segments);

	int m_Owner;
	int m_LastSecond;

	// Store laser line segments (from, to)
	std::vector<std::pair<vec2, vec2>> m_Lines;

	// Wall clock sizes (big!)
	static constexpr float CLOCK_RADIUS = 150.0f;
	static constexpr float HOUR_HAND_LENGTH = 70.0f;
	static constexpr float MINUTE_HAND_LENGTH = 110.0f;
	static constexpr float SECOND_HAND_LENGTH = 130.0f;
	static constexpr float MARK_LENGTH = 20.0f;
	static constexpr int MAX_LINES = 64;
};

#endif // GAME_SERVER_ENTITIES_LASERCLOCK_H
