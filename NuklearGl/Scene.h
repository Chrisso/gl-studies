#pragma once

#include <glm/glm.hpp>
#include <ShaderProgram.h>
#include <TextRendering.h>

class CScene
{
public:
	CScene();
	~CScene();

	bool Create();
	void Render(float time);
	void Resize(int width, int height);
};
