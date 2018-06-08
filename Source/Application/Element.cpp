/*
    Initial author: Convery (tcn@ayria.se)
    Started: 08-06-2018
    License: MIT

    Implements a simple element for building menus.
*/

#include "../Stdinclude.hpp"

// Create the element state.
void Element_t::Calculatedimentions(Rect Boundingbox)
{
    double XA = Boundingbox.x1 / (2 / (std::abs(Margin.x0) + std::abs(Margin.x1)));
    double YA = Boundingbox.y1 / (2 / (std::abs(Margin.y0) + std::abs(Margin.y1)));
    double XB = Boundingbox.x0 + Boundingbox.x1 - XA;
    double YB = Boundingbox.y0 + Boundingbox.y1 - YA;

    Dimensions.x1 = std::round(XA + XB);
    Dimensions.y1 = std::round(YA + YB);
    Dimensions.x0 = std::round(std::min(XA, XB));
    Dimensions.y0 = std::round(std::min(YA, YB));

    // Update our children.
    for (auto &Item : Children) Item->Calculatedimentions(Dimensions);
}
Texture_t Element_t::Settexture(Texture_t Newtexture)
{
    auto Oldtexture = Texture;
    Texture = Newtexture;
    return Oldtexture;
}
void Element_t::Reinitializebuffers(float ZIndex)
{
    if (ZIndex == -12345.0f) ZIndex = this->ZIndex;
    else this->ZIndex = ZIndex;

    double Vertices[] =
    {
        Margin.x1, Margin.y1, ZIndex, 1.0, 1.0,
        Margin.x1, Margin.y0, ZIndex, 1.0, 0.0,
        Margin.x0, Margin.y0, ZIndex, 0.0, 0.0,
        Margin.x0, Margin.y1, ZIndex, 0.0, 1.0
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
}
void Element_t::Addchild(Element_t &&Child)
{
    Addchild(new Element_t(Child));
}
void Element_t::Addchild(Element_t *Child)
{
    Child->Calculatedimentions(Dimensions);
    Child->Reinitializebuffers(ZIndex - 0.01f);
    if (Child->onInit) Child->onInit(Child);

    Children.push_back(Child);
}
void Element_t::Resize(Rect Dimensions)
{
    this->Margin = Dimensions;
    this->Reinitializebuffers();
}
void Element_t::Renderelement()
{
    // Fetch the parent viewport.
    GLint Viewport[4]{};
    glGetIntegerv(GL_VIEWPORT, Viewport);

    // Draw our item.
    glBindVertexArray(VAO);
    if (Shader) Shader->Select();
    glBindTexture(GL_TEXTURE_2D, Texture);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Render all the children in our viewport.
    glViewport(Dimensions.x0, Dimensions.y0, Dimensions.x1, Dimensions.y1);
    for (auto &Item : Children) Item->Renderelement();

    // Restore the viewport.
    glViewport(Viewport[0], Viewport[1], Viewport[2], Viewport[3]);
}

// Default sourcecode for the shading.
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

// Initialize with default values.
Element_t::Element_t(std::string Identity, Shader_t *Shader, Texture_t Texture) : Identifier(Identity), Shader(Shader), Texture(Texture)
{
    onKeyboard = [](Element_t *Caller, uint32_t Key, uint32_t Modifier, bool Released) { (void)Caller; (void)Key; (void)Released; (void)Modifier; return false; };
    onClick = [](Element_t *Caller, uint32_t Key, bool Released) { (void)Caller; (void)Key; (void)Released; return false; };
    onFocus = [](Element_t *Caller, bool Released) { (void)Caller; (void)Released; return false; };
}
Element_t::Element_t(std::string Identity, Shader_t *Shader) : Identifier(Identity), Shader(Shader)
{
    onKeyboard = [](Element_t *Caller, uint32_t Key, uint32_t Modifier, bool Released) { (void)Caller; (void)Key; (void)Released; (void)Modifier; return false; };
    onClick = [](Element_t *Caller, uint32_t Key, bool Released) { (void)Caller; (void)Key; (void)Released; return false; };
    onFocus = [](Element_t *Caller, bool Released) { (void)Caller; (void)Released; return false; };
}
Element_t::Element_t(std::string Identity) : Identifier(Identity)
{
    onKeyboard = [](Element_t *Caller, uint32_t Key, uint32_t Modifier, bool Released) { (void)Caller; (void)Key; (void)Released; (void)Modifier; return false; };
    onClick = [](Element_t *Caller, uint32_t Key, bool Released) { (void)Caller; (void)Key; (void)Released; return false; };
    onFocus = [](Element_t *Caller, bool Released) { (void)Caller; (void)Released; return false; };

    Shader = Graphics::Createshader(Vertexsource, Fragmentsource);
}
Element_t::Element_t()
{
    onKeyboard = [](Element_t *Caller, uint32_t Key, uint32_t Modifier, bool Released) { (void)Caller; (void)Key; (void)Released; (void)Modifier; return false; };
    onClick = [](Element_t *Caller, uint32_t Key, bool Released) { (void)Caller; (void)Key; (void)Released; return false; };
    onFocus = [](Element_t *Caller, bool Released) { (void)Caller; (void)Released; return false; };

    Identifier = "Uninitialized";
    Shader = Graphics::Createshader(Vertexsource, Fragmentsource);
}
