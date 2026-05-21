#pragma once

#include "AnmManager.hpp"
#include "GfxInterface.hpp"
#include <SDL2/SDL.h>
#include <vector>
#include <memory>

struct Texture {
    std::vector<u32> texels; //RGBA32
    i32 width, height;
    PixelFormat format;
    PixelDataType type;
    inline ZunColor GetPixel(i32 x, i32 y);
};

//It is extremely recommended that you compile as Release if you want to use the software rasterizer
//because it runs extremely slow on Debug

struct FragColor { //0..1
    f32 r;
    f32 g;
    f32 b;
    f32 a;

    FragColor()
    {
    }

    operator ColorData() const {
        return {(u32)(r*255.f),(u32)(g*255.f),(u32)(b*255.f),(u32)(a*255.f)};
    }
    
    operator ZunColor() const {
        return (((u32)(a*255.f)) << 24) | (((u32)(r*255.f)) << 16) | (((u32)(g*255.f)) << 8) | ((u32)(b*255.f));
    }

    FragColor(f32 r, f32 g, f32 b, f32 a) {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }

    FragColor(ZunColor color)
    {
        a = (color >> 24) / 255.f;
        r = ((color >> 16) & 0xFF)/ 255.f;
        g = ((color >> 8) & 0xFF) / 255.f;
        b = (color & 0xFF) / 255.f;
    };

    FragColor(const ColorData &color) {
        r = color.r / 255.f;
        g = color.g / 255.f;
        b = color.b / 255.f;
        a = color.a / 255.f;
    }

    FragColor operator*(const FragColor c) {
        return FragColor(r*c.r, g*c.g, b*c.b, a*c.a);
    }

    FragColor operator*(const f32 c) {
        return FragColor(r*c, g*c, b*c, a*c);
    }

    FragColor operator+(const FragColor c) {
        return FragColor(r+c.r, g+c.g, b+c.b, a+c.a);
    }

    FragColor operator+(const f32 c) {
        return FragColor(r+c, g+c, b+c, a+c);
    }

    FragColor InterpolateRGB(const FragColor &c2, f32 factor) const {
        return FragColor(r * (1-factor) + c2.r * factor, g * (1-factor) + c2.g * factor, b * (1-factor) + c2.b * factor, a);
    }
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
    f32 fogNear;
    f32 fogFar;
    ZunColor fogColor;
    f32 depthNear, depthFar;
    bool depthMask;
    DepthFunc depthFunc;

    ZunColor textureFactor;
    BlendMode blendMode;

    ZunMatrix model;
    ZunMatrix view;
    ZunMatrix projection;
    ZunMatrix textureMatrix;

    bool noVertexBuffer;
    bool noFog;
    bool useFragDepth;

    void* vertexData;
    std::size_t vertexStride;
    void* texCoordData;
    std::size_t texCoordStride;
    void* diffuseData;
    std::size_t diffuseStride;

    bool useTexCoord = false;
    bool useDiffuse = false;

    ColorOp colorOp;

    inline ZunVec3 ProjectToNDC(ZunVec3 vertex, ZunMatrix mv, ZunMatrix p, f32 &viewZ, f32 &W);
    inline ZunVec2 ProjectTexCoordToNDC(ZunVec2 texCoord, ZunMatrix textureMatrix);
    inline ZunVec3 NDCToScreen(ZunVec3 vertex);
};
