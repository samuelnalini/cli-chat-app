#pragma once

#include <ncurses.h>
#include <string>
#include <mutex>
#include <queue>
#include <atomic>

class NcursesUI
{
public:
    NcursesUI();
    ~NcursesUI();

    void Init();
    void Cleanup();

    bool GetInputChar(int& ch);
    void PushMessage(const std::string& msg);
    void PrintBufferedMessages();
    void RedrawInputLine(const std::string& prompt, const std::string& inputBuffer);

    std::string PromptInput(const std::string& prompt);
private:
    WINDOW* m_msgWin;
    WINDOW* m_inputWin;
    int m_rows, m_cols;
    std::mutex m_mutex;

    std::queue<std::string> m_msgQueue;
    std::mutex m_queueMutex;
    std::atomic<bool> running{ false };

    static const int INPUT_HEIGHT = 3;
};
