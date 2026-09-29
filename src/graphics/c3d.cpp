#include "c3d.hpp"
#include "GameWindow.hpp"
#include "Supervisor.hpp"

#include <cstring>

namespace
{
constexpr u32 MAX_VERTEX_COUNT = 0x18000;
constexpr u32 C3D_DISPLAY_TRANSFER_FLAGS =
    GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) |
    GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) |
    GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO);
constexpr u32 C3D_READBACK_FLAGS =
    GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) |
    GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGBA8) |
    GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO);

void setMatrix(C3D_Mtx *destination, const ZunMatrix &source)
{
    destination->r[0].x = source.m[0][0];
    destination->r[0].y = source.m[1][0];
    destination->r[0].z = source.m[2][0];
    destination->r[0].w = source.m[3][0];
    destination->r[1].x = source.m[0][1];
    destination->r[1].y = source.m[1][1];
    destination->r[1].z = source.m[2][1];
    destination->r[1].w = source.m[3][1];
    destination->r[2].x = source.m[0][2];
    destination->r[2].y = source.m[1][2];
    destination->r[2].z = source.m[2][2];
    destination->r[2].w = source.m[3][2];
    destination->r[3].x = source.m[0][3];
    destination->r[3].y = source.m[1][3];
    destination->r[3].z = source.m[2][3];
    destination->r[3].w = source.m[3][3];
}

void decodeTexturePixel(u8 *rgba, PixelFormat format, PixelDataType type, const u8 *source)
{
    if (type == PIXEL_UNSIGNED_BYTE)
    {
        rgba[0] = source[0];
        rgba[1] = source[1];
        rgba[2] = source[2];
        rgba[3] = format == PIXEL_RGBA ? source[3] : 0xFF;
        return;
    }

    u16 value;
    std::memcpy(&value, source, sizeof(value));
    switch (type)
    {
    case PIXEL_UNSIGNED_SHORT_5_5_5_1:
        rgba[0] = ((value >> 11) & 0x1F) * 255 / 31;
        rgba[1] = ((value >> 6) & 0x1F) * 255 / 31;
        rgba[2] = ((value >> 1) & 0x1F) * 255 / 31;
        rgba[3] = (value & 1) ? 0xFF : 0;
        break;
    case PIXEL_UNSIGNED_SHORT_5_6_5:
        rgba[0] = ((value >> 11) & 0x1F) * 255 / 31;
        rgba[1] = ((value >> 5) & 0x3F) * 255 / 63;
        rgba[2] = (value & 0x1F) * 255 / 31;
        rgba[3] = 0xFF;
        break;
    case PIXEL_UNSIGNED_SHORT_4_4_4_4:
        rgba[0] = ((value >> 12) & 0xF) * 17;
        rgba[1] = ((value >> 8) & 0xF) * 17;
        rgba[2] = ((value >> 4) & 0xF) * 17;
        rgba[3] = (value & 0xF) * 17;
        break;
    case PIXEL_UNSIGNED_BYTE:
        break;
    }
}

void uploadTexture(C3DTexture &texture)
{
    C3D_TexUpload(&texture.tex, texture.pixels);
}

bool ff_init(FFShader *shader)
{
    shader->dvlb = DVLB_ParseFile((u32 *)ff_shbin, ff_shbin_size);
    if (shader->dvlb == nullptr)
    {
        return false;
    }
    shaderProgramInit(&shader->program);
    shaderProgramSetVsh(&shader->program, &shader->dvlb->DVLE[0]);
    C3D_BindProgram(&shader->program);

    shader->uLoc_modelview = shaderInstanceGetUniformLocation(shader->program.vertexShader, "modelviewMatrix");
    shader->uLoc_projection = shaderInstanceGetUniformLocation(shader->program.vertexShader, "projectionMatrix");
    shader->uLoc_texMtx = shaderInstanceGetUniformLocation(shader->program.vertexShader, "textureMatrix");

    C3D_AttrInfo *attributes = C3D_GetAttrInfo();
    AttrInfo_Init(attributes);
    AttrInfo_AddLoader(attributes, 0, GPU_FLOAT, 3);
    AttrInfo_AddLoader(attributes, 1, GPU_FLOAT, 2);
    AttrInfo_AddLoader(attributes, 2, GPU_UNSIGNED_BYTE, 4);

    C3D_AlphaTest(true, GPU_GEQUAL, 4);
    return true;
}

void ff_free(FFShader *shader)
{
    shaderProgramFree(&shader->program);
    DVLB_Free(shader->dvlb);
}

void ff_set_color_op(ColorOp rgbOp, ColorOp alphaOp, bool useTexCoords, bool noVertexBuffer, u32 envDiffuse)
{
    const GPU_TEVSRC arg1 = useTexCoords ? GPU_TEXTURE0 : GPU_PRIMARY_COLOR;
    const GPU_TEVSRC arg2 = noVertexBuffer ? GPU_PRIMARY_COLOR : GPU_CONSTANT;
    C3D_TexEnv *env = C3D_GetTexEnv(0);
    C3D_TexEnvInit(env);
    C3D_TexEnvColor(env, envDiffuse);

    C3D_TexEnvSrc(env, C3D_RGB, arg1, arg2, GPU_PRIMARY_COLOR);
    switch (rgbOp)
    {
    case COLOR_OP_MODULATE:
        C3D_TexEnvFunc(env, C3D_RGB, GPU_MODULATE);
        break;
    case COLOR_OP_ADD:
        C3D_TexEnvFunc(env, C3D_RGB, GPU_ADD);
        break;
    case COLOR_OP_REPLACE:
        C3D_TexEnvSrc(env, C3D_RGB, arg1, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR);
        C3D_TexEnvFunc(env, C3D_RGB, GPU_REPLACE);
        break;
    }

    C3D_TexEnvSrc(env, C3D_Alpha, arg1, arg2, GPU_PRIMARY_COLOR);
    if (alphaOp == COLOR_OP_REPLACE)
    {
        C3D_TexEnvSrc(env, C3D_Alpha, arg1, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR);
        C3D_TexEnvFunc(env, C3D_Alpha, GPU_REPLACE);
    }
    else
    {
        C3D_TexEnvFunc(env, C3D_Alpha, GPU_MODULATE);
    }

    for (int stage = 1; stage < 6; ++stage)
    {
        C3D_TexEnvInit(C3D_GetTexEnv(stage));
    }
}

float depthToViewZ(float depth, float nearPlane, float farPlane)
{
    const float denominator = 1.0f - depth * (farPlane - nearPlane) / farPlane;
    return denominator <= 1e-6f ? farPlane : nearPlane / denominator;
}

void ff_set_fog(FFShader *shader, bool enabled, float fogNear, float fogFar, u32 fogColor,
                float projectionNear, float projectionFar)
{
    if (!enabled || fogFar <= fogNear || projectionNear <= 0.0f || projectionFar <= projectionNear)
    {
        C3D_FogGasMode(GPU_NO_FOG, GPU_PLAIN_DENSITY, false);
        return;
    }

    float lutData[256];
    for (int i = 0; i < 128; ++i)
    {
        const float depth = static_cast<float>(i) / 128.0f;
        const float viewZ = depthToViewZ(depth, projectionNear, projectionFar);
        const float coefficient = (fogFar - viewZ) / (fogFar - fogNear);
        lutData[i] = fminf(fmaxf(coefficient, 0.0f), 1.0f);
    }
    for (int i = 0; i < 128; ++i)
    {
        lutData[128 + i] = i < 127 ? lutData[i + 1] - lutData[i] : 0.0f;
    }

    FogLut_FromArray(&shader->fogLut, lutData);
    C3D_FogGasMode(GPU_FOG, GPU_PLAIN_DENSITY, false);
    C3D_FogColor(fogColor);
    C3D_FogLutBind(&shader->fogLut);
}
} // namespace

GfxInterface *C3D::Create()
{
    C3D *self = new C3D();
    if (!self->Init())
    {
        delete self;
        return nullptr;
    }
    return self;
}

bool C3D::Init()
{
    gfxInitDefault();
    this->gfxInitialized = true;
    if (!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE))
    {
        return false;
    }
    this->c3dInitialized = true;

    this->target = C3D_RenderTargetCreate(GAME_WINDOW_WIDTH_REAL, GAME_WINDOW_HEIGHT_REAL,
                                          GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    if (this->target == nullptr)
    {
        return false;
    }
    C3D_RenderTargetSetOutput(this->target, GFX_TOP, GFX_LEFT, C3D_DISPLAY_TRANSFER_FLAGS);

    this->vbo_data = linearAlloc(sizeof(FFVertex) * MAX_VERTEX_COUNT);
    if (this->vbo_data == nullptr)
    {
        return false;
    }
    C3D_BufInfo *bufferInfo = C3D_GetBufInfo();
    BufInfo_Init(bufferInfo);
    BufInfo_Add(bufferInfo, this->vbo_data, sizeof(FFVertex), 3, 0x210);

    if (!ff_init(&this->ffShader))
    {
        return false;
    }
    this->shaderInitialized = true;
    this->noVertexBuffer = (g_Supervisor.cfg.opts & (1 << GCOS_DONT_USE_VERTEX_BUF)) != 0;
    if ((g_Supervisor.cfg.opts & (1 << GCOS_NO_COLOR_COMP)) != 0)
    {
        this->colorOps[COMPONENT_RGB] = COLOR_OP_REPLACE;
        this->colorOps[COMPONENT_ALPHA] = COLOR_OP_REPLACE;
    }

    ZunMatrix identity;
    identity.Identity();
    C3D_Mtx matrix;
    setMatrix(&matrix, identity);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, this->ffShader.uLoc_modelview, &matrix);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, this->ffShader.uLoc_projection, &matrix);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, this->ffShader.uLoc_texMtx, &matrix);

    this->viewport[2] = GAME_WINDOW_WIDTH_REAL;
    this->viewport[3] = GAME_WINDOW_HEIGHT_REAL;
    C3D_SetViewport(0, 0, this->viewport[2], this->viewport[3]);
    C3D_DepthMap(true, 1.0f, 0.0f);
    C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA,
                   GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
    C3D_DepthTest(false, GPU_LEQUAL, GPU_WRITE_ALL);
    ff_set_color_op(COLOR_OP_MODULATE, COLOR_OP_MODULATE, false, this->noVertexBuffer,
                    ZunColorToBGR(this->textureFactor));
    ff_set_fog(&this->ffShader, false, this->fogNear, this->fogFar, 0,
               this->projectionNear, this->projectionFar);

    this->frameBegun = C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    if (!this->frameBegun || !C3D_FrameDrawOn(this->target))
    {
        this->hasError = true;
        return false;
    }
    return true;
}

void C3D::Exit()
{
    if (this->frameBegun)
    {
        C3D_FrameEnd(0);
        this->frameBegun = false;
    }
    if (this->c3dInitialized)
    {
        for (C3DTexture &texture : this->textures)
        {
            if (texture.initialized)
            {
                C3D_TexDelete(&texture.tex);
            }
            if (texture.pixels != nullptr)
            {
                linearFree(texture.pixels);
            }
        }
        if (this->shaderInitialized)
        {
            ff_free(&this->ffShader);
            this->shaderInitialized = false;
        }
        if (this->vbo_data != nullptr)
        {
            linearFree(this->vbo_data);
            this->vbo_data = nullptr;
        }
        if (this->target != nullptr)
        {
            C3D_RenderTargetDelete(this->target);
            this->target = nullptr;
        }
        C3D_Fini();
        this->c3dInitialized = false;
    }
    if (this->gfxInitialized)
    {
        gfxExit();
        this->gfxInitialized = false;
    }
}

void C3D::SetFogRange(f32 nearPlane, f32 farPlane)
{
    this->fogNear = nearPlane;
    this->fogFar = farPlane;
    const bool enabled = (g_Supervisor.cfg.opts & (1 << GCOS_DONT_USE_FOG)) == 0;
    ff_set_fog(&this->ffShader, enabled, this->fogNear, this->fogFar,
               ZunColorToBGR(this->fogColor), this->projectionNear, this->projectionFar);
}

void C3D::SetFogColor(ZunColor color)
{
    this->fogColor = color;
    const bool enabled = (g_Supervisor.cfg.opts & (1 << GCOS_DONT_USE_FOG)) == 0;
    ff_set_fog(&this->ffShader, enabled, this->fogNear, this->fogFar,
               ZunColorToBGR(this->fogColor), this->projectionNear, this->projectionFar);
}

void C3D::ToggleVertexAttribute(u8 attr, bool enable)
{
    if (attr & VERTEX_ATTR_TEX_COORD)
    {
        this->useTexCoords = enable;
        ff_set_color_op(this->colorOps[COMPONENT_RGB], this->colorOps[COMPONENT_ALPHA], this->useTexCoords,
                        this->noVertexBuffer, ZunColorToBGR(this->textureFactor));
    }
}

void C3D::SetAttributePointer(VertexAttributeArrays attr, std::size_t stride, void *ptr)
{
    if (attr < VERTEX_ARRAY_POSITION || attr > VERTEX_ARRAY_DIFFUSE)
    {
        return;
    }
    this->attributePointers[attr] = ptr;
    this->attributeStrides[attr] = stride;
}

void C3D::SetColorOp(TextureOpComponent component, ColorOp op)
{
    if (component > COMPONENT_ALPHA || op > COLOR_OP_REPLACE)
    {
        return;
    }
    this->colorOps[component] = op;
    ff_set_color_op(this->colorOps[COMPONENT_RGB], this->colorOps[COMPONENT_ALPHA], this->useTexCoords,
                    this->noVertexBuffer, ZunColorToBGR(this->textureFactor));
}

void C3D::SetTextureFactor(ZunColor factor)
{
    this->textureFactor = factor;
    ff_set_color_op(this->colorOps[COMPONENT_RGB], this->colorOps[COMPONENT_ALPHA], this->useTexCoords,
                    this->noVertexBuffer, ZunColorToBGR(this->textureFactor));
}

void C3D::SetTransformMatrix(TransformMatrix type, const ZunMatrix &matrix)
{
    C3D_Mtx c3dMatrix;
    setMatrix(&c3dMatrix, matrix);
    switch (type)
    {
    case MATRIX_MODEL:
    case MATRIX_VIEW:
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, this->ffShader.uLoc_modelview, &c3dMatrix);
        break;
    case MATRIX_PROJECTION:
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, this->ffShader.uLoc_projection, &c3dMatrix);
        if (matrix.m[2][3] > 0.5f && fabsf(matrix.m[3][3]) < 1e-6f && matrix.m[2][2] > 1.0f)
        {
            const f32 nearPlane = -matrix.m[3][2] / (matrix.m[2][2] + 1.0f);
            const f32 farPlane = -matrix.m[3][2] / (matrix.m[2][2] - 1.0f);
            if (nearPlane > 0.0f && farPlane > nearPlane)
            {
                this->projectionNear = nearPlane;
                this->projectionFar = farPlane;
                const bool fogEnabled = (g_Supervisor.cfg.opts & (1 << GCOS_DONT_USE_FOG)) == 0;
                ff_set_fog(&this->ffShader, fogEnabled, this->fogNear, this->fogFar,
                           ZunColorToBGR(this->fogColor), this->projectionNear, this->projectionFar);
            }
        }
        break;
    case MATRIX_TEXTURE:
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, this->ffShader.uLoc_texMtx, &c3dMatrix);
        break;
    }
}

void C3D::SetTextureFilter()
{
    if (this->currentTexture != 0 && this->currentTexture <= this->textures.size())
    {
        C3DTexture &texture = this->textures[this->currentTexture - 1];
        if (texture.initialized)
        {
            C3D_TexSetFilter(&texture.tex, GPU_LINEAR, GPU_LINEAR);
            C3D_TexSetWrap(&texture.tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
        }
    }
}

void C3D::GetViewport(u32 *viewport)
{
    std::memcpy(viewport, this->viewport, sizeof(this->viewport));
}

void C3D::GetDepthRange(f32 *depthRange)
{
    depthRange[0] = this->depthRange[0];
    depthRange[1] = this->depthRange[1];
}

void C3D::SetViewport(i32 x, i32 y, i32 width, i32 height)
{
    this->viewport[0] = x;
    this->viewport[1] = y;
    this->viewport[2] = width;
    this->viewport[3] = height;
    C3D_SetViewport(x, y, width, height);
}

void C3D::SetDepthRange(f32 nearPlane, f32 farPlane)
{
    this->depthRange[0] = nearPlane;
    this->depthRange[1] = farPlane;
    C3D_DepthMap(true, farPlane - nearPlane, nearPlane);
}

void C3D::Enable(Capabilities cap)
{
    if (cap == CAPS_BLEND)
    {
        return;
    }
    if (cap == CAPS_DEPTH_TEST)
    {
        this->depthTestEnabled = true;
        C3D_DepthTest(true, this->depthFunc == DEPTH_FUNC_ALWAYS ? GPU_ALWAYS : GPU_LEQUAL,
                      this->depthMask ? GPU_WRITE_ALL : GPU_WRITE_COLOR);
    }
}

bool C3D::HasError()
{
    const bool error = this->hasError;
    this->hasError = false;
    return error;
}

void C3D::SetBlendMode(BlendMode mode)
{
    const GPU_BLENDFACTOR destination = mode == BLEND_INV_SRC_ALPHA ? GPU_ONE_MINUS_SRC_ALPHA : GPU_ONE;
    C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, destination, GPU_SRC_ALPHA, destination);
}

void C3D::SetDepthMask(bool enable)
{
    this->depthMask = enable;
    if (this->depthTestEnabled)
    {
        C3D_DepthTest(true, this->depthFunc == DEPTH_FUNC_ALWAYS ? GPU_ALWAYS : GPU_LEQUAL,
                      this->depthMask ? GPU_WRITE_ALL : GPU_WRITE_COLOR);
    }
}

void C3D::SetDepthFunc(DepthFunc func)
{
    this->depthFunc = func;
    if (this->depthTestEnabled)
    {
        C3D_DepthTest(true, this->depthFunc == DEPTH_FUNC_ALWAYS ? GPU_ALWAYS : GPU_LEQUAL,
                      this->depthMask ? GPU_WRITE_ALL : GPU_WRITE_COLOR);
    }
}

void C3D::SetClearDepth(f32 depth)
{
    this->clearDepth = depth;
}

void C3D::SetClearColor(f32 r, f32 g, f32 b, f32 a)
{
    this->clearColor[0] = r;
    this->clearColor[1] = g;
    this->clearColor[2] = b;
    this->clearColor[3] = a;
}

void C3D::Clear(u32 clearBits)
{
    C3D_ClearBits bits = static_cast<C3D_ClearBits>(0);
    if (clearBits & CLEAR_COLOR_BUFFER)
    {
        bits = static_cast<C3D_ClearBits>(bits | C3D_CLEAR_COLOR);
    }
    if (clearBits & CLEAR_DEPTH_BUFFER)
    {
        bits = static_cast<C3D_ClearBits>(bits | C3D_CLEAR_DEPTH);
    }
    const u32 color = (static_cast<u32>(this->clearColor[0] * 255.0f) & 0xFF) |
                      ((static_cast<u32>(this->clearColor[1] * 255.0f) & 0xFF) << 8) |
                      ((static_cast<u32>(this->clearColor[2] * 255.0f) & 0xFF) << 16) |
                      ((static_cast<u32>(this->clearColor[3] * 255.0f) & 0xFF) << 24);
    const u32 depth = static_cast<u32>(this->clearDepth * 0xFFFFFF);
    C3D_FrameBufClear(&this->target->frameBuf, bits, color, depth);
}

GfxTextureHandle C3D::CreateTexture()
{
    this->textures.emplace_back();
    return static_cast<u32>(this->textures.size());
}

void C3D::BindTexture(GfxTextureHandle handle)
{
    this->currentTexture = handle.id;
    if (handle.id == 0 || handle.id > this->textures.size())
    {
        C3D_TexBind(0, nullptr);
        return;
    }
    C3DTexture &texture = this->textures[handle.id - 1];
    C3D_TexBind(0, texture.initialized ? &texture.tex : nullptr);
}

void C3D::DeleteTexture(GfxTextureHandle handle)
{
    if (handle.id == 0 || handle.id > this->textures.size())
    {
        return;
    }
    C3DTexture &texture = this->textures[handle.id - 1];
    if (texture.initialized)
    {
        C3D_TexDelete(&texture.tex);
        texture.initialized = false;
    }
    if (texture.pixels != nullptr)
    {
        linearFree(texture.pixels);
        texture.pixels = nullptr;
    }
    texture.width = 0;
    texture.height = 0;
    if (this->currentTexture == handle.id)
    {
        this->currentTexture = 0;
        C3D_TexBind(0, nullptr);
    }
}

void C3D::SetTextureImage(u32 width, u32 height, PixelFormat format, PixelDataType type, const void *data)
{
    if (this->currentTexture == 0 || this->currentTexture > this->textures.size() || width == 0 || height == 0 ||
        width > 1024 || height > 1024)
    {
        this->hasError = true;
        return;
    }

    C3DTexture &texture = this->textures[this->currentTexture - 1];
    if (texture.initialized)
    {
        C3D_TexDelete(&texture.tex);
        texture.initialized = false;
    }
    if (texture.pixels != nullptr)
    {
        linearFree(texture.pixels);
        texture.pixels = nullptr;
    }

    texture.width = width;
    texture.height = height;
    texture.pixels = static_cast<u8 *>(linearAlloc(static_cast<size_t>(width) * height * 4));
    if (texture.pixels == nullptr || !C3D_TexInit(&texture.tex, width, height, GPU_RGBA8))
    {
        if (texture.pixels != nullptr)
        {
            linearFree(texture.pixels);
            texture.pixels = nullptr;
        }
        texture.width = texture.height = 0;
        this->hasError = true;
        return;
    }
    texture.initialized = true;
    std::memset(texture.pixels, 0, static_cast<size_t>(width) * height * 4);

    if (data != nullptr)
    {
        const size_t sourcePixelSize = type == PIXEL_UNSIGNED_BYTE ? (format == PIXEL_RGBA ? 4 : 3) : 2;
        const u8 *source = static_cast<const u8 *>(data);
        for (size_t i = 0, count = static_cast<size_t>(width) * height; i < count; ++i)
        {
            decodeTexturePixel(texture.pixels + i * 4, format, type, source + i * sourcePixelSize);
        }
    }

    C3D_TexSetFilter(&texture.tex, GPU_LINEAR, GPU_LINEAR);
    C3D_TexSetWrap(&texture.tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
    uploadTexture(texture);
    C3D_TexBind(0, &texture.tex);
}

void C3D::SetTextureSubImage(i32 xoffset, i32 yoffset, i32 width, i32 height, const void *data)
{
    if (this->currentTexture == 0 || this->currentTexture > this->textures.size() || data == nullptr || width <= 0 ||
        height <= 0)
    {
        return;
    }
    C3DTexture &texture = this->textures[this->currentTexture - 1];
    if (!texture.initialized || xoffset < 0 || yoffset < 0 || static_cast<u32>(xoffset) > texture.width ||
        static_cast<u32>(yoffset) > texture.height || static_cast<u32>(width) > texture.width - xoffset ||
        static_cast<u32>(height) > texture.height - yoffset)
    {
        this->hasError = true;
        return;
    }

    const u8 *source = static_cast<const u8 *>(data);
    for (i32 row = 0; row < height; ++row)
    {
        u8 *destination = texture.pixels + (static_cast<size_t>(yoffset + row) * texture.width + xoffset) * 4;
        for (i32 column = 0; column < width; ++column)
        {
            destination[column * 4 + 0] = source[(row * width + column) * 3 + 0];
            destination[column * 4 + 1] = source[(row * width + column) * 3 + 1];
            destination[column * 4 + 2] = source[(row * width + column) * 3 + 2];
            destination[column * 4 + 3] = 0xFF;
        }
    }
    uploadTexture(texture);
    C3D_TexBind(0, &texture.tex);
}

void C3D::ReadPixels(i32 x, i32 y, i32 width, i32 height, const void *pixels)
{
    if (pixels == nullptr || width <= 0 || height <= 0 || this->target == nullptr)
    {
        return;
    }
    const u32 targetWidth = this->target->frameBuf.width;
    const u32 targetHeight = this->target->frameBuf.height;
    u8 *readback = static_cast<u8 *>(linearAlloc(static_cast<size_t>(targetWidth) * targetHeight * 4));
    if (readback == nullptr)
    {
        this->hasError = true;
        return;
    }

    C3D_FrameSync();
    C3D_SyncDisplayTransfer(static_cast<u32 *>(this->target->frameBuf.colorBuf), GX_BUFFER_DIM(targetWidth, targetHeight),
                            reinterpret_cast<u32 *>(readback), GX_BUFFER_DIM(targetWidth, targetHeight),
                            C3D_READBACK_FLAGS);
    u8 *destination = const_cast<u8 *>(static_cast<const u8 *>(pixels));
    for (i32 row = 0; row < height; ++row)
    {
        const i32 sourceY = y + row;
        if (sourceY < 0 || sourceY >= static_cast<i32>(targetHeight) || x < 0 ||
            x + width > static_cast<i32>(targetWidth))
        {
            continue;
        }
        std::memcpy(destination + static_cast<size_t>(row) * width * 4,
                    readback + (static_cast<size_t>(sourceY) * targetWidth + x) * 4, static_cast<size_t>(width) * 4);
    }
    linearFree(readback);
}

void C3D::Draw(PrimitiveType type, i32 start, i32 count)
{
    if (this->vbo_data == nullptr || start < 0 || count <= 0 || static_cast<u32>(count) > MAX_VERTEX_COUNT)
    {
        return;
    }
    FFVertex *vertices = static_cast<FFVertex *>(this->vbo_data);
    for (i32 index = 0; index < count; ++index)
    {
        FFVertex &vertex = vertices[index];
        vertex = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xFF, 0xFF, 0xFF, 0xFF};
        const size_t sourceIndex = static_cast<size_t>(start + index);
        if (this->attributePointers[VERTEX_ARRAY_POSITION] != nullptr &&
            this->attributeStrides[VERTEX_ARRAY_POSITION] >= sizeof(float) * 3)
        {
            std::memcpy(&vertex.x, static_cast<const u8 *>(this->attributePointers[VERTEX_ARRAY_POSITION]) +
                                       sourceIndex * this->attributeStrides[VERTEX_ARRAY_POSITION], sizeof(float) * 3);
        }
        if (this->attributePointers[VERTEX_ARRAY_TEX_COORD] != nullptr &&
            this->attributeStrides[VERTEX_ARRAY_TEX_COORD] >= sizeof(float) * 2)
        {
            std::memcpy(&vertex.u, static_cast<const u8 *>(this->attributePointers[VERTEX_ARRAY_TEX_COORD]) +
                                       sourceIndex * this->attributeStrides[VERTEX_ARRAY_TEX_COORD], sizeof(float) * 2);
        }
        if (this->attributePointers[VERTEX_ARRAY_DIFFUSE] != nullptr &&
            this->attributeStrides[VERTEX_ARRAY_DIFFUSE] >= 4)
        {
            std::memcpy(&vertex.r, static_cast<const u8 *>(this->attributePointers[VERTEX_ARRAY_DIFFUSE]) +
                                       sourceIndex * this->attributeStrides[VERTEX_ARRAY_DIFFUSE], 4);
        }
    }

    GPU_Primitive_t primitive;
    switch (type)
    {
    case PRIM_TRIANGLE_STRIP:
        primitive = GPU_TRIANGLE_STRIP;
        break;
    case PRIM_TRIANGLES:
        primitive = GPU_TRIANGLES;
        break;
    default:
        return;
    }
    C3D_DrawArrays(primitive, 0, count);
}

void C3D::SwapBuffers()
{
    if (this->frameBegun)
    {
        C3D_FrameEnd(0);
        this->frameBegun = false;
    }
    this->frameBegun = C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    if (this->frameBegun && !C3D_FrameDrawOn(this->target))
    {
        this->hasError = true;
    }
    else if (!this->frameBegun)
    {
        this->hasError = true;
    }
}