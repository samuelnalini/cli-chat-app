#include "headers/ncursesUI.hpp"
#include "utf8.h"

#include <ncurses.h>
#include <locale.h>
#include <optional>

NcursesUI::NcursesUI(Debugger& debugger)
    : m_msgWin(nullptr)
    , m_inputWin(nullptr)
    , m_debugger(&debugger)
{}

NcursesUI::~NcursesUI() {}

void NcursesUI::Init()
{
    m_debugger->Log("Initializing UI...");

    setlocale(LC_ALL, "");
    start_color();
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

    init_pair(COLOR_RED, COLOR_BLACK, 1);
    init_pair(COLOR_GREEN, COLOR_BLACK, 2);
    init_pair(COLOR_BLUE, COLOR_BLACK, 3);

    wrefresh(m_msgWin);
    wrefresh(m_inputWin);
    running = true;

    m_debugger->Log("UI Initialized");
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

bool NcursesUI::GetInputChar(wint_t& ch)
{
    if (!running)
        return false;

    std::lock_guard<std::mutex> lock(m_mutex);


    int result{ wget_wch(m_inputWin, &ch) };

    return result != ERR;
}

std::string to_utf8(const std::wstring& wstr)
{
    std::string result;
    utf8::utf32to8(wstr.begin(), wstr.end(), std::back_inserter(result));
    return result;
}

std::wstring from_utf8(const std::string& str)
{
    std::wstring out;
    utf8::utf8to32(str.begin(), str.end(), std::back_inserter(out));
    return out;
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

        std::wstring wmsg{ from_utf8(msg) };
        wmsg.push_back(L'\n');

        mvwaddwstr(m_msgWin, getcury(m_msgWin), 0, wmsg.c_str());
        printed = true;
    }

    if (printed)
    {
        wrefresh(m_msgWin);
        wrefresh(m_inputWin);
    }
}


void NcursesUI::RedrawInputLine(const std::string& prompt, const std::wstring& inputBuffer)
{
    if (!running)
        return;

    std::lock_guard<std::mutex> lock(m_mutex);

    werase(m_inputWin);
    box(m_inputWin, 0, 0);

    mvwprintw(m_inputWin, 1, 1, "%s", prompt.c_str());
    int startX{ 1 + (int)prompt.length() };

    mvwaddwstr(m_inputWin, 1, startX, inputBuffer.c_str());
    wmove(m_inputWin, 1, startX + (int)inputBuffer.length());

    wrefresh(m_inputWin);
}

std::optional<std::string> NcursesUI::PromptInput(const std::string& prompt)
{
    if (!running)
        return std::nullopt;

    std::wstring input;
    RedrawInputLine(prompt, input);

    wint_t ch{};
    while (true)
    {
        int result{ wget_wch(m_inputWin, &ch) };

        if (result == ERR)
            continue;

        if (ch == L'\n')
            break;

        if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b')
        {
            if (!input.empty())
            {
                input.pop_back();
            }
        } else if (iswprint(ch))
        {
            input.push_back(ch);
        }

        RedrawInputLine(prompt, input);
    }

    return to_utf8(input);
}
