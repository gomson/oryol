#pragma once
//------------------------------------------------------------------------------
/**
    @defgroup Render Render
    @brief 3D rendering

    @class Oryol::Render
    @ingroup Render
    @brief Render module facade
*/
#include "Core/Singleton.h"
#include "Render/Core/displayMgr.h"
#include "IO/Stream/Stream.h"
#include "Resource/Id.h"
#include "Resource/ResourceState.h"
#include "Resource/Locator.h"
#include "Render/Setup/RenderSetup.h"
#include "Render/Core/Enums.h"
#include "Render/Core/PrimitiveGroup.h"
#include "Render/Core/stateWrapper.h"
#include "Render/Core/resourceMgr.h"
#include "Render/Core/renderMgr.h"
#include "Render/Setup/MeshSetup.h"
#include "glm/vec4.hpp"

namespace Oryol {
    
class Render {
public:
    /// setup Render module
    static void Setup(const RenderSetup& setup);
    /// discard Render module
    static void Discard();
    /// check if Render module is setup
    static bool IsValid();
    
    /// test if the window system wants the application to quit
    static bool QuitRequested();
    
    /// attach a display event handler
    static void AttachEventHandler(const Ptr<Port>& handler);
    /// detach a display event handler
    static void DetachEventHandler(const Ptr<Port>& handler);
    
    /// get the original render setup object
    static const class RenderSetup& RenderSetup();
    /// get the current actual display attributes (can be different from setup)
    static const struct DisplayAttrs& DisplayAttrs();
    /// test if an optional feature is supported
    static bool Supports(RenderFeature::Code feat);
        
    /// create a resource, or return existing resource
    template<class SETUP> static Id CreateResource(const SETUP& setup);
    /// create a resource with data stream, or return existing resource
    template<class SETUP> static Id CreateResource(const SETUP& setup, const Ptr<Stream>& data);
    /// lookup a resource by resource locator (increments use-count of resource!)
    static Id LookupResource(const Locator& locator);
    /// release a resource (decrement use-count, free resource if use-count is 0)
    static void ReleaseResource(const Id& resId);
    /// get the loading state of a resource
    static ResourceState::Code QueryResourceState(const Id& resId);
    
    /// begin frame rendering
    static bool BeginFrame();
    /// end frame rendering
    static void EndFrame();
    /// reset internal state (must be called when directly rendering through GL; FIXME: better name?)
    static void ResetStateCache();

    /// make the default render target (backbuffer) current
    static void ApplyDefaultRenderTarget();
    /// apply an offscreen render target
    static void ApplyOffscreenRenderTarget(const Id& resId);
    /// apply view port
    static void ApplyViewPort(int32 x, int32 y, int32 width, int32 height);
    /// apply scissor rect (must also be enabled in DrawState.RasterizerState)
    static void ApplyScissorRect(int32 x, int32 y, int32 width, int32 height);
    /// apply blend color (see DrawState.BlendState)
    static void ApplyBlendColor(const glm::vec4& blendColor);
    /// apply draw state to use for rendering
    static void ApplyDrawState(const Id& resId);
    /// apply a shader constant block
    static void ApplyConstantBlock(const Id& resId);
    /// apply a shader variable
    template<class T> static void ApplyVariable(int32 index, const T& value);
    /// apply a shader variable array
    template<class T> static void ApplyVariableArray(int32 index, const T* values, int32 numValues);
    
    /// update dynamic vertex data (only complete replace possible at the moment)
    static void UpdateVertices(const Id& resId, int32 numBytes, const void* data);
    /// update dynamic index data (only complete replace possible at the moment)
    static void UpdateIndices(const Id& resId, int32 numBytes, const void* data);
    /// read current framebuffer pixels into client memory, this means a PIPELINE STALL!!
    static void ReadPixels(void* ptr, int32 numBytes);
    
    /// clear the currently assigned render target
    static void Clear(PixelChannel::Mask channels, const glm::vec4& color, float32 depth, uint8 stencil);
    /// submit a draw call with primitive group index in current mesh
    static void Draw(int32 primGroupIndex);
    /// submit a draw call with direct primitive group
    static void Draw(const PrimitiveGroup& primGroup);
    /// submit a draw call for instanced rendering
    static void DrawInstanced(int32 primGroupIndex, int32 numInstances);
    /// submit a draw call for instanced rendering with direct primitive group
    static void DrawInstanced(const PrimitiveGroup& primGroup, int32 numInstances);

private:
    struct _state {
        class RenderSetup renderSetup;
        _priv::displayMgr displayManager;
        _priv::renderMgr renderManager;
        _priv::stateWrapper stateWrapper;
        _priv::resourceMgr resourceManager;
    };
    static _state* state;
};

//------------------------------------------------------------------------------
inline bool
Render::IsValid() {
    return nullptr != state;
}

//------------------------------------------------------------------------------
template<class SETUP> inline Id
Render::CreateResource(const SETUP& setup) {
    o_assert_dbg(IsValid());
    return state->resourceManager.CreateResource(setup);
}

//------------------------------------------------------------------------------
template<class SETUP> inline Id
Render::CreateResource(const SETUP& setup, const Ptr<Stream>& data) {
    o_assert_dbg(IsValid());
    return state->resourceManager.CreateResource(setup, data);
}

//------------------------------------------------------------------------------
template<> inline void
Render::ApplyVariable(int32 index, const Id& texResId) {
    o_assert_dbg(IsValid());
    _priv::texture* tex = state->resourceManager.LookupTexture(texResId);
    state->renderManager.ApplyTexture(index, tex);
}

//------------------------------------------------------------------------------
template<class T> inline void
Render::ApplyVariable(int32 index, const T& value) {
    o_assert_dbg(IsValid());
    state->renderManager.ApplyVariable(index, value);
}

//------------------------------------------------------------------------------
template<class T> inline void
Render::ApplyVariableArray(int32 index, const T* values, int32 numValues) {
    o_assert_dbg(IsValid());
    state->renderManager.ApplyVariableArray(index, values, numValues);
}

//------------------------------------------------------------------------------
inline bool
Render::Supports(RenderFeature::Code feat) {
    o_assert_dbg(IsValid());
    return state->renderManager.Supports(feat);
}

//------------------------------------------------------------------------------
inline void
Render::ApplyViewPort(int32 x, int32 y, int32 width, int32 height) {
    o_assert_dbg(IsValid());
    state->stateWrapper.ApplyViewPort(x, y, width, height);
}

//------------------------------------------------------------------------------
inline void
Render::ApplyScissorRect(int32 x, int32 y, int32 width, int32 height) {
    o_assert_dbg(IsValid());
    state->stateWrapper.ApplyScissorRect(x, y, width, height);
}

//------------------------------------------------------------------------------
inline void
Render::ApplyBlendColor(const glm::vec4& blendColor) {
    o_assert_dbg(IsValid());
    state->stateWrapper.ApplyBlendColor(blendColor.x, blendColor.y, blendColor.z, blendColor.w);
}

} // namespace Oryol