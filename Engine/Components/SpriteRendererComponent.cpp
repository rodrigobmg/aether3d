#include "SpriteRendererComponent.hpp"
#include <vector>
#include <algorithm>
//#include <cassert>
#include "GfxDevice.hpp"
#include "Renderer.hpp"
#include "Texture2D.hpp"
#include "Vec3.hpp"
#include "VertexBuffer.hpp"
#include "System.hpp"

extern ae3d::Renderer renderer;

ae3d::SpriteRendererComponent spriteRendererComponents[100];
int nextFreeSpriteComponent = 0;

struct Drawable
{
    ae3d::Texture2D* texture = nullptr;
    int bufferStart = 0;
    int bufferEnd = 0;
};

struct Sprite
{
    ae3d::Texture2D* texture = nullptr;
    ae3d::Vec3 position;
    ae3d::Vec3 dimension;
    ae3d::Vec4 tint;
};

static void CreateVertexBuffer( std::vector< Sprite >& sprites, ae3d::VertexBuffer& outVertexBuffer )
{
    const int quadVertexCount = 4;
    const int quadFaceCount = 2;
    std::vector< ae3d::VertexBuffer::VertexPTC > vertices( sprites.size() * quadVertexCount );
    std::vector< ae3d::VertexBuffer::Face > faces( sprites.size() * quadFaceCount );
    
    // TODO: evaluate perf and consider other containers.
    std::sort( sprites.begin(), sprites.end(), [](const Sprite& a, const Sprite& b) { return a.texture->GetID() <= b.texture->GetID(); } );
    
    for (unsigned short i = 0; i < static_cast<unsigned short>(sprites.size()); ++i)
    {
        const auto& dim = sprites[ i ].dimension;
        vertices[ i * quadVertexCount + 0 ].position = sprites[ i ].position;
        vertices[ i * quadVertexCount + 1 ].position = sprites[ i ].position + ae3d::Vec3( dim.x, 0, 0 );
        vertices[ i * quadVertexCount + 2 ].position = sprites[ i ].position + ae3d::Vec3( dim.x, dim.y, 0 );
        vertices[ i * quadVertexCount + 3 ].position = sprites[ i ].position + ae3d::Vec3( 0, dim.y, 0 );
        
        for (int v = 0; v < quadVertexCount; ++v)
        {
            vertices[ i * quadVertexCount + v ].color = sprites[i].tint;
        }
        
        const ae3d::Vec4& so = sprites[i].texture->GetScaleOffset();

        const float u0 = 0.0f * so.x + so.z;
        const float u1 = 1.0f * so.x + so.z;

        const float v0 = 0.0f * so.y + so.w;
        const float v1 = 1.0f * so.y + so.w;

        vertices[ i * quadVertexCount + 0 ].u = u0;
        vertices[ i * quadVertexCount + 0 ].v = v0;
        
        vertices[ i * quadVertexCount + 1 ].u = u1;
        vertices[ i * quadVertexCount + 1 ].v = v0;
        
        vertices[ i * quadVertexCount + 2 ].u = u1;
        vertices[ i * quadVertexCount + 2 ].v = v1;
        
        vertices[ i * quadVertexCount + 3 ].u = u0;
        vertices[ i * quadVertexCount + 3 ].v = v1;
        
        auto& tri1 = faces[ i * quadFaceCount + 0 ];
        tri1.a = i * quadVertexCount + 0;
        tri1.b = i * quadVertexCount + 1;
        tri1.c = i * quadVertexCount + 2;
        
        auto& tri2 = faces[ i * quadFaceCount + 1 ];
        tri2.a = i * quadVertexCount + 2;
        tri2.b = i * quadVertexCount + 3;
        tri2.c = i * quadVertexCount + 0;
    }
    
    outVertexBuffer.Generate(faces.data(), static_cast<int>(faces.size()), vertices.data(), static_cast<int>(vertices.size()));
}

static void CreateDrawables( const std::vector< Sprite >& sprites, std::vector< Drawable >& outDrawables )
{
    outDrawables.clear();
    
    if (sprites.empty())
    {
        return;
    }
    
    outDrawables.emplace_back( Drawable() );
    auto& back = outDrawables.back();
    back.texture = sprites[0].texture;
    back.bufferStart = 0;
    back.bufferEnd = 2;
    
    for (std::size_t s = 1; s < sprites.size(); ++s)
    {
        if (sprites[s].texture == sprites[s - 1].texture)
        {
            outDrawables.back().bufferEnd += 2;
        }
        else
        {
            const int oldEnd = outDrawables.back().bufferEnd;
            
            outDrawables.emplace_back( Drawable() );
            auto& back2 = outDrawables.back();
            back2.texture = sprites[s].texture;
            back2.bufferStart = oldEnd;
            back2.bufferEnd = oldEnd + 2;
        }
    }
}

struct RenderQueue
{
    void Clear();
    void Build();
    void Render();
    
    bool isDirty = true;
    std::vector< Sprite > sprites;
    std::vector< Drawable > drawables;
    ae3d::VertexBuffer vertexBuffer;
};

void RenderQueue::Clear()
{
    sprites.clear();
    isDirty = true;
}

void RenderQueue::Build()
{
    CreateVertexBuffer( sprites, vertexBuffer );
    CreateDrawables( sprites, drawables );
    isDirty = false;
}

void RenderQueue::Render()
{
    if (isDirty)
    {
        Build();
    }
    
    vertexBuffer.Bind();
    
    for (auto& drawable : drawables)
    {
        renderer.builtinShaders.spriteRendererShader.SetTexture("textureMap", drawable.texture, 0);
        vertexBuffer.DrawRange( drawable.bufferStart, drawable.bufferEnd );
    }
}

struct ae3d::SpriteRendererComponent::Impl
{
    void Render();

    RenderQueue opaqueRenderQueue;
    RenderQueue transparentRenderQueue;
};

void ae3d::SpriteRendererComponent::Impl::Render()
{
    GfxDevice::SetBlendMode( ae3d::GfxDevice::BlendMode::Off );
    opaqueRenderQueue.Render();
    GfxDevice::SetBlendMode( ae3d::GfxDevice::BlendMode::AlphaBlend );
    transparentRenderQueue.Render();
}

int ae3d::SpriteRendererComponent::New()
{
    return nextFreeSpriteComponent++;
}

ae3d::SpriteRendererComponent* ae3d::SpriteRendererComponent::Get(int index)
{
    return &spriteRendererComponents[index];
}

ae3d::SpriteRendererComponent::SpriteRendererComponent()
{
    new(&_storage)Impl();
}

ae3d::SpriteRendererComponent::~SpriteRendererComponent()
{
    reinterpret_cast< Impl* >(&_storage)->~Impl();
}

void ae3d::SpriteRendererComponent::Clear()
{
    m().opaqueRenderQueue.Clear();
    m().transparentRenderQueue.Clear();
}

void ae3d::SpriteRendererComponent::SetTexture( Texture2D* aTexture, const Vec3& position, const Vec3& dimensionPixels, const Vec4& tintColor )
{
    if (!aTexture)
    {
        return;
    }
    
    Sprite sprite;
    sprite.texture = aTexture;
    sprite.position = position;
    sprite.dimension = dimensionPixels;
    sprite.tint = tintColor;
    
    if (!aTexture->IsOpaque() || static_cast<int>(tintColor.w) != 1)
    {
        m().transparentRenderQueue.sprites.emplace_back( sprite );
        m().transparentRenderQueue.isDirty = true;
    }
    else
    {
        m().opaqueRenderQueue.sprites.emplace_back( sprite );
        m().opaqueRenderQueue.isDirty = true;
    }
}

void ae3d::SpriteRendererComponent::Render( const float* projectionModelMatrix )
{
    renderer.builtinShaders.spriteRendererShader.Use();
    renderer.builtinShaders.spriteRendererShader.SetMatrix( "_ProjectionModelMatrix", projectionModelMatrix );
    m().Render();
}