#include "Software.hpp"
#include "Supervisor.hpp"
#include "GameWindow.hpp"
#include "i18n.hpp"
#include <SDL2/SDL.h>
#include "utils.hpp"

GfxInterface *Software::Init()
{

    SDL_Init(SDL_INIT_VIDEO);

    u32 flags = 0;
    i32 height = GAME_WINDOW_HEIGHT_REAL;
    i32 width = GAME_WINDOW_WIDTH_REAL;
    i32 x = SDL_WINDOWPOS_UNDEFINED;
    i32 y = SDL_WINDOWPOS_UNDEFINED;

    if (g_Supervisor.cfg.windowed == 0)
    {
        flags |= SDL_WINDOW_FULLSCREEN;
    }
    Software* self = new Software;

    SDL_Window* window = SDL_CreateWindow(TH_WINDOW_TITLE, x, y, width, height, flags);
    self->window = window;
    if (window == NULL)
    {
        delete self;
        return NULL;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    self->renderer = renderer;
    if (renderer == NULL)
    {
        delete self;
        return NULL;
    }

    self->model.Identity();
    self->view.Identity();
    self->projection.Identity();
    self->textureMatrix.Identity();

    return self;
}


void Software::Exit()
{
    if (this->renderer)
    {
        SDL_DestroyRenderer(this->renderer);
        this->renderer = NULL;
    }

    if (this->window)
    {
        SDL_DestroyWindow(this->window);
        this->window = NULL;
    }
}

void Software::SetFogRange(f32 nearPlane, f32 farPlane)
{
    //g_glFuncTable.glFogf(GL_FOG_START, nearPlane);
    //g_glFuncTable.glFogf(GL_FOG_END, farPlane);
}

void Software::SetFogColor(ZunColor color)
{
    f32 normalizedFogColor[4] = {((color >> 16) & 0xFF) / 255.0f, ((color >> 8) & 0xFF) / 255.0f,
                                     (color & 0xFF) / 255.0f, ((color >> 24) & 0xFF) / 255.0f};

    //g_glFuncTable.glFogfv(GL_FOG_COLOR, normalizedFogColor);
}

void Software::ToggleVertexAttribute(u8 attr, bool enable)
{
    /*if (attr & VERTEX_ATTR_TEX_COORD)
    {
        // Arg 0 will be the texture is it's used, and diffuse otherwise. Arg 1 will always be diffuse
        if (enable)
        {
            g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
            g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
            g_glFuncTable.glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        else
        {
            g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR);
            g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
            g_glFuncTable.glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
    }

    if (attr & VERTEX_ATTR_DIFFUSE)
    {
        if (enable)
        {
            g_glFuncTable.glEnableClientState(GL_COLOR_ARRAY);
        }
        else
        {
            g_glFuncTable.glDisableClientState(GL_COLOR_ARRAY);
        }
    }*/
}

void Software::SetAttributePointer(VertexAttributeArrays attr, std::size_t stride, void *ptr)
{
    switch (attr)
    {
    case VERTEX_ARRAY_POSITION:
        this->vertexData = ptr;
        this->vertexStride = stride;
        break;
    case VERTEX_ARRAY_TEX_COORD:
        this->texCoordData = ptr;
        this->texCoordStride = stride;
        break;
    case VERTEX_ARRAY_DIFFUSE:
        this->diffuseData = ptr;
        this->diffuseStride = stride;
        break;
    }
}

void Software::SetColorOp(TextureOpComponent component, ColorOp op)
{
    /*const GLenum opEnums[3] = {GL_MODULATE, GL_ADD, GL_REPLACE};

    if (component > COMPONENT_ALPHA || op > COLOR_OP_REPLACE)
    {
        return;
    }

    GLenum componentEnum = component == COMPONENT_ALPHA ? GL_COMBINE_ALPHA : GL_COMBINE_RGB;

    g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, componentEnum, opEnums[op]);*/
}

void Software::SetTextureFactor(ZunColor factor)
{
    f32 tfactorColor[4] = {((factor >> 16) & 0xFF) / 255.0f, ((factor >> 8) & 0xFF) / 255.0f,
                               (factor & 0xFF) / 255.0f, ((factor >> 24) & 0xFF) / 255.0f};


    //g_glFuncTable.glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, tfactorColor);
}

void Software::SetTransformMatrix(TransformMatrix type, const ZunMatrix &matrix)
{
    // This is not going to work for modelview
    //GLenum matrixEnum[4] = {GL_MODELVIEW, GL_MODELVIEW, GL_PROJECTION, GL_TEXTURE};
    switch (type) {
        case MATRIX_MODEL:
            model = matrix;
            break;
        case MATRIX_VIEW:
            view = matrix;
            break;
        case MATRIX_PROJECTION:
            projection = matrix;
            break;
        case MATRIX_TEXTURE:
            textureMatrix = matrix;
            break;
    }

    //g_glFuncTable.glMatrixMode(matrixEnum[type]);
    //g_glFuncTable.glLoadMatrixf((const GLfloat *)&matrix);
}


void Software::Enable(Capabilities cap) {
    switch (cap) {
        case CAPS_BLEND:
        //    g_glFuncTable.glEnable(GL_BLEND);
            break;
        case CAPS_DEPTH_TEST:
        //    g_glFuncTable.glEnable(GL_DEPTH_TEST);
            break;
    }
}

void Software::SetBlendMode(BlendMode mode) {
    if (mode == BLEND_INV_SRC_ALPHA)
    {
    //    g_glFuncTable.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
    //    g_glFuncTable.glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    }
}

void Software::SetViewport(i32 x, i32 y, i32 width, i32 height) {
    //g_glFuncTable.glViewport(x, y, width, height);
    viewport[0] = x;
    viewport[1] = y;
    viewport[2] = width;
    viewport[3] = height;
}

void Software::GetViewport(u32* viewport) {
    //g_glFuncTable.glGetIntegerv(GL_VIEWPORT, (GLint*)viewport);
    for (int i = 0; i < 4; i++) {
        viewport[i] = this->viewport[i];
    }
}

void Software::GetDepthRange(f32* depthRange) {
    //g_glFuncTable.glGetFloatv(GL_DEPTH_RANGE, depthRange);
    depthRange[0] = this->depthRange[0];
    depthRange[1] = this->depthRange[1];
}

void Software::SetClearColor(f32 r, f32 g, f32 b, f32 a) {
    //g_glFuncTable.glClearColor(r, g, b, a);
    clearColor[0] = r;
    clearColor[1] = g;
    clearColor[2] = b;
    clearColor[3] = a;
}

void Software::SetTextureFilter() {
    //g_glFuncTable.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void Software::SetClearDepth(f32 depth) {
    //g_glFuncTable.glClearDepthf(depth);
    clearDepth = depth;
}

void Software::Clear(u32 clearBits) {
    if (clearBits & CLEAR_COLOR_BUFFER) {
        SDL_SetRenderDrawColor(this->renderer, clearColor[0] * 255, clearColor[1] * 255, clearColor[2] * 255, clearColor[3] * 255);
        SDL_RenderClear(this->renderer);
    }
    //if (clearBits & CLEAR_DEPTH_BUFFER) mask |= GL_DEPTH_BUFFER_BIT;

    //g_glFuncTable.glClear(mask);
}

void Software::SetDepthRange(f32 near, f32 far) {
    //g_glFuncTable.glDepthRangef(near, far);
    depthRange[0] = near;
    depthRange[1] = far;
}

void Software::SetDepthMask(bool enable) {
    //g_glFuncTable.glDepthMask(enable);
    depthMask = enable;
}

void Software::SetDepthFunc(DepthFunc func) {
    depthFunc = func;
}

GfxTextureHandle Software::CreateTexture() {
    textures.push_back(std::unique_ptr<Texture>(new Texture()));

    return {textures.size()-1};
}

void Software::BindTexture(GfxTextureHandle handle) {
    boundTexture = textures[handle.id].get();
}

void Software::DeleteTexture(GfxTextureHandle handle) {
    textures[handle.id].reset();

    textures[handle.id] = 0;
}

void Software::SetTextureImage(u32 width, u32 height, PixelFormat fmt, PixelDataType type, const void* data) {
    if (boundTexture) {
        boundTexture->data = data;
        boundTexture->width = width;
        boundTexture->height = height;
        boundTexture->format = fmt;
        boundTexture->type = type;
    }
}

void Software::SetTextureSubImage(i32 xoffset, i32 yoffset, i32 width, i32 height, const void *data)
{
    //g_glFuncTable.glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
}

void Software::ReadPixels(i32 x, i32 y, i32 width, i32 height, const void* pixels) {
    //g_glFuncTable.glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (void*)pixels);
}

void Software::drawLine(i32 x1, i32 y1, i32 x2, i32 y2) {
    SDL_SetRenderDrawColor(this->renderer, 255, 255, 255, 255);
    SDL_RenderDrawLine(this->renderer, x1, y1, x2, y2);
}

ZunVec3 ParseVec3(const void* data) { //bleeeh bleeh bbleeeeehhhhh (it doubles as a hack for alignment so yay)
    ZunVec3 vec;
    memcpy(&vec, data, sizeof(ZunVec3));
    return vec;
}

inline ZunVec3 Software::ProjectToNDC(ZunVec3 vertex) {
    ZunVec4 clip = projection * view * model * ZunVec4(vertex, 1.0f);
    ZunVec3 ndc = {clip.x, clip.y, clip.z};
    if (clip.w != 0) {
        ndc /= clip.w;
    }

    return ndc;
}

inline ZunVec3 Software::NDCToScreen(ZunVec3 vertex) {
    ZunVec3 screen;
    screen.x = (vertex.x + 1) / 2.0f * viewport[2] + viewport[0];
    screen.y = (1 - (vertex.y + 1) / 2.0f) * viewport[3] + viewport[1];
    screen.z = vertex.z;
    return screen;
}

// >0 for CCW, <0 for CW
inline float CalculateWinding(ZunVec3 p1, ZunVec3 p2, ZunVec3 p3) {
    return (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);
}

void Software::Draw(PrimitiveType type, i32 start, i32 count)
{
    switch (type) {
        case PRIM_TRIANGLE_STRIP:
            break;
        case PRIM_TRIANGLES:
            if(!vertexData) return;
            u32 index = start;
            while (index < start + count) {

                //project vertices to screen space
                ZunVec3 v0 = NDCToScreen(ProjectToNDC(ParseVec3((char*)vertexData + vertexStride * index)));
                ZunVec3 v1 = NDCToScreen(ProjectToNDC(ParseVec3((char*)vertexData + vertexStride * (index+1))));
                ZunVec3 v2 = NDCToScreen(ProjectToNDC(ParseVec3((char*)vertexData + vertexStride * (index+2))));

                // we dont do backface culling because the engine makes sure that the draw order is always consistent from back to front
                
                drawLine(v0.x, v0.y, v1.x, v1.y);
                drawLine(v1.x, v1.y, v2.x, v2.y);
                drawLine(v2.x, v2.y, v0.x, v0.y);
                index += 3;
            }
            break;
    }
}

void Software::SwapBuffers()
{
    //SDL_GL_SwapWindow(window);
    SDL_RenderPresent(renderer);
}