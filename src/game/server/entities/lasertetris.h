#ifndef GAME_SERVER_ENTITIES_LASERTETRIS_H
#define GAME_SERVER_ENTITIES_LASERTETRIS_H
#include <game/server/entity.h>

class CLaserTetris : public CEntity
{
public:
	CLaserTetris(CGameWorld *pGameWorld, vec2 Pos, int Owner);
	void Reset() override;
	void Tick() override;
	void Snap(int SnappingClient) override;
	
	void OnHammerHit(int ClientId);
	bool IsPlaying() const { return m_PlayerId >= 0; }
	int GetPlayer() const { return m_PlayerId; }

private:
	static const int BOARD_WIDTH = 10;
	static const int BOARD_HEIGHT = 20;
	static const int CELL_SIZE = 16;
	static const int MAX_LASERS = 250;
	
	int m_Owner;
	int m_PlayerId;
	int m_Board[BOARD_HEIGHT][BOARD_WIDTH];
	
	// Current piece
	int m_PieceType;
	int m_PieceX;
	int m_PieceY;
	int m_PieceRotation;
	int m_Piece[4][4];
	
	int m_DropTick;
	int m_Score;
	bool m_GameOver;
	
	// Snap IDs for lasers
	int m_aSnapIds[MAX_LASERS];
	int m_NumLasers;
	
	void StartGame();
	void SpawnNewPiece();
	void RotatePiece();
	void MovePiece(int dx);
	void DropPiece();
	bool CanMove(int dx, int dy);
	void LockPiece();
	void ClearLines();
	void GetPieceShape(int type, int rotation, int out[4][4]);
	void DrawLaser(int &NumLasers, int x1, int y1, int x2, int y2);
};
#endif
