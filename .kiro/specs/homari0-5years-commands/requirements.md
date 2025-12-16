# Requirements Document

## Introduction

Данная спецификация описывает две серверные команды для DDNet сервера:
- `homari0` - меняет ник всех игроков на "homari0"
- `5years` - меняет ник всех игроков на "5years" и скин на "hedwige"

Эти команды предназначены для развлекательных целей и администрирования сервера.

## Glossary

- **DDNet Server**: Серверная часть игры DDNet, обрабатывающая игровую логику
- **Console Command**: Команда, выполняемая через серверную консоль
- **Player**: Игрок, подключенный к серверу
- **Nickname (Nick)**: Отображаемое имя игрока
- **Skin**: Визуальное оформление персонажа игрока (tee)
- **CTeeInfo**: Структура данных, хранящая информацию о скине игрока

## Requirements

### Requirement 1

**User Story:** As a server administrator, I want to change all players' nicknames to "homari0", so that I can create a fun event where everyone has the same name.

#### Acceptance Criteria

1. WHEN an administrator executes the "homari0" command THEN the DDNet Server SHALL change the nickname of all connected players to "homari0"
2. WHEN the "homari0" command is executed THEN the DDNet Server SHALL broadcast a chat message notifying all players about the nickname change
3. WHEN a player's nickname is changed by the "homari0" command THEN the DDNet Server SHALL update the player info for all connected clients immediately

### Requirement 2

**User Story:** As a server administrator, I want to change all players' nicknames to "5years" and their skin to "hedwige", so that I can create a themed event.

#### Acceptance Criteria

1. WHEN an administrator executes the "5years" command THEN the DDNet Server SHALL change the nickname of all connected players to "5years"
2. WHEN an administrator executes the "5years" command THEN the DDNet Server SHALL change the skin of all connected players to "hedwige"
3. WHEN the "5years" command is executed THEN the DDNet Server SHALL broadcast a chat message notifying all players about the changes
4. WHEN a player's nickname and skin are changed by the "5years" command THEN the DDNet Server SHALL update the player info for all connected clients immediately
