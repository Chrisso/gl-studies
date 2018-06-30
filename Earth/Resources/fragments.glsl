//[FRAGMENT SHADER]
#version 400

in vec2 fragUV; // from vertex shader

out vec4 fragColor;

void main()
{
	fragColor = vec4(0.0, fragUV.s, fragUV.t, 1.0);
}
