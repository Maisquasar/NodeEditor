#pragma once

struct ImFont;
class Font
{
public:
    static bool LoadFont();

    static void GetFontData(const unsigned char*& data, unsigned int& size);

    static ImFont* GetFont();
    static ImFont* GetFontScaled();

private:
    static ImFont* m_default;
    static ImFont* m_defaultScaled;
};
