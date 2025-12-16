# Requirements Document

## Introduction

Функция локальной тренировки (Local Practice) для клиента DDNet позволяет игрокам тренироваться на сложных участках карты без влияния на других игроков. Сервер видит игрока стоящим на месте, в то время как локально игрок может свободно двигаться. При попадании во фриз игрок автоматически телепортируется на последнюю безопасную позицию.

## Glossary

- **Practice_System**: Компонент клиента DDNet, отвечающий за локальную симуляцию движения
- **Practice_Mode**: Режим, в котором локальное движение отделено от серверного состояния
- **Safe_Position**: Последняя позиция игрока, где он не был заморожен
- **Fake_Character**: Локально симулируемый персонаж, видимый только игроку
- **Server_Character**: Реальная позиция персонажа на сервере (стоит на месте)

## Requirements

### Requirement 1

**User Story:** As a player, I want to toggle practice mode on and off, so that I can train on difficult parts without affecting others.

#### Acceptance Criteria

1. WHEN a player executes the practice_toggle command THEN the Practice_System SHALL enable practice mode and save the current position as Safe_Position
2. WHEN a player executes the practice_toggle command while practice mode is active THEN the Practice_System SHALL disable practice mode and sync position with server
3. WHILE practice mode is active THEN the Practice_System SHALL display a visual indicator showing "PRACTICE" on screen

### Requirement 2

**User Story:** As a player, I want to move freely in practice mode, so that I can explore and train on the map.

#### Acceptance Criteria

1. WHILE practice mode is active THEN the Practice_System SHALL simulate local character movement using full physics
2. WHILE practice mode is active THEN the Practice_System SHALL render the Fake_Character at the simulated position
3. WHILE practice mode is active THEN the Server_Character SHALL remain at the original position where practice mode was enabled

### Requirement 3

**User Story:** As a player, I want to be teleported back when I get frozen, so that I can retry difficult sections quickly.

#### Acceptance Criteria

1. WHEN the Fake_Character enters a freeze tile THEN the Practice_System SHALL teleport the Fake_Character to the last Safe_Position
2. WHILE the Fake_Character is not frozen THEN the Practice_System SHALL continuously update the Safe_Position
3. WHEN teleporting to Safe_Position THEN the Practice_System SHALL reset velocity to zero

### Requirement 4

**User Story:** As a player, I want to manually reset to safe position, so that I can retry from a specific point.

#### Acceptance Criteria

1. WHEN a player executes the practice_reset command THEN the Practice_System SHALL teleport the Fake_Character to the Safe_Position
2. WHEN resetting position THEN the Practice_System SHALL reset velocity to zero
3. WHEN resetting position THEN the Practice_System SHALL provide visual feedback

### Requirement 5

**User Story:** As a player, I want to set a custom checkpoint, so that I can practice from specific positions.

#### Acceptance Criteria

1. WHEN a player executes the practice_checkpoint command THEN the Practice_System SHALL save the current Fake_Character position as Safe_Position
2. WHEN checkpoint is saved THEN the Practice_System SHALL display confirmation message

### Requirement 6

**User Story:** As a player, I want to access practice settings in a dedicated menu tab, so that I can easily configure the feature.

#### Acceptance Criteria

1. WHEN the player opens settings THEN the system SHALL display a "5years" tab in the settings menu
2. WHEN the player selects the "5years" tab THEN the system SHALL display practice mode toggle and related options
3. WHILE in the "5years" settings tab THEN the system SHALL show keybind options for practice commands
