//[FRAGMENT SHADER]
#version 400

uniform sampler2D nkTexture;

in vec2 fragUV; // from vertex shader
in vec4 fragColor; // from vertex shader

out vec4 outColor;

void main()
{
	outColor = fragColor * texture(nkTexture, fragUV.st);
}
