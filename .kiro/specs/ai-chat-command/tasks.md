# Implementation Plan

- [x] 1. Add configuration variables for AI chat


  - Add `sv_ai_enabled`, `sv_ai_api_url`, `sv_ai_api_key`, `sv_ai_cooldown`, `sv_ai_max_response` to config_variables.h
  - _Requirements: 2.1, 2.2, 2.3, 2.4_





- [ ] 2. Create AI chat handler class
  - [x] 2.1 Create aichat.h header file


    - Define CAIChat class with Init, ProcessCommand, Tick methods
    - Define rate limiting data structures
    - Define pending request structure
    - _Requirements: 1.1, 3.1_
  - [ ] 2.2 Create aichat.cpp implementation
    - Implement rate limiting logic
    - Implement HTTP request construction with JSON payload
    - Implement response parsing and message splitting
    - _Requirements: 1.1, 1.3, 3.2, 3.3, 4.1, 4.2_



  - [ ]* 2.3 Write property test for message splitting
    - **Property 2: Response message splitting**
    - **Validates: Requirements 1.3**


  - [ ]* 2.4 Write property test for rate limiting
    - **Property 3: Rate limiting enforcement**


    - **Validates: Requirements 3.2, 3.3**

- [ ] 3. Register chat command and integrate with GameContext
  - [x] 3.1 Add CAIChat member to CGameContext

    - Include aichat.h in gamecontext.h

    - Add m_AIChat member variable
    - _Requirements: 1.1_
  - [x] 3.2 Register /ai command in RegisterChatCommands

    - Add Console()->Register for "ai" command
    - Implement ConAI callback function
    - _Requirements: 1.1, 1.2_
  - [ ] 3.3 Call AIChat.Tick() in GameContext tick
    - Process pending HTTP responses each tick
    - _Requirements: 1.1_

  - [ ]* 3.4 Write property test for empty input rejection
    - **Property 1: Empty input rejection**
    - **Validates: Requirements 1.2**


- [ ] 4. Implement response formatting
  - [ ] 4.1 Implement SplitMessage helper function
    - Split long responses into 128-char chunks


    - Handle UTF-8 character boundaries


    - _Requirements: 1.3_
  - [ ] 4.2 Implement response prefix formatting
    - Add "[AI]: " prefix to responses
    - _Requirements: 4.2_
  - [ ]* 4.3 Write property test for response prefix
    - **Property 4: Response prefix formatting**
    - **Validates: Requirements 4.2**

- [ ] 5. Implement error handling
  - Handle HTTP timeout and errors
  - Handle JSON parse errors
  - Handle disabled state
  - _Requirements: 1.4, 2.2_

- [ ] 6. Checkpoint - Make sure all tests are passing
  - Ensure all tests pass, ask the user if questions arise.

- [ ] 7. Add CMakeLists.txt entries
  - Add aichat.cpp and aichat.h to server source files
  - _Requirements: 1.1_

- [ ] 8. Final Checkpoint - Make sure all tests are passing
  - Ensure all tests pass, ask the user if questions arise.
