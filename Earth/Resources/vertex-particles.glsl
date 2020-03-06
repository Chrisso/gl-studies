//[VERTEX SHADER]
#version 400

uniform mat4 transformation;

layout (location=0) in vec3 position;

void main()
{
	gl_Position = transformation * vec4(position, 1.0); 
}
