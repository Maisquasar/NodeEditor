#version 330 core
in vec2 TexCoords;
uniform float Time;
out vec4 FragColor;
void main()
{
vec2 TexCoords_16922580609918647625_0 = TexCoords;
vec3 To_Vector3_Vector2_1645320118562651443_0 = vec3(TexCoords_16922580609918647625_0, 0.0);

// Output to screen
FragColor = vec4(To_Vector3_Vector2_1645320118562651443_0, 1.0);
}
