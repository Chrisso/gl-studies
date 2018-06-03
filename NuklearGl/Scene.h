#pragma once

#include <glm/glm.hpp>
#include <ShaderProgram.h>

class CScene
{
private:
	nk_context           m_nkContext;
	nk_font_atlas        m_nkFontAtlas;
	nk_draw_null_texture m_nkNull;
	nk_convert_config    m_nkConvertConfig;
	nk_buffer            m_nkCommands;

	CShaderProgram *m_pNuklearShader = nullptr;
	glm::mat4       m_matNuklear;
	glm::vec2       m_vecSize;

	GLuint m_nFontTexture = 0;
	GLuint m_nVertexArray = 0;
	GLuint m_nVertexBuffer = 0;
	GLuint m_nElementBuffer = 0;

public:
	CScene();
	~CScene();

	bool Create();
	void Render(float time);
	void Resize(int width, int height);
};
