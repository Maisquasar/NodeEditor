#version 330 core
in vec2 TexCoords;
uniform float Time;
out vec4 FragColor;
void main()
{
vec2 TexCoords_18345370797154596921_0 = TexCoords;
vec2 Add_Vector2_10542144629066542510_0 = TexCoords_18345370797154596921_0 + vec2(2.000000, 2.000000);
vec2 Multiply_scalar_Vector2_16461964224107398085_0 = Add_Vector2_10542144629066542510_0 * 0.500000;
vec3 To_Vector3_Vector2_14391896122424304809_0 = vec3(Multiply_scalar_Vector2_16461964224107398085_0, 0.0);

// Output to screen
FragColor = vec4(To_Vector3_Vector2_14391896122424304809_0, 1.0);
}
