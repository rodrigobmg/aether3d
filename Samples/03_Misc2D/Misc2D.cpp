#include <string>
#include "AudioClip.hpp"
#include "AudioSourceComponent.hpp"
#include "Font.hpp"
#include "CameraComponent.hpp"
#include "SpriteRendererComponent.hpp"
#include "TextRendererComponent.hpp"
#include "TransformComponent.hpp"
#include "GameObject.hpp"
#include "Scene.hpp"
#include "System.hpp"
#include "FileSystem.hpp"
#include "Vec3.hpp"
#include "Window.hpp"
#include "Texture2D.hpp"
#include "RenderTexture.hpp"

using namespace ae3d;

// Sample assets can be downloaded from here:  http://twiren.kapsi.fi/files/aether3d_sample_v0.1.zip

int main()
{
    const int width = 640;
    const int height = 480;
    
    System::EnableWindowsMemleakDetection();
    Window::Instance().Create( width, height, WindowCreateFlags::Empty );
    System::LoadBuiltinAssets();
    System::InitAudio();
    
    GameObject camera;
    camera.AddComponent<CameraComponent>();
    camera.GetComponent<CameraComponent>()->SetProjection( 0, (float)width, (float)height, 0, 0, 1 );
    camera.GetComponent<CameraComponent>()->SetClearColor( Vec3( 0.5f, 0.5f, 0.5f ) );

    Texture2D spriteTex;
    spriteTex.Load( FileSystem::FileContents("glider.png"), TextureWrap::Repeat, TextureFilter::Nearest, Mipmaps::None );

    Texture2D spriteTex2;
    spriteTex2.Load( FileSystem::FileContents("test_dxt1.dds"), TextureWrap::Repeat, TextureFilter::Nearest, Mipmaps::None );

    Texture2D spriteTexFromAtlas;
    spriteTexFromAtlas.LoadFromAtlas(FileSystem::FileContents("atlas_cegui.png"), FileSystem::FileContents("atlas_cegui.xml"), "granite", TextureWrap::Repeat, TextureFilter::Nearest);

    GameObject spriteContainer;
    spriteContainer.AddComponent<SpriteRendererComponent>();
    auto sprite = spriteContainer.GetComponent<SpriteRendererComponent>();
    sprite->SetTexture( &spriteTex, Vec3( 320, 0, -0.6f ), Vec3( (float)spriteTex.GetWidth(), (float)spriteTex.GetHeight(), 1 ), Vec4( 1, 0.5f, 0.5f, 1 ) );
    sprite->SetTexture( &spriteTex, Vec3( 340, 80, -0.5f ), Vec3( (float)spriteTex.GetWidth()/2, (float)spriteTex.GetHeight()/2, 1 ), Vec4( 0.5f, 1, 0.5f, 1 ) );
    sprite->SetTexture( &spriteTex2, Vec3( 280, 60, -0.4f ), Vec3( (float)spriteTex.GetWidth(), (float)spriteTex.GetHeight(), 1 ), Vec4( 1, 1, 1, 0.5f ) );
    sprite->SetTexture( &spriteTexFromAtlas, Vec3( 260, 160, -0.4f ), Vec3( (float)spriteTexFromAtlas.GetWidth(), (float)spriteTexFromAtlas.GetHeight(), 1 ), Vec4( 1, 1, 1, 1 ) );

    spriteContainer.AddComponent<TransformComponent>();
    spriteContainer.GetComponent<TransformComponent>()->SetLocalPosition( Vec3( 20, 0, 0 ) );

    AudioClip audioClip;
    audioClip.Load(FileSystem::FileContents("explosion.wav"));
    
    GameObject audioContainer;
    audioContainer.AddComponent<AudioSourceComponent>();
    audioContainer.GetComponent<AudioSourceComponent>()->SetClipId( audioClip.GetId() );
    audioContainer.GetComponent<AudioSourceComponent>()->Play();
    
    Texture2D fontTex;
    fontTex.Load(FileSystem::FileContents("font.png"), TextureWrap::Clamp, TextureFilter::Linear, Mipmaps::None);

    Font font;
    font.LoadBMFont(&fontTex, FileSystem::FileContents("font_txt.fnt"));

    //Texture2D fontTexSDF;
    //fontTexSDF.Load( FileSystem::FileContents( "font_sdf.png" ), TextureWrap::Clamp, TextureFilter::Linear, Mipmaps::None );

    //Font sdfFont;
    //sdfFont.LoadBMFont( &fontTexSDF, FileSystem::FileContents( "font_sdf.txt" ) );

    GameObject textContainer;
    textContainer.AddComponent<TextRendererComponent>();
    textContainer.GetComponent<TextRendererComponent>()->SetText( "Aether3D \nGame Engine" );
    textContainer.GetComponent<TextRendererComponent>()->SetFont( &font );
    textContainer.GetComponent<TextRendererComponent>()->SetShader( TextRendererComponent::ShaderType::Sprite );
    textContainer.AddComponent<TransformComponent>();
    textContainer.GetComponent<TransformComponent>()->SetLocalPosition( Vec3( 20, 160, 0 ) );
    //textContainer.GetComponent<TransformComponent>()->SetLocalScale( 2 );
    textContainer.GetComponent<TransformComponent>()->SetLocalRotation( Quaternion::FromEuler( Vec3( 0, 0, 45 ) ) );

    GameObject statsParent;
    statsParent.AddComponent<TransformComponent>();
    statsParent.GetComponent<TransformComponent>()->SetLocalPosition( Vec3( 0, -80, 0 ) );
    
    GameObject statsContainer;
    statsContainer.AddComponent<TextRendererComponent>();
    statsContainer.GetComponent<TextRendererComponent>()->SetText( "Aether3D \nGame Engine" );
    statsContainer.GetComponent<TextRendererComponent>()->SetFont( &font );
    statsContainer.AddComponent<TransformComponent>();
    statsContainer.GetComponent<TransformComponent>()->SetLocalPosition( Vec3( 20, 80, 0 ) );
    statsContainer.GetComponent<TransformComponent>()->SetParent( statsParent.GetComponent<TransformComponent>() );

    RenderTexture2D rtTex;
    rtTex.Create( 512, 512, TextureWrap::Clamp, TextureFilter::Linear );
    
    GameObject renderTextureContainer;
    renderTextureContainer.AddComponent<SpriteRendererComponent>();
    spriteContainer.GetComponent<SpriteRendererComponent>()->SetTexture( &rtTex, Vec3( 150, 250, -0.6f ), Vec3( (float)spriteTex.GetWidth(), (float)spriteTex.GetHeight(), 1 ), Vec4( 1, 1, 1, 1 ) );

    GameObject rtCamera;
    rtCamera.AddComponent<CameraComponent>();
    rtCamera.GetComponent<CameraComponent>()->SetProjection( 0, (float)rtTex.GetWidth(), 0,(float)rtTex.GetHeight(), 0, 1 );
    rtCamera.GetComponent<CameraComponent>()->SetClearColor( Vec3( 0.5f, 0.5f, 0.5f ) );
    rtCamera.GetComponent<CameraComponent>()->SetTargetTexture( &rtTex );
    
    Scene scene;
    scene.Add( &camera );
    scene.Add( &spriteContainer );
    scene.Add( &textContainer );
    scene.Add( &statsContainer );
    scene.Add( &statsParent );
    scene.Add( &renderTextureContainer );
    scene.Add( &rtCamera );
    
    bool quit = false;
    
    while (Window::Instance().IsOpen() && !quit)
    {
        Window::Instance().PumpEvents();
        WindowEvent event;

        while (Window::Instance().PollEvent( event ))
        {
            if (event.type == WindowEventType::Close)
            {
                quit = true;
            }
            
            if (event.type == WindowEventType::KeyDown ||
                event.type == WindowEventType::KeyUp)
            {
                KeyCode keyCode = event.keyCode;

                if (keyCode == KeyCode::Escape)
                {
                    quit = true;
                }
                if (keyCode == KeyCode::A)
                {
                    System::ReloadChangedAssets();
                }
                if (keyCode == KeyCode::B && event.type == WindowEventType::KeyUp)
                {
                    audioContainer.GetComponent<AudioSourceComponent>()->Play();
                }
            }
        }

        const std::string drawCalls = std::string("draw calls: ") + std::to_string( System::Statistics::GetDrawCallCount() );
        statsContainer.GetComponent<TextRendererComponent>()->SetText( drawCalls.c_str() );

        scene.Render();

        Window::Instance().SwapBuffers();
    }

    System::Deinit();
}