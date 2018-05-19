//[FRAGMENT SHADER]
#version 400

uniform sampler2D tex;
uniform vec4 color = vec4(1.0, 1.0, 1.0, 1.0);

in vec2 texCoord; // from vertex shader

out vec4 fragColor;

void main()
{
	vec4 tone = vec4(1.0, 1.0, 1.0, texture(tex, texCoord).r);
	fragColor = color * tone;
}
