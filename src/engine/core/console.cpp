#include "console.h"

#include <algorithm>
#include <cassert>

#include "app.h"

namespace hob {
    Console::Console(const App& app) :
        m_app(app) {
        m_commands = {"help", "history", "clear"};
    }

    bool Console::is_open() const {
        return m_open;
    }

    void Console::toggle_open() {
        m_open = !m_open;
        clear_input_buffer();
    }

    void Console::clear_input_buffer() {
        m_input_buffer[0] = '\0';
    }

    void Console::clear_log() {
        m_log.clear();
        m_scroll_to_bottom = true;
    }

    void Console::render() {
        const GraphicsConfig& graphics_config = m_app.get_config().graphics_config;
        const float width = static_cast<float>(graphics_config.logical_resolution_width);
        const float height = static_cast<float>(graphics_config.logical_resolution_height) * 0.5f;

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(width, height));
        if (!ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoResize)) {
            ImGui::End();
            return;
        }

        ImGui::TextWrapped("Enter 'help' for help, press TAB to use text completion.");

        ImGui::SameLine();
        ImGui::Text("|");

        ImGui::SameLine();
        const bool copy_to_clipboard = ImGui::SmallButton("Copy");

        ImGui::SameLine();
        if (ImGui::SmallButton("Scroll to bottom")) {
            m_scroll_to_bottom = true;
        }

        ImGui::Separator();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        static ImGuiTextFilter filter;
        filter.Draw("Filter", width - 75.0f);
        ImGui::PopStyleVar();

        ImGui::Separator();

        ImGui::BeginChild("ScrollingRegion",
                          ImVec2(0, -ImGui::GetFrameHeightWithSpacing()),
                          false,
                          ImGuiWindowFlags_HorizontalScrollbar);
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));

            if (copy_to_clipboard) {
                ImGui::LogToClipboard();
            }

            for (const std::string& s : m_log) {
                const char* item_cstr = s.c_str();
                if (!filter.PassFilter(item_cstr)) {
                    continue;
                }

                const std::string_view item{s};

                ImVec4 col = LOG_ENTRY_COLOR_WHITE;
                if (item.starts_with("[error]")) {
                    col = LOG_ENTRY_COLOR_RED;
                }
                else if (item.starts_with("# ")) {
                    col = LOG_ENTRY_COLOR_ORANGE;
                }

                ImGui::PushStyleColor(ImGuiCol_Text, col);
                ImGui::TextUnformatted(item_cstr);
                ImGui::PopStyleColor();
            }

            if (copy_to_clipboard) {
                ImGui::LogFinish();
            }

            if (m_scroll_to_bottom) {
                ImGui::SetScrollHereY(1.0f);
            }

            m_scroll_to_bottom = false;

            ImGui::PopStyleVar();
        }
        ImGui::EndChild();

        ImGui::Separator();

        // Command-line
        ImGui::SetNextItemWidth(width - 75.0f);
        if (ImGui::InputText("Input",
                             m_input_buffer,
                             IM_ARRAYSIZE(m_input_buffer),
                             ImGuiInputTextFlags_EnterReturnsTrue |
                             ImGuiInputTextFlags_CallbackCompletion |
                             ImGuiInputTextFlags_CallbackHistory,
                             &text_edit_callback_stub,
                             this)) {
            trim_right_spaces(m_input_buffer);

            if (m_input_buffer[0] != '\0') {
                execute_command(m_input_buffer);
            }

            clear_input_buffer();
        }

        // Keep auto-focus on the input box
        if (ImGui::IsItemHovered() ||
            (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
             !ImGui::IsAnyItemActive() &&
             !ImGui::IsMouseClicked(0))) {
            ImGui::SetKeyboardFocusHere(-1);
        }

        ImGui::End();
    }

    void Console::execute_command(std::string_view command_line_sv) {
        std::string command_line = std::string(command_line_sv);

        log("# {}", command_line);

        // Insert into history: remove duplicates, push to back
        m_history_index = -1;
        auto it = std::find_if(m_history.begin(), m_history.end(),
                               [&](const std::string& h) {
                                   return equals_ci(h, command_line);
                               });

        if (it != m_history.end()) {
            m_history.erase(it);
        }

        m_history.push_back(command_line);

        // Process command
        if (equals_ci(command_line, "clear")) {
            clear_log();
        }
        else if (equals_ci(command_line, "help")) {
            log("Commands:");
            for (const auto& cmd : m_commands) {
                log("- {}", cmd);
            }
        }
        else if (equals_ci(command_line, "history")) {
            const size_t start = (m_history.size() >= 10) ? (m_history.size() - 10) : 0;
            for (size_t i = start; i < m_history.size(); ++i) {
                log("{:3}: {}", static_cast<int>(i), m_history[i]);
            }
        }
        else {
            log_error("Unknown command: '{}'", command_line);
        }
    }

    int Console::text_edit_callback_stub(ImGuiInputTextCallbackData* data) {
        auto* console = static_cast<Console*>(data->UserData);
        return console->text_edit_callback(data);
    }

    int Console::text_edit_callback(ImGuiInputTextCallbackData* data) {
        switch (data->EventFlag) {
            case ImGuiInputTextFlags_CallbackCompletion: {
                // Locate beginning of current word
                const char* word_end = data->Buf + data->CursorPos;
                const char* word_start = word_end;
                while (word_start > data->Buf) {
                    const char c = word_start[-1];
                    if (c == ' ' || c == '\t' || c == ',' || c == ';') {
                        break;
                    }

                    --word_start;
                }

                const std::string_view typed(word_start, static_cast<size_t>(word_end - word_start));

                // Build candidates
                std::vector<std::string_view> candidates;
                candidates.reserve(m_commands.size());
                for (const auto& cmd : m_commands) {
                    if (starts_with_ci(cmd, typed)) {
                        candidates.emplace_back(cmd);
                    }
                }

                if (candidates.empty()) {
                    log("No match for '{}'!", typed);
                }
                else if (candidates.size() == 1) {
                    // Replace with single candidate + space
                    data->DeleteChars(static_cast<int>(word_start - data->Buf),
                                      static_cast<int>(word_end - word_start));

                    data->InsertChars(data->CursorPos,
                                      candidates[0].data(),
                                      candidates[0].data() + candidates[0].size());

                    data->InsertChars(data->CursorPos, " ");
                }
                else {
                    // Find longest common prefix among candidates (case-insensitive)
                    size_t match_len = typed.size();
                    while (true) {
                        bool all_match = true;
                        int next_ch = -1;
                        for (size_t i = 0; i < candidates.size() && all_match; ++i) {
                            if (match_len >= candidates[i].size()) {
                                all_match = false;
                                break;
                            }

                            const int ch = std::toupper(static_cast<unsigned char>(candidates[i][match_len]));
                            if (i == 0) {
                                next_ch = ch;
                            }
                            else if (ch != next_ch) {
                                all_match = false;
                            }
                        }

                        if (!all_match) {
                            break;
                        }

                        ++match_len;
                    }

                    if (match_len > 0) {
                        data->DeleteChars(static_cast<int>(word_start - data->Buf),
                                          static_cast<int>(word_end - word_start));

                        data->InsertChars(data->CursorPos,
                                          candidates[0].data(),
                                          candidates[0].data() + match_len);
                    }

                    log("Possible matches:");
                    for (auto c : candidates) {
                        log("- {}", c);
                    }
                }

                break;
            }

            case ImGuiInputTextFlags_CallbackHistory: {
                const int prev_history_pos = m_history_index;

                if (data->EventKey == ImGuiKey_UpArrow) {
                    if (m_history_index == -1) {
                        m_history_index = static_cast<int>(m_history.size() - 1);
                    }
                    else if (m_history_index > 0) {
                        --m_history_index;
                    }
                }
                else if (data->EventKey == ImGuiKey_DownArrow) {
                    if (m_history_index != -1) {
                        if (++m_history_index >= static_cast<int>(m_history.size())) {
                            m_history_index = -1;
                        }
                    }
                }

                if (prev_history_pos != m_history_index) {
                    const char* history_str = (m_history_index >= 0) ? m_history[m_history_index].c_str() : "";
                    data->DeleteChars(0, data->BufTextLen);
                    data->InsertChars(0, history_str);
                }
                break;
            }

            default: {
                log_error("Invalid event flag");
                return 1;
            };
        }

        return 0;
    }

    void Console::trim_right_spaces(char* s) {
        assert(s != nullptr && "cstring is null");

        char* end = s + std::strlen(s);
        while (end > s && end[-1] == ' ') {
            --end;
        }

        *end = '\0';
    }

    unsigned char Console::to_upper(unsigned char c) {
        return static_cast<unsigned char>(std::toupper(c));
    }

    unsigned char Console::to_lower(unsigned char c) {
        return static_cast<unsigned char>(std::tolower(c));
    }

    bool Console::equals_ci(std::string_view a, std::string_view b) {
        if (a.size() != b.size()) {
            return false;
        }

        for (size_t i = 0; i < a.size(); ++i) {
            if (to_lower(static_cast<unsigned char>(a[i])) != to_lower(static_cast<unsigned char>(b[i]))) {
                return false;
            }
        }

        return true;
    }

    bool Console::starts_with_ci(std::string_view s, std::string_view prefix) {
        if (prefix.size() > s.size()) {
            return false;
        }

        for (size_t i = 0; i < prefix.size(); ++i) {
            if (to_lower(static_cast<unsigned char>(s[i])) != to_lower(static_cast<unsigned char>(prefix[i]))) {
                return false;
            }
        }

        return true;
    }
}
