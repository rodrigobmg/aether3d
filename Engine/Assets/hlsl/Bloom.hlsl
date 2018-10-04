#if !VULKAN
#define layout(a,b)  
#else
#define register(a) blank
#endif

#include "ubo.h"

layout( set = 0, binding = 1 ) Texture2D colorTex : register(t0);
layout( set = 0, binding = 2 ) RWTexture2D<float4> bloomTex : register(u0);

groupshared uint helper[ 128 ];

[numthreads( 8, 8, 1 )]
void CSMain( uint3 globalIdx : SV_DispatchThreadID, uint3 localIdx : SV_GroupThreadID, uint3 groupIdx : SV_GroupID )
{
    float4 color = colorTex.Load( uint3( globalIdx.x, globalIdx.y, 0 ) );
    const float luminance = dot( color.rgb, float3( 0.2126f, 0.7152f, 0.0722f ) );
    const float threshold = 0.8f;
    
    if (luminance > threshold)
    {
        bloomTex[ globalIdx.xy ] = float4(0, 1, 0, 1);
    }

    bloomTex[ globalIdx.xy ] = color;
}