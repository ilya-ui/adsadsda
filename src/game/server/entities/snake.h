/* Laser Snake Entity */
#ifndef GAME_SERVER_ENTITIES_SNAKE_H
#define GAME_SERVER_ENTITIES_SNAKE_H

#include <game/server/entity.h>
#include <vector>

class CLaserSnake : public CEntity
{
public:
	CLaserSnake(CGameWorld *pGameWorld, vec2 Pos, int Owner);
	~CLaserSnake();

	void Reset() override;
	void Tick() override;
	void Snap(int SnappingClient) override;

private:
	static const int MAX_SEGMENTS = 50;
	static const int INITIAL_SEGMENTS = 5;

	struct SnakeSegment
	{
		vec2 m_Pos;
		int m_SnapId;
	};

	std::vector<SnakeSegment> m_aSegments;
	vec2 m_Direction;
	int m_Owner;
	int m_LastMoveTick;
	int m_MoveDelay;
	bool m_Alive;

	void Move();
	void EatPlayer(int ClientId);
	void Grow();
	int FindNearestPlayer();
};

#endif
