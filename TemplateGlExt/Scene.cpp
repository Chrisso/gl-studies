#include "stdafx.h"
#include "Scene.h"

/////////////////////////////////////////////////////////////////
// Construction/ Destruction
////////////////////////////////////////////////////////////////

CScene::CScene() : m_nVertexBuffer(0)
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe

	if (glGenBuffers)
	{
		float *pGeometry = new float[18];
		pGeometry[0] = 0.0f; pGeometry[1] = 1.0f; pGeometry[2] = 0.0f;
		pGeometry[3] = -1.0f; pGeometry[4] = 0.0f; pGeometry[5] = -1.0f;
		pGeometry[6] = 1.0f; pGeometry[7] = 0.0f; pGeometry[8] = -1.0f;
		pGeometry[9] = 1.0f; pGeometry[10] = 0.0f; pGeometry[11] = 1.0f;
		pGeometry[12] = -1.0f; pGeometry[13] = 0.0f; pGeometry[14] = 1.0f;
		pGeometry[15] = -1.0f; pGeometry[16] = 0.0f; pGeometry[17] = -1.0f;

		glGenBuffers(1, &m_nVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_nVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 18, pGeometry, GL_STATIC_DRAW);
		glEnableClientState(GL_VERTEX_ARRAY);

		delete[] pGeometry;
	}
}

CScene::~CScene()
{
	if (m_nVertexBuffer)
	{
		glDeleteBuffers(1, &m_nVertexBuffer);
		m_nVertexBuffer = 0;
	}
}

/////////////////////////////////////////////////////////////////
// Method Implementation
/////////////////////////////////////////////////////////////////

void CScene::Render(double time)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glRotatef(-time / 20.0f, 0.0f, 1.0f, 0.0f);
	glPushMatrix();
	
	if (m_nVertexBuffer)
	{
		glColor3f(0.0f, 1.0f, 0.0f);
		glBindBuffer(GL_ARRAY_BUFFER, m_nVertexBuffer);
		glVertexPointer(3, GL_FLOAT, 0, NULL);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
	}

	glPopMatrix();
	glFlush();
}

void CScene::Resize(int width, int height)
{
	if (height == 0)
		height = 1; // avoid division by zero

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLdouble)width/(GLdouble)height, 0.0f, 32.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(
		0.0f, 2.0f, -5.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f);
}