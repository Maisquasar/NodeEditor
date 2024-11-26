#pragma once
#include <Maths.h>

class Selectable
{
public:
    virtual ~Selectable() = default;
    
    virtual bool IsSelected() const { return p_selected; }

    virtual bool IsSelected(const Vec2f& point, const Vec2f& origin, float zoom) const = 0;

    virtual bool IsSelected(const Vec2f& rectMin, const Vec2f& rectMax, const Vec2f& origin, float zoom) const = 0;
protected:
    bool p_selected = false;
    
};
