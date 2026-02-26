#include "console.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <ranges>

#include "app.h"

namespace hob {
    // Helper functions
    namespace {
        void trim_right_spaces(char* s) {
            assert(s != nullptr && "cstring is null");

            char* end = s + std::strlen(s);
            while (end > s && end[-1] == ' ') {
                --end;
            }

            *end = '\0';
        }

        unsigned char to_lower(unsigned char c) {
            return static_cast<unsigned char>(std::tolower(c));
        }

        bool equals_ci(std::string_view a, std::string_view b) {
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

        bool starts_with_ci(std::string_view s, std::string_view prefix) {
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

    // Console Backend
    ConsoleBackend::ConsoleBackend() {
        register_command("help", "List commands and cvars", [this](Args) { cmd_help(); });
        register_command("cmdlist", "List commands", [this](Args) { cmd_cmdlist(); });
        register_command("cvarlist", "List cvars", [this](Args) { cmd_cvarlist(); });
    }

    bool ConsoleBackend::register_command(std::string name, std::string help, CmdFunc func) {
        std::string key = key_of(name);
        Command command;
        command.name = std::move(name);
        command.help = std::move(help);
        command.func = std::move(func);

        bool registered = m_commands.emplace(std::move(key), std::move(command)).second;
        return registered;
    }

    bool ConsoleBackend::register_cvar(std::string name,
                                       std::string help,
                                       std::string default_value,
                                       CVarType type,
                                       CVarFlags flags,
                                       std::function<void(const CVar&)> on_changed) {
        std::string key = key_of(name);
        CVar cvar;
        cvar.name = std::move(name);
        cvar.help = std::move(help);
        cvar.type = type;
        cvar.flags = flags;
        cvar.value = default_value;
        cvar.default_value = std::move(default_value);
        cvar.on_changed = std::move(on_changed);

        bool registered = m_cvars.emplace(std::move(key), std::move(cvar)).second;
        return registered;
    }

    const ConsoleBackend::Command* ConsoleBackend::find_command(std::string_view name) const {
        const auto it = m_commands.find(key_of(name));
        if (it != m_commands.end()) {
            return &it->second;
        }

        return nullptr;
    }

    const ConsoleBackend::CVar* ConsoleBackend::find_cvar(std::string_view name) const {
        const auto it = m_cvars.find(key_of(name));
        if (it != m_cvars.end()) {
            return &it->second;
        }

        return nullptr;
    }

    std::vector<std::string_view> ConsoleBackend::get_candidates_for_prefix(std::string_view prefix) {
        std::vector<std::string_view> candidates;
        candidates.reserve(m_commands.size() + m_cvars.size());

        for (auto& [_, cmd] : m_commands) {
            if (starts_with_ci(cmd.name, prefix)) {
                candidates.push_back(cmd.name);
            }
        }

        for (auto& [_, cvar] : m_cvars)
            if (starts_with_ci(cvar.name, prefix))
                candidates.push_back(cvar.name);

        std::sort(candidates.begin(), candidates.end(), [](std::string_view a, std::string_view b) {
            return a < b;
        });

        return candidates;
    }

    void ConsoleBackend::execute_line(std::string_view line) {
        std::vector<std::string> tokens = tokenize(line);
        if (tokens.empty()) {
            return;
        }

        std::string key = key_of(tokens[0]);
        auto args = tokens | std::views::drop(1);

        // Command?
        if (const auto it = m_commands.find(key); it != m_commands.end()) {
            execute_command(it->second, args);
            return;
        }

        // CVar?
        if (const auto it = m_cvars.find(key); it != m_cvars.end()) {
            execute_cvar(it->second, args);
            return;
        }

        if (print_error) {
            print_error(std::format("Unknown command/cvar: '{}'", std::string(tokens[0])));
        }
    }

    void ConsoleBackend::execute_command(const Command& command, Args args) {
        command.func(args);
    }

    void ConsoleBackend::execute_cvar(CVar& cvar, Args args) {
        if (args.empty()) {
            if (print) {
                print(cvar.to_string());
            }

            return;
        }

        if (cvar.flags & ReadOnly) {
            if (print_error) {
                print_error(std::format("{} is read-only.", cvar.name));
            }

            return;
        }

        // Join args with spaces (so quotes are optional)
        std::string new_value = args[0];
        for (size_t i = 1; i < args.size(); ++i) {
            new_value.push_back(' ');
            new_value += args[i];
        }

        cvar.value = std::move(new_value);
        if (cvar.on_changed) {
            cvar.on_changed(cvar);
        }

        if (print) {
            print(std::format("{} set to '{}'", cvar.name, cvar.value));
        }
    }

    std::string ConsoleBackend::key_of(std::string_view s) {
        std::string key(s);
        std::transform(key.begin(), key.end(), key.begin(), to_lower);

        return key;
    }

    std::vector<std::string> ConsoleBackend::tokenize(std::string_view line) {
        std::vector<std::string> tokens;
        std::string current_token;
        bool in_quotes = false;

        auto push = [&] {
            if (!current_token.empty()) {
                tokens.push_back(current_token);
                current_token.clear();
            }
        };

        for (char ch : line) {
            if (ch == '"') {
                in_quotes = !in_quotes;
                continue;
            }

            if (!in_quotes && (ch == ' ' || ch == '\t')) {
                push();
                continue;
            }

            current_token.push_back(ch);
        }

        push();

        return tokens;
    }

    void ConsoleBackend::cmd_help() const {
        uint32_t max_name_size = 0;
        for (const auto& [_, cmd] : m_commands) {
            uint32_t name_size = cmd.name.size();
            if (name_size > max_name_size) {
                max_name_size = name_size;
            }
        }

        for (const auto& [_, cvar] : m_cvars) {
            uint32_t name_size = cvar.name.size();
            if (name_size > max_name_size) {
                max_name_size = name_size;
            }
        }

        cmd_cmdlist(max_name_size);
        cmd_cvarlist(max_name_size);
    }

    void ConsoleBackend::cmd_cmdlist(uint32_t indent) const {
        if (!print) {
            return;
        }

        print("Commands:");

        std::vector<std::string_view> names;
        names.reserve(m_commands.size());

        uint32_t max_name_size = indent;
        for (const auto& [_, cmd] : m_commands) {
            names.push_back(cmd.name);

            uint32_t name_size = cmd.name.size();
            if (name_size > max_name_size) {
                max_name_size = name_size;
            }
        }

        std::sort(names.begin(), names.end());

        for (const auto& name : names) {
            const Command* command = find_command(name);
            print(std::format("- {}", command->to_string(max_name_size)));
        }
    }

    void ConsoleBackend::cmd_cvarlist(uint32_t indent) const {
        if (!print) {
            return;
        }

        print("CVars:");

        std::vector<std::string_view> names;
        names.reserve(m_cvars.size());

        uint32_t max_name_size = indent;
        for (const auto& [_, cvar] : m_cvars) {
            names.push_back(cvar.name);

            uint32_t name_size = cvar.name.size();
            if (name_size > max_name_size) {
                max_name_size = name_size;
            }
        }

        std::sort(names.begin(), names.end());

        std::vector<std::string> dummy_args;
        for (const auto& name : names) {
            const CVar* cvar = find_cvar(name);
            print(std::format("- {}", cvar->to_string(max_name_size)));
        }
    }

    // Console Frontend
    Console::Console(const App& app)
        : m_app(app) {
        // Wire backend output into frontend log
        m_backend.print = [this](std::string_view s) {
            log("{}", s);
        };

        m_backend.print_error = [this](std::string_view s) {
            log_error("{}", s);
        };

        // Frontend-specific commands that touch UI state
        m_backend.register_command("clear", "Clear console log", [this](ConsoleBackend::Args) {
            clear_log();
        });

        m_backend.register_command("history", "Show the last 10 commands used", [this](ConsoleBackend::Args) {
            const size_t start = (m_history.size() >= 10) ? (m_history.size() - 10) : 0;
            for (size_t i = start; i < m_history.size(); ++i) {
                log("{:3}: {}", i, m_history[i]);
            }
        });
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
                execute_line(m_input_buffer);
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

    void Console::execute_line(std::string_view line) {
        std::string command_line(line);

        log("# {}", command_line);

        // history (frontend-owned)
        m_history_index = -1;
        const auto it = std::find_if(m_history.begin(), m_history.end(),
                                     [&](const std::string& h) {
                                         return equals_ci(h, command_line);
                                     });

        if (it != m_history.end()) {
            m_history.erase(it);
        }

        m_history.push_back(command_line);

        m_backend.execute_line(command_line);
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

                // Log candidates
                const std::string_view typed(word_start, static_cast<size_t>(word_end - word_start));
                std::vector<std::string_view> candidates = m_backend.get_candidates_for_prefix(typed);

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
                    // Find longest common prefix among candidates
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
}
