// SPDX-License-Identifier: GPL-2.0-or-later
// In-game overlay localization. English is the authoritative fallback.
// Keep the catalog order aligned with Key; do not translate persisted bbport.ini keys.
#pragma once

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <cstdio>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace BbLocale {

enum class Language { English, Portuguese, Russian };

inline bool Matches(const char* value, char first, char second) {
    if (!value || !value[0] || !value[1]) return false;
    auto lower = [](char c) { return c >= 'A' && c <= 'Z' ? char(c + ('a' - 'A')) : c; };
    return lower(value[0]) == first && lower(value[1]) == second &&
           (value[2] == '\0' || value[2] == '-' || value[2] == '_' || value[2] == '.');
}

inline Language LanguageFrom(const char* code) {
    if (Matches(code, 'p', 't')) return Language::Portuguese;
    if (Matches(code, 'r', 'u')) return Language::Russian;
    return Language::English; // Explicit but unsupported languages fall back to English.
}

inline Language SystemLanguage() {
#ifdef _WIN32
    const unsigned short primary = GetUserDefaultUILanguage() & 0x3ff;
    if (primary == 0x16) return Language::Portuguese;
    if (primary == 0x19) return Language::Russian;
    return Language::English;
#else
    for (const char* var : {"LC_ALL", "LC_MESSAGES", "LANG"}) {
        if (const char* value = std::getenv(var); value && *value)
            return LanguageFrom(value);
    }
    return Language::English;
#endif
}

// The patch-only Windows test builds replace bin/bb-probe.exe but keep the user's existing
// Bloodborne.exe launcher. Read its saved UI preference as a fallback when an older launcher
// does not forward BB_UI_LANGUAGE yet. Never read the game's language option here.
inline Language LauncherLanguageOrSystem() {
#ifdef _WIN32
    wchar_t appdata[32768];
    const DWORD length = GetEnvironmentVariableW(L"APPDATA", appdata, 32768);
    if (!length || length >= 32768) return SystemLanguage();

    const std::wstring config = std::wstring(appdata, length) +
                                L"\\bbport-launcher\\settings.json";
    FILE* file = _wfopen(config.c_str(), L"rb");
    if (!file) return SystemLanguage();

    char buffer[16384] = {};
    const std::size_t count = std::fread(buffer, 1, sizeof(buffer) - 1, file);
    std::fclose(file);
    const std::string json(buffer, count);

    const std::size_t key = json.find("\"ui_language\"");
    if (key == std::string::npos) return SystemLanguage();
    std::size_t pos = json.find(':', key + sizeof("\"ui_language\"") - 1);
    if (pos == std::string::npos) return SystemLanguage();
    ++pos;
    while (pos < json.size() &&
           (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\r' || json[pos] == '\n')) {
        ++pos;
    }
    if (pos >= json.size() || json[pos] != '"') return SystemLanguage();
    const std::size_t end = json.find('"', pos + 1);
    if (end == std::string::npos) return SystemLanguage();
    const std::string choice = json.substr(pos + 1, end - pos - 1);
    return choice.empty() ? SystemLanguage() : LanguageFrom(choice.c_str());
#else
    return SystemLanguage();
#endif
}

inline Language CurrentLanguage() {
    static const Language language = [] {
        // New launcher: explicit choice passed to the game, including desktop shortcuts.
        if (const char* choice = std::getenv("BB_UI_LANGUAGE"); choice) {
            if (std::strcmp(choice, "auto") == 0 || std::strcmp(choice, "system") == 0)
                return SystemLanguage();
            if (*choice) return LanguageFrom(choice);
        }
        // Older Windows launcher: read the existing app settings directly, then OS UI locale.
        return LauncherLanguageOrSystem();
    }();
    return language;
}

enum class Key : std::size_t {
    Overlay0,
    Overlay1,
    Overlay2,
    Overlay3,
    Overlay7,
    Overlay10,
    Overlay11,
    Overlay12,
    Overlay14,
    Overlay15,
    Overlay16,
    Overlay17,
    Overlay18,
    Overlay19,
    Overlay20,
    Overlay22,
    Overlay23,
    Overlay24,
    Overlay25,
    Overlay26,
    Overlay28,
    Overlay29,
    Overlay30,
    Overlay31,
    Overlay32,
    Overlay33,
    Overlay34,
    Overlay35,
    Overlay36,
    Overlay37,
    Overlay38,
    Overlay39,
    Overlay40,
    Overlay41,
    Overlay42,
    Overlay43,
    Overlay44,
    Overlay45,
    Overlay46,
    Overlay47,
    Overlay52,
    Overlay53,
    Overlay54,
    Overlay55,
    Overlay56,
    Overlay57,
    Overlay58,
    Overlay59,
    Overlay60,
    Overlay61,
    Overlay62,
    Overlay63,
    Overlay64,
    Overlay65,
    Overlay66,
    Overlay67,
    Overlay68,
    Overlay69,
    Overlay70,
    Overlay71,
    Overlay72,
    Overlay73,
    Overlay77,
    Overlay78,
    Overlay80,
    PresetNative,
    PresetQuality,
    PresetBalanced,
    PresetPerformance,
    PresetUltra,
    Effect0,
    Effect1,
    Effect2,
    Effect3,
    Effect4,
    Effect5,
    Effect6,
    Effect7,
    Effect8,
    Effect9,
    Effect10,
    Effect11,
    Effect12,
    Effect13,
    Effect14,
    Effect15,
    Effect16,
    Effect17,
    Effect18,
    Effect19,
    Count
};

struct Translation {
    const char* english;
    const char* portuguese;
    const char* russian;
};

inline constexpr Translation catalog[] = {
    {"Bloodborne — Settings (Insert / L3+R3)", "Bloodborne — Configurações (Insert / L3+R3)", "Bloodborne — настройки  (Insert / L3+R3)"}, // Overlay0
    {"%.0f FPS  (%.1f ms)", "%.0f FPS  (%.1f ms)", "%.0f FPS  (%.1f мс)"}, // Overlay1
    {"Temporal upscaling", "Upscaling temporal", "Временной апскейлер"}, // Overlay2
    {"Off", "Desativado", "Выкл"}, // Overlay3
    {"TAA (native anti-aliasing)", "TAA (suavização nativa)", "TAA (нативное сглаживание)"}, // Overlay7
    {"Upscaler", "Upscaler", "Апскейлер"}, // Overlay10
    {"— not supported by your GPU", "— não compatível com a GPU", "— не поддерживается видеокартой"}, // Overlay11
    {"— in development", "— em desenvolvimento", "— в работе"}, // Overlay12
    {"FSR 4 unavailable: %s", "FSR 4 indisponível: %s", "FSR 4 недоступен: %s"}, // Overlay14
    {"The mode selected above is active. You can select FSR 4 again.", "O modo selecionado acima está ativo. Você pode selecionar FSR 4 novamente.", "Активен режим, выбранный выше. FSR 4 можно выбрать снова."}, // Overlay15
    {"FSR 4.1.1 INT8: model from the AMD 4.1.1 DLL reproduced in Vulkan (same results as the DLL). One model covers Native through Performance, with another for Ultra Performance. Assets: tools/fsr4cap/build_assets.sh (requires DLL and Proton).", "FSR 4.1.1 INT8: modelo da DLL AMD 4.1.1 reproduzido no Vulkan (resultado equivalente ao da DLL). Um modelo para Nativo a Desempenho e outro para Ultra desempenho. Arquivos: tools/fsr4cap/build_assets.sh (exige DLL e Proton).", "FSR 4.1.1 в режиме INT8: модель из DLL AMD 4.1.1, воспроизведённая в Vulkan (результат совпадает с DLL). Одна модель для Native..Performance и отдельная для Ultra Performance. Ассеты: tools/fsr4cap/build_assets.sh (нужны DLL и Proton)."}, // Overlay16
    {"FSR 4 INT8 (v07 model from the AMD FidelityFX SDK). Better quality than FSR 3.1, but more expensive. Changing presets rebuilds the model (short pause). Assets: tools/fetch_fsr4_assets.sh.", "FSR 4 INT8 (modelo v07 do AMD FidelityFX SDK). Qualidade superior à do FSR 3.1, mas com maior custo de processamento. Mudar a predefinição recompila o modelo (breve pausa). Arquivos: tools/fetch_fsr4_assets.sh.", "FSR 4 в режиме INT8 (модель v07 из исходников AMD FidelityFX SDK). Качество выше, чем у FSR 3.1, но проход тяжелее. Смена пресета пересобирает модель (короткая пауза). Ассеты: tools/fetch_fsr4_assets.sh."}, // Overlay17
    {"FSR 4: auto exposure", "FSR 4: exposição automática", "FSR 4: авто-экспозиция"}, // Overlay18
    {"FSR 4: invert jitter sign", "FSR 4: inverter sinal do jitter", "FSR 4: обратный знак jitter"}, // Overlay19
    {"Diagnostic: FSR 4 normalizes color using exposure and uses it to decide when to discard previous frames. Changes apply immediately without restarting.", "Teste: a rede do FSR 4 normaliza as cores pela exposição e a usa para decidir quando descartar quadros anteriores. Mudanças imediatas, sem reiniciar.", "Проверка при гостинге: сеть FSR 4 нормирует цвет по экспозиции и по ней решает, когда отбросить прошлые кадры. Меняются сразу, без перезапуска."}, // Overlay20
    {"Preset", "Predefinição", "Пресет"}, // Overlay22
    {"%s (x%.1f, render %dx%d)", "%s (x%.1f, renderização %dx%d)", "%s (x%.1f, рендер %dx%d)"}, // Overlay23
    {"TAA anti-aliases the scene at output resolution without FSR or upscaling. Your saved FSR preset will return when you select FSR.", "O TAA suaviza a cena na resolução de saída, sem modelo FSR nem upscaling. A predefinição de FSR salva será restaurada ao selecionar FSR.", "TAA сглаживает сцену в разрешении вывода, без модели FSR и апскейлинга. Сохранённый пресет FSR восстановится при выборе FSR."}, // Overlay24
    {"Active scene rendering: %d x %d", "Resolução interna atual: %d x %d", "Активный рендер сцены: %d x %d"}, // Overlay25
    {"Preset at startup: %s", "Predefinição inicial: %s", "Пресет при запуске: %s"}, // Overlay26
    {"With output other than 1080p, the entire game renders at the selected preset resolution (startup patch). This is fastest on Steam Deck and less powerful GPUs. Changing preset or output resolution requires a restart. Live resolution switching below allows changes without a restart, but post-processing remains at 1080p (slower).", "Ao usar saída diferente de 1080p, o jogo é renderizado na resolução da predefinição (patch de inicialização). É mais rápido no Steam Deck e em GPUs menos potentes. Mudar predefinição ou resolução de saída exige reiniciar. A opção de resolução dinâmica abaixo permite mudar sem reiniciar, mas mantém o pós-processamento em 1080p (mais lento).", "При выводе не 1080p вся игра рисуется в разрешении пресета (патч при запуске): это быстрее всего на Steam Deck и слабых GPU. Смена пресета или разрешения вывода — после перезапуска. Пункт «Смена разрешения на лету» ниже включает смену без перезапуска (постобработка тогда остаётся в 1080p, медленнее)."}, // Overlay28
    {"BB_RENDER_RES fixes scene resolution at startup. Remove this explicit environment variable to change resolution and presets without restarting.", "BB_RENDER_RES fixa a resolução interna na inicialização. Remova essa variável para mudar a resolução e as predefinições sem reiniciar o jogo.", "BB_RENDER_RES фиксирует размер сцены при запуске. Уберите эту явную переменную для смены разрешения и пресетов без перезапуска игры."}, // Overlay29
    {"Native AA: FSR is used for anti-aliasing. Other presets reduce scene resolution relative to output resolution. The UI renders at output resolution. The preset applies on the next frame without restarting.", "AA nativo: o FSR funciona apenas como suavização. As outras predefinições reduzem a resolução interna em relação à de saída. A interface é renderizada na resolução de saída. A predefinição entra em vigor no próximo quadro, sem reiniciar.", "Native AA: FSR работает как сглаживание. Остальные пресеты уменьшают разрешение отрисовки сцены относительно вывода. Интерфейс рисуется в разрешении вывода. Пресет применяется со следующего кадра без перезапуска игры."}, // Overlay30
    {"Sharpening (RCAS)", "Nitidez (RCAS)", "Резкость (RCAS)"}, // Overlay31
    {"Sharpening strength", "Intensidade da nitidez", "Сила резкости"}, // Overlay32
    {"Up to 1: upscaler sharpening (RCAS). Above 1: an additional RCAS pass. Ctrl+click the slider to type an exact value.", "Até 1: nitidez do próprio upscaler (RCAS). Acima de 1: adiciona outra etapa RCAS. Ctrl + clique no controle para digitar um valor exato.", "До 1 — резкость самого апскейлера (RCAS). Выше 1 добавляется ещё один проход RCAS. Ctrl+клик по ползунку — ввести точное значение."}, // Overlay33
    {"Subpixel jitter", "Deslocamento subpixel (jitter)", "Субпиксельный сдвиг (jitter)"}, // Overlay34
    {"Each frame is shifted by a fraction of a pixel so the upscaler can reconstruct details from multiple frames. Without jitter, only temporal anti-aliasing is applied.", "A cena é deslocada uma fração de pixel a cada quadro; o upscaler combina informações de vários quadros para recuperar detalhes. Sem isso, há só suavização temporal.", "Каждый кадр сцена сдвигается на долю пикселя, и апскейлер собирает из нескольких кадров больше деталей. Без него получается только сглаживание по истории."}, // Overlay35
    {"Reactive mask", "Máscara reativa", "Маска реактивности"}, // Overlay36
    {"Enable mask", "Ativar máscara", "Включить маску"}, // Overlay37
    {"Marks transparent effects (particles, mist) so the upscaler relies less on previous frames. This reduces ghosting behind effects but can increase shimmering.", "Identifica efeitos transparentes (partículas e névoa) para o upscaler depender menos dos quadros anteriores. Reduz rastros, mas pode aumentar a cintilação.", "Помечает прозрачные эффекты (частицы, дымку), чтобы апскейлер меньше опирался на прошлые кадры. Меньше шлейфов за эффектами, но под ними возвращается дрожание."}, // Overlay38
    {"Scale", "Escala", "Масштаб"}, // Overlay39
    {"Threshold", "Limite", "Порог"}, // Overlay40
    {"Maximum", "Máximo", "Максимум"}, // Overlay41
    {"Show mask (debug)", "Exibir máscara (depuração)", "Показать маску (отладка)"}, // Overlay42
    {"Character motion vectors", "Vetores de movimento dos personagens", "Векторы движения персонажей"}, // Overlay43
    {"Accurate motion vectors for animated objects reduce artifacts on moving clothing and weapons. Static scenes need no extra pass. Changes require a restart.", "Vetores precisos para objetos animados: roupas e armas apresentam menos artefatos ao se mover. A cena estática não recebe processamento extra. A alteração exige reiniciar o jogo.", "Точные векторы для анимированных объектов: одежда и оружие меньше рассыпаются при движении. Статичная сцена не получает дополнительный проход. Изменение применяется после перезапуска игры."}, // Overlay44
    {"Show motion vectors (debug)", "Exibir vetores de movimento (depuração)", "Показать векторы движения (отладка)"}, // Overlay45
    {"Red/green: horizontal/vertical motion (8 pixels = full brightness). Blue: the pixel has an object motion vector, not just camera motion. Without blue or red/green, the upscaler sees a moving object as still, causing ghosting.", "Vermelho/verde: movimento horizontal/vertical (8 pixels = brilho máximo). Azul: pixel com vetor de movimento do objeto, não apenas da câmera. Um objeto que se move sem azul nem vermelho/verde é interpretado pelo upscaler como parado, causando rastros.", "Красный/зелёный: движение по горизонтали/вертикали (8 пикселей = полная яркость). Синий: пиксель получил точный вектор объекта, а не только движение камеры. Движущийся предмет без синего и без красного/зелёного апскейлер считает неподвижным, отсюда шлейф."}, // Overlay46
    {"Output resolution", "Resolução de saída", "Разрешение вывода"}, // Overlay47
    {"Output resolution", "Resolução de saída", "Разрешение вывода"}, // Overlay52
    {"The final frame and UI resolution. The preset controls internal resolution relative to output: 4K Performance = 1920x1080. Changes require a restart.", "Resolução do quadro final e da interface. A predefinição determina a resolução interna em relação à saída: 4K em Desempenho = 1920x1080. Exige reiniciar.", "Размер готового кадра и интерфейса. Пресет задаёт размер сцены относительно вывода: 4K Performance = 1920x1080. Применяется после перезапуска игры."}, // Overlay53
    {"The final frame and UI resolution changes on the next frame. The preset controls internal resolution relative to output: 4K Performance = 1920x1080. Changing the size resets FSR history and can cause a short pause.", "A resolução final e da interface muda no próximo quadro. A predefinição determina a resolução interna: 4K em Desempenho = 1920x1080. Mudar a resolução redefine o histórico do FSR e pode provocar uma breve pausa.", "Размер готового кадра и интерфейса меняется на границе следующего кадра. Пресет задаёт размер сцены относительно вывода: 4K Performance = 1920x1080. Смена размера сбрасывает историю FSR и может вызвать короткую паузу."}, // Overlay54
    {"Auto (based on GPU)", "Automático (conforme a GPU)", "Авто (по видеокарте)"}, // Overlay55
    {"Off (faster)", "Desativado (mais rápido)", "Выключена (быстрее)"}, // Overlay56
    {"On", "Ativado", "Включена"}, // Overlay57
    {"Change resolution without restarting", "Alterar resolução sem reiniciar", "Смена разрешения на лету"}, // Overlay58
    {"On: output resolution and preset can change without restarting, but game post-processing stays at 1080p; this is noticeably slower on Steam Deck and older GPUs. Off: render everything at the preset resolution and restart to change it. Auto enables the feature on powerful discrete GPUs. Changes require a restart.", "Ativado: a resolução de saída e a predefinição mudam sem reiniciar, mas o pós-processamento do jogo permanece em 1080p, com perda de desempenho no Steam Deck e em GPUs antigas. Desativado: tudo é renderizado na resolução da predefinição, exigindo reinício. Automático ativa o modo em GPUs dedicadas potentes. A alteração exige reiniciar.", "Включена: разрешение вывода и пресет меняются без перезапуска, но постобработка игры остаётся в 1080p — на Steam Deck и старых видеокартах это заметно медленнее. Выключена: всё рисуется в разрешении пресета, смена — через перезапуск. Авто включает её на мощных дискретных видеокартах. Применяется после перезапуска игры."}, // Overlay59
    {"Game effects (restart required)", "Efeitos do jogo (exigem reiniciar)", "Эффекты игры (после перезапуска)"}, // Overlay60
    {"Highest (-2)", "Máximo (-2)", "Максимальная (-2)"}, // Overlay61
    {"As in game", "Padrão do jogo", "Как в игре"}, // Overlay62
    {"Lower (1)", "Reduzido (1)", "Ниже (1)"}, // Overlay63
    {"Lowest (2)", "Mínimo (2)", "Минимальная (2)"}, // Overlay64
    {"Model detail", "Nível de detalhe dos modelos", "Детализация моделей"}, // Overlay65
    {"Game effects are enabled/disabled by startup patches (patches/Bloodborne.xml). Motion blur and dynamic light shadows can significantly increase GPU load.", "Os efeitos são ativados ou desativados por patches na inicialização (patches/Bloodborne.xml). Desfoque de movimento e sombras de luzes dinâmicas podem pesar na GPU.", "Эффекты включаются и выключаются патчами игры при запуске (patches/Bloodborne.xml). Размытие в движении и тени от динамических источников заметно нагружают GPU."}, // Overlay66
    {"Free camera: hold Cross and press L3 (keyboard: Space + Z). Debug menu: left touchpad / Tab. Requires DbgFont14h.ccm and DbgFont14h.tpf from Nexus mod #253 in dvdroot_ps4/font. Right touchpad: Backspace.", "Câmera livre: segure Cross e pressione L3 (teclado: Espaço + Z). Menu de depuração: touchpad esquerdo / Tab. Exige DbgFont14h.ccm e DbgFont14h.tpf em dvdroot_ps4/font, do mod Nexus #253. Touchpad direito: Backspace.", "Свободная камера: удерживайте Cross и нажимайте L3 (клавиатура: Space + Z). Debug menu: левый touchpad / Tab. Нужны DbgFont14h.ccm и DbgFont14h.tpf в dvdroot_ps4/font из мода Nexus #253. Правый touchpad: Backspace."}, // Overlay67
    {"Changes take effect after restarting the game", "Alterações pendentes: é necessário reiniciar o jogo", "Изменения применятся после перезапуска игры"}, // Overlay68
    {"Apply and restart game", "Aplicar e reiniciar o jogo", "Применить и перезапустить игру"}, // Overlay69
    {"Other", "Outros", "Прочее"}, // Overlay70
    {"FPS counter in corner", "Contador de FPS no canto", "Счётчик FPS в углу"}, // Overlay71
    {"Close", "Fechar", "Закрыть"}, // Overlay72
    {"Settings are saved to bbport.ini", "Configurações salvas em bbport.ini", "Настройки сохраняются в bbport.ini"}, // Overlay73
    {"Keyboard: type, Backspace to erase, Enter = OK, Esc = cancel", "Teclado: digite; Backspace apaga; Enter confirma; Esc cancela", "Keyboard: type, Backspace to erase, Enter = OK, Esc = cancel"}, // Overlay77
    {"Controller: Cross (A) = OK, Circle (B) = cancel", "Controle: Cross (A) confirma; Circle (B) cancela", "Controller: Cross (A) = OK, Circle (B) = cancel"}, // Overlay78
    {"%.0f FPS  %.1f ms  %s", "%.0f FPS  %.1f ms  %s", "%.0f FPS  %.1f мс  %s"}, // Overlay80
    {"Native AA", "AA nativo", "Нативное AA"}, // PresetNative
    {"Quality", "Qualidade", "Качество"}, // PresetQuality
    {"Balanced", "Equilibrado", "Баланс"}, // PresetBalanced
    {"Performance", "Desempenho", "Производительность"}, // PresetPerformance
    {"Ultra Performance", "Ultra desempenho", "Ультра-производительность"}, // PresetUltra
    {"Chromatic aberration", "Aberração cromática", "Хроматическая аберрация"}, // Effect0
    {"Depth of field (DoF)", "Profundidade de campo (DoF)", "Глубина резкости (DoF)"}, // Effect1
    {"Motion blur", "Desfoque de movimento", "Размытие в движении"}, // Effect2
    {"Ambient occlusion (SSAO)", "Oclusão de ambiente (SSAO)", "Затенение SSAO"}, // Effect3
    {"Game's original anti-aliasing", "Suavização original do jogo", "Собственное сглаживание игры"}, // Effect4
    {"Dynamic light shadows", "Sombras de luzes dinâmicas", "Тени от динамических источников"}, // Effect5
    {"SSR reflections (not in original game)", "Reflexos SSR (não presentes no original)", "Отражения SSR (не было в игре)"}, // Effect6
    {"Skip intro logos and video", "Pular vídeos e logos de abertura", "Пропуск заставок при запуске"}, // Effect7
    {"Free camera (Cross + L3)", "Câmera livre (Cross + L3)", "Свободная камера (Cross + L3)"}, // Effect8
    {"Debug menu (requires font files)", "Menu de depuração (exige arquivos de fonte)", "Debug menu (нужны файлы шрифтов)"}, // Effect9
    {"Cheat: invincibility (at least 1 HP)", "Cheat: invencibilidade (mínimo de 1 HP)", "Чит: бессмертие (не ниже 1 HP)"}, // Effect10
    {"Cheat: enemies cannot see you", "Cheat: inimigos não detectam você", "Чит: враги не замечают"}, // Effect11
    {"Cheat: enemies cannot hear you", "Cheat: inimigos não ouvem você", "Чит: враги не слышат"}, // Effect12
    {"Cheat: Rally never decays", "Cheat: Rally não diminui", "Чит: Rally не угасает"}, // Effect13
    {"Cheat: control enemies (R3 / L3)", "Cheat: controlar inimigos (R3 / L3)", "Чит: управление врагом (R3 / L3)"}, // Effect14
    {"Disable Rally (HP recovery)", "Desativar Rally (recuperação de HP)", "Без Rally (возврата HP)"}, // Effect15
    {"Camera farther away", "Câmera mais afastada", "Камера дальше"}, // Effect16
    {"Disable auto-rotation of camera", "Desativar rotação automática da câmera", "Без автоповорота камеры"}, // Effect17
    {"Run with less stick tilt", "Correr com menor inclinação do analógico", "Бег с меньшим наклоном стика"}, // Effect18
    {"Ragdoll physics like Dark Souls", "Física dos corpos como em Dark Souls", "Физика тел как в Dark Souls"}, // Effect19
};

static_assert(sizeof(catalog) / sizeof(catalog[0]) == static_cast<std::size_t>(Key::Count));

inline const char* Text(Key key) {
    const auto& entry = catalog[static_cast<std::size_t>(key)];
    switch (CurrentLanguage()) {
    case Language::Portuguese: return entry.portuguese;
    case Language::Russian: return entry.russian;
    default: return entry.english;
    }
}

inline const char* EffectLabel(int index) {
    return Text(static_cast<Key>(static_cast<std::size_t>(Key::Effect0) + index));
}

} // namespace BbLocale
