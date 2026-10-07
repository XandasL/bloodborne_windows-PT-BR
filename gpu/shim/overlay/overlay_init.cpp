// SPDX-License-Identifier: GPL-2.0-or-later
#include "overlay_internal.h"
#include "overlay_font.h"
#include "imgui_impl_vulkan.h"
#include <SDL3/SDL.h>

namespace BbOverlay {

std::mutex imgui_mutex;
bool initialized = false;
std::atomic<bool> menu_open{false};
bool l3_down = false, r3_down = false;
bool dirty = false;
float base_scale = 1.0f;
std::chrono::steady_clock::time_point last_present{};
float frame_ms_avg = 0.0f;

void SetOpen(bool value) {
    if (menu_open.exchange(value) == value) return;
    ImGui::GetIO().MouseDrawCursor = value;
    if (!value && dirty) {
        dirty = false;
        BbSettings::Save();
    }
}

void Init(VkInstance instance, VkPhysicalDevice phys, VkDevice dev,
          uint32_t queue_family, VkQueue queue, VkDescriptorPool pool,
          VkRenderPass rp, uint32_t image_count, float scale) {
    std::scoped_lock lock{imgui_mutex};
    if (initialized) return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    base_scale = scale > 0.0f ? scale : 1.0f;
    io.FontGlobalScale = 1.0f;

    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false;
    const ImWchar ranges[] = {0x0020, 0x00FF, 0x0400, 0x04FF, 0};
    const float size = std::round(16.0f * base_scale);
    io.Fonts->AddFontFromMemoryTTF(
        const_cast<unsigned char*>(bb_font_ttf),
        int(bb_font_ttf_end - bb_font_ttf), size, &cfg, ranges);

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(base_scale);

    ImGui_ImplVulkan_InitInfo info{};
    info.Instance = instance; info.PhysicalDevice = phys; info.Device = dev;
    info.QueueFamily = queue_family; info.Queue = queue; info.DescriptorPool = pool;
    info.RenderPass = rp; info.MinImageCount = image_count;
    info.ImageCount = image_count; info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&info);
    initialized = true;
}

} // namespace BbOverlay
