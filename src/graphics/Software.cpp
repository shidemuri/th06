#include "Software.hpp"
#include "Supervisor.hpp"
#include "GameWindow.hpp"
#include "i18n.hpp"
#include <SDL2/SDL.h>
#include <algorithm>
#include <cmath>
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

    SDL_Texture* framebufferTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT);
    self->framebufferTexture = framebufferTexture;
    if (framebufferTexture == NULL)    {
        delete self;
        return NULL;
    }
    u32* framebuffer = new u32[GAME_WINDOW_WIDTH * GAME_WINDOW_HEIGHT];
    self->framebuffer = framebuffer;

    f32* depthBuffer = new f32[GAME_WINDOW_WIDTH * GAME_WINDOW_HEIGHT];
    self->depthBuffer = depthBuffer;

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
    if (this->framebufferTexture)
    {
        SDL_DestroyTexture(this->framebufferTexture);
        this->framebufferTexture = NULL;
    }
    if (this->framebuffer)    {
        delete[] this->framebuffer;
        this->framebuffer = NULL;
    }
    if (this->depthBuffer)
    {
        delete[] this->depthBuffer;
        this->depthBuffer = NULL;
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
    if (attr & VERTEX_ATTR_TEX_COORD)
    {
        useTexCoord = enable;
    }
    if (attr & VERTEX_ATTR_DIFFUSE)
    {
        useDiffuse = enable;
    }
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
    //f32 tfactorColor[4] = {((factor >> 16) & 0xFF) / 255.0f, ((factor >> 8) & 0xFF) / 255.0f,
    //                           (factor & 0xFF) / 255.0f, ((factor >> 24) & 0xFF) / 255.0f};

    textureFactor = factor;

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


inline ZunColor RGBAToZunColor(u8 r, u8 g, u8 b, u8 a) {
    return ((ZunColor)a << 24) | ((ZunColor)r << 16) | ((ZunColor)g << 8) | (ZunColor)b;
}

void Software::SetClearColor(f32 r, f32 g, f32 b, f32 a) {
    //g_glFuncTable.glClearColor(r, g, b, a);
    clearColor = RGBAToZunColor((u8)(r * 255), (u8)(g * 255), (u8)(b * 255), (u8)(a * 255));
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
        //SDL_SetRenderDrawColor(this->renderer, clearColor[0] * 255, clearColor[1] * 255, clearColor[2] * 255, clearColor[3] * 255);
        //SDL_RenderClear(this->renderer);
        std::fill(framebuffer, framebuffer + GAME_WINDOW_WIDTH * GAME_WINDOW_HEIGHT, clearColor);
    }
    if (clearBits & CLEAR_DEPTH_BUFFER) {
        std::fill(depthBuffer, depthBuffer + GAME_WINDOW_WIDTH * GAME_WINDOW_HEIGHT, clearDepth);
    }
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


inline SDL_PixelFormatEnum GetSDLPixelFormat(PixelFormat fmt, PixelDataType type) {
    switch(type) {
        case PIXEL_UNSIGNED_BYTE:
            if(type == PIXEL_RGB) return SDL_PIXELFORMAT_RGB24;
            else return SDL_PIXELFORMAT_RGBA32;
        case PIXEL_UNSIGNED_SHORT_4_4_4_4:
            return SDL_PIXELFORMAT_RGBA4444;
        case PIXEL_UNSIGNED_SHORT_5_5_5_1:
            return SDL_PIXELFORMAT_RGBA5551;
        case PIXEL_UNSIGNED_SHORT_5_6_5:
            return SDL_PIXELFORMAT_RGB565;
    }
}

void Software::SetTextureImage(u32 width, u32 height, PixelFormat fmt, PixelDataType type, const void* data) {
    if (boundTexture) {
        u32 bpp = 2;
        if(type == PIXEL_UNSIGNED_BYTE) {
            if(fmt == PIXEL_RGB) bpp = 3;
            else bpp = 4;
        }
        boundTexture->texels.resize(width * height);
        if(data) SDL_ConvertPixels(width,height, GetSDLPixelFormat(fmt,type), data, width * bpp, SDL_PIXELFORMAT_RGBA32, boundTexture->texels.data(), width * sizeof(u32));
        boundTexture->width = width;
        boundTexture->height = height;
        boundTexture->format = fmt;
        boundTexture->type = type;
    }
}

void Software::SetTextureSubImage(i32 xoffset, i32 yoffset, i32 width, i32 height, const void *data)
{
    if (boundTexture) {
        SDL_ConvertPixels(width,height, SDL_PIXELFORMAT_RGB24, data, width*3, SDL_PIXELFORMAT_RGBA32, boundTexture->texels.data() + (yoffset * boundTexture->width) + xoffset, boundTexture->width*sizeof(u32));
    }
}

void Software::ReadPixels(i32 x, i32 y, i32 width, i32 height, const void* pixels) {
    //g_glFuncTable.glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (void*)pixels);
}

void Software::drawLine(i32 x1, i32 y1, i32 x2, i32 y2) {
    //SDL_SetRenderDrawColor(this->renderer, 255, 255, 255, 255);
    //SDL_RenderDrawLine(this->renderer, x1, y1, x2, y2);
}

void Software::drawPoint(i32 x, i32 y, ZunColor color) {
    //SDL_SetRenderDrawColor(this->renderer, 255, 255, 255, 255);
    //SDL_RenderDrawPoint(this->renderer, x, y);
    //x = std::clamp(x, 0, GAME_WINDOW_WIDTH - 1);
    //y = std::clamp(y, 0, GAME_WINDOW_HEIGHT - 1);
    framebuffer[y * GAME_WINDOW_WIDTH + x] = color;
}

ZunVec3 ParseVec3(const void* data) { //bleeeh bleeh bbleeeeehhhhh (it doubles as a hack for alignment so yay)
    ZunVec3 vec;
    memcpy(&vec, data, sizeof(ZunVec3));
    return vec;
}

ZunVec2 ParseVec2(const void* data) {
    ZunVec2 vec;
    memcpy(&vec, data, sizeof(ZunVec2));
    return vec;
}

inline ZunVec3 Software::ProjectToNDC(ZunVec3 vertex, ZunMatrix mvp) {
    ZunVec4 clip = mvp * ZunVec4(vertex, 1.0f);
    ZunVec3 ndc = {clip.x, clip.y, clip.z};
    if (clip.w != 0) {
        ndc /= clip.w;
    }

    return ndc;
}

inline ZunVec2 Software::ProjectTexCoordToNDC(ZunVec2 texCoord, ZunMatrix textureMatrix) {
    ZunVec4 clip = textureMatrix * ZunVec4(ZunVec3(texCoord.x, texCoord.y, 1.0f), 1.0f);
    ZunVec2 ndc = {clip.x, clip.y};
    //if (clip.w != 0) {
    //    ndc /= clip.w;
    //}

    return ndc;
}

inline ZunVec3 Software::NDCToScreen(ZunVec3 vertex) {
    ZunVec3 screen;
    screen.x = (vertex.x + 1) / 2.0f * viewport[2] + viewport[0];
    screen.y = (1 - (vertex.y + 1) / 2.0f) * viewport[3] + viewport[1];
    screen.z = vertex.z;
    return screen;
}

// https://www.cs.drexel.edu/~deb39/Classes/Papers/comp175-06-pineda.pdf

inline float EdgeFunction(ZunVec3 v0, ZunVec3 v1, ZunVec3 v2) {
    return (v1.x - v0.x) * (v2.y - v0.y) - (v1.y - v0.y) * (v2.x - v0.x);
}

void Software::Draw(PrimitiveType type, i32 start, i32 count)
{
    if (count == 0) return;
    ZunMatrix mvp = projection * view * model;
    u32 increment = type == PRIM_TRIANGLE_STRIP ? 1 : 3;
    u32 index = start;
    u32 last_index = start + count;
    if(type == PRIM_TRIANGLE_STRIP) last_index -= 2;
    while (index < last_index) {
        //project vertices to screen space
        ZunVec3 v0 = NDCToScreen(ProjectToNDC(ParseVec3((char*)vertexData + vertexStride * index), mvp)) + ZunVec3(0.5f/GAME_WINDOW_WIDTH, 0.5f/GAME_WINDOW_HEIGHT, 0);
        ZunVec3 v1 = NDCToScreen(ProjectToNDC(ParseVec3((char*)vertexData + vertexStride * (index+1)), mvp)) + ZunVec3(0.5f/GAME_WINDOW_WIDTH, 0.5f/GAME_WINDOW_HEIGHT, 0);;
        ZunVec3 v2 = NDCToScreen(ProjectToNDC(ParseVec3((char*)vertexData + vertexStride * (index+2)), mvp)) + ZunVec3(0.5f/GAME_WINDOW_WIDTH, 0.5f/GAME_WINDOW_HEIGHT, 0);;

        ZunVec2 tc0, tc1, tc2;
        if(useTexCoord) {
            const ZunVec2 texDim = {boundTexture ? boundTexture->width : 0, boundTexture ? boundTexture->height : 0};
            tc0 = ProjectTexCoordToNDC(ParseVec2((char*)texCoordData + texCoordStride * index), textureMatrix) * texDim;
            tc1 = ProjectTexCoordToNDC(ParseVec2((char*)texCoordData + texCoordStride * (index+1)), textureMatrix) * texDim;
            tc2 = ProjectTexCoordToNDC(ParseVec2((char*)texCoordData + texCoordStride * (index+2)), textureMatrix) * texDim;
        }
        
        // we dont do backface culling because the engine makes sure that the draw order is always consistent from back to front
        
        if (type == PRIM_TRIANGLE_STRIP && ((index - start) & 1))
        {
            std::swap(v0, v1);
            std::swap(tc0, tc1);
        }

        //force counter clockwise winding for rasterization
        if(EdgeFunction(v0, v1, v2) < 0) {
            std::swap(v1, v2);
            std::swap(tc1, tc2);
        }
        //do rasterization
        //barycentric magic
        i32 xmin = std::max(viewport[0],(i32)std::floor(std::min({v0.x, v1.x, v2.x})));
        i32 xmax = std::min(viewport[0] + viewport[2] - 1,(i32)std::ceil(std::max({v0.x, v1.x, v2.x})));
        i32 ymin = std::max(viewport[1],(i32)std::floor(std::min({v0.y, v1.y, v2.y})));
        i32 ymax = std::min(viewport[1] + viewport[3] - 1,(i32)std::ceil(std::max({v0.y, v1.y, v2.y})));

        const ZunVec3 vP = ZunVec3(xmin+0.5f, ymin+0.5f, 0);
        ZunVec3 edges = {EdgeFunction(v1, v2, vP), EdgeFunction(v2, v0, vP), EdgeFunction(v0, v1, vP)};
        f32 area = EdgeFunction(v0, v1, v2);

        if (area < 1) {
            index += increment;
            continue;
        }

        ZunVec3 e_dx = {v1.y - v2.y, v2.y - v0.y, v0.y - v1.y};
        ZunVec3 e_dy = {v2.x - v1.x, v0.x - v2.x, v1.x - v0.x};
    
        float invarea = 1.0f / area;
        ZunVec3 w0 = edges * invarea;
        const ZunVec3 w_dx = e_dx * invarea;
        const ZunVec3 w_dy = e_dy * invarea;

        ZunVec2 uv0 = tc0 * w0.x + tc1 * w0.y + tc2 * w0.z;
        const ZunVec2 uv_dx = tc0 * w_dx.x + tc1 * w_dx.y + tc2 * w_dx.z;
        const ZunVec2 uv_dy = tc0 * w_dy.x + tc1 * w_dy.y + tc2 * w_dy.z;

        for (int y = ymin; y <= ymax; y++) {
            ZunVec3 w = w0;
            ZunVec2 uv_row = uv0;
            for (int x = xmin; x <= xmax; x++) {
                if (w.x >= 0 && w.y >= 0 && w.z >= 0) {
                    if (boundTexture && useTexCoord) {
                        ZunColor texColor = boundTexture->GetPixel(uv.x, uv.y);
                        drawPoint(x, y, texColor);
                    }
                    else {
                        drawPoint(x, y, COLOR_LAVENDER);
                    }
                }
                w += w_dx;
                uv += uv_dx;
            }
            w0 += w_dy;
            uv0 += uv_dy;
        }
        index += increment;
    }
}

void Software::SwapBuffers()
{
    SDL_UpdateTexture(framebufferTexture, NULL, framebuffer, GAME_WINDOW_WIDTH * sizeof(u32));
    SDL_RenderCopy(renderer, framebufferTexture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

ZunColor Texture::GetPixel(i32 x, i32 y)
{
    ZunColor color = COLOR_LIGHTCYAN;
    return texels.data()[y * width + x];


    /*switch (type) { //lovely UB
        case PIXEL_UNSIGNED_BYTE: {
            const u8* pixel = (const u8*)data + (y * width + x) * 4;
            color = RGBAToZunColor(pixel[0], pixel[1], pixel[2], pixel[3]);
            break;
        }
        case PIXEL_UNSIGNED_SHORT_4_4_4_4:
        {
            const u16* pixel16 = (const u16*)data + (y * width + x);
            u16 p = pixel16[0];

            u8 r = ((p >> 12) & 0xF) * 17;
            u8 g = ((p >> 8)  & 0xF) * 17;
            u8 b = ((p >> 4)  & 0xF) * 17;
            u8 a = (p & 0xF) * 17;

            color = (a << 24) | (r << 16) | (g << 8) | b;
            break;
        }

        case PIXEL_UNSIGNED_SHORT_5_5_5_1:
        {
            const u16* pixel5551 = (const u16*)data + (y * width + x);
            u16 p = pixel5551[0];

            u8 r = ((p >> 11) & 0x1F) * 255 / 31;
            u8 g = ((p >> 6)  & 0x1F) * 255 / 31;
            u8 b = ((p >> 1)  & 0x1F) * 255 / 31;
            u8 a = (p & 0x1) ? 255 : 0;

            color = (a << 24) | (r << 16) | (g << 8) | b;
            break;
        }

        case PIXEL_UNSIGNED_SHORT_5_6_5:
        {
            const u16* pixel565 = (const u16*)data + (y * width + x);
            u16 p = pixel565[0];

            u8 r = ((p >> 11) & 0x1F) * 255 / 31;
            u8 g = ((p >> 5)  & 0x3F) * 255 / 63;
            u8 b = (p & 0x1F) * 255 / 31;

            color = (255 << 24) | (r << 16) | (g << 8) | b;
            break;
        }
    }
    if (format == PIXEL_RGB) color |= 0xFF000000;

    return color;*/
}
