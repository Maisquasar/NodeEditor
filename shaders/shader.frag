#version 330 core
in vec2 TexCoords;
uniform float Time;
out vec4 FragColor;
void main()
{
vec2 Param_15288064261559914098_0 = TexCoords;
vec2 Multiply_Vector2_12384920069304736412_0 = Param_15288064261559914098_0 * vec2(2.000000, 2.000000);
vec2 Subtract_Vector2_5328920000519551746_0 = Multiply_Vector2_12384920069304736412_0 - vec2(1.000000, 1.000000);
vec3 To_Vector3_Vector2_10706890388836413277_0 = vec3(Subtract_Vector2_5328920000519551746_0, 0.0);

// Output to screen
FragColor = vec4(To_Vector3_Vector2_10706890388836413277_0, 1.0);
}
