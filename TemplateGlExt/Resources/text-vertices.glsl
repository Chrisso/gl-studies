//[VERTEX SHADER]
#version 400

uniform mat4 transformation;

layout (location=0) in vec2 vertex;
layout (location=1) in vec2 texcoords;

out vec2 texCoord;

void main()
{
	gl_Position = transformation * vec4(vertex, 0.0, 1.0);
	texCoord = texcoords;
}
