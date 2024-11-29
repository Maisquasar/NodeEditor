#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
void Custom_Node_15349960965377174161_Func(in float In, out float Out)
{
Out = sin(In);
}
void Custom_Node_6201238049271601472_Func(in float In, out float Out)
{
Out = sin(In);
}
void Custom_Node_9321761857394756168_Func(in vec3 In, out vec3 Out)
{
Out = vec3(1.0) - In;
}
void main()
{
vec2 Param_8501008334821994681_0 = TexCoords;
float Break_Vector2_4246438757948581997_0 = Param_8501008334821994681_0.x;
float Break_Vector2_4246438757948581997_1 = Param_8501008334821994681_0.y;
float Custom_Node_15349960965377174161_0;
Custom_Node_15349960965377174161_Func(Break_Vector2_4246438757948581997_0, Custom_Node_15349960965377174161_0);
float Custom_Node_6201238049271601472_0;
Custom_Node_6201238049271601472_Func(Break_Vector2_4246438757948581997_1, Custom_Node_6201238049271601472_0);
vec3 Make_Vector3_3011884563408881031_0 = vec3(Custom_Node_15349960965377174161_0, Custom_Node_6201238049271601472_0, 0.000000);
vec3 Custom_Node_9321761857394756168_0;
Custom_Node_9321761857394756168_Func(Make_Vector3_3011884563408881031_0, Custom_Node_9321761857394756168_0);

// Output to screen
FragColor = vec4(Custom_Node_9321761857394756168_0, 1.0);
}
