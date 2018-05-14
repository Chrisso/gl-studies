//[FRAGMENT SHADER]
#version 400

uniform sampler2D tex;

in vec2 texCoord; // from vertex shader

out vec4 fragColor;

void main()
{
	vec4 tone = texture(tex, texCoord);
	fragColor = vec4(tone.rrr, 1.0);
}
