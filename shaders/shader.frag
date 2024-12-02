#version 330 core
in vec2 TexCoords;
uniform float Time;
out vec4 FragColor;
void Custom_Node_16024430264665270863_Func(in vec2 uv, in float iTime, out vec3 Out)
{
    float time = 51.384;

    // MurmurHash13 intégré dans la fonction principale
    const uint M = 0x5bd1e995u;
    uint h = 1190494759u;
    uvec3 src = floatBitsToUint(vec3(uv, time));
    src *= M; src ^= src >> 24u; src *= M;
    h *= M; h ^= src.x; h *= M; h ^= src.y; h *= M; h ^= src.z;
    h ^= h >> 13u; h *= M; h ^= h >> 15u;

    // Conversion en float entre 0 et 1
    float hash = uintBitsToFloat(h & 0x007fffffu | 0x3f800000u) - 1.0;

    uv = uv * 2.0 - 1.0;
    if (dot(uv, uv) > 0.25 + 0.25 * sin(iTime * 3.0)) {
        src = floatBitsToUint(vec3(uv, iTime * 100.0));
        src *= M; src ^= src >> 24u; src *= M;
        h = 1190494759u;
        h *= M; h ^= src.x; h *= M; h ^= src.y; h *= M; h ^= src.z;
        h ^= h >> 13u; h *= M; h ^= h >> 15u;
        hash = uintBitsToFloat(h & 0x007fffffu | 0x3f800000u) - 1.0;
    }

    Out = vec3(hash, hash, hash);
}
void main()
{
vec2 TexCoords_14104528305666461794_0 = TexCoords;
float Time_4219223008401019250_0 = Time;
vec3 Custom_Node_16024430264665270863_0;
Custom_Node_16024430264665270863_Func(TexCoords_14104528305666461794_0, Time_4219223008401019250_0, Custom_Node_16024430264665270863_0);

// Output to screen
FragColor = vec4(Custom_Node_16024430264665270863_0, 1.0);
}
