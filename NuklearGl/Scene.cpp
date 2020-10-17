#include "stdafx.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "resource.h"
#include "Scene.h"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

// vertex attrib spec
struct nk_vertex_t {
	float position[2];
	float texCoord[2];
	nk_byte color[4];
};

// vertex attrib desc
static const nk_draw_vertex_layout_element g_vertex_layout[] = {
	{ NK_VERTEX_POSITION, NK_FORMAT_FLOAT, offsetof(nk_vertex_t, position) },
	{ NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, offsetof(nk_vertex_t, texCoord) },
	{ NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, offsetof(nk_vertex_t, color) },
	{ NK_VERTEX_LAYOUT_END }
};

/////////////////////////////////////////////////////////////////
// Construction/ Destruction
/////////////////////////////////////////////////////////////////

CScene::CScene() : m_vecSize()
{
	m_matNuklear = glm::mat4(); // identity

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

CScene::~CScene()
{
	ATLTRACE(_T("Cleaning up...\n"));

	if (m_nFontTexture)
	{
		glDeleteTextures(1, &m_nFontTexture);
		m_nFontTexture = 0;
	}

	nk_input_end(&m_nkContext); // end input tracking
	nk_buffer_free(&m_nkCommands);
	nk_free(&m_nkContext);
	nk_font_atlas_clear(&m_nkFontAtlas);

	if (m_pNuklearShader)
	{
		delete m_pNuklearShader;
		m_pNuklearShader = nullptr;
	}

	if (m_nVertexArray)
	{
		glDeleteVertexArrays(1, &m_nVertexArray);
		m_nVertexArray = 0;
	}

	if (m_nVertexBuffer)
	{
		glDeleteBuffers(1, &m_nVertexBuffer);
		m_nVertexBuffer = 0;
	}

	if (m_nElementBuffer)
	{
		glDeleteBuffers(1, &m_nElementBuffer);
		m_nElementBuffer = 0;
	}
}

/////////////////////////////////////////////////////////////////
// Method Implementation
/////////////////////////////////////////////////////////////////

bool CScene::Create()
{
	ATLTRACE(_T("Initializing...\n"));

	nk_font_atlas_init_default(&m_nkFontAtlas);
	nk_font_atlas_begin(&m_nkFontAtlas);

	int width, height;
	nk_font *pFont = nk_font_atlas_add_default(&m_nkFontAtlas, 13, 0);
	const void * image = nk_font_atlas_bake(&m_nkFontAtlas, &width, &height, NK_FONT_ATLAS_RGBA32);

	glGenTextures(1, &m_nFontTexture);
	glBindTexture(GL_TEXTURE_2D, m_nFontTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		(GLsizei)width, (GLsizei)height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, image
	);
	glBindTexture(GL_TEXTURE_2D, 0);

	nk_font_atlas_end(&m_nkFontAtlas, nk_handle_id((int)m_nFontTexture), &m_nkNull);

	if (!nk_init_default(&m_nkContext, &pFont->handle))
	{
		ATLTRACE(_T("Could not initialize Nuklear context!\n"));
		return false;
	}

	memset(&m_nkConvertConfig, 0, sizeof(nk_convert_config));
	m_nkConvertConfig.vertex_layout = g_vertex_layout;
	m_nkConvertConfig.vertex_size = sizeof(nk_vertex_t);
	m_nkConvertConfig.vertex_alignment = NK_ALIGNOF(nk_vertex_t);
	m_nkConvertConfig.null = m_nkNull;
	m_nkConvertConfig.circle_segment_count = 22;
	m_nkConvertConfig.curve_segment_count = 22;
	m_nkConvertConfig.arc_segment_count = 22;
	m_nkConvertConfig.global_alpha = 1.0f;
	m_nkConvertConfig.shape_AA = NK_ANTI_ALIASING_ON;
	m_nkConvertConfig.line_AA = NK_ANTI_ALIASING_ON;

	nk_buffer_init_default(&m_nkCommands);

	m_pNuklearShader = new CShaderProgram();
	if (!m_pNuklearShader->CreateSimple(
		_Module.GetResourceInstance(), _T("GLSL_SHADER"),
		IDR_GLSL_NK_VERTEX_SHADER,
		IDR_GLSL_NK_FRAGMENT_SHADER))
	{
		// trigger immediate window destruction and thus cascading destructor
		return false;
	}

	glGenVertexArrays(1, &m_nVertexArray);
	glGenBuffers(1, &m_nVertexBuffer);
	glGenBuffers(1, &m_nElementBuffer);

	glBindVertexArray(m_nVertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, m_nVertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nElementBuffer);

	glBufferData(GL_ARRAY_BUFFER, MAX_VERTEX_BUFFER, NULL, GL_DYNAMIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_ELEMENT_BUFFER, NULL, GL_DYNAMIC_DRAW);

	GLsizei stride = sizeof(nk_vertex_t);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(nk_vertex_t, position));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(nk_vertex_t, texCoord));
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (void*)offsetof(nk_vertex_t, color));

	glEnableVertexAttribArray(0); // position
	glEnableVertexAttribArray(1); // texcoord
	glEnableVertexAttribArray(2); // color

	glBindVertexArray(0);

	nk_input_begin(&m_nkContext); // start input tracking

	return true;
}

void CScene::Render(float time)
{
	nk_input_end(&m_nkContext); // pause input tracking

	// specification

	if (!nk_begin(&m_nkContext, "Show",
		nk_rect(m_vecSize.x / 2.0f - 110.0f, m_vecSize.y / 2.0f - 110.0f, 220.0f, 220.0f),
		NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE))
	{
		ATLTRACE(_T("Could not configure Nuklear UI!\n"));
		return;
	}

	// fixed widget pixel width
	nk_layout_row_static(&m_nkContext, 30.0f, 80, 1);
	if (nk_button_label(&m_nkContext, "Button")) { ATLTRACE(_T("Button pressed.\n")); }

	// fixed widget window ratio width
	static bool easy = true;
	nk_layout_row_dynamic(&m_nkContext, 30.0f, 2);
	if (nk_option_label(&m_nkContext, "easy", easy)) { easy = true; }
	if (nk_option_label(&m_nkContext, "hard", !easy)) { easy = false; }

	// custom widget pixel width
	nk_layout_row_begin(&m_nkContext, NK_STATIC, 30.0f, 2);
	{
		static float value = 0.6f;
		nk_layout_row_push(&m_nkContext, 50.0f);
		nk_label(&m_nkContext, "Volume:", NK_TEXT_LEFT);
		nk_layout_row_push(&m_nkContext, 110.0f);
		nk_slider_float(&m_nkContext, 0, &value, 1.0f, 0.1f);

		static char textBuffer[128] = { 0 };
		nk_layout_row_push(&m_nkContext, 50.0f);
		nk_label(&m_nkContext, "Name:", NK_TEXT_LEFT);
		nk_layout_row_push(&m_nkContext, 110.0f);
		nk_edit_string_zero_terminated(
			&m_nkContext, NK_EDIT_BOX,
			textBuffer, sizeof(textBuffer),
			nk_filter_ascii
		);
	}
	nk_layout_row_end(&m_nkContext);

	nk_end(&m_nkContext);

	// preparation

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);

	glUseProgram(*m_pNuklearShader);
	glUniformMatrix4fv(
		glGetUniformLocation(*m_pNuklearShader, "projection"),
		1, GL_FALSE, glm::value_ptr(m_matNuklear)
	);

	// conversion

	glBindBuffer(GL_ARRAY_BUFFER, m_nVertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_nElementBuffer);

	void *vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	void *elements = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

	nk_buffer vbuf, ebuf;
	nk_buffer_init_fixed(&vbuf, vertices, MAX_VERTEX_BUFFER);
	nk_buffer_init_fixed(&ebuf, elements, MAX_ELEMENT_BUFFER);
	nk_convert(&m_nkContext, &m_nkCommands, &vbuf, &ebuf, &m_nkConvertConfig);

	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glUnmapBuffer(GL_ARRAY_BUFFER);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// rendering

	glBindVertexArray(m_nVertexArray);

	const nk_draw_command *cmd;
	const nk_draw_index *offset = 0;
	nk_draw_foreach(cmd, &m_nkContext, &m_nkCommands)
	{
		if (!cmd->elem_count) continue;
		glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
		glScissor(
			(GLint)(cmd->clip_rect.x),
			(GLint)(m_vecSize.y - (cmd->clip_rect.y + cmd->clip_rect.h)),
			(GLint)(cmd->clip_rect.w),
			(GLint)(cmd->clip_rect.h));
		glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
		offset += cmd->elem_count;
	}

	glBindVertexArray(0);

	// cleanup

	glDisable(GL_SCISSOR_TEST);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	nk_clear(&m_nkContext);

	nk_input_begin(&m_nkContext); // resume input tracking
}

void CScene::Resize(int width, int height)
{
	if (height == 0) height = 1; // avoid division by zero

	m_vecSize.x = (float)width;
	m_vecSize.y = (float)height;

	m_matNuklear = glm::ortho(0.0f, m_vecSize.x, m_vecSize.y, 0.0f);
}
