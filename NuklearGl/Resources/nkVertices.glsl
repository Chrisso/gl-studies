//[VERTEX SHADER]
#version 400

uniform mat4 projection;

layout (location=0) in vec2 position;
layout (location=1) in vec2 texCoord;
layout (location=2) in vec4 color;

out vec2 fragUV;
out vec4 fragColor;

void main()
{
	fragUV = texCoord;
	fragColor = color;
	gl_Position = projection * vec4(position.xy, 0.0, 1.0);
}
