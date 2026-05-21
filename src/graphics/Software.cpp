#include "Software.hpp"
#include "Supervisor.hpp"
#include "GameWindow.hpp"
#include "i18n.hpp"
#include <SDL2/SDL.h>
#include <algorithm>
#include <cmath>
#include "utils.hpp"

constexpr f32 alphaThreshold = 1.0f/255.0f;

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

    SDL_Texture* framebufferTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT);
    self->framebufferTexture = framebufferTexture;
    if (framebufferTexture == NULL)    {
        delete self;
        return NULL;
    }
    u32* framebuffer = new u32[GAME_WINDOW_WIDTH * GAME_WINDOW_HEIGHT];
    self->framebuffer = framebuffer;

    f32* depthBuffer = new f32[GAME_WINDOW_WIDTH * GAME_WINDOW_HEIGHT];
    self->depthBuffer = depthBuffer;
    self->noVertexBuffer = g_Supervisor.cfg.opts & (1 << GCOS_DONT_USE_VERTEX_BUF);
    self->noFog = g_Supervisor.cfg.opts & (1 << GCOS_DONT_USE_FOG);

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
    fogNear = nearPlane;
    fogFar = farPlane;
}

void Software::SetFogColor(ZunColor color)
{
    fogColor = color;
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

    if (component == COMPONENT_ALPHA)
    {
        return;
    }
    colorOp = op;

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
    blendMode = mode;
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
    depthRange[0] = this->depthNear;
    depthRange[1] = this->depthFar;
}


inline ZunColor RGBAToZunColor(u8 r, u8 g, u8 b, u8 a) {
    return ((ZunColor)a << 24) | ((ZunColor)r << 16) | ((ZunColor)g << 8) | (ZunColor)b;
}

void Software::SetClearColor(f32 r, f32 g, f32 b, f32 a) {
    clearColor = RGBAToZunColor((u8)(r * 255), (u8)(g * 255), (u8)(b * 255), (u8)(a * 255));
}

void Software::SetTextureFilter() {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
}

void Software::SetClearDepth(f32 depth) {
    clearDepth = depth;
}

void Software::Clear(u32 clearBits) {
    if (clearBits & CLEAR_COLOR_BUFFER) {
        std::fill(framebuffer, framebuffer + GAME_WINDOW_WIDTH * GAME_WINDOW_HEIGHT, clearColor);
    }
    if (clearBits & CLEAR_DEPTH_BUFFER) {
        std::fill(depthBuffer, depthBuffer + GAME_WINDOW_WIDTH * GAME_WINDOW_HEIGHT, clearDepth);
    }
    //g_glFuncTable.glClear(mask);
}

void Software::SetDepthRange(f32 near, f32 far) {
    //g_glFuncTable.glDepthRangef(near, far);
    depthNear = near;
    depthFar = far;
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
        if(data) SDL_ConvertPixels(width,height, GetSDLPixelFormat(fmt,type), data, width * bpp, SDL_PIXELFORMAT_ARGB8888, boundTexture->texels.data(), width * sizeof(u32));
        boundTexture->width = width;
        boundTexture->height = height;
        boundTexture->format = fmt;
        boundTexture->type = type;
    }
}

void Software::SetTextureSubImage(i32 xoffset, i32 yoffset, i32 width, i32 height, const void *data)
{
    if (boundTexture) {
        SDL_ConvertPixels(width,height, SDL_PIXELFORMAT_RGB24, data, width*3, SDL_PIXELFORMAT_ARGB8888, boundTexture->texels.data() + (yoffset * boundTexture->width) + xoffset, boundTexture->width*sizeof(u32));
    }
}

void Software::ReadPixels(i32 x, i32 y, i32 width, i32 height, const void* pixels) {
    u8* dst = (u8*)pixels;
    i32 pitch = width * 4;
    for (i32 row = 0; row < height; row++) {
        const u8* src = (u8*)framebuffer + ((GAME_WINDOW_HEIGHT - 1 - (y + row)) * GAME_WINDOW_WIDTH + x) * 4;
        memcpy(dst + row * pitch, src, pitch);
    }
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

ColorData ParseColorData(const void* data) {
    ColorData col;
    memcpy(&col, data, sizeof(ColorData));
    return col;
}

inline ZunVec3 Software::ProjectToNDC(ZunVec3 vertex, ZunMatrix mv, ZunMatrix p, f32 &viewZ, f32 &W) {
    ZunVec4 clip = mv * ZunVec4(vertex, 1.0f);
    viewZ = clip.z;
    clip = p * clip;
    ZunVec3 ndc = {clip.x, clip.y, clip.z};
    if (clip.w != 0) {
        ndc /= clip.w;
        W = 1.0f/clip.w;
    }

    return ndc;
}

inline ZunVec2 Software::ProjectTexCoordToNDC(ZunVec2 texCoord, ZunMatrix textureMatrix) {
    ZunVec4 clip = textureMatrix * ZunVec4(ZunVec3(texCoord.x, texCoord.y, 1.0f), 1.0f);
    ZunVec2 ndc = {clip.x, clip.y};
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
    u32 increment = type == PRIM_TRIANGLE_STRIP ? 1 : 3;
    u32 index = start;
    u32 last_index = start + count;
    if(type == PRIM_TRIANGLE_STRIP) last_index -= 2;
    ZunMatrix modelview = view * model;
    while (index < last_index) {
        //project vertices to screen space
        
        f32 invw0 = 0, invw1 = 0, invw2 = 0; //store inverse W to correct for perspective projection
        f32 viewZ0, viewZ1, viewZ2;
        f32 ndcZ0, ndcZ1, ndcZ2;
        ZunVec3 v0 = ProjectToNDC(ParseVec3((u8*)vertexData + vertexStride * index),modelview,projection,viewZ0,invw0);
        ZunVec3 v1 = ProjectToNDC(ParseVec3((u8*)vertexData + vertexStride * (index+1)),modelview,projection,viewZ1,invw1);
        ZunVec3 v2 = ProjectToNDC(ParseVec3((u8*)vertexData + vertexStride * (index+2)),modelview,projection,viewZ2,invw2);
        ndcZ0 = v0.z;
        ndcZ1 = v1.z;
        ndcZ2 = v2.z;
        v0 = NDCToScreen(v0) + ZunVec3(0.5f, 0.5f, 0);
        v1 = NDCToScreen(v1) + ZunVec3(0.5f, 0.5f, 0);
        v2 = NDCToScreen(v2) + ZunVec3(0.5f, 0.5f, 0);


        ZunVec2 tc0, tc1, tc2;
        ColorData diffuse;
        if(useTexCoord) {
            const ZunVec2 texDim = {boundTexture ? boundTexture->width : 0, boundTexture ? boundTexture->height : 0};
            tc0 = ProjectTexCoordToNDC(ParseVec2((u8*)texCoordData + texCoordStride * index), textureMatrix) * texDim;
            tc1 = ProjectTexCoordToNDC(ParseVec2((u8*)texCoordData + texCoordStride * (index+1)), textureMatrix) * texDim;
            tc2 = ProjectTexCoordToNDC(ParseVec2((u8*)texCoordData + texCoordStride * (index+2)), textureMatrix) * texDim;
        }

        if(useDiffuse) {
            diffuse = ParseColorData((u8*)diffuseData + diffuseStride * index); // the engine always sends the same diffuse value for all 3 vertices
        }
        
        // we dont do backface culling because the engine makes sure that the draw order is always consistent from back to front
        
        if (type == PRIM_TRIANGLE_STRIP && ((index - start) & 1))
        {
            std::swap(v0, v1);
            std::swap(tc0, tc1);
            std::swap(invw0, invw1);
            std::swap(ndcZ0, ndcZ1);
        }

        //force counter clockwise winding for rasterization
        if(EdgeFunction(v0, v1, v2) < 0) {
            std::swap(v1, v2);
            std::swap(tc1, tc2);
            std::swap(invw1, invw2);
            std::swap(ndcZ1, ndcZ2);
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

        ZunVec3 e_dx = {v1.y - v2.y, v2.y - v0.y, v0.y - v1.y};
        ZunVec3 e_dy = {v2.x - v1.x, v0.x - v2.x, v1.x - v0.x};
    
        float invarea = 1.0f / area;
        ZunVec3 w0 = edges * invarea;
        const ZunVec3 w_dx = e_dx * invarea;
        const ZunVec3 w_dy = e_dy * invarea;

        //ZunVec2 uv0 = (tc0*invz0) * w0.x + (tc1*invz1) * w0.y + (tc2*invz2) * w0.z;
        const ZunVec2 uv_dx = ((tc0*invw0) * w_dx.x + (tc1*invw1) * w_dx.y + (tc2*invw2) * w_dx.z);
        const ZunVec2 uv_dy = ((tc0*invw0) * w_dy.x + (tc1*invw1) * w_dy.y + (tc2*invw2) * w_dy.z);

        for (int y = ymin; y <= ymax; y++) {
            ZunVec3 w = w0;
            //ZunVec2 uv = uv0;
            for (int x = xmin; x <= xmax; x++) {
                if (w.x >= 0 && w.y >= 0 && w.z >= 0) {
                    const i32 pixelCoord = y * GAME_WINDOW_WIDTH + x;
                    f32 invw = w.x * invw0 + w.y * invw1 + w.z * invw2;
                    f32 depth = (w.x * ndcZ0*invw0 + w.y * ndcZ1*invw1 + w.z * ndcZ2*invw2)/invw * (depthFar - depthNear) + depthNear;
                    if(depthFunc == DEPTH_FUNC_LEQUAL && depth > depthBuffer[pixelCoord]) {
                        w += w_dx;
                        continue;
                    }
                    if(depthMask) depthBuffer[pixelCoord] = depth;
                    FragColor fragColor = 0;
                    FragColor fragArg1 = fragColor;
                    FragColor fragArg2 = fragColor;
                    if (boundTexture && useTexCoord) {
                        ZunVec2 uv = ((tc0*invw0) * w.x + (tc1*invw1) * w.y + (tc2*invw2) * w.z) / invw;
                        fragArg1 = boundTexture->GetPixel(uv.x, uv.y);
                    } else {
                        fragArg1 = diffuse;
                    }

                    if(!noVertexBuffer) {
                        fragArg2 = textureFactor;
                    } else {
                        fragArg2 = diffuse;
                    }

                    switch(colorOp) {
                        case COLOR_OP_MODULATE:
                            fragColor = fragArg1 * fragArg2;
                            break;
                        case COLOR_OP_ADD:
                            fragColor.a = fragArg1.a * fragArg2.a;
                            fragColor.r = ZUN_MIN(fragArg1.r+fragArg2.r, 1.0f);
                            fragColor.g = ZUN_MIN(fragArg1.g+fragArg2.g, 1.0f);
                            fragColor.b = ZUN_MIN(fragArg1.b+fragArg2.b, 1.0f);
                            break;
                        case COLOR_OP_REPLACE:
                            fragColor = fragArg1;
                    }

                    if(!noFog) {
                        f32 depth = (w.x * viewZ0*invw0 + w.y * viewZ1*invw1 + w.z * viewZ2*invw2)/invw;
                        f32 fogCoefficient = (fogFar - depth) / (fogFar - fogNear);
                        fogCoefficient = ZUN_MIN(ZUN_MAX(fogCoefficient, 0.0f), 1.0f);
                        fragColor = fragColor.InterpolateRGB(fogColor, 1.0f - fogCoefficient);
                    }

                    FragColor src = fragColor;
                    FragColor dst = framebuffer[pixelCoord];

                    f32 sourceFactor = src.a;
                    f32 destFactor = 1.0f;
                    if(blendMode == BLEND_INV_SRC_ALPHA) destFactor -= sourceFactor;
                    fragColor = src * sourceFactor + dst * destFactor;

                    if(fragColor.a >= alphaThreshold) {
                        framebuffer[pixelCoord] = fragColor;
                    }
                }
                w += w_dx;
                //uv += uv_dx;
            }
            w0 += w_dy;
            //uv0 += uv_dy;
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

inline ZunColor Texture::GetPixel(i32 x, i32 y)
{
    y %= height;
    if (y < 0) y += height;
    x %= width;
    if (x < 0) x += width;
    return texels.data()[y * width + x];
}
