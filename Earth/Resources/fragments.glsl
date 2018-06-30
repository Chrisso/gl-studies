//[FRAGMENT SHADER]
#version 400

uniform sampler2D tex;

in vec2 fragUV; // from vertex shader

out vec4 fragColor;

void main()
{
	fragColor = texture(tex, fragUV);
}
