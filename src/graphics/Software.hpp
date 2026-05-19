#pragma once

#include "GfxInterface.hpp"
#include <SDL2/SDL.h>
#include <vector>
#include <memory>

struct Texture {
    std::vector<u32> texels; //RGBA32
    i32 width, height;
    PixelFormat format;
    PixelDataType type;
    ZunColor GetPixel(i32 x, i32 y);
};

struct Software : GfxInterface
{
    static GfxInterface *Init();
    static void SetContextFlags();
    virtual void Exit();
    ~Software() override {
        Exit();
    };

    virtual void SetFogRange(f32 nearPlane, f32 farPlane);
    virtual void SetFogColor(ZunColor color);
    virtual void ToggleVertexAttribute(u8 attr, bool enable);
    virtual void SetAttributePointer(VertexAttributeArrays attr, std::size_t stride, void *ptr);
    virtual void SetColorOp(TextureOpComponent component, ColorOp op);
    virtual void SetTextureFactor(ZunColor factor);
    virtual void SetTransformMatrix(TransformMatrix type, const ZunMatrix &matrix);

    virtual void SetTextureFilter();

    virtual void GetViewport(u32* viewport);
    virtual void GetDepthRange(f32* depthRange);
    virtual void SetViewport(i32 x, i32 y, i32 width, i32 height);
    virtual void SetDepthRange(f32 near, f32 far);

    virtual void Enable(Capabilities cap);
    virtual void SetBlendMode(BlendMode mode);
    virtual void SetDepthMask(bool enable);
    virtual void SetDepthFunc(DepthFunc func);

    virtual void SetClearDepth(f32 depth);
    virtual void SetClearColor(f32 r, f32 g, f32 b, f32 a);
    virtual void Clear(u32 clearBits);

    virtual GfxTextureHandle CreateTexture();
    virtual void BindTexture(GfxTextureHandle handle);
    virtual void DeleteTexture(GfxTextureHandle handle);
    virtual void SetTextureImage(u32 width, u32 height, PixelFormat fmt, PixelDataType type, const void* data);
    virtual void SetTextureSubImage(i32 xoffset, i32 yoffset, i32 width, i32 height, const void* data);

    virtual void ReadPixels(i32 x, i32 y, i32 width, i32 height, const void* pixels);

    virtual void Draw(PrimitiveType type, i32 start, i32 count);
    virtual void SwapBuffers();


  private:
    std::vector<std::unique_ptr<Texture>> textures;

    Texture* boundTexture = nullptr;

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* framebufferTexture;
    u32* framebuffer;

    f32* depthBuffer;

    i32 viewport[4];   //x, y, w, h
    ZunColor clearColor; //r, g, b, a
    f32 clearDepth;    //0..1
    f32 depthRange[2]; //near, far
    bool depthMask;
    DepthFunc depthFunc;

    ZunColor textureFactor;

    ZunMatrix model;
    ZunMatrix view;
    ZunMatrix projection;
    ZunMatrix textureMatrix;

    void* vertexData;
    std::size_t vertexStride;
    void* texCoordData;
    std::size_t texCoordStride;
    void* diffuseData;
    std::size_t diffuseStride;

    bool useTexCoord;
    bool useDiffuse;

    void drawLine(i32 x1, i32 y1, i32 x2, i32 y2);
    void drawPoint(i32 x, i32 y, ZunColor color);
    inline ZunVec3 ProjectToNDC(ZunVec3 vertex, ZunMatrix mvp);
    inline ZunVec2 ProjectTexCoordToNDC(ZunVec2 texCoord, ZunMatrix textureMatrix);
    inline ZunVec3 NDCToScreen(ZunVec3 vertex);
};
