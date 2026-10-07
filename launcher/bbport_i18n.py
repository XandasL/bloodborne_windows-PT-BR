# SPDX-License-Identifier: GPL-2.0-or-later
"""Launcher internationalization for the cross-platform GUI.

English is the source language. Brazilian Portuguese is selected automatically
for pt-BR Windows/system locales, and can be forced with BB_UI_LANGUAGE=pt_BR.
Set BB_UI_LANGUAGE=en to force English.
"""

import ctypes
import locale
import os
import sys

PT_BR = {
    "Bloodborne Runner Launcher & Mod Manager": "Bloodborne Runner — Launcher e Gerenciador de Mods",
    "Cross-Platform HLE Recompiler & Translation Launcher": "Launcher multiplataforma de recompilação HLE e tradução",
    "Game Directory:": "Pasta do jogo:",
    "Browse...": "Procurar...",
    "Settings & Graphics": "Configurações e Gráficos",
    "Mod Manager": "Gerenciador de Mods",
    "Diagnostics": "Diagnósticos",
    "LAUNCH BLOODBORNE": "INICIAR BLOODBORNE",
    "Stop": "Parar",
    "Vulkan Smoke Test": "Teste rápido do Vulkan",
    "Select Bloodborne Game Directory": "Selecionar a pasta do Bloodborne",
    "Mod Archives": "Arquivos de mod",
    "Success": "Sucesso",
    "Installed mod: {name}": "Mod instalado: {name}",
    "Error": "Erro",
    "Only .zip mod archives are supported.": "Apenas arquivos de mod .zip são suportados.",
    "PERFORMANCE & RESOLUTION": "DESEMPENHO E RESOLUÇÃO",
    "Target Framerate:": "Taxa de quadros:",
    "Output Resolution:": "Resolução de saída:",
    "Temporal Upscaler:": "Upscaler temporal:",
    "Upscaler Quality Preset:": "Predefinição de qualidade do upscaler:",
    "GRAPHICAL EFFECTS & PATCHES": "EFEITOS GRÁFICOS E PATCHES",
    "Skip Intro Videos": "Pular vídeos de introdução",
    "Motion Blur": "Desfoque de movimento",
    "Depth of Field (DoF)": "Profundidade de campo (DoF)",
    "Chromatic Aberration": "Aberração cromática",
    "Ambient Occlusion (SSAO)": "Oclusão de ambiente (SSAO)",
    "SSR Reflections (Experimental)": "Reflexos SSR (experimental)",
    "Show In-Game FPS Overlay": "Mostrar FPS durante o jogo",
    "INSTALLED MODS": "MODS INSTALADOS",
    "Add Mod (.zip)": "Adicionar mod (.zip)",
    "Open Folder": "Abrir pasta",
    "Refresh": "Atualizar",
    "HARDWARE & RUNTIME DIAGNOSTICS": "DIAGNÓSTICOS DE HARDWARE E RUNTIME",
    "Run Vulkan Smoke Test Now": "Executar teste rápido do Vulkan",
    "OS: Windows 64-bit / Linux x86-64": "SO: Windows 64 bits / Linux x86-64",
    "Architecture: Zero CPU Emulation (SysV AMD64 ABI Direct Call Gates)": "Arquitetura: sem emulação de CPU (chamadas diretas via ABI SysV AMD64)",
    "Graphics: Vulkan 1.3 translation layer with temporal reconstruction": "Gráficos: camada de tradução Vulkan 1.3 com reconstrução temporal",
    "Memory: Direct Win32 Pagefile Section physical memory aliasing (< 1 TiB)": "Memória: aliasing direto de memória física via Win32 Pagefile Section (< 1 TiB)",
    "To verify your graphics drivers and swapchain negotiation without running the game,": "Para verificar o driver gráfico e a negociação da swapchain sem iniciar o jogo,",
    "click 'Run Vulkan Smoke Test Now' below.": "clique em 'Executar teste rápido do Vulkan' abaixo.",
    "[SMOKE TEST] Initializing Vulkan hardware smoke test...": "[TESTE VULKAN] Inicializando teste rápido de hardware Vulkan...",
    "Not Found": "Não encontrado",
    "Could not find bb-probe executable.": "Não foi possível encontrar o executável bb-probe.",
    "[SMOKE TEST] Completed with exit code {code}": "[TESTE VULKAN] Concluído com código de saída {code}",
    "Please select a valid game folder containing 'eboot.bin'.": "Selecione uma pasta válida do jogo contendo 'eboot.bin'.",
    "Preparing Bloodborne from: {path}": "Preparando o Bloodborne a partir de: {path}",
    ">> Booting Bloodborne via {probe}...": ">> Iniciando o Bloodborne via {probe}...",
    ">> Game terminated (Exit code: {code})": ">> Jogo encerrado (código de saída: {code})",
    "Error: {error}": "Erro: {error}",
    "Directory does not exist. Click 'Browse...' to select.": "A pasta não existe. Clique em 'Procurar...' para selecionar.",
    "Directory found, but 'eboot.bin' is missing inside.": "Pasta encontrada, mas o arquivo 'eboot.bin' não está nela.",
    "'eboot.bin' does not have a standard PS4 SELF header.": "'eboot.bin' não possui um cabeçalho SELF padrão do PS4.",
    "Cannot read eboot.bin: {error}": "Não foi possível ler eboot.bin: {error}",
    "Ready: Title ID: {title_id} | Version: {app_ver} | Verified plaintext ELF": "Pronto: Title ID: {title_id} | Versão: {app_ver} | ELF descriptografado verificado",
    "Unknown": "Desconhecida",
}


def _normalize_language(value):
    if not value:
        return None
    value = str(value).strip().lower().replace("-", "_").replace(".", "_")
    if value in ("en", "english") or value.startswith("en_"):
        return "en"
    if (
        value in ("pt", "pt_br", "portuguese_brazil", "portuguese_brazilian")
        or value.startswith("pt_br")
        or ("portuguese" in value and "brazil" in value)
    ):
        return "pt_BR"
    return None


def _windows_user_locale():
    if sys.platform != "win32":
        return None
    try:
        locale_name = ctypes.create_unicode_buffer(85)
        if ctypes.windll.kernel32.GetUserDefaultLocaleName(locale_name, len(locale_name)):
            return locale_name.value
    except Exception:
        pass
    return None


def system_language():
    """Return en or pt_BR from the host locale, preferring the Windows user locale."""
    for value in (
        _windows_user_locale(),
        os.environ.get("LC_ALL"),
        os.environ.get("LC_MESSAGES"),
        os.environ.get("LANG"),
        os.environ.get("LANGUAGE"),
    ):
        normalized = _normalize_language(value)
        if normalized:
            return normalized

    try:
        normalized = _normalize_language(locale.getlocale()[0])
        if normalized:
            return normalized
    except Exception:
        pass
    return "en"


_language = "en"


def set_language(choice=None):
    """Select en/pt_BR. Empty/system/None follows the system locale."""
    global _language
    if choice in (None, "", "system"):
        _language = system_language()
    else:
        _language = _normalize_language(choice) or "en"
    return _language


def language():
    return _language


def tr(text):
    if _language == "pt_BR":
        return PT_BR.get(text, text)
    return text


set_language(os.environ.get("BB_UI_LANGUAGE"))
