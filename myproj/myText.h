#include "myVBO.h"
#include "myShader.h"

// used for text rendering
#include <ft2build.h>
#include FT_FREETYPE_H

using string = std::string;

class myText
{
public:
    myText(const string& fontFileName);
    ~myText();

    // Render text using the currently loaded font and currently set font size.
    // Rendering starts at coordinates (x, y), z is always 0.
    // The pixel coordinates that the FreeType2 library uses are scaled by (sx, sy).
    void render_text(const string& text, float x, float y, float sx, float sy);

    void setPixelSize(int size = 48);

    void setColor(glm::vec4 color);
private:
    FT_Face face; // this texts true type
    GLuint shader; // the respective shader

    GLint attribute_coord;
    GLint uniform_tex;
    GLint uniform_color;
    GLuint vbo;
};
