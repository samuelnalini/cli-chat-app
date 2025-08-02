#pragma once

#include "debug.hpp"

#include <ncurses.h>
#include <string>
#include <mutex>
#include <queue>
#include <atomic>
#include <optional>

class NcursesUI
{
public:
    NcursesUI(Debugger& debugger);
    ~NcursesUI();

    void Init();
    void Cleanup();

    bool GetInputChar(int& ch);
    void PushMessage(const std::string& msg);
    void PrintBufferedMessages();
    void RedrawInputLine(const std::string& prompt, const std::string& inputBuffer);

    std::optional<std::string> PromptInput(const std::string& prompt);

public:
    std::atomic<bool> running{ false };

private:
    WINDOW* m_msgWin;
    WINDOW* m_inputWin;
    int m_rows, m_cols;
    std::mutex m_mutex;

    std::queue<std::string> m_msgQueue;
    std::mutex m_queueMutex;

    Debugger* m_debugger;

    static const int INPUT_HEIGHT = 3;
};
