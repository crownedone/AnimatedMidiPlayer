#pragma once
#include "myObject.h"

class mySkybox : public myObject
{

private:
    float rotation_speed;

    //std::vector <std::string> daycubemaps = { "models/skybox/day/left.png", "models/skybox/day/right.png", "models/skybox/day/top.png", "models/skybox/day/bottom.png", "models/skybox/day/front.png", "models/skybox/day/back.png" };
    std::vector <std::string> nightcubemaps = { "models/skybox/abstract/nightLeft.png", "models/skybox/abstract/nightRight.png", "models/skybox/abstract/nightTop.png", "models/skybox/abstract/nightBottom.png", "models/skybox/abstract/nightFront.png", "models/skybox/abstract/nightBack.png" };

    // myTexture* dayTexture;
    myTexture* nightTexture;

    float blendFactor;

public:
    mySkybox();
    ~mySkybox();
    void reset();

    void rotate();
    void incrementRotationSpeed();
    void decrementRotationSpeed();


    void displayObjects(myShader*, glm::mat4, float rotation);

};
