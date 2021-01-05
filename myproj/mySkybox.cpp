#include "mySkybox.h"
#include <iostream>

#include <glm/gtc/matrix_transform.hpp> 


#define DEFAULT_ROTATION_SPEED 0.00001f

mySkybox::mySkybox() : myObject()
{
    name = "skybox";

    // dayTexture = new myTexture(daycubemaps);
    nightTexture = new myTexture(nightcubemaps);

    rotation_speed = DEFAULT_ROTATION_SPEED;

}


mySkybox::~mySkybox()
{
}

void mySkybox::reset()
{
    rotation_speed = DEFAULT_ROTATION_SPEED;
}

void mySkybox::rotate()
{
    model_matrix = glm::rotate(model_matrix, glm::radians(rotation_speed), glm::vec3(0.0, 1.0, 0.0));
}

void mySkybox::incrementRotationSpeed()
{
    rotation_speed += DEFAULT_ROTATION_SPEED;
}

void mySkybox::decrementRotationSpeed()
{
    rotation_speed -= DEFAULT_ROTATION_SPEED;
}

void mySkybox::displayObjects(myShader* shader, glm::mat4 view_matrix, float rotation)
{

    setTexture(nightTexture, mySubObject::CUBEMAP);

    //if (timeMs >= 0 && timeMs < 5000) {
    //    setTexture(nightTexture, mySubObject::CUBEMAP);
    //    setTexture(nightTexture, mySubObject::CUBEMAP1);
    //    blendFactor = (float)(timeMs - 0) / (5000 - 0);
    //}
    //else if (timeMs >= 5000 && timeMs < 8000) {
    //    setTexture(nightTexture, mySubObject::CUBEMAP);
    //    setTexture(dayTexture, mySubObject::CUBEMAP1);
    //    blendFactor = (float)(timeMs - 5000.0f) / (8000 - 5000);
    //}
    //else if (timeMs >= 8000 && timeMs < 19000) {
    //    setTexture(dayTexture, mySubObject::CUBEMAP);
    //    setTexture(dayTexture, mySubObject::CUBEMAP1);
    //    blendFactor = (float)(timeMs - 8000) / (19000 - 8000);
    //}
    //else {
    //    setTexture(dayTexture, mySubObject::CUBEMAP); 
    //    setTexture(nightTexture, mySubObject::CUBEMAP1);
    //    blendFactor = (float)(timeMs - 19000) / (24000 - 19000);
    //}

    //shader->setUniform("blendFactor", blendFactor);
    myObject::rotate(glm::vec3(1, 1, 0), rotation);
    myObject::displayObjects(shader, view_matrix);

}
