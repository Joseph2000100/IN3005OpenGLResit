#pragma once

#include "VertexBufferObject.h"

// Class for generating a cuboid
class CCuboid
{
public:
    CCuboid();
    ~CCuboid();
    void Create(float width, float height, float depth);
    void Render();
    void Release();
private:
    UINT m_vao;
    CVertexBufferObject m_vbo;
    float m_width;
    float m_height;
    float m_depth;
};
