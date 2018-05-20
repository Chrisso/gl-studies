//[VERTEX SHADER]
#version 400

uniform mat4 transformation;

in vec3 vertex;

void main()
{
	gl_Position = transformation * vec4(vertex, 1.0); 
}
