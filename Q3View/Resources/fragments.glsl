//[FRAGMENT SHADER]
#version 400

uniform sampler2D tex;
uniform bool wireframe;

in vec2 fragUV; // from vertex shader

out vec4 fragColor;

void main()
{
	if (wireframe)
	{
		fragColor = vec4(0.0, 1.0, 0.0, 1.0);
	}
	else
	{
		fragColor = texture(tex, fragUV);
	}
}
