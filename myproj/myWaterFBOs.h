#pragma once
#include <GL/glew.h>
#include "myTexture.h"
#include "myFBO.h"

class myWaterFBOs
{
public:

	myWaterFBOs();
	~myWaterFBOs();

	void initWaterFBOs(int width = 1280, int height = 720);

	myFBO *currentFrameBuffer;
	myFBO *reflectionFrameBuffer;
	myFBO *refractionFrameBuffer;

	void bindReflectionFrameBuffer();
	void bindRefractionFrameBuffer();

	void unbind();

	void clear();

};

