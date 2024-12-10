#version 330 core
in vec2 TexCoords;
uniform float Time;
out vec4 FragColor;

float smin(float a, float b, float delta) {
    float h = clamp(0.5 + 0.5 * (a - b) / delta, 0.0, 1.0);
    return mix(a, b, h) - delta * h * (1.0 - h);
}

float circle(vec2 pos, vec2 center, float diameter)
{
    return length(pos-center) > diameter ? 0. : 1.;
}

float circle2(vec2 pos, vec2 centerA, vec2 centerB, float diameter, float delta)
{
    float a = length(pos-centerA) - diameter;
    float b = length(pos-centerB) - diameter;
    return smin(a,b,delta) < 0. ? 1.0 : 0.0;
}

void mainImage( out vec4 fragColor, in vec2 uv)
{
    vec3 col = vec3(0);
    col = mix(col, vec3(0.9,0.15,0.15), circle2(uv, vec2(0.58,0.35), vec2(0.58,0.15), 0.03, 0.37));
    col = mix(col, vec3(0.9,0.15,0.15), circle2(uv, vec2(0.35,0.65), vec2(0.35,0.45), 0.04, 0.35));
    col = mix(col, vec3(1.0,0.2,0.2), circle2(uv, vec2(0.5,0.7), vec2(0.5,0.4), 0.15, 0.3));
    col = mix(col, vec3(1.0,0.2,0.2), circle2(uv, vec2(0.42,0.35), vec2(0.42,0.15), 0.03, 0.37));
    col = mix(col, vec3(0.1,0.1,1.0), circle(uv, vec2(0.6,0.65), 0.1));
    col = mix(col, vec3(0.5,0.5,1.0), circle(uv, vec2(0.55,0.7), 0.015));
    col = mix(col, vec3(1.0,1.0,1.0), circle(uv, vec2(0.55,0.7), 0.01));

    // Output to screen
    fragColor = vec4(col,1.0);
}

void Custom_Node_3160466387115903642_Func(in vec2 In, out vec4 Out)
{
   mainImage(Out, In);
}void main()
{
vec2 TexCoords_14597498595621895345_0 = TexCoords;
vec4 Custom_Node_3160466387115903642_0;
Custom_Node_3160466387115903642_Func(TexCoords_14597498595621895345_0, Custom_Node_3160466387115903642_0);
vec3 To_Vector3_11041958152828690902_0 = vec3(Custom_Node_3160466387115903642_0);

// Output to screen
FragColor = vec4(To_Vector3_11041958152828690902_0, 1.0);
}
