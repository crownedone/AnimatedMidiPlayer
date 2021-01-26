#include "myText.h"
#include <cassert>
#include <iostream>



// global library handle
FT_Library ft = nullptr;

char* file_read(const char* filename)
{
    FILE* in = fopen(filename, "rb");
    if (in == NULL) return NULL;

    int res_size = BUFSIZ;
    char* res = (char*)malloc(res_size);
    int nb_read_total = 0;

    while (!feof(in) && !ferror(in)) {
        if (nb_read_total + BUFSIZ > res_size) {
            if (res_size > 10 * 1024 * 1024) break;
            res_size = res_size * 2;
            res = (char*)realloc(res, res_size);
        }
        char* p_res = res + nb_read_total;
        nb_read_total += fread(p_res, 1, BUFSIZ, in);
    }

    fclose(in);
    res = (char*)realloc(res, nb_read_total + 1);
    res[nb_read_total] = '\0';
    return res;
}

/**
* Display compilation errors from the OpenGL shader compiler
*/
void print_log(GLuint object)
{
    GLint log_length = 0;
    if (glIsShader(object))
        glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
    else if (glIsProgram(object))
        glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
    else {
        fprintf(stderr, "printlog: Not a shader or a program\n");
        return;
    }

    char* log = (char*)malloc(log_length);

    if (glIsShader(object))
        glGetShaderInfoLog(object, log_length, NULL, log);
    else if (glIsProgram(object))
        glGetProgramInfoLog(object, log_length, NULL, log);

    fprintf(stderr, "%s", log);
    free(log);
}

/**
* Compile the shader from file 'filename', with error handling
*/
GLuint create_shader(const char* filename, GLenum type)
{
    const GLchar* source = file_read(filename);
    if (source == NULL) {
        fprintf(stderr, "Error opening %s: ", filename); perror("");
        return 0;
    }
    GLuint res = glCreateShader(type);
    const GLchar* sources[] = {
        // Define GLSL version
#ifdef GL_ES_VERSION_2_0
        "#version 100\n"  // OpenGL ES 2.0
#else
        "#version 120\n"  // OpenGL 2.1
#endif
        ,
        // GLES2 precision specifiers
#ifdef GL_ES_VERSION_2_0
        // Define default float precision for fragment shaders:
        (type == GL_FRAGMENT_SHADER) ?
        "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
        "precision highp float;           \n"
        "#else                            \n"
        "precision mediump float;         \n"
        "#endif                           \n"
        : ""
        // Note: OpenGL ES automatically defines this:
        // #define GL_ES
#else
        // Ignore GLES 2 precision specifiers:
        "#define lowp   \n"
        "#define mediump\n"
        "#define highp  \n"
#endif
        ,
        source };
    glShaderSource(res, 3, sources, NULL);
    free((void*)source);

    glCompileShader(res);
    GLint compile_ok = GL_FALSE;
    glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
    if (compile_ok == GL_FALSE) {
        fprintf(stderr, "%s:", filename);
        print_log(res);
        glDeleteShader(res);
        return 0;
    }

    return res;
}

GLuint create_program(const char* vertexfile, const char* fragmentfile) {
    GLuint program = glCreateProgram();
    GLuint shader;

    if (vertexfile) {
        shader = create_shader(vertexfile, GL_VERTEX_SHADER);
        if (!shader)
            return 0;
        glAttachShader(program, shader);
    }

    if (fragmentfile) {
        shader = create_shader(fragmentfile, GL_FRAGMENT_SHADER);
        if (!shader)
            return 0;
        glAttachShader(program, shader);
    }

    glLinkProgram(program);
    GLint link_ok = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        fprintf(stderr, "glLinkProgram:");
        print_log(program);
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

GLint get_attrib(GLuint program, const char* name) {
    GLint attribute = glGetAttribLocation(program, name);
    if (attribute == -1)
        fprintf(stderr, "Could not bind attribute %s\n", name);
    return attribute;
}

GLint get_uniform(GLuint program, const char* name) {
    GLint uniform = glGetUniformLocation(program, name);
    if (uniform == -1)
        fprintf(stderr, "Could not bind uniform %s\n", name);
    return uniform;
}

myText::myText(const string& fontFileName)
{
    if (!ft)
    {
        if (FT_Init_FreeType(&ft)) {
            std::cerr << "Could not init freetype library\n";
            assert(false);
        }
    }
    /* Load a font */
    if (FT_New_Face(ft, fontFileName.c_str(), 0, &face)) {
        std::cerr << "Could not open font " << fontFileName << std::endl;
        assert(false);
    }

    shader = create_program("shaders/text.v.glsl", "shaders/text.f.glsl");
    if (shader == 0)
        assert(false);

    attribute_coord = get_attrib(shader, "coord");
    uniform_tex = get_uniform(shader, "tex");
    uniform_color = get_uniform(shader, "color");

    if (attribute_coord == -1 || uniform_tex == -1 || uniform_color == -1)
        assert(false);

    // Create the vertex buffer object
    glGenBuffers(1, &vbo);
}

myText::~myText()
{
    glDeleteProgram(shader);
}

void myText::setPixelSize(int size)
{
    FT_Set_Pixel_Sizes(face, 0, size);
}

GLfloat c[4];
void myText::setColor(glm::vec4 color) {
    c[0] = color.x;
    c[1] = color.y;
    c[2] = color.z;
    c[3] = color.w;
}



struct point {
    GLfloat x;
    GLfloat y;
    GLfloat s;
    GLfloat t;
};

void myText::render_text(const string& text, float x, float y, float sx, float sy) {
    glUseProgram(shader);
    FT_GlyphSlot g = face->glyph;

    /* Create a texture that will be used to hold one "glyph" */
    GLuint tex;

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(uniform_tex, tex);
    glUniform4fv(uniform_color, 1, c);


    /* Set up the VBO for our vertex data */
    glEnableVertexAttribArray(attribute_coord);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(attribute_coord, 4, GL_FLOAT, GL_FALSE, 0, 0);

    /* Loop through all characters */
    for (size_t i = 0; i < text.size(); ++i) {
        char p = text[i];

        /* Try to load and render the character */
        if (FT_Load_Char(face, p, FT_LOAD_RENDER))
            continue;

        /* Upload the "bitmap", which contains an 8-bit grayscale image, as an alpha texture */
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, g->bitmap.width, g->bitmap.rows, 0, GL_ALPHA, GL_UNSIGNED_BYTE, g->bitmap.buffer);

        /* Calculate the vertex and texture coordinates */
        float x2 = x + g->bitmap_left * sx;
        float y2 = -y - g->bitmap_top * sy;
        float w = g->bitmap.width * sx;
        float h = g->bitmap.rows * sy;

        //glm::ivec4 box[4]
        point box[4] = {
            {x2, -y2, 0, 0},
            {x2 + w, -y2, 1, 0},
            {x2, -y2 - h, 0, 1},
            {x2 + w, -y2 - h, 1, 1},
        };

        /* Draw the character on the screen */
        glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        /* Advance the cursor to the start of the next character */
        x += (g->advance.x >> 6) * sx;
        y += (g->advance.y >> 6) * sy;
    }

    glDisableVertexAttribArray(attribute_coord);
    glDeleteTextures(1, &tex);
}
