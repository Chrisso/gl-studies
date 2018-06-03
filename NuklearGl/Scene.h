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

	void MouseMove(unsigned int flags, int x, int y)
	{
		nk_input_begin(&m_nkContext);
		nk_input_motion(&m_nkContext, x, y);
		nk_input_end(&m_nkContext);
	}

	void MouseButton(unsigned int button, bool pressed, int x, int y)
	{
		nk_buttons btn = NK_BUTTON_LEFT;

		switch (btn)
		{
		case MK_LBUTTON: btn = NK_BUTTON_LEFT; break;
		case MK_RBUTTON: btn = NK_BUTTON_RIGHT; break;
		case MK_MBUTTON: btn = NK_BUTTON_MIDDLE; break;
		}

		nk_input_begin(&m_nkContext);
		nk_input_button(&m_nkContext, btn, x, y, pressed ? 1 : 0);
		nk_input_end(&m_nkContext);
	}

	void Char(char c, unsigned int flags)
	{
		if (c >= 32)
		{
			nk_input_begin(&m_nkContext);
			nk_input_char(&m_nkContext, c);
			nk_input_end(&m_nkContext);
		}
	}

	void Key(char key, unsigned int flags, bool pressed)
	{
		nk_keys keys = NK_KEY_NONE;

		switch (key)
		{
		case VK_SHIFT: keys = NK_KEY_SHIFT; break;
		case VK_DELETE: keys = NK_KEY_DEL; break;
		case VK_BACK: keys = NK_KEY_BACKSPACE; break;
		case VK_UP: keys = NK_KEY_UP; break;
		case VK_DOWN: keys = NK_KEY_DOWN; break;
		case VK_LEFT: keys = NK_KEY_LEFT; break;
		case VK_RIGHT: keys = NK_KEY_RIGHT; break;
		}

		if (keys != NK_KEY_NONE)
		{
			nk_input_begin(&m_nkContext);
			nk_input_key(&m_nkContext, keys, pressed ? 1 : 0);
			nk_input_end(&m_nkContext);
		}
	}
};
