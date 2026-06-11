#include "renderer.h"

#include <unordered_map>

#include <imgui.h>

#include "engine/core/debug.h"
#include "engine/core/systems/console.h"

namespace hob {
    void Renderer::register_cvars(Console& console) {
        console.register_cvar("r_log_texture_ref",
                              "Log every texture load/unload/cache-hit",
                              "0",
                              ConsoleVariableType::Bool,
                              ConsoleVariableFlags::None,
                              [this](const ConsoleVariable& cvar) {
                                  m_cvar_log_texture_ref = cvar.bool_value();
                              });

        console.register_cvar("r_show_texture_refs",
                              "Show a texture cache window (size, game refs, all refs, path)",
                              "0",
                              ConsoleVariableType::Bool,
                              ConsoleVariableFlags::None,
                              [this](const ConsoleVariable& cvar) {
                                  m_cvar_show_texture_refs = cvar.bool_value();
                              });

        console.register_cvar("r_log_sprite_queue",
                              "Log sprite queue (z_index, shader_id, texture) each frame",
                              "0",
                              ConsoleVariableType::Bool,
                              ConsoleVariableFlags::None,
                              [this](const ConsoleVariable& cvar) {
                                  m_cvar_log_sprite_queue = cvar.bool_value();
                              });

        console.register_cvar("r_show_sprite_queue",
                              "Show a sprite queue window (z_index, shader_id, texture)",
                              "0",
                              ConsoleVariableType::Bool,
                              ConsoleVariableFlags::None,
                              [this](const ConsoleVariable& cvar) {
                                  m_cvar_show_sprite_queue = cvar.bool_value();
                              });
    }

    void Renderer::debug_texture_refs() {
        if (!m_cvar_show_texture_refs) {
            return;
        }

        if (ImGui::Begin("Texture Refs")) {
            // Count per-texture refs held by the renderer's draw data.
            // World sprite draws hold a persistent TextureRef and overlay sprites a per-frame one;
            // both inflate use_count() without being "logical" holders.
            std::unordered_map<const Texture*, int> pending_refs;
            for (const auto& draw : m_sprite_draws) {
                if (draw.texture) {
                    pending_refs[draw.texture.get()] += 1;
                }
            }
            for (const auto& sp : m_pending_overlay_sprites) {
                if (sp.texture) {
                    pending_refs[sp.texture.get()] += 1;
                }
            }

            int total_game = 0;
            int total_all = 0;
            for (const auto& [path, weak] : m_textures) {
                if (auto tex = weak.lock()) {
                    // Subtract 1 because `tex` itself is a strong ref held only for this iteration.
                    const int all = static_cast<int>(tex.use_count()) - 1;
                    const auto pit = pending_refs.find(tex.get());
                    const int pending = pit != pending_refs.end() ? pit->second : 0;
                    total_all += all;
                    total_game += all - pending;
                }
            }
            ImGui::Text("Textures: %zu | Game refs: %d | All refs: %d",
                        m_textures.size(),
                        total_game,
                        total_all);

            const ImGuiTableFlags flags = ImGuiTableFlags_Borders |
                                          ImGuiTableFlags_RowBg |
                                          ImGuiTableFlags_ScrollY;

            if (ImGui::BeginTable("texture_refs", 4, flags)) {
                ImGui::TableSetupColumn("size", ImGuiTableColumnFlags_WidthFixed, 90.0f);
                ImGui::TableSetupColumn("game refs", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableSetupColumn("all refs", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableSetupColumn("path", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                for (const auto& [path, weak] : m_textures) {
                    auto tex = weak.lock();
                    if (!tex) {
                        continue;
                    }
                    const int all = static_cast<int>(tex.use_count()) - 1;
                    const auto pit = pending_refs.find(tex.get());
                    const int pending = pit != pending_refs.end() ? pit->second : 0;
                    const int game = all - pending;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%ux%u", tex->get_width(), tex->get_height());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%d", game);
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%d", all);
                    ImGui::TableSetColumnIndex(3);
                    ImGui::TextUnformatted(tex->get_path().c_str());
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();
    }

    void Renderer::debug_sprite_queue() {
        if (m_cvar_log_sprite_queue) {
            debug::log("Renderer sprite order ({} draws):", m_sprite_draw_order.size());
            for (size_t i = 0; i < m_sprite_draw_order.size(); ++i) {
                const SpriteDrawData& draw = m_sprite_draws[m_sprite_draw_order[i]];
                const char* tex_path = draw.texture ? draw.texture->get_path().c_str() : "<unknown>";
                debug::log("  [{}] z={} shader={} tex={}", i, draw.z_index, draw.material.shader_id, tex_path);
            }
        }

        if (m_cvar_show_sprite_queue) {
            if (ImGui::Begin("Sprite Queue")) {
                ImGui::Text("Total: %zu", m_sprite_draw_order.size());
                const int columns = 4;
                const ImGuiTabBarFlags flags = ImGuiTableFlags_Borders |
                                               ImGuiTableFlags_RowBg |
                                               ImGuiTableFlags_ScrollY;

                if (ImGui::BeginTable("queue", columns, flags)) {
                    ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                    ImGui::TableSetupColumn("z_index", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    ImGui::TableSetupColumn("shader_id", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    ImGui::TableSetupColumn("texture", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableHeadersRow();

                    for (size_t i = 0; i < m_sprite_draw_order.size(); ++i) {
                        const SpriteDrawData& draw = m_sprite_draws[m_sprite_draw_order[i]];
                        const char* tex_path = draw.texture ? draw.texture->get_path().c_str() : "<unknown>";
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%zu", i);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%d", draw.z_index);
                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("%d", draw.material.shader_id);
                        ImGui::TableSetColumnIndex(3);
                        ImGui::TextUnformatted(tex_path);
                    }
                    ImGui::EndTable();
                }
            }
            ImGui::End();
        }
    }
}
