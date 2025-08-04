#pragma once
#include "VertexBufferObject.h"
#include "Common.h"

// Class for generating a pyramid
class CPyramid
{
public:
    CPyramid();
    ~CPyramid();
    void Create(float width, float height);
    void Render();
    void Release();
    void Update(float dt);
private:
    UINT m_vao;
    CVertexBufferObject m_vbo;
    float m_width;
    float m_height;
    float m_blinkTimer;
    float m_isVisible;
};

