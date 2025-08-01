#pragma once

#include "VertexBufferObject.h"

// Class for generating a pyramid
class CPyramid
{
public:
    CPyramid();
    ~CPyramid();
    void Create(float width, float height);
    void Render();
    void Release();
private:
    UINT m_vao;
    CVertexBufferObject m_vbo;
    float m_width;
    float m_height;
};
