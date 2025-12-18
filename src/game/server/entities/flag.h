/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_FLAG_H
#define GAME_SERVER_ENTITIES_FLAG_H

#include <game/server/entity.h>

class CFlag : public CEntity
{
public:
	CFlag(CGameWorld *pGameWorld, int Team, vec2 StandPos);

	virtual void Reset() override;
	virtual void Tick() override;
	virtual void Snap(int SnappingClient) override;

	int GetTeam() const { return m_Team; }
	int GetCarrier() const { return m_Carrier; }
	void SetCarrier(int ClientId) { m_Carrier = ClientId; }
	void Grab(int ClientId);
	void Respawn();

private:
	int m_Team;
	int m_Carrier;
	vec2 m_StandPos;
};

#endif
