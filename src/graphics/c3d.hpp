#pragma once

#include "GfxInterface.hpp"
#include <3ds.h>
#include <citro3d.h>
#include <vector>


/*
 * ff_pica_setup.c - citro3d state that replaces ff.frag on the PICA200.
 *
 *  ff.frag feature          PICA200 equivalent
 *  ----------------------   ---------------------------------------------
 *  useTexCoords ? tex : diffuse   TEV source: TEXTURE0 or PRIMARY_COLOR
 *  envDiffuse                     TEV source: CONSTANT (C3D_TexEnvColor)
 *  colorOp MODULATE/ADD/REPLACE   TEV combiner function
 *  fog (near/far/color)           fog LUT + C3D_FogColor
 *  discard if a < 4/255           alpha test GEQUAL 4
 */
#include <math.h>
#include <string.h>
#include "ff_shbin.h"   /* generated from ff.v.pica */

typedef enum { FF_OP_MODULATE = 0, FF_OP_ADD = 1, FF_OP_REPLACE = 2 } FFColorOp;

typedef struct {
    DVLB_s*        dvlb;
    shaderProgram_s program;
    int uLoc_modelview, uLoc_projection, uLoc_texMtx;
    C3D_FogLut     fogLut;
} FFShader;

struct C3DTexture
{
    C3D_Tex tex;
    u32 width, height;
    u8* pixels;
    bool initialized;
};

inline u32 ZunColorToBGR(ZunColor color)
{
    return ((color & 0xFF) << 16) | (color & 0xFF00) | ((color >> 16) & 0xFF);
}

/* Attribute layout: v0 = position, v1 = texCoords, v2 = diffuse */
typedef struct { float x, y, z; float u, v; u8 r, g, b, a; } FFVertex;

struct C3D : GfxInterface
{
    static GfxInterface *Create();

    bool Init();
    virtual void Exit();
    ~C3D() override
    {
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

    virtual void GetViewport(u32 *viewport);
    virtual void GetDepthRange(f32 *depthRange);
    virtual void SetViewport(i32 x, i32 y, i32 width, i32 height);
    virtual void SetDepthRange(f32 nearPlane, f32 farPlane);

    virtual void Enable(Capabilities cap);
    virtual bool HasError();
    virtual void SetBlendMode(BlendMode mode);
    virtual void SetDepthMask(bool enable);
    virtual void SetDepthFunc(DepthFunc func);

    virtual void SetClearDepth(f32 depth);
    virtual void SetClearColor(f32 r, f32 g, f32 b, f32 a);
    virtual void Clear(u32 clearBits);

    virtual GfxTextureHandle CreateTexture();
    virtual void BindTexture(GfxTextureHandle handle);
    virtual void DeleteTexture(GfxTextureHandle handle);
    virtual void SetTextureImage(u32 width, u32 height, PixelFormat fmt, PixelDataType type, const void *data);
    virtual void SetTextureSubImage(i32 xoffset, i32 yoffset, i32 width, i32 height, const void *data);

    virtual void ReadPixels(i32 x, i32 y, i32 width, i32 height, const void *pixels);

    virtual void Draw(PrimitiveType type, i32 start, i32 count);
    virtual void SwapBuffers();

  private:
    C3D_RenderTarget *target = nullptr;
    bool gfxInitialized = false;
    bool c3dInitialized = false;
    bool frameBegun = false;
    bool shaderInitialized = false;
    bool hasError = false;
    bool depthTestEnabled = false;
    bool depthMask = true;
    bool useTexCoords = false;
    bool noVertexBuffer = false;
    u32 viewport[4] = {};
    f32 depthRange[2] = {0.0f, 1.0f};
    f32 fogNear = 0.0f, fogFar = 1.0f;
    f32 projectionNear = 100.0f, projectionFar = 10000.0f;
    ZunColor fogColor = 0;
    f32 clearDepth = 1.0f;
    f32 clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    DepthFunc depthFunc = DEPTH_FUNC_LEQUAL;
    ColorOp colorOps[2] = {COLOR_OP_MODULATE, COLOR_OP_MODULATE};
    ZunColor textureFactor = 0xFFFFFFFF;
    FFShader ffShader;
    void *vbo_data = nullptr;
    const void *attributePointers[3] = {nullptr, nullptr, nullptr};
    std::size_t attributeStrides[3] = {0, 0, 0};
    std::vector<C3DTexture> textures;
    u32 currentTexture = 0;
};
