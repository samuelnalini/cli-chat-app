#include "style.hpp"

const std::unordered_map<Style::STYLE_TYPE, std::string> Style::styleMap = {
        {STYLE_TYPE::BOLD, "1"},
        {STYLE_TYPE::ITALIC, "3"},
        {STYLE_TYPE::UNDERLINE, "4"},
        {STYLE_TYPE::STRIKETHROUGH, "9"},
        {STYLE_TYPE::RED, "31"},
        {STYLE_TYPE::GREEN, "32"},
        {STYLE_TYPE::YELLOW, "33"},
        {STYLE_TYPE::BLUE, "34"},
        {STYLE_TYPE::MAGENTA, "35"},
        {STYLE_TYPE::CYAN, "36"},
        {STYLE_TYPE::WHITE, "37"},
        {STYLE_TYPE::BRIGHT_RED, "91"},
        {STYLE_TYPE::BRIGHT_GREEN, "92"},
        {STYLE_TYPE::BRIGHT_YELLOW, "93"},
        {STYLE_TYPE::BG_RED, "41"},
        {STYLE_TYPE::BG_GREEN, "42"},
        {STYLE_TYPE::BG_BLUE, "44"}
    };
