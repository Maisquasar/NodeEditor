#version 330 core
in vec2 TexCoords;
uniform float Time;
out vec4 FragColor;
void main()
{
vec3 Make_Vector3_1152256414187620642_0 = vec3(0.500000, 0.500000, 0.500000);
vec3 Sqrt_Vector3_9173135560075038867_0 = sqrt(Make_Vector3_1152256414187620642_0);

// Output to screen
FragColor = vec4(Sqrt_Vector3_9173135560075038867_0, 1.0);
}
