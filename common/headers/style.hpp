#pragma once

#include <string>
#include <sstream>
#include <unordered_map>

class Style
{
public:

    enum class STYLE_TYPE
    {
        BOLD = 1,
        ITALIC = 3,
        UNDERLINE = 4,
        STRIKETHROUGH = 9,
        RED = 31,
        GREEN = 32,
        YELLOW = 33,
        BLUE = 34,
        MAGENTA = 35,
        CYAN = 36,
        WHITE = 37,
        BRIGHT_RED = 91,
        BRIGHT_GREEN = 92,
        BRIGHT_YELLOW = 93,
        BG_RED = 41,
        BG_GREEN = 42,
        BG_BLUE = 44
    };

    static std::string reset()
    {
        return "\033[0m";
    }

    // Styles

    static std::string bold(const std::string& text)
    {
        return wrap(text, STYLE_TYPE::BOLD);
    }

    static std::string italic(const std::string& text)
    {
        return wrap(text, STYLE_TYPE::ITALIC);
    }

    static std::string underline(const std::string& text)
    {
        return wrap(text, STYLE_TYPE::UNDERLINE);
    }

    static std::string strikethrough(const std::string& text)
    {
        return wrap(text, STYLE_TYPE::STRIKETHROUGH);
    }

    // Foreground

    static std::string red(const std::string& text)
    {
        return wrap(text, STYLE_TYPE::RED);
    }

    static std::string green(const std::string& text)
    {
        return wrap(text, STYLE_TYPE::GREEN);
    }

    static std::string yellow(const std::string& text)
    {
        return wrap(text, STYLE_TYPE::YELLOW);
    }

    static std::string blue(const std::string& text)
    {
        return wrap(text, STYLE_TYPE::BLUE);
    }

    static std::string magenta(const std::string& text)
    {
        return wrap(text, STYLE_TYPE::MAGENTA);
    }

    static std::string cyan(const std::string& text)
    {
        return wrap(text, STYLE_TYPE::CYAN);
    }

    static std::string white(const std::string& text)
    {
        return wrap(text, STYLE_TYPE::WHITE);
    }

    static std::string brightRed(const std::string& text)
    {
        return wrap(text, STYLE_TYPE::BRIGHT_RED);
    }

    static std::string brightGreen(const std::string& text)
    {
        return wrap(text, STYLE_TYPE::BRIGHT_GREEN);
    }

    static std::string brightYellow(const std::string& text)
    {
        return wrap(text, STYLE_TYPE::BRIGHT_YELLOW);
    }

    // Background Colors

    static std::string bgRed(const std::string& text)
    {
        return wrap(text, STYLE_TYPE::BG_RED);
    }

    static std::string bgGreen(const std::string& text)
    {
        return wrap(text, STYLE_TYPE::BG_GREEN);
    }

    static std::string bgBlue(const std::string& text)
    {
        return wrap(text, STYLE_TYPE::BG_BLUE);
    }

    // Custom style

    static std::string style(const std::string& text, std::initializer_list<std::string> codes)
    {
        std::ostringstream codeStream;
        for (auto it{ codes.begin() }; it != codes.end(); ++it)
        {
            if (it != codes.begin())
                codeStream << ';';
            
            codeStream << *it;
        }

        return "\033[" + codeStream.str() + 'm' + text + reset();
    }

    static std::string style(const std::string& text, std::initializer_list<STYLE_TYPE> codes)
    {
        std::ostringstream codeStream;
        for (auto it{ codes.begin() }; it != codes.end(); ++it)
        {
            auto style{ styleMap.find(*it) };

            if (style == styleMap.end())
                continue;

            if (it != codes.begin())
                codeStream << ';';
            
            codeStream << style->second;
        }

        return "\033[" + codeStream.str() + 'm' + text + reset();
    }

private:
    static const std::unordered_map<STYLE_TYPE, std::string> styleMap;

    static std::string wrap(const std::string& text, const std::string& code)
    {
        return "\033[" + code + 'm' + text + reset();
    }

    static std::string wrap(const std::string& text, const STYLE_TYPE code)
    {
        auto style{ styleMap.find(code) };
        if (style == styleMap.end())
            return "\033[m" + text + reset();

        return "\033[" + style->second + 'm' + text + reset();
    }
};


