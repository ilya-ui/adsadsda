# Requirements Document

## Introduction

Интеграция игры Doomgeneric в DDNet сервер. Игрок вводит команду `doom` и на карте появляется экран с игрой DOOM, отрисованный с помощью разноцветных лазеров. Команда `cleardoom` убирает экран. Все игроки на сервере видят геймплей. Управляющий игрок использует свои клавиши движения для управления DOOM.

## Glossary

- **Doomgeneric**: Портативная версия исходного кода DOOM, предназначенная для встраивания в различные платформы
- **DDNet_Server**: Серверная часть DDNet, управляющая игровой логикой и сущностями
- **Laser_Entity**: Игровая сущность DDNet, представляющая лазерный луч с настраиваемым цветом
- **Doom_Screen**: Интерактивная сущность-экран на карте, отображающая DOOM через лазеры
- **Frame_Buffer**: Буфер пикселей, содержащий текущий кадр DOOM
- **Active_Player**: Игрок, который вызвал команду doom и управляет игрой
- **DOOM_Controller**: Модуль, перехватывающий ввод Active_Player и передающий его в DOOM

## Requirements

### Requirement 1

**User Story:** As a player, I want to spawn a DOOM screen with a command, so that I can play DOOM inside DDNet.

#### Acceptance Criteria

1. WHEN a player executes the doom command THEN the DDNet_Server SHALL create a Doom_Screen entity near the player position
2. WHEN a player executes the doom command THEN the DDNet_Server SHALL set that player as Active_Player
3. WHEN a player becomes Active_Player THEN the DDNet_Server SHALL freeze the player character in place near the screen
4. WHEN a player executes the cleardoom command THEN the DDNet_Server SHALL remove the Doom_Screen and unfreeze the player

### Requirement 2

**User Story:** As a player controlling DOOM, I want to use my movement keys to play, so that the controls feel natural.

#### Acceptance Criteria

1. WHEN Active_Player presses left or right movement keys THEN the DOOM_Controller SHALL send corresponding turn commands to DOOM
2. WHEN Active_Player presses up movement key THEN the DOOM_Controller SHALL send forward movement command to DOOM
3. WHEN Active_Player presses down movement key THEN the DOOM_Controller SHALL send backward movement command to DOOM
4. WHEN Active_Player presses fire button THEN the DOOM_Controller SHALL send fire command to DOOM
5. WHEN Active_Player presses jump button THEN the DOOM_Controller SHALL send use/open door command to DOOM

### Requirement 3

**User Story:** As a spectator, I want to see DOOM gameplay on the laser screen, so that I can watch other players play.

#### Acceptance Criteria

1. WHEN the DOOM frame buffer updates THEN the Doom_Screen SHALL convert pixel colors to corresponding laser colors
2. WHEN rendering the display THEN the Doom_Screen SHALL create a grid of Laser_Entity objects representing the DOOM screen
3. WHEN a pixel color changes between frames THEN the Doom_Screen SHALL update only the affected Laser_Entity objects
4. WHEN no Active_Player is controlling THEN the Doom_Screen SHALL display a title screen or demo mode

### Requirement 4

**User Story:** As a developer, I want the DOOM integration to be modular, so that it can be enabled or disabled at compile time.

#### Acceptance Criteria

1. WHEN compiling the server THEN the build system SHALL support a DOOM_INTEGRATION CMake option to enable or disable the feature
2. WHEN DOOM integration is disabled THEN the DDNet_Server SHALL exclude all Doomgeneric code from the build
3. WHEN DOOM integration is enabled THEN the DDNet_Server SHALL compile and link the Doomgeneric source files
4. WHEN DOOM commands are used with integration disabled THEN the DDNet_Server SHALL display an informative error message

### Requirement 5

**User Story:** As a developer, I want to serialize and deserialize DOOM frame data, so that the display state can be processed efficiently.

#### Acceptance Criteria

1. WHEN converting frame buffer to laser data THEN the Doom_Screen SHALL map DOOM palette colors to a reduced DDNet laser color palette
2. WHEN encoding frame data THEN the Doom_Screen SHALL produce a compact representation for delta updates
3. WHEN decoding frame data THEN the Doom_Screen SHALL reconstruct the laser colors from the encoded format
4. WHEN round-tripping frame data through encode and decode THEN the Doom_Screen SHALL produce equivalent color values

### Requirement 6

**User Story:** As a player, I want smooth gameplay experience, so that DOOM is playable and enjoyable.

#### Acceptance Criteria

1. WHEN DOOM is running THEN the Doom_Screen SHALL update at a minimum rate of 15 frames per second
2. WHEN Active_Player provides input THEN the DOOM_Controller SHALL process input with latency under 100 milliseconds
3. WHEN multiple Doom_Screen entities exist THEN the DDNet_Server SHALL run independent DOOM instances for each screen
4. WHEN server tick rate is lower than DOOM frame rate THEN the Doom_Screen SHALL skip intermediate frames to maintain synchronization
