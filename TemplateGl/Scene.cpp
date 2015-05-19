#include "stdafx.h"
#include "Scene.h"

/////////////////////////////////////////////////////////////////
// Construction/ Destruction
////////////////////////////////////////////////////////////////

CScene::CScene()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe
}

CScene::~CScene()
{
}

/////////////////////////////////////////////////////////////////
// Method Implementation
/////////////////////////////////////////////////////////////////

void CScene::Render(double time)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glRotatef(-time / 20.0f, 0.0f, 1.0f, 0.0f);
	glPushMatrix();
	
	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_TRIANGLE_FAN);
		glVertex3f( 0.0f, 1.0f,  0.0f);
		glVertex3f(-1.0f, 0.0f, -1.0f);
		glVertex3f( 1.0f, 0.0f, -1.0f);
		glVertex3f( 1.0f, 0.0f,  1.0f);
		glVertex3f(-1.0f, 0.0f,  1.0f);
		glVertex3f(-1.0f, 0.0f, -1.0f);
	glEnd();

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