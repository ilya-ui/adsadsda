# Implementation Plan

- [x] 1. Настройка сборки и интеграция Doomgeneric

  - [x] 1.1 Добавить Doomgeneric исходники в проект


    - Создать директорию `src/game/server/doom/`
    - Скопировать core файлы doomgeneric из doomgeneric-master/doomgeneric/


    - _Requirements: 4.1, 4.3_
  - [x] 1.2 Создать CMake конфигурацию для DOOM интеграции


    - Добавить опцию `DOOM_INTEGRATION` в CMakeLists.txt
    - Настроить условную компиляцию
    - _Requirements: 4.1, 4.2_
  - [x] 1.3 Создать кастомный бэкенд doomgeneric_ddnet.c


    - Реализовать DG_Init, DG_DrawFrame, DG_GetTicksMs
    - Реализовать DG_GetKey для получения ввода
    - Реализовать DG_GetFrameBuffer для доступа к кадру
    - _Requirements: 4.3_

- [x] 2. Реализация цветового маппинга

  - [x] 2.1 Создать модуль маппинга цветов doom_colors.h/cpp


    - Реализовать функцию MapDoomColorToLaser()
    - Определить пороговые значения для каждого типа лазера
    - _Requirements: 3.1, 5.1_
  - [x] 2.2 Написать property-based тест для маппинга цветов

    - **Property 4: Color mapping completeness**
    - **Validates: Requirements 3.1, 5.1**



- [ ] 3. Реализация CLaserPixel
  - [ ] 3.1 Создать класс CLaserPixel в entities/laserpixel.h/cpp
    - Определить конструктор с позицией и типом лазера
    - Реализовать метод SetColor() для смены типа
    - Реализовать Snap() для отправки клиентам
    - _Requirements: 3.2_

- [ ] 4. Реализация CDoomController
  - [ ] 4.1 Создать класс CDoomController в doom/doomcontroller.h/cpp
    - Реализовать Init() для запуска DOOM
    - Реализовать Shutdown() для корректного завершения


    - Реализовать Tick() и GetFrameBuffer()
    - _Requirements: 6.1_

  - [ ] 4.2 Реализовать маппинг ввода DDNet -> DOOM
    - Создать функцию TranslateInput()
    - Маппинг: Direction -> поворот, Jump -> вперёд, Hook -> назад, Fire -> стрельба
    - _Requirements: 2.1, 2.2, 2.3, 2.4, 2.5_
  - [ ] 4.3 Написать property-based тест для маппинга ввода
    - **Property 3: Input mapping consistency**

    - **Validates: Requirements 2.1, 2.2, 2.3, 2.4, 2.5**

- [ ] 5. Checkpoint - Проверка базовой функциональности
  - Ensure all tests pass, ask the user if questions arise.

- [ ] 6. Реализация CDoomScreen
  - [x] 6.1 Создать класс CDoomScreen в entities/doomscreen.h/cpp

    - Определить члены: позиция, размеры, владелец
    - Объявить вектор CLaserPixel указателей
    - _Requirements: 1.1, 1.2_
  - [ ] 6.2 Реализовать конструктор CDoomScreen
    - Создание сетки CLaserPixel сущностей


    - Инициализация CDoomController
    - _Requirements: 3.2_
  - [ ] 6.3 Написать property-based тест для размеров сетки
    - **Property 5: Pixel grid dimensions**

    - **Validates: Requirements 3.2**
  - [ ] 6.4 Реализовать CDoomScreen::Tick()
    - Обновление CDoomController
    - Получение frame buffer
    - Delta-обновление пикселей (только изменённые)
    - _Requirements: 3.3, 6.1_

  - [x] 6.5 Написать property-based тест для delta-обновлений

    - **Property 6: Delta update correctness**
    - **Validates: Requirements 3.3**
  - [ ] 6.6 Реализовать ProcessInput()
    - Перехват ввода владельца

    - Передача в CDoomController
    - _Requirements: 2.1-2.5_



- [x] 7. Интеграция команд doom и cleardoom

  - [ ] 7.1 Добавить команду doom в ddracecommands.cpp
    - Создать CDoomScreen рядом с игроком
    - Заморозить игрока
    - Установить игрока как владельца
    - _Requirements: 1.1, 1.2, 1.3_
  - [ ] 7.2 Добавить команду cleardoom в ddracecommands.cpp
    - Найти и удалить CDoomScreen игрока
    - Разморозить игрока
    - _Requirements: 1.4_
  - [ ] 7.3 Написать property-based тест для doom/cleardoom round-trip
    - **Property 2: Doom/cleardoom round-trip restores player state**
    - **Validates: Requirements 1.3, 1.4**

- [ ] 8. Интеграция с GameContext
  - [ ] 8.1 Добавить регистрацию CDoomScreen в gameworld
    - Добавить ENTTYPE_DOOMSCREEN
    - Регистрация в списке сущностей
    - _Requirements: 1.1_
  - [ ] 8.2 Интегрировать перехват ввода владельца
    - В CCharacter::OnDirectInput() проверять владение DOOM экраном
    - Перенаправлять ввод в CDoomScreen::ProcessInput()
    - _Requirements: 2.1-2.5_

- [ ] 9. Обработка отключения игрока
  - [ ] 9.1 Реализовать автоматическое удаление экрана при disconnect
    - В CDoomScreen::Tick() проверять владельца
    - Удалять экран если владелец отключился
    - _Requirements: 1.4_

- [ ] 10. Финальный Checkpoint
  - Ensure all tests pass, ask the user if questions arise.
