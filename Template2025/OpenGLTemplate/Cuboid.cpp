#include "Common.h"
#include "Cuboid.h"
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

CCuboid::CCuboid()
{
}

CCuboid::~CCuboid()
{
}

void CCuboid::Create(float width, float height, float depth)
{
    m_width = width;
    m_height = height;
    m_depth = depth;

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    m_vbo.Create();
    m_vbo.Bind();

    float halfWidth = m_width / 2.0f;
    float halfHeight = m_height / 2.0f;
    float halfDepth = m_depth / 2.0f;

    // Define the vertices
    glm::vec3 vertices[] = {
        // Front face
        glm::vec3(-halfWidth, -halfHeight, halfDepth),
        glm::vec3(halfWidth, -halfHeight, halfDepth),
        glm::vec3(-halfWidth, halfHeight, halfDepth),
        glm::vec3(halfWidth, -halfHeight, halfDepth),
        glm::vec3(halfWidth, halfHeight, halfDepth),
        glm::vec3(-halfWidth, halfHeight, halfDepth),

        // Back face
        glm::vec3(halfWidth, -halfHeight, -halfDepth),
        glm::vec3(-halfWidth, -halfHeight, -halfDepth),
        glm::vec3(halfWidth, halfHeight, -halfDepth),
        glm::vec3(-halfWidth, -halfHeight, -halfDepth),
        glm::vec3(-halfWidth, halfHeight, -halfDepth),
        glm::vec3(halfWidth, halfHeight, -halfDepth),

        // Right face
        glm::vec3(halfWidth, -halfHeight, halfDepth),
        glm::vec3(halfWidth, -halfHeight, -halfDepth),
        glm::vec3(halfWidth, halfHeight, halfDepth),
        glm::vec3(halfWidth, -halfHeight, -halfDepth),
        glm::vec3(halfWidth, halfHeight, -halfDepth),
        glm::vec3(halfWidth, halfHeight, halfDepth),

        // Left face
        glm::vec3(-halfWidth, -halfHeight, -halfDepth),
        glm::vec3(-halfWidth, -halfHeight, halfDepth),
        glm::vec3(-halfWidth, halfHeight, -halfDepth),
        glm::vec3(-halfWidth, -halfHeight, halfDepth),
        glm::vec3(-halfWidth, halfHeight, halfDepth),
        glm::vec3(-halfWidth, halfHeight, -halfDepth),

        // Top face
        glm::vec3(-halfWidth, halfHeight, halfDepth),
        glm::vec3(halfWidth, halfHeight, halfDepth),
        glm::vec3(-halfWidth, halfHeight, -halfDepth),
        glm::vec3(halfWidth, halfHeight, halfDepth),
        glm::vec3(halfWidth, halfHeight, -halfDepth),
        glm::vec3(-halfWidth, halfHeight, -halfDepth),

        // Bottom face
        glm::vec3(-halfWidth, -halfHeight, -halfDepth),
        glm::vec3(halfWidth, -halfHeight, -halfDepth),
        glm::vec3(-halfWidth, -halfHeight, halfDepth),
        glm::vec3(halfWidth, -halfHeight, -halfDepth),
        glm::vec3(halfWidth, -halfHeight, halfDepth),
        glm::vec3(-halfWidth, -halfHeight, halfDepth)
    };

    // Define normals for each face
    glm::vec3 normals[] = {
        // Front face normals
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),

        // Back face normals
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),

        // Right face normals
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),

        // Left face normals
        glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(-1.0f, 0.0f, 0.0f),

        // Top face normals
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),

        // Bottom face normals
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f)
    };

    // Add data to VBO
    for (int i = 0; i < 36; i++) {
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

void CCuboid::Render()
{
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void CCuboid::Release()
{
    glDeleteVertexArrays(1, &m_vao);
    m_vbo.Release();
}
