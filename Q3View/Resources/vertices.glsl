//[VERTEX SHADER]
#version 400

uniform mat4 projection;
uniform mat4 modelview;

in vec3 vertex;

void main()
{
	gl_Position = (projection * modelview) * vec4(vertex, 1.0); 
}
