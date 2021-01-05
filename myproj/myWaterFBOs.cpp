#include "myWaterFBOs.h"



myWaterFBOs::myWaterFBOs()
{
	currentFrameBuffer = nullptr;
	reflectionFrameBuffer = new myFBO();
	refractionFrameBuffer = new myFBO();
}

myWaterFBOs::~myWaterFBOs()
{
	if (reflectionFrameBuffer) delete reflectionFrameBuffer;
	if (refractionFrameBuffer) delete refractionFrameBuffer;
}

void myWaterFBOs::initWaterFBOs(int width, int height)
{
	reflectionFrameBuffer->initFBO(width, height);
	refractionFrameBuffer->initFBO(width, height);
}

void myWaterFBOs::bindReflectionFrameBuffer()
{
	currentFrameBuffer = reflectionFrameBuffer;
	currentFrameBuffer->bind();
}

void myWaterFBOs::bindRefractionFrameBuffer()
{
	currentFrameBuffer = refractionFrameBuffer;
	currentFrameBuffer->bind();
}

void myWaterFBOs::unbind()
{
	if (currentFrameBuffer) currentFrameBuffer->unbind();
}

void myWaterFBOs::clear()
{
	if (currentFrameBuffer) currentFrameBuffer->clear();
}
