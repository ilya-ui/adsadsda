# Requirements Document

## Introduction

Данная функция добавляет чат-команду `/ai` на сервер DDNet, которая позволяет игрокам задавать вопросы нейросети прямо из игрового чата. Сервер отправляет запрос к бесплатному AI API и возвращает ответ игроку в чат.

## Glossary

- **AI_Chat_System**: Серверная система обработки команды `/ai` и взаимодействия с внешним AI API
- **AI_API**: Внешний бесплатный API для генерации ответов (например, OpenRouter с бесплатными моделями)
- **Chat_Command**: Команда, вводимая игроком в чат, начинающаяся с `/`
- **Player**: Игрок, подключённый к серверу DDNet

## Requirements

### Requirement 1

**User Story:** As a player, I want to ask questions to an AI through the chat, so that I can get helpful responses without leaving the game.

#### Acceptance Criteria

1. WHEN a player types `/ai` followed by a question THEN the AI_Chat_System SHALL send the question to the AI_API and display the response in the player's chat
2. WHEN a player types `/ai` without any question THEN the AI_Chat_System SHALL display a usage hint message to the player
3. WHEN the AI_API returns a response THEN the AI_Chat_System SHALL split long responses into multiple chat messages of 128 characters maximum each
4. WHEN the AI_API request fails or times out THEN the AI_Chat_System SHALL display an error message to the player

### Requirement 2

**User Story:** As a server administrator, I want to configure the AI feature, so that I can control API settings and enable/disable the feature.

#### Acceptance Criteria

1. WHEN the server starts THEN the AI_Chat_System SHALL read the API URL and API key from server configuration variables
2. WHERE the `sv_ai_enabled` configuration is set to 0 THEN the AI_Chat_System SHALL reject `/ai` commands with a disabled message
3. WHEN the `sv_ai_api_url` configuration is empty THEN the AI_Chat_System SHALL use a default free API endpoint
4. WHEN the `sv_ai_api_key` configuration is set THEN the AI_Chat_System SHALL include the API key in request headers

### Requirement 3

**User Story:** As a server administrator, I want rate limiting for AI requests, so that the server is protected from spam and API abuse.

#### Acceptance Criteria

1. WHEN a player sends an `/ai` command THEN the AI_Chat_System SHALL check if the player has exceeded the rate limit
2. WHEN a player exceeds the rate limit of 1 request per 10 seconds THEN the AI_Chat_System SHALL reject the request and inform the player of the cooldown time
3. WHEN the cooldown period expires THEN the AI_Chat_System SHALL allow the player to send a new `/ai` request

### Requirement 4

**User Story:** As a player, I want to see that my AI request is being processed, so that I know the system is working.

#### Acceptance Criteria

1. WHEN a player sends a valid `/ai` command THEN the AI_Chat_System SHALL immediately display a "Processing..." message to the player
2. WHEN the AI response is received THEN the AI_Chat_System SHALL display the response prefixed with "[AI]:"
