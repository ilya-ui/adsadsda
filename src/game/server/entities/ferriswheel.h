/* DDNet Ferris Wheel Entity */
#ifndef GAME_SERVER_ENTITIES_FERRISWHEEL_H
#define GAME_SERVER_ENTITIES_FERRISWHEEL_H

#include <game/server/entity.h>

class CFerrisWheel : public CEntity
{
public:
	CFerrisWheel(CGameWorld *pGameWorld, vec2 Pos);

	void Reset() override;
	void Tick() override;
	void Snap(int SnappingClient) override;

	// Called when player hammers near a seat
	bool TryEnterSeat(int ClientId, vec2 HammerPos);
	void ExitSeat(int SeatIndex);

private:
	static const int NUM_SEATS = 8;
	static constexpr float WHEEL_RADIUS = 150.0f;
	static constexpr float ROTATION_SPEED = 0.01f;
	
	float m_Angle;
	int m_aSeatedPlayers[NUM_SEATS]; // ClientID or -1
	
	int m_aIds[40]; // For laser rendering
};

#endif
