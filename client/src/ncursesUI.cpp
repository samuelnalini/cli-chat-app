#include "headers/ncursesUI.hpp"
#include <ncurses.h>

NcursesUI::NcursesUI()
    : m_msgWin(nullptr)
    , m_inputWin(nullptr)
{}

NcursesUI::~NcursesUI()
{
    Cleanup();
}

void NcursesUI::Init()
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(1);

    getmaxyx(stdscr, m_rows, m_cols);

    m_msgWin = newwin(m_rows - INPUT_HEIGHT, m_cols, 0, 0) ;
    m_inputWin = newwin(INPUT_HEIGHT, m_cols, m_rows - INPUT_HEIGHT, 0);

    scrollok(m_msgWin, TRUE);
    box(m_inputWin, 0, 0);

    keypad(m_inputWin, TRUE);
    nodelay(m_inputWin, TRUE);

    wrefresh(m_msgWin);
    wrefresh(m_inputWin);
    running = true;
}

void NcursesUI::Cleanup()
{
    running = false;

    if (m_msgWin)
        delwin(m_msgWin);

    if (m_inputWin)
        delwin(m_inputWin);

    endwin();
}

bool NcursesUI::GetInputChar(int& ch)
{
    if (!running)
        return false;

    std::lock_guard<std::mutex> lock(m_mutex);
    ch = wgetch(m_inputWin);
    
    if (ch == ERR)
        return false;

    return true;
}

void NcursesUI::PushMessage(const std::string& msg)
{
    if (!running)
        return;

    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_msgQueue.push(msg);
}

void NcursesUI::PrintBufferedMessages()
{
    if (!running)
        return;

    std::unique_lock<std::mutex> lock(m_mutex, std::defer_lock);
    std::unique_lock<std::mutex> lock2(m_queueMutex, std::defer_lock);

    std::lock(lock, lock2);

    bool printed{ false };

    while (!m_msgQueue.empty())
    {
        std::string msg{ m_msgQueue.front() };
        m_msgQueue.pop();
        wprintw(m_msgWin, "%s\n", msg.c_str());
        printed = true;
    }

    if (printed)
        wrefresh(m_msgWin);
}


void NcursesUI::RedrawInputLine(const std::string& prompt, const std::string& inputBuffer)
{
    if (!running)
        return;

    std::lock_guard<std::mutex> lock(m_mutex);

    werase(m_inputWin);
    box(m_inputWin, 0, 0);

    mvwprintw(m_inputWin, 1, 1, "%s%s", prompt.c_str(), inputBuffer.c_str());

    wmove(m_inputWin, 1, 1 + (int)prompt.length() + (int)inputBuffer.length());

    wrefresh(m_inputWin);
}

std::string NcursesUI::PromptInput(const std::string& prompt)
{
    if (!running)
        return "\0";

    std::lock_guard<std::mutex> lock(m_mutex);
    std::string input;
    int startX = 1 + prompt.length();

    werase(m_inputWin);
    box(m_inputWin, 0, 0);
    mvwprintw(m_inputWin, 1, 1, "%s", prompt.c_str());
    wmove(m_inputWin, 1, startX);
    wrefresh(m_inputWin);

    int ch;
    while ((ch = wgetch(m_inputWin)) != '\n')
    {
        if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b')
        {
            if (!input.empty())
            {
                input.pop_back();
                mvwprintw(m_inputWin, 1, startX, std::string(m_cols - startX - 1, ' ').c_str());
                mvwprintw(m_inputWin, 1, startX, "%s", input.c_str());
                wmove(m_inputWin, 1, startX + input.length());
                wrefresh(m_msgWin);
            }
        } else if (isprint(ch))
        {
            input.push_back(ch);
            mvwprintw(m_inputWin, 1, startX, "%s", input.c_str());
        }

        wrefresh(m_inputWin);
    }

    return input;
}

