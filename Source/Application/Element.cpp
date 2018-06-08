/*
    Initial author: Convery (tcn@ayria.se)
    Started: 08-06-2018
    License: MIT

    Implements a simple element for building menus.
*/

#include "../Stdinclude.hpp"

// Manage the state.
void Element_t::onModifiedstate()
{
    double Vertices[] =
    {
         1 - Margin.x1,  1 - Margin.y1,  ZIndex, 1.0, 1.0,
         1 - Margin.x1, -1 + Margin.y0,  ZIndex, 1.0, 0.0,
        -1 + Margin.x0, -1 + Margin.y0,  ZIndex, 0.0, 0.0,
        -1 + Margin.x0,  1 - Margin.y1,  ZIndex, 0.0, 1.0
    };
    uint32_t Indices[] =
    {
        0, 1, 3,
        1, 2, 3
    };

    // Clear the previous state.
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);

    // Create the buffers.
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Select the vertex and add properties.
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

    // Position attribute.
    glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 5 * sizeof(double), (void *)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute.
    glVertexAttribPointer(1, 2, GL_DOUBLE, GL_FALSE, 5 * sizeof(double), (void *)(3 * sizeof(double)));
    glEnableVertexAttribArray(1);

    // Calculate the dimensions.
    double Width = (Boundingbox.x1 - Boundingbox.x0) / 2;
    Dimensions.x0 = std::round(Boundingbox.x0 + Width * Margin.x0);
    Dimensions.x1 = std::round(Boundingbox.x1 - Width * Margin.x1);
    double Height = (Boundingbox.y1 - Boundingbox.y0) / 2;
    Dimensions.y0 = std::round(Boundingbox.y0 + Height * Margin.y0);
    Dimensions.y1 = std::round(Boundingbox.y1 - Height * Margin.y1);

    // Update all children.
    for (auto &Item : Children)
    {
        Item->Boundingbox = Dimensions;
        Item->onModifiedstate();
    }
}
void Element_t::onRender()
{
    // Set the viewport to our box.
    glViewport(Boundingbox.x0, Boundingbox.y0, Boundingbox.x1, Boundingbox.y1);

    // Draw our item.
    glBindVertexArray(VAO);
    if (Shader) Shader->Select();
    glBindTexture(GL_TEXTURE_2D, Texture);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Draw all children.
    for (auto &Item : Children) Item->onRender();
}

// Always require an identity.
Element_t::Element_t(std::string Identity) : Identifier(Identity)
{
    static const char *Vertexsource =
    R"(
        #version 330 core
        layout (location = 0) in vec3 Position;
        layout (location = 1) in vec2 Texturecoords;
        out vec2 Vertextexturemap;
        void main()
        {
            gl_Position = vec4(Position, 1.0);
            Vertextexturemap = Texturecoords;
        }
    )";
    static const char *Fragmentsource =
    R"(
        #version 330 core
        out vec4 Fragmentcolor;
        in vec2 Vertextexturemap;
        uniform sampler2D Texture;
        void main()
        {
            Fragmentcolor = texture(Texture, Vertextexturemap);
        }
    )";
    Shader = Graphics::Createshader(Vertexsource, Fragmentsource);

    onKeyboard = [](Element_t *Caller, uint32_t Key, uint32_t Modifier, bool Released) { (void)Caller; (void)Key; (void)Released; (void)Modifier; return false; };
    onClick = [](Element_t *Caller, uint32_t Key, bool Released) { (void)Caller; (void)Key; (void)Released; return false; };
    onFocus = [](Element_t *Caller, bool Released) { (void)Caller; (void)Released; return false; };
}
