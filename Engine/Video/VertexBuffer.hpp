#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#if AETHER3D_IOS
#import <Metal/Metal.h>
#endif
#include "Vec3.hpp"

namespace ae3d
{
    class VertexBuffer
    {
    public:
        struct Face
        {
            Face() : a(0), b(0), c(0) {}
            
            Face(unsigned short fa, unsigned short fb, unsigned short fc)
            : a(fa)
            , b(fb)
            , c(fc)
            {}
            
            unsigned short a, b, c;
        };

        /// Vertex with position, texture coordinate and color.
        struct VertexPTC
        {
            VertexPTC() {}
            VertexPTC(const Vec3& pos, float aU, float aV)
            : position(pos)
            , u(aU)
            , v(aV)
            {
            }
            
            Vec3 position;
            float u, v;
            Vec4 color;
        };

        /// Binds the buffer. Must be called before Draw or DrawRange.
        void Bind() const;

        bool IsGenerated() const { return elementCount != 0; }

        /// Generates the buffer from supplied geometry.
        /// \param faces Faces.
        /// \param faceCount Face count.
        /// \param vertices Vertices.
        /// \param vertexCount Vertex count.
        void Generate( const Face* faces, int faceCount, const VertexPTC* vertices, int vertexCount );
        
        /// Draws the whole buffer.
        void Draw() const;
        
        /// Draws a range of triangles.
        /// \param start Start index.
        /// \param end End index.
        void DrawRange( int start, int end ) const;

    private:
        unsigned vaoId = 0;
        unsigned vboId = 0;
        unsigned iboId = 0;
        int elementCount = 0;
#if AETHER3D_IOS
        id<MTLBuffer> vertexBuffer;
        id<MTLBuffer> indexBuffer;
#endif
    };
}
#endif
