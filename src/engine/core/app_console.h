#ifndef HOB_ENGINE_APP_CONSOLE_H
#define HOB_ENGINE_APP_CONSOLE_H

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>

namespace hob {
    class AppConsole {
        static constexpr size_t INPUT_BUFF_SIZE = 256;

        char m_input_buff[INPUT_BUFF_SIZE] = {};
        std::vector<std::string> m_items;

        bool m_scroll_to_bottom = false;

        std::vector<std::string> m_history;
        int m_history_pos = -1; // -1: new line, [0..history-1] browsing history.

        std::vector<std::string> m_commands;

    public:
        AppConsole() {
            clear_log();
            m_commands = {"help", "history", "clear"};
        }

        ~AppConsole() = default;

        void clear_log() {
            m_items.clear();
            m_scroll_to_bottom = true;
        }

        void add_log(const char* fmt, ...) IM_FMTARGS(2) {
            char buff[1024];

            va_list args;
            va_start(args, fmt);
            std::vsnprintf(buff, IM_ARRAYSIZE(buff), fmt, args);
            buff[IM_ARRAYSIZE(buff) - 1] = 0;
            va_end(args);

            m_items.emplace_back(buff);
            m_scroll_to_bottom = true;
        }

        void draw(const char* title, bool* p_open) {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(1152, 324));
            if (!ImGui::Begin(title, p_open, ImGuiWindowFlags_NoResize)) {
                ImGui::End();
                return;
            }

            ImGui::TextWrapped("Enter 'help' for help, press TAB to use text completion.");

            ImGui::SameLine();
            ImGui::Text("|");

            ImGui::SameLine();
            bool copy_to_clipboard = ImGui::SmallButton("Copy");

            ImGui::SameLine();
            if (ImGui::SmallButton("Scroll to bottom")) {
                m_scroll_to_bottom = true;
            }

            ImGui::Separator();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            static ImGuiTextFilter filter;
            filter.Draw("Filter", 1075);
            ImGui::PopStyleVar();

            ImGui::Separator();

            ImGui::BeginChild("ScrollingRegion",
                              ImVec2(0, -ImGui::GetFrameHeightWithSpacing()),
                              false,
                              ImGuiWindowFlags_HorizontalScrollbar);
            {
                if (ImGui::BeginPopupContextWindow()) {
                    if (ImGui::Selectable("Clear")) {
                        clear_log();
                    }

                    ImGui::EndPopup();
                }

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));

                if (copy_to_clipboard) {
                    ImGui::LogToClipboard();
                }

                for (const std::string& s : m_items) {
                    const char* item = s.c_str();
                    if (!filter.PassFilter(item)) {
                        continue;
                    }

                    ImVec4 col(1, 1, 1, 1);
                    if (std::strstr(item, "[error]")) {
                        col = ImColor(1.0f, 0.4f, 0.4f, 1.0f);
                    }
                    else if (std::strncmp(item, "# ", 2) == 0) {
                        col = ImColor(1.0f, 0.78f, 0.58f, 1.0f);
                    }

                    ImGui::PushStyleColor(ImGuiCol_Text, col);
                    ImGui::TextUnformatted(item);
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
            ImGui::SetNextItemWidth(1075);
            if (ImGui::InputText("Input",
                                 m_input_buff,
                                 IM_ARRAYSIZE(m_input_buff),
                                 ImGuiInputTextFlags_EnterReturnsTrue |
                                 ImGuiInputTextFlags_CallbackCompletion |
                                 ImGuiInputTextFlags_CallbackHistory,
                                 &text_edit_callback_stub,
                                 this)) {
                trim_right_spaces(m_input_buff);
                if (m_input_buff[0]) {
                    exec_command(m_input_buff);
                }

                m_input_buff[0] = '\0';
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

    private:
        static void trim_right_spaces(char* s) {
            if (!s) {
                return;
            }

            char* end = s + std::strlen(s);
            while (end > s && end[-1] == ' ') {
                --end;
            }

            *end = '\0';
        }

        static unsigned char lower_uc(unsigned char c) {
            return static_cast<unsigned char>(std::tolower(c));
        }

        static bool iequals(std::string_view a, std::string_view b) {
            if (a.size() != b.size()) {
                return false;
            }

            for (size_t i = 0; i < a.size(); ++i) {
                if (lower_uc(static_cast<unsigned char>(a[i])) != lower_uc(static_cast<unsigned char>(b[i]))) {
                    return false;
                }
            }

            return true;
        }

        static bool istarts_with(std::string_view s, std::string_view prefix) {
            if (prefix.size() > s.size()) {
                return false;
            }

            for (size_t i = 0; i < prefix.size(); ++i) {
                if (lower_uc(static_cast<unsigned char>(s[i])) != lower_uc(static_cast<unsigned char>(prefix[i]))) {
                    return false;
                }
            }

            return true;
        }

        void exec_command(const char* command_line_cstr) {
            std::string command_line = command_line_cstr;

            add_log("# %s", command_line.c_str());

            // Insert into history: remove duplicates, push to back
            m_history_pos = -1;
            auto it = std::find_if(m_history.begin(), m_history.end(),
                                   [&](const std::string& h) {
                                       return iequals(h, command_line);
                                   });
            if (it != m_history.end()) {
                m_history.erase(it);
            }

            m_history.push_back(command_line);

            // Process command
            if (iequals(command_line, "clear")) {
                clear_log();
            }
            else if (iequals(command_line, "help")) {
                add_log("Commands:");
                for (const auto& cmd : m_commands) {
                    add_log("- %s", cmd.c_str());
                }
            }
            else if (iequals(command_line, "history")) {
                const size_t start = m_history.size() >= 10 ? m_history.size() - 10 : 0;
                for (size_t i = start; i < m_history.size(); ++i)
                    add_log("%3d: %s", static_cast<int>(i), m_history[i].c_str());
            }
            else {
                add_log("Unknown command: '%s'", command_line.c_str());
            }
        }

        static int text_edit_callback_stub(ImGuiInputTextCallbackData* data) {
            auto* console = static_cast<AppConsole*>(data->UserData);
            return console->text_edit_callback(data);
        }

        int text_edit_callback(ImGuiInputTextCallbackData* data) {
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

                    std::string_view typed(word_start, static_cast<size_t>(word_end - word_start));

                    // Build candidates
                    std::vector<std::string_view> candidates;
                    candidates.reserve(m_commands.size());
                    for (const auto& cmd : m_commands) {
                        if (istarts_with(cmd, typed)) {
                            candidates.emplace_back(cmd);
                        }
                    }

                    if (candidates.empty()) {
                        add_log("No match for \"%.*s\"!", static_cast<int>(typed.size()), typed.data());
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

                                const int c = std::toupper(static_cast<unsigned char>(candidates[i][match_len]));
                                if (i == 0) {
                                    next_ch = c;
                                }
                                else if (c != next_ch) {
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

                        add_log("Possible matches:");
                        for (auto c : candidates) {
                            add_log("- %.*s", static_cast<int>(c.size()), c.data());
                        }
                    }

                    break;
                }

                case ImGuiInputTextFlags_CallbackHistory: {
                    const int prev_history_pos = m_history_pos;

                    if (data->EventKey == ImGuiKey_UpArrow) {
                        if (m_history_pos == -1) {
                            m_history_pos = static_cast<int>(m_history.size() - 1);
                        }
                        else if (m_history_pos > 0) {
                            --m_history_pos;
                        }
                    }
                    else if (data->EventKey == ImGuiKey_DownArrow) {
                        if (m_history_pos != -1) {
                            if (++m_history_pos >= static_cast<int>(m_history.size())) {
                                m_history_pos = -1;
                            }
                        }
                    }

                    if (prev_history_pos != m_history_pos) {
                        const char* history_str = (m_history_pos >= 0) ? m_history[m_history_pos].c_str() : "";
                        data->DeleteChars(0, data->BufTextLen);
                        data->InsertChars(0, history_str);
                    }
                    break;
                }

                default: {
                    add_log("Invalid event flag");
                    return 1;
                };
            }

            return 0;
        }
    };

}

#endif // HOB_ENGINE_APP_CONSOLE_H
