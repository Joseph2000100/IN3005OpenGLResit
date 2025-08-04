#include "Common.h"
#include "Pyramid.h"
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

CPyramid::CPyramid() :
    m_vao(0),
    m_width(0),
    m_height(0),
    m_blinkTimer(0.0f),
    m_isVisible(true)
{
}

CPyramid::~CPyramid()
{
}

void CPyramid::Create(float width, float height)
{
    m_width = width;
    m_height = height;

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    m_vbo.Create();
    m_vbo.Bind();

    float halfWidth = m_width / 2.0f;

    // Define the vertices for our pyramid
    glm::vec3 vertices[] = {
        // Base - Triangle 1 
        glm::vec3(-halfWidth, 0.0f, -halfWidth),
        glm::vec3(halfWidth, 0.0f, halfWidth),
        glm::vec3(-halfWidth, 0.0f, halfWidth),

        // Base - Triangle 2 
        glm::vec3(-halfWidth, 0.0f, -halfWidth),
        glm::vec3(halfWidth, 0.0f, -halfWidth),
        glm::vec3(halfWidth, 0.0f, halfWidth),

        // Front face 
        glm::vec3(-halfWidth, 0.0f, halfWidth),
        glm::vec3(halfWidth, 0.0f, halfWidth),
        glm::vec3(0.0f, height, 0.0f),

        // Right face 
        glm::vec3(halfWidth, 0.0f, halfWidth),
        glm::vec3(halfWidth, 0.0f, -halfWidth),
        glm::vec3(0.0f, height, 0.0f),

        // Back face
        glm::vec3(halfWidth, 0.0f, -halfWidth),
        glm::vec3(-halfWidth, 0.0f, -halfWidth),
        glm::vec3(0.0f, height, 0.0f),

        // Left face
        glm::vec3(-halfWidth, 0.0f, -halfWidth),
        glm::vec3(-halfWidth, 0.0f, halfWidth),
        glm::vec3(0.0f, height, 0.0f)
    };

    // Calculate normals for each face
    glm::vec3 normals[] = {
        // Base normals
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),

        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),

        // Front face normals
        glm::vec3(0.0f, 0.5f, 1.0f),
        glm::vec3(0.0f, 0.5f, 1.0f),
        glm::vec3(0.0f, 0.5f, 1.0f),

        // Right face normals
        glm::vec3(1.0f, 0.5f, 0.0f),
        glm::vec3(1.0f, 0.5f, 0.0f),
        glm::vec3(1.0f, 0.5f, 0.0f),

        // Back face normals
        glm::vec3(0.0f, 0.5f, -1.0f),
        glm::vec3(0.0f, 0.5f, -1.0f),
        glm::vec3(0.0f, 0.5f, -1.0f),

        // Left face normals
        glm::vec3(-1.0f, 0.5f, 0.0f),
        glm::vec3(-1.0f, 0.5f, 0.0f),
        glm::vec3(-1.0f, 0.5f, 0.0f)
    };


    // Add data to VBO
    for (int i = 0; i < 18; i++) {
        m_vbo.AddData(&vertices[i], sizeof(glm::vec3));
        m_vbo.AddData(&normals[i], sizeof(glm::vec3));
    }

    m_vbo.UploadDataToGPU(GL_STATIC_DRAW);

    // Set the vertex attributes
    GLsizei stride = sizeof(glm::vec3) * 2;  // Stride is vertex + normal

    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);

    // Normal vectors
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));
}

void CPyramid::Render()
{
    if (!m_isVisible)
        return;
    
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 18);  // 6 triangles * 3 vertices
}

void CPyramid::Release()
{
    glDeleteVertexArrays(1, &m_vao);
    m_vbo.Release();
}

void CPyramid::Update(float dt)
{
    m_blinkTimer += dt;

    // Toggle visibility every 0.5 seconds
    if (m_blinkTimer >= 500.0f)
    {
        m_blinkTimer = 0.0f;
        m_isVisible = !m_isVisible;
    }
}
