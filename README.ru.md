# bloodborne_windows — Нативный порт Bloodborne для Windows и Linux

[English](README.md) · **Русский**

`bloodborne_windows` — это нативный раннер x86-64 и высокопроизводительный HLE-рантайм для игры **Bloodborne** (PlayStation 4, CUSA03173, версия 1.09), работающий напрямую на **Windows** и **Linux**.

> [!IMPORTANT]
> **Происхождение форка и благодарности:**
> Данный репозиторий является форком проекта [**deadinside28/bloodborne_pc**](https://github.com/deadinside28/bloodborne_pc) разработчика deadinside28.
> В оригинальном проекте была заложена фундаментальная основа исполнения eboot Bloodborne на Linux. В настоящем форке произведен полный рефакторинг архитектуры: кодовая база разделена на независимые модули со строгим лимитом $\le 75$ строк на файл, полностью устранены платформенные зависимости в HLE-слое, реализованы нативные сервисы Windows (отображение физической памяти через Win32 Section aliasing, VEH-перехват исключений, QPC-тайминг, нативные потоки и сервисы), а также добавлены готовые кроссплатформенные сценарии сборки и запуска (`run.bat`, `run.ps1`, `run.sh`, `CMakeLists.txt`).

> [!NOTE]
> **Файлов игры в репозитории нет.** Для запуска необходим собственный расшифрованный дамп *Bloodborne* (PS4 CUSA03173 v1.09). Проект не связан с Sony Interactive Entertainment, FromSoftware или AMD.

---

## 🌟 Ключевые особенности и архитектура

- **Нативное исполнение (Без эмуляции CPU):** PS4 ELF отображается непосредственно в адресное пространство хоста в диапазоне до 40 бит (< 1 ТиБ), как того требуют дескрипторы GPU PS4. Машинный код x86-64 исполняется процессором напрямую на полной скорости через переходники вызовов SysV AMD64 ABI.
- **Строгий лимит строк ($\le 75$ строк на файл):** Каждый исходный файл C и заголовочный файл в каталогах `src/platform/`, `src/runtime/` и `src/loader/` строго соблюдает лимит $\le 75$ строк кода.
- **Ноль `#ifdef` платформы в рантайме HLE:** Логика `src/runtime/` и `src/loader/` обращается к ОС только через абстрактные интерфейсы `bb_platform_*`. Системные заголовки (`<windows.h>`, `<sys/mman.h>`, `<pthread.h>` и т.д.) изолированы исключительно в `src/platform/`.
- **Эмуляция прямой физической памяти на Windows:** Реализована через Win32 page-file секции (`CreateFileMappingA`, `MapViewOfFileEx`), что позволяет нескольким виртуальным адресам ссылаться на один физический пул. Очистка участков памяти реализована через коммит с обнулением (`VirtualAlloc(MEM_COMMIT)`).
- **Vectored Exception Handling (VEH):** На Windows перехват обращений к памяти осуществляется через `AddVectoredExceptionHandler` (`STATUS_ACCESS_VIOLATION`), обеспечивая трекинг страниц GPU и спекулятивное чтение аналогично Linux `sigaction` / `SA_SIGINFO`.
- **Изоляция сбоев и корректная деградация:** Отсутствие физических устройств (аудиокарты, геймпада) не приводит к падению игры — рантайм автоматически переключается на таймерную заглушку или клавиатуру.
- **Исправление уязвимостей:** В модуль `r_ajm_atrac9.c` внедрено исправление уязвимости переполнения буфера RIFF заголовка ATRAC9 (предотвращающее беззнаковый андерфлоу на 18 эксабайт).
- **GPU-ядро Vulkan:** Видеоядро на основе shadPS4 с двухстадийным конвейером отрисовки, векторами движения объектов и временным апскейлингом AMD FSR 3.1, FSR 4 (INT8) и FSR 4.1.1.

---

## 🎮 Как протестировать и запустить

### Требования
1. **Операционная система:** Windows 10/11 (64-бит) или 64-битный Linux.
2. **Видеокарта:** С поддержкой Vulkan 1.3+ (NVIDIA GeForce GTX 10-серии и новее, AMD Radeon RX 5000+ / RDNA, Intel Arc).
3. **Компилятор:**
   - **Windows:** MinGW GCC 13+ / Clang 16+ или MSVC 2022 (с CMake 3.24+).
   - **Linux:** GCC 11+ или Clang 14+.
4. **Python:** Python 3.8+ для оффлайн-подготовки файлов и линковки смещений.
5. **Дамп игры:** Расшифрованная копия *Bloodborne* CUSA03173 (v1.09).

---

### Шаг 1: Быстрый тест Vulkan (Без файлов игры)

Вы можете сразу проверить видеодрайвер и графический стек:

#### Сборка через CMake:
```powershell
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Запуск дымового теста Vulkan:
.\build\bb-probe.exe --vulkan-only
```

#### Запуск через скрипт PowerShell:
```powershell
.\run.ps1 -SmokeTest
```

При успехе инициализируется Vulkan, создается тестовый буфер и подтверждается готовность окна вывода.

---

### Шаг 2: Подготовка и запуск игры

1. Поместите папку с дампом **CUSA03173** рядом с репозиторием (`../CUSA03173`) либо укажите путь к ней через переменную среды `BB_GAME_DIR`.
2. Запустите раннер:

#### Windows (PowerShell):
```powershell
$env:BB_GAME_DIR = "C:\Games\Bloodborne\CUSA03173"
.\run.ps1
```

#### Windows (Командная строка CMD):
```cmd
set BB_GAME_DIR=C:\Games\Bloodborne\CUSA03173
run.bat
```

#### Linux:
```bash
export BB_GAME_DIR=/path/to/CUSA03173
./run.sh
```

Скрипт автоматически выполнит четыре этапа подготовки в папку `out/`:
1. `scripts/prepare.py`: Разбор заголовков ELF и сегментов SELF.
2. `scripts/link_libc.py`: Привязка вызовов PS4 libc к рантайму.
3. `scripts/link_modules.py`: Привязка динамических модулей игры и TLS.
4. `scripts/content_profile.py` & `scripts/patches.py`: Профиль контента и патчи разблокировки 60/90/uncapped FPS.

---

## 📁 Обзор модульной структуры

| Подсистема | Папка | Описание |
| :--- | :--- | :--- |
| **Абстракция платформы** | [`src/platform/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/platform/) | Изолированные сервисы Win32 и Linux: память, синхронизация, потоки, ФС, время, сигналы/VEH. |
| **Виртуальная память** | [`src/runtime/memory/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/memory/) | 13 модулей ($\le 75$ строк): Пул прямой памяти, VMA, секции, защита страниц. |
| **Потоки и ядро** | [`src/runtime/threads/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/threads/), [`kernel/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/kernel/) | 14 модулей ($\le 75$ строк): Жизненный цикл потоков Orbis, TLS, сигналы, время. |
| **Аудио и AJM** | [`src/runtime/audio/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/audio/), [`ajm/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/ajm/) | 17 модулей ($\le 75$ строк): Потоки SDL3, декодер ATRAC9 с патчем переполнения RIFF. |
| **Сохранения** | [`src/runtime/savedata/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/savedata/) | 7 модулей ($\le 75$ строк): Точки монтирования, param.sfo, потокобезопасный поиск. |
| **Часы (RTC)** | [`src/runtime/rtc/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/rtc/) | 5 модулей ($\le 75$ строк): Григорианский календарь, Orbis-тики, time_t, RFC2822. |
| **AppContent** | [`src/runtime/content/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/content/) | 4 модуля ($\le 75$ строк): Учет системных модулей, оффлайн-профиль контента. |
| **Системные сервисы** | [`src/runtime/services/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/services/) | 10 модулей ($\le 75$ строк): UserService, NetCtl, HTTP, NP/PSN, диалоги, виртуальная клавиатура, трофеи, PlayGo. |
| **Управление / Геймпад** | [`src/runtime/pad/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/pad/) | 5 модулей ($\le 75$ строк): Опрос DualShock 4, тачпад, вибрация, воспроизведение скриптов ввода. |
| **Диспетчеризация** | [`src/runtime/core/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/core/) | 6 модулей ($\le 75$ строк): TLS модулей, обработчики atexit, статические гарды C++, NID lookup. |
| **Загрузчик** | [`src/loader/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/loader/) | 9 модулей ($\le 75$ строк): Загрузка BBPROBE, перемещения, патчи BBPATCH2, точка входа `main()`. |

---

## ⌨️ Управление

### Геймпад (DualShock 4 / DualSense / Xbox / стандартные геймпады)
- Автоматически определяется через SDL3.
- Поддерживается вибрация и эмуляция тачпада (кнопка Select / Back работает как нажатие на левую половину тачпада).

### Клавиатура (При отсутствии геймпада)
| Клавиша | Действие на геймпаде |
| :--- | :--- |
| **W, A, S, D** | Левый стик (Движение) |
| **Стрелки** | Правый стик (Камера) |
| **Пробел** | Крест ($\times$) — Кувырок / Бег / Подтверждение |
| **Left Shift** | Круг ($\bigcirc$) — Отмена / Назад |
| **E** | Квадрат ($\square$) — Использовать предмет |
| **Q** | Треугольник ($\triangle$) — Лечение / Смена формы оружия |
| **1 / 3** | L1 / R1 — Левая / Правая обычная атака |
| **R / F** | L2 / R2 — Выстрел из пистолета / Сильная атака |
| **Z / C** | L3 / R3 — Захват цели / Приседание |
| **Enter** | Options / Меню паузы |
| **Tab / Backspace** | Нажатие левой / правой стороны тачпада |
| **I, K, J, L** | Крестовина Вверх / Вниз / Влево / Вправо |
| **Insert / L3+R3** | Открыть встроенное оверлей-меню настроек |

---

## ⚖️ Лицензии и благодарности

- Исходный репозиторий: [deadinside28/bloodborne_pc](https://github.com/deadinside28/bloodborne_pc) (лицензия MIT).
- Графическое ядро GPU основано на [shadPS4](https://github.com/shadps4-emu/shadPS4) (лицензия GPL-3.0).
- Декодер ATRAC9: [LibAtrac9](https://github.com/Thealexbarney/LibAtrac9).
- AMD FSR: Технология FidelityFX Super Resolution (AMD / лицензия MIT).
