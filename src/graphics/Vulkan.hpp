#pragma once
#include "GfxInterface.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <set>

struct Vulkan : GfxInterface
{
    static GfxInterface *Init();
    static void SetContextFlags();
    virtual void Exit();
    ~Vulkan() override
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
    virtual bool HasError()
    {
        return false;
    };
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
    SDL_Window *window = nullptr;
    VkInstance vkInstance = nullptr;
    VkSurfaceKHR vkSurface = nullptr;
    VkDevice vkDevice = nullptr;
    VkPhysicalDevice vkPhysicalDevice = nullptr;

    u32 graphics_QueueFamilyIndex = 0;
    u32 present_QueueFamilyIndex = 0;

    VkDevice vkDevice = nullptr;
    VkQueue vkGraphicsQueue = nullptr;
    VkQueue vkPresentQueue = nullptr;

    VkSwapchainKHR vkSwapchain = nullptr;
    VkSurfaceCapabilitiesKHR vkSurfaceCapabilities = {};
    VkSurfaceFormatKHR vkSurfaceFormat = {};
    VkExtent2D vkSwapchainExtent = {};
    std::vector<VkImage> vkSwapchainImages;
    u32 vkSwapchainImageCount = 0;
    std::vector<VkImageView> vkSwapchainImageViews;

};