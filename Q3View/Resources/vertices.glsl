//[VERTEX SHADER]
#version 400

uniform mat4 projection;
uniform mat4 modelview;

layout (location=0) in vec3 vertex;
layout (location=1) in vec2 texcoord;

out vec2 fragUV;

void main()
{
	fragUV = vec2(texcoord.x, texcoord.y);
	gl_Position = (projection * modelview) * vec4(vertex, 1.0); 
}
