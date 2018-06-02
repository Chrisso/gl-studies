#include "stdafx.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "resource.h"
#include "Scene.h"

/////////////////////////////////////////////////////////////////
// Construction/ Destruction
/////////////////////////////////////////////////////////////////

CScene::CScene()
{
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

CScene::~CScene()
{
	ATLTRACE(_T("Cleaning up...\n"));
}

/////////////////////////////////////////////////////////////////
// Method Implementation
/////////////////////////////////////////////////////////////////

bool CScene::Create()
{
	ATLTRACE(_T("Initializing...\n"));

	// TODO

	return true;
}

void CScene::Render(float time)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// TODO
}

void CScene::Resize(int width, int height)
{
	if (height == 0) height = 1; // avoid division by zero

	// TODO
}
