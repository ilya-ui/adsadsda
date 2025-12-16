# Requirements Document

## Introduction

BigHammer - это команда для DDNet сервера, которая даёт игроку огромный лазерный молоток. При ударе молотком создаётся визуальный эффект из множества лазерных лучей, формирующих форму гигантского молотка. Эффект наносит урон/отталкивание всем игрокам в большом радиусе.

## Glossary

- **BigHammer**: Специальная способность, создающая визуальный эффект огромного лазерного молотка
- **Laser**: Лазерный луч, используемый для визуализации молотка
- **DDNet Server**: Игровой сервер DDRace Network
- **Player**: Игрок на сервере
- **Knockback**: Отталкивание игрока при попадании

## Requirements

### Requirement 1

**User Story:** As a player, I want to use the bighammer command, so that I can get a giant laser hammer effect.

#### Acceptance Criteria

1. WHEN a player types "/bighammer" command THEN the DDNet Server SHALL enable BigHammer mode for that player
2. WHEN BigHammer mode is enabled THEN the DDNet Server SHALL display a confirmation message to the player
3. WHEN a player types "/unbighammer" command THEN the DDNet Server SHALL disable BigHammer mode for that player

### Requirement 2

**User Story:** As a player with BigHammer enabled, I want to see a giant laser hammer when I attack, so that I get a cool visual effect.

#### Acceptance Criteria

1. WHEN a player with BigHammer mode fires the hammer weapon THEN the DDNet Server SHALL create multiple laser entities forming a hammer shape
2. WHEN the laser hammer is created THEN the DDNet Server SHALL position the lasers in the direction the player is aiming
3. WHEN the laser hammer is created THEN the DDNet Server SHALL make the hammer size at least 5 times larger than normal hammer range
4. WHEN the laser hammer effect is displayed THEN the DDNet Server SHALL remove the lasers after 500 milliseconds

### Requirement 3

**User Story:** As a player with BigHammer, I want to hit other players in a large area, so that the hammer has gameplay impact.

#### Acceptance Criteria

1. WHEN the BigHammer hits players THEN the DDNet Server SHALL apply knockback to all players within the hammer area
2. WHEN the BigHammer hits players THEN the DDNet Server SHALL unfreeze frozen players within the hammer area
3. WHEN the BigHammer is used THEN the DDNet Server SHALL play the hammer sound effect
