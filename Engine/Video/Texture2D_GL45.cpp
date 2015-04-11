#include "Texture2D.hpp"
#include <algorithm>
#include <sstream>
#include <map>
#include <GL/glxw.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.c"
#include "DDSLoader.hpp"
#include "GfxDevice.hpp"
#include "FileWatcher.hpp"
#include "System.hpp"

extern ae3d::FileWatcher fileWatcher;

namespace Texture2DGlobal
{
    std::map< std::string, ae3d::Texture2D > pathToCachedTexture;
}

void TexReload( const std::string& path )
{
    auto& tex = Texture2DGlobal::pathToCachedTexture[ path ];

    tex.Load( ae3d::System::FileContents( path.c_str() ), tex.GetWrap(), tex.GetFilter() );
}

static bool HasStbExtension( const std::string& path )
{    
    // Checks for uncompressed formats in texture's file name.
    static const std::string extensions[] =
    {
        ".png", ".PNG", ".jpg", ".JPG", ".tga", ".TGA",
        ".bmp", ".BMP", ".gif", ".GIF"
    };

    const bool extensionFound = std::any_of( std::begin( extensions ), std::end( extensions ),
        [&]( const std::string& extension ) { return path.find( extension ) != std::string::npos; } );
    
    return extensionFound;
}

/// \todo Create StringUtil.hpp when more string methods are needed.
static void Tokenize( const std::string& str,
    std::vector< std::string >& tokens,
    const std::string& delimiters = " " )
{
    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of( delimiters, 0 );
    // Find first "non-delimiter".
    std::string::size_type pos = str.find_first_of( delimiters, lastPos );

    while (std::string::npos != pos || std::string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back( str.substr( lastPos, pos - lastPos ) );
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of( delimiters, pos );
        // Find next "non-delimiter"
        pos = str.find_first_of( delimiters, lastPos );
    }
}

void ae3d::Texture2D::Load( const System::FileContentsData& fileContents, TextureWrap aWrap, TextureFilter aFilter )
{
    filter = aFilter;
    wrap = aWrap;

    if (!fileContents.isLoaded)
    {
        return;
    }
    
    const bool isCached = Texture2DGlobal::pathToCachedTexture.find( fileContents.path ) != Texture2DGlobal::pathToCachedTexture.end();

    if (isCached && id == 0)
    {
        *this = Texture2DGlobal::pathToCachedTexture[ fileContents.path ];
        return;
    }
    
    // First load.
    if (id == 0)
    {
        id = GfxDevice::CreateTextureId();
        
        if (GfxDevice::HasExtension( "KHR_debug" ))
        {
            glObjectLabel( GL_TEXTURE, id, (GLsizei)fileContents.path.size(), fileContents.path.c_str() );
        }
        
        fileWatcher.AddFile( fileContents.path, TexReload );
    }

    glBindTexture( GL_TEXTURE_2D, id );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter == TextureFilter::Nearest ? GL_NEAREST : GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter == TextureFilter::Nearest ? GL_NEAREST : GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap == TextureWrap::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap == TextureWrap::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE );

    const bool isDDS = fileContents.path.find( ".dds" ) != std::string::npos || fileContents.path.find( ".DDS" ) != std::string::npos;
    
    if (HasStbExtension( fileContents.path ))
    {
        LoadSTB( fileContents );
    }
    else if (isDDS)
    {
        LoadDDS( fileContents.path.c_str() );
    }

    Texture2DGlobal::pathToCachedTexture[ fileContents.path ] = *this;
}

void ae3d::Texture2D::LoadFromAtlas( const System::FileContentsData& atlasTextureData, const System::FileContentsData& atlasMetaData, const char* textureName, TextureWrap aWrap, TextureFilter aFilter )
{
    Load( atlasTextureData, aWrap, aFilter );

    const std::string metaStr = std::string( std::begin( atlasMetaData.data ), std::end( atlasMetaData.data ) );
    std::stringstream metaStream( metaStr );

    if (atlasMetaData.path.find( ".xml" ) != std::string::npos || atlasMetaData.path.find( ".XML" ) != std::string::npos)
    {
        std::string line;

        while (std::getline( metaStream, line ))
        {
            if (line.find( "<Image Name" ) != std::string::npos)
            {
                std::vector< std::string > tokens;
                Tokenize( line, tokens, "\"" );
                bool found = false;

                for (std::size_t t = 0; t < tokens.size(); ++t)
                {
                    if (tokens[ t ].find( "Name" ) != std::string::npos)
                    {
                        if (tokens[ t + 1 ] == textureName)
                        {
                            found = true;
                        }
                    }

                    if (found)
                    {
                        if (tokens[ t ].find( "XPos" ) != std::string::npos)
                        {
                            scaleOffset.z = std::stoi( tokens[ t + 1 ] ) / static_cast<float>(width);
                        }
                        else if (tokens[ t ].find( "YPos" ) != std::string::npos)
                        {
                            scaleOffset.w = std::stoi( tokens[ t + 1 ] ) / static_cast<float>(height);
                        }
                        else if (tokens[ t ].find( "Width" ) != std::string::npos)
                        {
                            const int w = std::stoi( tokens[ t + 1 ] );
                            scaleOffset.x = 1.0f / (static_cast<float>(width) / static_cast<float>(w));
                            width = w;
                        }
                        else if (tokens[ t ].find( "Height" ) != std::string::npos)
                        {
                            const int h = std::stoi( tokens[ t + 1 ] );
                            scaleOffset.y = 1.0f / (static_cast<float>(height) / static_cast<float>(h));
                            height = h;
                        }
                    }
                }

                if (found)
                {
                    return;
                }
            }
        }
    }
}

void ae3d::Texture2D::LoadDDS( const char* path )
{
    const bool success = DDSLoader::Load( path, 0, width, height, opaque );

    if (!success)
    {
        ae3d::System::Print( "DDS Loader could not load %s", path );
    }
}

void ae3d::Texture2D::LoadSTB( const System::FileContentsData& fileContents )
{
    int components;
    unsigned char* data = stbi_load_from_memory( &fileContents.data[ 0 ], static_cast<int>(fileContents.data.size()), &width, &height, &components, 4 );

    if (data == nullptr)
    {
        const std::string reason( stbi_failure_reason() );
        System::Print( "%s failed to load. stb_image's reason: %s", fileContents.path.c_str(), reason.c_str() );
        return;
    }
  
    opaque = (components == 3 || components == 1);

    if (GfxDevice::HasExtension("GL_ARB_texture_storage"))
    {
        glTexStorage2D( GL_TEXTURE_2D, 1, GL_RGBA8, width, height );
        glTexSubImage2D( GL_TEXTURE_2D,
                        0,
                        0, 0,
                        width, height,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        data );
    }
    else
    {
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
    }

    stbi_image_free( data );
}