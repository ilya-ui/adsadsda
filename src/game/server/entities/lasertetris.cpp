#include "lasertetris.h"
#include <engine/server.h>
#include <game/server/gamecontext.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <generated/protocol.h>
#include <cstring>

// Tetris pieces (I, O, T, S, Z, J, L)
static const int PIECES[7][4][4][4] = {
	// I piece - 4 rotations
	{{{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
	 {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}},
	 {{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}},
	 {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}}},
	// O piece
	{{{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
	 {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
	 {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
	 {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}}},
	// T piece
	{{{0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
	 {{0,1,0,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}},
	 {{0,0,0,0},{1,1,1,0},{0,1,0,0},{0,0,0,0}},
	 {{0,1,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}}},
	// S piece
	{{{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
	 {{0,1,0,0},{0,1,1,0},{0,0,1,0},{0,0,0,0}},
	 {{0,0,0,0},{0,1,1,0},{1,1,0,0},{0,0,0,0}},
	 {{1,0,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}}},
	// Z piece
	{{{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
	 {{0,0,1,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}},
	 {{0,0,0,0},{1,1,0,0},{0,1,1,0},{0,0,0,0}},
	 {{0,1,0,0},{1,1,0,0},{1,0,0,0},{0,0,0,0}}},
	// J piece
	{{{1,0,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
	 {{0,1,1,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}},
	 {{0,0,0,0},{1,1,1,0},{0,0,1,0},{0,0,0,0}},
	 {{0,1,0,0},{0,1,0,0},{1,1,0,0},{0,0,0,0}}},
	// L piece
	{{{0,0,1,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
	 {{0,1,0,0},{0,1,0,0},{0,1,1,0},{0,0,0,0}},
	 {{0,0,0,0},{1,1,1,0},{1,0,0,0},{0,0,0,0}},
	 {{1,1,0,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}}}
};

CLaserTetris::CLaserTetris(CGameWorld *pGameWorld, vec2 Pos, int Owner) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER)
{
	m_Pos = Pos;
	m_Owner = Owner;
	m_PlayerId = -1;
	m_GameOver = false;
	m_Score = 0;
	m_NumLasers = 0;
	m_DropTick = 0;

	// Clear board
	memset(m_Board, 0, sizeof(m_Board));
	memset(m_Piece, 0, sizeof(m_Piece));
	
	// Initialize snap IDs
	for(int i = 0; i < MAX_LASERS; i++)
		m_aSnapIds[i] = Server()->SnapNewId();
	
	GameWorld()->InsertEntity(this);
}

void CLaserTetris::Reset()
{
	for(int i = 0; i < MAX_LASERS; i++)
		Server()->SnapFreeId(m_aSnapIds[i]);
	m_MarkedForDestroy = true;
}

void CLaserTetris::StartGame()
{
	memset(m_Board, 0, sizeof(m_Board));
	m_Score = 0;
	m_GameOver = false;
	m_DropTick = Server()->Tick();
	SpawnNewPiece();
}

void CLaserTetris::SpawnNewPiece()
{
	m_PieceType = rand() % 7;
	m_PieceX = BOARD_WIDTH / 2 - 2;
	m_PieceY = 0;
	m_PieceRotation = 0;
	GetPieceShape(m_PieceType, m_PieceRotation, m_Piece);
	
	// Check game over
	if(!CanMove(0, 0))
	{
		m_GameOver = true;
		m_PlayerId = -1;
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "*** TETRIS GAME OVER! Score: %d ***", m_Score);
		GameServer()->SendChat(-1, TEAM_ALL, aBuf);
	}
}

void CLaserTetris::GetPieceShape(int type, int rotation, int out[4][4])
{
	for(int y = 0; y < 4; y++)
		for(int x = 0; x < 4; x++)
			out[y][x] = PIECES[type][rotation][y][x];
}

bool CLaserTetris::CanMove(int dx, int dy)
{
	for(int py = 0; py < 4; py++)
	{
		for(int px = 0; px < 4; px++)
		{
			if(m_Piece[py][px])
			{
				int nx = m_PieceX + px + dx;
				int ny = m_PieceY + py + dy;
				if(nx < 0 || nx >= BOARD_WIDTH || ny >= BOARD_HEIGHT)
					return false;
				if(ny >= 0 && m_Board[ny][nx])
					return false;
			}
		}
	}
	return true;
}

void CLaserTetris::MovePiece(int dx)
{
	if(CanMove(dx, 0))
		m_PieceX += dx;
}

void CLaserTetris::RotatePiece()
{
	int oldRotation = m_PieceRotation;
	int newRotation = (m_PieceRotation + 1) % 4;
	int oldPiece[4][4];
	memcpy(oldPiece, m_Piece, sizeof(m_Piece));
	
	GetPieceShape(m_PieceType, newRotation, m_Piece);
	m_PieceRotation = newRotation;
	
	if(!CanMove(0, 0))
	{
		// Revert rotation
		m_PieceRotation = oldRotation;
		memcpy(m_Piece, oldPiece, sizeof(m_Piece));
	}
}

void CLaserTetris::DropPiece()
{
	while(CanMove(0, 1))
		m_PieceY++;
	LockPiece();
}

void CLaserTetris::LockPiece()
{
	for(int py = 0; py < 4; py++)
	{
		for(int px = 0; px < 4; px++)
		{
			if(m_Piece[py][px])
			{
				int bx = m_PieceX + px;
				int by = m_PieceY + py;
				if(by >= 0 && by < BOARD_HEIGHT && bx >= 0 && bx < BOARD_WIDTH)
					m_Board[by][bx] = 1;
			}
		}
	}
	ClearLines();
	SpawnNewPiece();
}

void CLaserTetris::ClearLines()
{
	int linesCleared = 0;
	for(int y = BOARD_HEIGHT - 1; y >= 0; y--)
	{
		bool full = true;
		for(int x = 0; x < BOARD_WIDTH; x++)
		{
			if(!m_Board[y][x])
			{
				full = false;
				break;
			}
		}
		if(full)
		{
			linesCleared++;
			// Move all lines above down
			for(int yy = y; yy > 0; yy--)
				for(int x = 0; x < BOARD_WIDTH; x++)
					m_Board[yy][x] = m_Board[yy-1][x];
			for(int x = 0; x < BOARD_WIDTH; x++)
				m_Board[0][x] = 0;
			y++; // Check same line again
		}
	}
	if(linesCleared > 0)
	{
		m_Score += linesCleared * 100;
		if(m_PlayerId >= 0)
		{
			char aBuf[64];
			str_format(aBuf, sizeof(aBuf), "Lines: %d | Score: %d", linesCleared, m_Score);
			GameServer()->SendBroadcast(aBuf, m_PlayerId, false);
		}
	}
}

void CLaserTetris::OnHammerHit(int ClientId)
{
	if(m_PlayerId == ClientId)
	{
		// Player exits - unfreeze them
		CCharacter *pChr = GameServer()->GetPlayerChar(ClientId);
		if(pChr)
			pChr->UnFreeze();
		m_PlayerId = -1;
		m_GameOver = false;
		memset(m_Board, 0, sizeof(m_Board));
		GameServer()->SendChatTarget(ClientId, "Exited Tetris!");
	}
	else if(m_PlayerId < 0)
	{
		// New player enters - freeze them so they can't move
		CCharacter *pChr = GameServer()->GetPlayerChar(ClientId);
		if(pChr)
			pChr->Freeze();
		m_PlayerId = ClientId;
		StartGame();
		GameServer()->SendChatTarget(ClientId, "Tetris! A/D=move, W/Space=rotate, S=soft drop, Hook=hard drop. Hammer to exit.");
	}
}


void CLaserTetris::Tick()
{
	// Handle game over - unfreeze player
	if(m_GameOver && m_PlayerId >= 0)
	{
		CCharacter *pChr = GameServer()->GetPlayerChar(m_PlayerId);
		if(pChr)
			pChr->UnFreeze();
		m_PlayerId = -1;
		return;
	}
	
	if(m_PlayerId < 0)
		return;
	
	CPlayer *pPlayer = GameServer()->m_apPlayers[m_PlayerId];
	if(!pPlayer)
	{
		m_PlayerId = -1;
		return;
	}
	
	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
	{
		m_PlayerId = -1;
		return;
	}
	
	// Keep player frozen while playing
	pChr->Freeze();
	
	// Get player input for controls
	CNetObj_PlayerInput Input = GameServer()->GetLastPlayerInput(m_PlayerId);
	static int LastDir = 0;
	static int LastJump = 0;
	static int LastHook = 0;
	static int SoftDropTick = 0;
	
	// Left/Right movement (use Direction)
	if(Input.m_Direction != LastDir)
	{
		if(Input.m_Direction < 0)
			MovePiece(-1);
		else if(Input.m_Direction > 0)
			MovePiece(1);
		LastDir = Input.m_Direction;
	}
	
	// Rotate (Jump/Space/W)
	if(Input.m_Jump && !LastJump)
		RotatePiece();
	LastJump = Input.m_Jump;
	
	// Hard drop (Hook/RMB) - instant drop to bottom
	if(Input.m_Hook && !LastHook)
		DropPiece();
	LastHook = Input.m_Hook;
	
	// Soft drop (S key / aim down) - faster drop while holding
	bool SoftDrop = Input.m_TargetY > 100; // Aiming down = soft drop
	
	// Normal auto drop speed or fast soft drop
	int DropSpeed = SoftDrop ? Server()->TickSpeed() / 15 : Server()->TickSpeed() / 2;
	
	if(Server()->Tick() - m_DropTick >= DropSpeed)
	{
		m_DropTick = Server()->Tick();
		if(CanMove(0, 1))
			m_PieceY++;
		else
			LockPiece();
	}
}

// Helper to draw a laser line
void CLaserTetris::DrawLaser(int &NumLasers, int x1, int y1, int x2, int y2)
{
	if(NumLasers >= MAX_LASERS)
		return;
	CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_aSnapIds[NumLasers++], sizeof(CNetObj_Laser)));
	if(pObj)
	{
		pObj->m_X = x2;
		pObj->m_Y = y2;
		pObj->m_FromX = x1;
		pObj->m_FromY = y1;
		pObj->m_StartTick = Server()->Tick() - 2;
	}
}

void CLaserTetris::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;
	
	m_NumLasers = 0;
	
	// Draw border
	float left = m_Pos.x - BOARD_WIDTH * CELL_SIZE / 2;
	float top = m_Pos.y - BOARD_HEIGHT * CELL_SIZE / 2;
	float right = left + BOARD_WIDTH * CELL_SIZE;
	float bottom = top + BOARD_HEIGHT * CELL_SIZE;
	
	// Left border
	if(m_NumLasers < MAX_LASERS)
	{
		CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_aSnapIds[m_NumLasers++], sizeof(CNetObj_Laser)));
		if(pObj)
		{
			pObj->m_X = (int)left;
			pObj->m_Y = (int)bottom;
			pObj->m_FromX = (int)left;
			pObj->m_FromY = (int)top;
			pObj->m_StartTick = Server()->Tick() - 2;
		}
	}
	// Right border
	if(m_NumLasers < MAX_LASERS)
	{
		CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_aSnapIds[m_NumLasers++], sizeof(CNetObj_Laser)));
		if(pObj)
		{
			pObj->m_X = (int)right;
			pObj->m_Y = (int)bottom;
			pObj->m_FromX = (int)right;
			pObj->m_FromY = (int)top;
			pObj->m_StartTick = Server()->Tick() - 2;
		}
	}
	// Bottom border
	if(m_NumLasers < MAX_LASERS)
	{
		CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_aSnapIds[m_NumLasers++], sizeof(CNetObj_Laser)));
		if(pObj)
		{
			pObj->m_X = (int)right;
			pObj->m_Y = (int)bottom;
			pObj->m_FromX = (int)left;
			pObj->m_FromY = (int)bottom;
			pObj->m_StartTick = Server()->Tick() - 2;
		}
	}
	// Top border
	if(m_NumLasers < MAX_LASERS)
	{
		CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_aSnapIds[m_NumLasers++], sizeof(CNetObj_Laser)));
		if(pObj)
		{
			pObj->m_X = (int)right;
			pObj->m_Y = (int)top;
			pObj->m_FromX = (int)left;
			pObj->m_FromY = (int)top;
			pObj->m_StartTick = Server()->Tick() - 2;
		}
	}
	
	// Draw board cells
	for(int y = 0; y < BOARD_HEIGHT && m_NumLasers < MAX_LASERS - 4; y++)
	{
		for(int x = 0; x < BOARD_WIDTH && m_NumLasers < MAX_LASERS - 4; x++)
		{
			if(m_Board[y][x])
			{
				float cx = left + x * CELL_SIZE;
				float cy = top + y * CELL_SIZE;
				// Draw cell as small square
				CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_aSnapIds[m_NumLasers++], sizeof(CNetObj_Laser)));
				if(pObj)
				{
					pObj->m_X = (int)(cx + CELL_SIZE);
					pObj->m_Y = (int)(cy + CELL_SIZE/2);
					pObj->m_FromX = (int)cx;
					pObj->m_FromY = (int)(cy + CELL_SIZE/2);
					pObj->m_StartTick = Server()->Tick() - 2;
				}
			}
		}
	}
	
	// Draw current piece
	if(m_PlayerId >= 0 && !m_GameOver)
	{
		for(int py = 0; py < 4 && m_NumLasers < MAX_LASERS - 4; py++)
		{
			for(int px = 0; px < 4 && m_NumLasers < MAX_LASERS - 4; px++)
			{
				if(m_Piece[py][px])
				{
					int bx = m_PieceX + px;
					int by = m_PieceY + py;
					if(by >= 0)
					{
						float cx = left + bx * CELL_SIZE;
						float cy = top + by * CELL_SIZE;
						CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_aSnapIds[m_NumLasers++], sizeof(CNetObj_Laser)));
						if(pObj)
						{
							pObj->m_X = (int)(cx + CELL_SIZE);
							pObj->m_Y = (int)(cy + CELL_SIZE/2);
							pObj->m_FromX = (int)cx;
							pObj->m_FromY = (int)(cy + CELL_SIZE/2);
							pObj->m_StartTick = Server()->Tick() - 2;
						}
					}
				}
			}
		}
	}
	
	// Draw "TETRIS" title when no one is playing
	if(m_PlayerId < 0)
	{
		float cx = m_Pos.x;
		float cy = m_Pos.y - 20; // Center of board
		float s = 12.0f; // Letter scale
		float spacing = 28.0f; // Space between letters
		
		// T
		float tx = cx - spacing * 2.5f;
		DrawLaser(m_NumLasers, (int)(tx - s), (int)(cy - s*2), (int)(tx + s), (int)(cy - s*2)); // top
		DrawLaser(m_NumLasers, (int)tx, (int)(cy - s*2), (int)tx, (int)(cy + s*2)); // vertical
		
		// E
		float ex = cx - spacing * 1.5f;
		DrawLaser(m_NumLasers, (int)(ex - s), (int)(cy - s*2), (int)(ex - s), (int)(cy + s*2)); // left
		DrawLaser(m_NumLasers, (int)(ex - s), (int)(cy - s*2), (int)(ex + s), (int)(cy - s*2)); // top
		DrawLaser(m_NumLasers, (int)(ex - s), (int)cy, (int)(ex + s*0.5f), (int)cy); // middle
		DrawLaser(m_NumLasers, (int)(ex - s), (int)(cy + s*2), (int)(ex + s), (int)(cy + s*2)); // bottom
		
		// T
		float t2x = cx - spacing * 0.5f;
		DrawLaser(m_NumLasers, (int)(t2x - s), (int)(cy - s*2), (int)(t2x + s), (int)(cy - s*2)); // top
		DrawLaser(m_NumLasers, (int)t2x, (int)(cy - s*2), (int)t2x, (int)(cy + s*2)); // vertical
		
		// R
		float rx = cx + spacing * 0.5f;
		DrawLaser(m_NumLasers, (int)(rx - s), (int)(cy - s*2), (int)(rx - s), (int)(cy + s*2)); // left
		DrawLaser(m_NumLasers, (int)(rx - s), (int)(cy - s*2), (int)(rx + s), (int)(cy - s*2)); // top
		DrawLaser(m_NumLasers, (int)(rx + s), (int)(cy - s*2), (int)(rx + s), (int)cy); // right top
		DrawLaser(m_NumLasers, (int)(rx - s), (int)cy, (int)(rx + s), (int)cy); // middle
		DrawLaser(m_NumLasers, (int)(rx - s), (int)cy, (int)(rx + s), (int)(cy + s*2)); // diagonal
		
		// I
		float ix = cx + spacing * 1.5f;
		DrawLaser(m_NumLasers, (int)(ix - s), (int)(cy - s*2), (int)(ix + s), (int)(cy - s*2)); // top
		DrawLaser(m_NumLasers, (int)ix, (int)(cy - s*2), (int)ix, (int)(cy + s*2)); // vertical
		DrawLaser(m_NumLasers, (int)(ix - s), (int)(cy + s*2), (int)(ix + s), (int)(cy + s*2)); // bottom
		
		// S
		float sx = cx + spacing * 2.5f;
		DrawLaser(m_NumLasers, (int)(sx - s), (int)(cy - s*2), (int)(sx + s), (int)(cy - s*2)); // top
		DrawLaser(m_NumLasers, (int)(sx - s), (int)(cy - s*2), (int)(sx - s), (int)cy); // left top
		DrawLaser(m_NumLasers, (int)(sx - s), (int)cy, (int)(sx + s), (int)cy); // middle
		DrawLaser(m_NumLasers, (int)(sx + s), (int)cy, (int)(sx + s), (int)(cy + s*2)); // right bottom
		DrawLaser(m_NumLasers, (int)(sx - s), (int)(cy + s*2), (int)(sx + s), (int)(cy + s*2)); // bottom
	}
}
