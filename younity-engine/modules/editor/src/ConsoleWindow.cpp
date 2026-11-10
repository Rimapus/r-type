/*
** EPITECH PROJECT, 2026
** younity-engine/modules/editor/src/ConsoleWindow
** File description:
** ConsoleWindow source file
*/

#include "younity/editor/ConsoleWindow.hpp"
#include "younity/logger/Formatter.hpp"
#include "younity/logger/ISink.hpp"
#include "younity/logger/LoggerRegistry.hpp"
#include "younity/render/DebugWindow.hpp"
#include "younity/tools/Console.hpp"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <deque>
#include <expected>
#include <imgui.h>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace younity::editor {

namespace {

constexpr std::string_view SINK_NAME = "editor.console";
constexpr std::size_t INPUT_CAPACITY = 256;

/**
 * @brief What a line of the console scrollback holds.
 */
enum class EntryKind {
    /** @brief A message written through the logger. */
    Log,
    /** @brief A line typed by the user, shown before it runs. */
    Command,
    /** @brief What a command returned. */
    Output,
    /** @brief Why a command line could not run. */
    Error,
};

/**
 * @brief One line of the console scrollback.
 */
struct Entry
{
    /** @brief Where the line came from. */
    EntryKind kind;
    /** @brief The logger level of a log line. */
    logger::Level level;
    /** @brief The text to draw. */
    std::string text;
};

/**
 * @brief A command or variable name and its description, used by the completion.
 */
struct CompletionCandidate
{
    /** @brief The name to insert. */
    std::string name;
    /** @brief What it does, shown in the completion hint. */
    std::string description;
};

bool isSpace(char character) noexcept
{
    return std::isspace(static_cast<unsigned char>(character)) != 0;
}

ImVec4 levelColor(logger::Level level)
{
    switch (level) {
    case logger::Level::Trace:
        return ImVec4(0.55F, 0.55F, 0.55F, 1.0F);
    case logger::Level::Debug:
        return ImVec4(0.70F, 0.70F, 0.70F, 1.0F);
    case logger::Level::Info:
        return ImGui::GetStyleColorVec4(ImGuiCol_Text);
    case logger::Level::Warn:
        return ImVec4(1.0F, 0.85F, 0.35F, 1.0F);
    case logger::Level::Error:
        return ImVec4(1.0F, 0.40F, 0.40F, 1.0F);
    case logger::Level::Critical:
        return ImVec4(1.0F, 0.25F, 0.65F, 1.0F);
    case logger::Level::Off:
        return ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
    }
    return ImGui::GetStyleColorVec4(ImGuiCol_Text);
}

/**
 * @brief Draw one line of the scrollback with the color of what it holds.
 *
 * @param entry The line to draw
 */
void drawEntry(const Entry &entry)
{
    switch (entry.kind) {
    case EntryKind::Log:
        ImGui::TextColored(levelColor(entry.level), "%s", entry.text.c_str());
        break;
    case EntryKind::Command:
        ImGui::TextColored(ImVec4(0.45F, 0.75F, 1.0F, 1.0F), "> %s", entry.text.c_str());
        break;
    case EntryKind::Output:
        ImGui::TextUnformatted(entry.text.c_str());
        break;
    case EntryKind::Error:
        ImGui::TextColored(ImVec4(1.0F, 0.40F, 0.40F, 1.0F), "error: %s", entry.text.c_str());
        break;
    }
}

/**
 * @brief Find the longest prefix shared by the names of the candidates.
 *
 * @param candidates The sorted completion candidates, not empty
 *
 * @return The longest prefix shared by every name.
 */
std::string commonNamePrefix(const std::vector<CompletionCandidate> &candidates)
{
    std::string prefix = candidates.front().name;

    for (const CompletionCandidate &candidate : candidates) {
        std::size_t length = 0;

        while (length < prefix.size() && length < candidate.name.size() && prefix[length] == candidate.name[length]) {
            ++length;
        }
        prefix.resize(length);
        if (prefix.empty()) {
            break;
        }
    }
    return prefix;
}

/**
 * @brief Describe the candidates of a completion in one line.
 *
 * @param candidates The matching commands and variables, not empty
 *
 * @return The names and descriptions, separated by semicolons.
 */
std::string completionHint(const std::vector<CompletionCandidate> &candidates)
{
    std::string hint = candidates.size() == 1 ? "" : "Matches: ";

    for (std::size_t index = 0; index < candidates.size(); ++index) {
        if (index != 0) {
            hint += "; ";
        }
        hint += candidates[index].name;
        if (candidates[index].description.empty() == false) {
            hint += ": ";
            hint += candidates[index].description;
        }
    }
    return hint;
}

} // namespace

class ConsoleWindow::Impl
{
public:
    /**
     * @brief The sink that adds the log messages to the scrollback of the window.
     */
    class LogSink;

    Impl(tools::Console &console, logger::LoggerRegistry &logs, std::size_t maxLines);
    ~Impl(void);

    void draw(const std::optional<Rectf> &area);

private:
    /**
     * @brief Add a line to the scrollback, dropping the oldest one when it is full.
     */
    void append(EntryKind kind, logger::Level level, std::string text);

    /**
     * @brief Add a logger record to the scrollback.
     */
    void appendLog(const logger::Record &record);

    /**
     * @brief Run the current input line and show its result.
     */
    void submit(void);

    /**
     * @brief Handle the ImGui callbacks of the input line.
     */
    static int inputCallback(ImGuiInputTextCallbackData *data);

    /**
     * @brief Handle a resize, a history key or a completion key.
     */
    int handleInput(ImGuiInputTextCallbackData *data);

    /**
     * @brief Recall the previous or the next command of the history.
     */
    void handleHistory(ImGuiInputTextCallbackData *data);

    /**
     * @brief Complete the first word of the input line.
     */
    void handleCompletion(ImGuiInputTextCallbackData *data);

    /**
     * @brief Replace the word under the cursor.
     */
    static void replaceWord(ImGuiInputTextCallbackData *data, std::size_t start, std::size_t end, std::string_view replacement);

    /**
     * @brief Replace the whole input line with a text.
     */
    static void replaceLine(ImGuiInputTextCallbackData *data, std::string_view text);

    tools::Console &_console;
    logger::LoggerRegistry &_logs;
    std::size_t _maxLines;
    std::mutex _mutex;
    std::deque<Entry> _entries;
    bool _scrollToBottom = false;
    std::string _input;
    std::string _completionHint;
    bool _completionUsed = false;
    bool _focusInput = false;
    std::vector<std::string> _history;
    std::size_t _historyIndex = 0;
};

class ConsoleWindow::Impl::LogSink : public logger::ISink
{
public:
    /**
     * @brief Create a sink forwarding the records to a console window.
     *
     * @param owner The window whose scrollback receives the records
     */
    explicit LogSink(Impl &owner)
        : _owner(owner) {}

    void write(const logger::Record &record) override
    {
        this->_owner.appendLog(record);
    }

private:
    Impl &_owner;
};

ConsoleWindow::Impl::Impl(tools::Console &console, logger::LoggerRegistry &logs, std::size_t maxLines)
    : _console(console), _logs(logs), _maxLines(maxLines)
{
    if (this->_maxLines == 0) {
        throw std::invalid_argument("younity::editor::ConsoleWindow: the scrollback needs at least one line");
    }
    if (this->_logs.hasSink(SINK_NAME)) {
        throw std::invalid_argument("younity::editor::ConsoleWindow: this logger registry already has a console window");
    }
    this->_input.reserve(INPUT_CAPACITY);
    this->_logs.addSink(std::string(SINK_NAME), std::make_unique<LogSink>(*this));
}

ConsoleWindow::Impl::~Impl(void)
{
    this->_logs.removeSink(SINK_NAME);
}

void ConsoleWindow::Impl::draw(const std::optional<Rectf> &area)
{
    ImGui::SetNextWindowSize(ImVec2(720.0F, 420.0F), ImGuiCond_FirstUseEver);
    if (!render::beginDebugWindow(CONSOLE_TITLE, area)) {
        ImGui::End();
        return;
    }

    std::vector<Entry> entries;
    bool scrollToBottom = false;
    {
        const std::lock_guard lock(this->_mutex);

        entries.assign(this->_entries.begin(), this->_entries.end());
        scrollToBottom = this->_scrollToBottom;
        this->_scrollToBottom = false;
    }

    const float hintHeight = this->_completionHint.empty() ? 0.0F : ImGui::GetTextLineHeightWithSpacing();
    const float footerHeight = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing() + hintHeight;

    ImGui::BeginChild("##console.scrollback", ImVec2(0.0F, -footerHeight), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
    for (const Entry &entry : entries) {
        drawEntry(entry);
    }
    if (scrollToBottom) {
        ImGui::SetScrollHereY(1.0F);
    }
    ImGui::EndChild();
    ImGui::Separator();

    if (this->_focusInput) {
        ImGui::SetKeyboardFocusHere();
        this->_focusInput = false;
    }
    ImGui::SetNextItemWidth(-1.0F);
    const std::string previousInput = this->_input;
    const bool submitted = ImGui::InputTextWithHint(
        "##console.input",
        "Enter a command or variable",
        this->_input.data(),
        this->_input.capacity() + 1,
        ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_CallbackCompletion |
            ImGuiInputTextFlags_CallbackHistory,
        &Impl::inputCallback,
        this);
    const bool completionUsed = this->_completionUsed;

    this->_completionUsed = false;
    if (submitted) {
        this->submit();
    } else if (completionUsed == false && this->_input != previousInput) {
        this->_completionHint.clear();
    }
    if (this->_completionHint.empty() == false) {
        ImGui::PushTextWrapPos(0.0F);
        ImGui::TextDisabled("%s", this->_completionHint.c_str());
        ImGui::PopTextWrapPos();
    }
    ImGui::End();
}

void ConsoleWindow::Impl::append(EntryKind kind, logger::Level level, std::string text)
{
    const std::lock_guard lock(this->_mutex);

    this->_entries.push_back(Entry{kind, level, std::move(text)});
    while (this->_entries.size() > this->_maxLines) {
        this->_entries.pop_front();
    }
    this->_scrollToBottom = true;
}

void ConsoleWindow::Impl::appendLog(const logger::Record &record)
{
    this->append(EntryKind::Log, record.getLevel(), logger::formatRecord(record));
}

void ConsoleWindow::Impl::submit(void)
{
    const std::string line = this->_input;
    const bool isBlank = std::all_of(line.begin(), line.end(), isSpace);

    if (isBlank) {
        this->_input.clear();
        this->_completionHint.clear();
        this->_focusInput = true;
        return;
    }
    this->append(EntryKind::Command, logger::Level::Info, line);
    const std::expected<std::string, tools::ConsoleError> result = this->_console.execute(line);

    if (result.has_value()) {
        if (result->empty() == false) {
            this->append(EntryKind::Output, logger::Level::Info, *result);
        }
    } else {
        this->append(EntryKind::Error, logger::Level::Error, result.error().message);
    }
    this->_history.push_back(line);
    this->_historyIndex = this->_history.size();
    this->_input.clear();
    this->_completionHint.clear();
    this->_focusInput = true;
}

int ConsoleWindow::Impl::inputCallback(ImGuiInputTextCallbackData *data)
{
    auto *window = static_cast<Impl *>(data->UserData);

    return window->handleInput(data);
}

int ConsoleWindow::Impl::handleInput(ImGuiInputTextCallbackData *data)
{
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        this->_input.resize(static_cast<std::size_t>(data->BufTextLen));
        data->Buf = this->_input.data();
        return 0;
    }
    if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory) {
        this->handleHistory(data);
    } else if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion) {
        this->handleCompletion(data);
    }
    return 0;
}

void ConsoleWindow::Impl::handleHistory(ImGuiInputTextCallbackData *data)
{
    if (this->_history.empty()) {
        return;
    }
    if (data->EventKey == ImGuiKey_UpArrow) {
        if (this->_historyIndex > 0) {
            --this->_historyIndex;
        }
        this->replaceLine(data, this->_history[this->_historyIndex]);
    } else if (data->EventKey == ImGuiKey_DownArrow) {
        if (this->_historyIndex < this->_history.size()) {
            ++this->_historyIndex;
        }
        this->replaceLine(data, this->_historyIndex == this->_history.size() ? std::string_view{} : this->_history[this->_historyIndex]);
    }
    this->_completionHint.clear();
}

void ConsoleWindow::Impl::handleCompletion(ImGuiInputTextCallbackData *data)
{
    const std::string_view buffer(data->Buf, static_cast<std::size_t>(data->BufTextLen));
    const int cursorPosition = std::max(data->CursorPos, 0);
    const std::size_t cursor = std::min(static_cast<std::size_t>(cursorPosition), buffer.size());
    std::size_t start = cursor;
    std::size_t end = cursor;

    while (start > 0 && isSpace(buffer[start - 1]) == false) {
        --start;
    }
    while (end < buffer.size() && isSpace(buffer[end]) == false) {
        ++end;
    }
    for (std::size_t index = 0; index < start; ++index) {
        if (isSpace(buffer[index]) == false) {
            this->_completionHint.clear();
            return;
        }
    }
    const std::string_view prefix = buffer.substr(start, end - start);

    if (prefix.empty()) {
        this->_completionHint.clear();
        return;
    }

    std::vector<CompletionCandidate> candidates;

    for (const tools::Variable &variable : this->_console.getVariables()) {
        if (std::string_view(variable.name).starts_with(prefix)) {
            candidates.push_back(CompletionCandidate{variable.name, variable.description});
        }
    }
    for (const tools::CommandInfo &command : this->_console.getCommands()) {
        if (std::string_view(command.name).starts_with(prefix)) {
            candidates.push_back(CompletionCandidate{command.name, command.description});
        }
    }
    if (tools::HELP_COMMAND.starts_with(prefix)) {
        candidates.push_back(CompletionCandidate{std::string(tools::HELP_COMMAND), "List the commands and the variables"});
    }
    std::sort(candidates.begin(), candidates.end(), [](const CompletionCandidate &left, const CompletionCandidate &right) {
        return left.name < right.name;
    });
    if (candidates.empty()) {
        this->_completionHint = "No command or variable starts with " + std::string(prefix);
        this->_completionUsed = true;
        return;
    }
    if (candidates.size() == 1) {
        this->replaceWord(data, start, end, candidates.front().name);
    } else {
        const std::string common = commonNamePrefix(candidates);

        if (common.size() > prefix.size()) {
            this->replaceWord(data, start, end, common);
        }
    }
    this->_completionHint = completionHint(candidates);
    this->_completionUsed = true;
}

void ConsoleWindow::Impl::replaceWord(ImGuiInputTextCallbackData *data, std::size_t start, std::size_t end, std::string_view replacement)
{
    const std::string text(replacement);

    data->DeleteChars(static_cast<int>(start), static_cast<int>(end - start));
    data->InsertChars(static_cast<int>(start), text.c_str());
    data->CursorPos = static_cast<int>(start + text.size());
    data->SelectionStart = data->CursorPos;
    data->SelectionEnd = data->CursorPos;
}

void ConsoleWindow::Impl::replaceLine(ImGuiInputTextCallbackData *data, std::string_view text)
{
    const std::string line(text);

    data->DeleteChars(0, data->BufTextLen);
    if (line.empty() == false) {
        data->InsertChars(0, line.c_str());
    }
    data->CursorPos = data->BufTextLen;
    data->SelectionStart = data->CursorPos;
    data->SelectionEnd = data->CursorPos;
}

ConsoleWindow::ConsoleWindow(tools::Console &console, logger::LoggerRegistry &logs, std::size_t maxLines)
    : _impl(std::make_unique<Impl>(console, logs, maxLines)) {}

ConsoleWindow::~ConsoleWindow(void) = default;

void ConsoleWindow::draw(const std::optional<Rectf> &area)
{
    this->_impl->draw(area);
}

} // namespace younity::editor
