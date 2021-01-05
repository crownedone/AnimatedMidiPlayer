#define WIN32_LEAN_AND_MEAN
#include "cxxmidi/output/default.hpp"
#include "cxxmidi/file.hpp"
#include "cxxmidi/player/synchronous.hpp"
#include "cxxmidi/note.hpp"



#include <fstream>
#include <string>
#include <vector>

#include <GL/glew.h>

#include <SDL2/SDL_main.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#undef main

#include "helperFunctions.h"

#include "myShader.h"
#include "myCamera.h"
#include "mySubObject.h"
#include "myWaterFBOs.h"
#include "mySkybox.h"
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <iostream>
#include "myObject.h"
#include "myLights.h"
#include "myFBO.h"
#include "default_constants.h"
#include "myScene.h"
#include "myShaders.h"

#include <math.h>
#include <future>

#include <mutex>

/// Options::
constexpr const bool moveLights = true;

using namespace std;
// SDL variables
SDL_Window* window;
SDL_GLContext glContext;

int mouse_position[2];
bool mouse_button_pressed = false;
bool quit = false;
bool windowsize_changed = true;
bool crystalballorfirstperson_view = false;
float movement_stepsize = DEFAULT_KEY_MOVEMENT_STEPSIZE;
bool automoveCamera = true;
int automoveCameraTimer = true;

// speed vars
const int FRAMES_PER_SECOND = 50;
const int SKIP_TICKS = 1000 / FRAMES_PER_SECOND;

// Camera parameters.
myCamera* cam1;

// All the meshes 
myScene scene;

//Triangle to draw to illustrate picking
size_t picked_triangle_index = 0;
myObject* picked_object = nullptr;

// Process the event.  
void processEvents(SDL_Event current_event)
{
    switch (current_event.type)
    {
        // window close button is pressed
    case SDL_QUIT:
    {
        quit = true;
        break;
    }
    case SDL_KEYDOWN:
    {
        if (current_event.key.keysym.sym == SDLK_ESCAPE)
            quit = true;
        if (current_event.key.keysym.sym == SDLK_r)
            cam1->reset();
        if (current_event.key.keysym.sym == SDLK_UP || current_event.key.keysym.sym == SDLK_w)
            cam1->moveForward(movement_stepsize);
        if (current_event.key.keysym.sym == SDLK_DOWN || current_event.key.keysym.sym == SDLK_s)
            cam1->moveBack(movement_stepsize);
        if (current_event.key.keysym.sym == SDLK_LEFT || current_event.key.keysym.sym == SDLK_a)
            cam1->turnLeft(DEFAULT_LEFTRIGHTTURN_MOVEMENT_STEPSIZE);
        if (current_event.key.keysym.sym == SDLK_RIGHT || current_event.key.keysym.sym == SDLK_d)
            cam1->turnRight(DEFAULT_LEFTRIGHTTURN_MOVEMENT_STEPSIZE);
        if (current_event.key.keysym.sym == SDLK_v)
            crystalballorfirstperson_view = !crystalballorfirstperson_view;
        else if (current_event.key.keysym.sym == SDLK_o)
        {
            //nfdchar_t *outPath = NULL;
            //nfdresult_t result = NFD_OpenDialog("obj", NULL, &outPath);
            //if (result != NFD_OKAY) return;

            //myObject *obj_tmp = new myObject();
            //if (!obj_tmp->readObjects(outPath))
            //{
            //  delete obj_tmp;
            //  return;
            //}
            //delete obj1;
            //obj1 = obj_tmp;
        }
        break;
    }
    case SDL_MOUSEBUTTONDOWN:
    {
        mouse_position[0] = current_event.button.x;
        mouse_position[1] = current_event.button.y;
        mouse_button_pressed = true;

        const Uint8* state = SDL_GetKeyboardState(nullptr);
        if (state[SDL_SCANCODE_LCTRL])
        {
            glm::vec3 ray = cam1->constructRay(mouse_position[0], mouse_position[1]);
            scene.closestObject(ray, cam1->camera_eye, picked_object, picked_triangle_index);
        }
        break;
    }
    case SDL_MOUSEBUTTONUP:
    {
        mouse_button_pressed = false;
        break;
    }
    case SDL_MOUSEMOTION:
    {
        automoveCamera = false;
        automoveCameraTimer = 1500;

        int x = current_event.motion.x;
        int y = current_event.motion.y;

        int dx = x - mouse_position[0];
        int dy = y - mouse_position[1];

        mouse_position[0] = x;
        mouse_position[1] = y;

        if ((dx == 0 && dy == 0) || !mouse_button_pressed) return;

        if ((SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_LEFT)) && crystalballorfirstperson_view)
            cam1->crystalball_rotateView(dx, dy);
        else if ((SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_LEFT)) && !crystalballorfirstperson_view)
            cam1->firstperson_rotateView(dx, dy);
        else if (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_RIGHT))
            cam1->panView(dx, dy);

        break;
    }
    case SDL_WINDOWEVENT:
    {
        if (current_event.window.event == SDL_WINDOWEVENT_RESIZED)
            windowsize_changed = true;
        break;
    }
    case SDL_MOUSEWHEEL:
    {
        if (current_event.wheel.y < 0)
            cam1->moveBack(DEFAULT_MOUSEWHEEL_MOVEMENT_STEPSIZE);
        else if (current_event.wheel.y > 0)
            cam1->moveForward(DEFAULT_MOUSEWHEEL_MOVEMENT_STEPSIZE);
        break;
    }
    default:
        break;
    }
}



// ball and key movement handler (one per key + 10 balls)
struct keyBallHandler
{
    keyBallHandler(myObject* key, std::vector<myObject*> spheres) : m_Key(key), m_Spheres(spheres) {
        // Calculate the trace from sphere to key to access it fast later.


        nSpheres = spheres.size();
        trajectories.resize(nSpheres);
        sphere_start = spheres[0]->objectAverage();
        glm::vec3 end = key->objectAverage();
        auto gend = end - sphere_start;

        // Init a number of different trajectories
        for (auto& trajectory : trajectories)
        {
            trajectory.resize(FRAMES_PER_SECOND + 1, glm::vec3(0));

            const double duration = 1.; // s
            double height = 0.5 + ((std::rand() % 200) - 100) * 0.001f; // m (randomly change the height to get some uniqueness in trajectories)
            double c = (2. / duration) * std::sqrt(height);
            auto f_t = [=](double t) {return static_cast<float>(-(c * t - std::sqrt(height)) * (c * t - std::sqrt(height)) + height); };

            for (int t = 0; t <= FRAMES_PER_SECOND; ++t)
            {
                double tt = t * (1. / FRAMES_PER_SECOND);
                trajectory[t] = sphere_start + gend * (float)tt + glm::vec3(0.f, 1.f, 0.f) * f_t(tt);
            }

            // delta (we only translate delta)
            for (int t = 0; t < FRAMES_PER_SECOND; ++t)
                trajectory[t] = trajectory[t + 1] - trajectory[t];
            trajectory[FRAMES_PER_SECOND] = trajectory[FRAMES_PER_SECOND - 1];
        }


        for (std::unordered_multimap<string, mySubObject*>::iterator it = m_Key->objects.begin(), end = m_Key->objects.end(); it != end; ++it)
            if (it->second->end > it->second->start && it->second->material)
            {
                m_KeyMaterial = it->second->material;
                m_key_white = m_KeyMaterial->kd;
                m_key_pressed = glm::vec4(1, 1, 0, 1);
            }


        // every second activation is the NoteOff command
        aiaswitch = true;
    };

    ~keyBallHandler() {
    }
    // activate this key, the key will be pressed in ms milliseconds (and a ball will move there accordingly)
    void activate() {
        //std::lock_guard<mutex> lock(m_Mutex);
        // switchactive inactive
        if (aiaswitch)
        {
            m_Activate.push_back(FRAMES_PER_SECOND);
            m_Spheresidx.push_back(sid % nSpheres);
            sid++;
        }

        //else
          //  m_DeActivate.push_back(cycles);
        aiaswitch = !aiaswitch;
    };

    // Called with consistent frequency: 
    void Update()
    {
        //std::lock_guard<mutex> lock(m_Mutex);

        // last 16 activations
        for (int i = m_Activate.size() - 1; i >= std::max(0, static_cast<int>(m_Activate.size()) - 17); --i)
        {
            auto& a = m_Activate[i];
            if (a >= 0)
            {
                // balls movement:
                m_Spheres[m_Spheresidx[i]]->translate(trajectories[m_Spheresidx[i]][(FRAMES_PER_SECOND - a)]);

                if (a < 4)
                {
                    m_Key->translate(0, -0.005, 0);
                }
                if (a < 2)
                {
                    m_KeyMaterial->kd = m_key_pressed;
                }

            }
            else if (a > -FRAMES_PER_SECOND) // a < 0
            {
                m_Spheres[m_Spheresidx[i]]->translate(trajectories[m_Spheresidx[i]][-a] / 2.f); // half the impact

                if (a > -5)
                {
                    m_Key->translate(0, 0.005, 0);
                }
                if (a == -5)
                {
                    m_KeyMaterial->kd = m_key_white;
                }

                else if (a == -(FRAMES_PER_SECOND - 1))
                    m_Spheres[m_Spheresidx[i]]->model_matrix = glm::mat4(1.f);
            }
            else
            {
                break;
            }
            // ball dropout
            a--;
        }

        //for (int i = m_DeActivate.size() - 1; i >= 0; --i)
        //{
        //    auto& a = m_DeActivate[i];
        //    //if (a > 0)
        //    //{
        //    //    // balls movement:
        //    //}
        //    //else 
        //    if (a == 0)
        //    {
        //        m_Key->translate(0, 0.02, 0);
        //    }
        //    //else if (a == -5)
        //    //{
        //    //    m_Key->translate(0, 0.02, 0);
        //    //}
        //
        //    a--;
        //
        //    if (a < -10)
        //        break; // no need to go further
        //}

        //100 activations per second max
        if (m_Activate.size() > 1000)
        {
            // erase the first 1000 as they will be inactive by now
            m_Activate.erase(m_Activate.begin(), m_Activate.end() - 1000);
            m_Spheresidx.erase(m_Spheresidx.begin(), m_Spheresidx.end() - 1000);
        }
    }

    bool aiaswitch;
    // 1 key
    myObject* m_Key;
    myMaterial* m_KeyMaterial;
    glm::vec4 m_key_white;
    glm::vec4 m_key_pressed;
    // 10 spheres, that are goind to be reused
    std::vector<myObject*> m_Spheres;
    int nSpheres;
    glm::vec3 sphere_start;
    // current sphere id
    int sid = 0;

    // Activate in milliseconds
    vector<int> m_Activate;
    vector<int> m_Spheresidx;
    std::mutex m_Mutex;

    std::vector<std::vector<glm::vec3>> trajectories;
};

std::map<string, keyBallHandler*> g_keys;

class CustomOutput :public CxxMidi::Output::Abstract
{
public:
    CustomOutput() : CxxMidi::Output::Abstract() {};
    ~CustomOutput() {};


    virtual void openPort(unsigned int portNumber_ = 0) override {};
    virtual void closePort() override {};
    virtual void openVirtualPort(const std::string& portName_ = std::string("RtMidi Virtual Output"))override {};
    virtual size_t getPortCount() override { return 0; };
    virtual std::string getPortName(unsigned int portNumber_ = 0) override { return ""; };
    virtual void sendMessage(const std::vector<uint8_t>* msg_) override {
        if (msg_)
        {
            const CxxMidi::Message* msg = static_cast<const CxxMidi::Message*>(msg_);
            string nname = CxxMidi::Note::name(((*msg_)[1]));
            const auto it = g_keys.find(nname);
            if (it != g_keys.end())
                it->second->activate(); // activate AND deactivate a note sends the same signal (its handled in the keyBallHandler)
        }
    };
protected:
    inline virtual void initialize() {};
};
inline bool isKey(const string& str)
{
    // 2 strings that can also be returned by CxxMidi::Note::name().
    if (str == "Undefined" || str == "Error")
        return false;

    for (int i = 0; i < 127; ++i)
    {
        if (CxxMidi::Note::name(i) == str)
            return true;

    }
    return false;
}

int main(int argc, char* argv[])
{
    auto custout = new CustomOutput();
    CxxMidi::Player::Synchronous splayer(new CxxMidi::Output::Default(0));
    CxxMidi::Player::Synchronous splayer2(custout);

    // dont now how concurrency works within cxxmidi: better to load file 2 times
    auto file = std::make_shared<CxxMidi::File>("test.mid");
    auto file2 = std::make_shared<CxxMidi::File>("test.mid");
    splayer.setFile(file.get());
    splayer2.setFile(file2.get());

    // Initialize video subsystem
    SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);

    // Using OpenGL 3.1 core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    // Create window
    window = nullptr;
    window = SDL_CreateWindow("IN4I24", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    // Create OpenGL context
    glContext = SDL_GL_CreateContext(window);

    // Initialize glew
    glewInit();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);

    glEnable(GL_MULTISAMPLE);
    glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_FASTEST);

    cam1 = new myCamera();
    SDL_GetWindowSize(window, &cam1->window_width, &cam1->window_height);
    cam1->moveBack(0.5f);
    cam1->camera_eye = glm::vec3(0., 2., 2.);
    cam1->camera_forward = glm::normalize(glm::vec3(0.f, 0.5f, 0.f) - cam1->camera_eye);
    //cam1->camera_up = glm::cross()
    glm::mat3 cRotMat = glm::eulerAngleY(0.001);

    checkOpenGLInfo(true);


    /**************************INITIALIZING FBO ***************************/
    //plane will draw the color_texture of the framebufferobject fbo.
    myWaterFBOs* waterFBOs = new myWaterFBOs();
    waterFBOs->initWaterFBOs(cam1->window_width, cam1->window_height);

    string dudvmap = "models/waterDUDV.png";
    string waterNormalmap = "models/waterNormalMap.png";

    // load water
    //{
    //    //reading a scene with texture coordinates and normals read from the obj file
    //    auto obj = new myObject();
    //    //obj->readObjects("models/Water.obj");
    //    obj->readObjects("models/Lake.obj");
    //    //obj->setTexture(new myTexture(cubemaps), mySubObject::CUBEMAP);
    //    //obj->setTexture(new myTexture("models/cloud.jpg"), mySubObject::COLORMAP);
    //    obj->setTexture(waterFBOs->reflectionFrameBuffer->colortexture, mySubObject::REFLECTIONMAP);
    //    obj->setTexture(waterFBOs->refractionFrameBuffer->colortexture, mySubObject::REFRACTIONMAP);
    //    obj->setTexture(new myTexture(dudvmap), mySubObject::DUDVMAP);
    //    obj->setTexture(new myTexture(waterNormalmap), mySubObject::BUMPMAP);
    //    obj->createmyVAO();
    //    scene.addObjects(obj, "lake");
    //
    //    obj = new myObject();
    //    //obj->readObjects("models/Water.obj");
    //    obj->readObjects("models/Waterplane.obj");
    //    //obj->setTexture(new myTexture(cubemaps), mySubObject::CUBEMAP);
    //    //obj->setTexture(new myTexture("models/cloud.jpg"), mySubObject::COLORMAP);
    //    obj->setTexture(waterFBOs->reflectionFrameBuffer->colortexture, mySubObject::REFLECTIONMAP);
    //    obj->setTexture(waterFBOs->refractionFrameBuffer->colortexture, mySubObject::REFRACTIONMAP);
    //    obj->setTexture(new myTexture(dudvmap), mySubObject::DUDVMAP);
    //    obj->setTexture(new myTexture(waterNormalmap), mySubObject::BUMPMAP);
    //    obj->createmyVAO();
    //    scene.addObjects(obj, "waterAdvanced");
    //
    //
    //    obj = new myObject();
    //    obj->readObjects("models/Water.obj");
    //    //obj->setTexture(new myTexture(cubemaps), mySubObject::CUBEMAP);
    //    obj->setTexture(new myTexture("models/cloud.jpg"), mySubObject::COLORMAP);
    //    obj->createmyVAO();
    //    //obj->translate(glm::vec3(0, WATER_HEIGHT, 0));
    //    scene.addObjects(obj, "water");
    //}

    // skybox
    mySkybox* skybox;
    {
        skybox = new mySkybox();
        //obj = new myObject();
        skybox->readObjects("models/skybox.obj", false, false);
        skybox->createmyVAO();
        scene.addObjects(skybox, "skybox");
    }
    // load piano
    {
        auto obj = new myObject();
        obj->readObjects("models/piano_body.obj", false, false);
        obj->createmyVAO();
        scene.addObjects(obj, "piano_body");
    }

    std::vector<myObject*> lanterns;
    std::vector<glm::vec3> lantern_tvec;
    // lights / lanterns
    {
        scene.readObjectsFromFolder("models/lantern");
        // Add them as lights
        scene.lights = new myLights();
        for (auto& lantern : scene.all_objects)
            if (lantern->name.find("lantern") != std::string::npos)
            {
                lanterns.push_back(lantern);
                if (moveLights)
                    lantern_tvec.push_back(glm::vec3(((std::rand() % 200) - 100) * 0.000001, ((std::rand() % 200) - 100) * 0.000001, ((std::rand() % 200) - 100) * 0.000001));

                auto avg = lantern->objectAverage();
                avg -= glm::vec3(0, 0.01, 0);
                // add a light at the average vertices position of any lantern
                scene.lights->lights.push_back(new myLight(avg, glm::vec3(0.4, 0.4, 0.4), myLight::POINTLIGHT));

            }
    }
    size_t lanternSize = lanterns.size();



    // load piano keys
    scene.readObjectsFromFolder("models/piano_parts");


    /**************************INITIALIZING OBJECTS THAT WILL BE DRAWN ***************************/
    //myObject* obj;
    //reading a scene with texture coordinates and normals read from the obj file
    //obj = new myObject();
    //scene->readObjects("models/bus/bus.obj", true);
    //scene->readObjects("models/Pikachu/model.obj", true);
    //obj->readObjects("models/piano_compl.obj", false, false);
    //obj->scale(0.001f, 0.001f, 0.001f);
    //obj->rotate(0, 1, 0, 1.57f);
    //obj->translate(20.0f, 0.0f, 0.0f);
    //obj->createmyVAO();

    //std::map<string, bool> exclude_keys;

    auto s = new myObject();
    s->readObjects("models/Sphere.obj", false, false);
    s->createmyVAO();
    // 128 notes a 7 spheres: every key gets 7 spheres to use simultaneously
    vector<vector<myObject*>> spheres(128);
    for (auto& sv : spheres)
    {
        sv = s->createFromThis(16, true);
    }

    {
        int k = 0;
        for (auto& o : scene.all_objects)
        {
            string name = o->name;
            if (name.find("-") != std::string::npos)
            {
                // need to adapt the name:
                name.replace(name.find("-"), 1, "/");
            }
            if (isKey(name))
            {
                g_keys[name] = new keyBallHandler(o, spheres.at(k));
                k++;
            }

        }
    }

    /**************************SETTING UP OPENGL SHADERS ***************************/
    myShaders shaders;
    //shaders.addShader(new myShader("shaders/basic-vertexshader.glsl", "shaders/basic-fragmentshader.glsl"), "shader_basic");
    //shaders.addShader(new myShader("shaders/phong-vertexshader.glsl", "shaders/phong-fragmentshader.glsl"), "shader_phong");  
    //shaders.addShader(new myShader("shaders/texture-vertexshader.glsl", "shaders/texture-fragmentshader.glsl"), "shader_texture");
    shaders.addShader(new myShader("shaders/texture+phong-vertexshader.glsl", "shaders/texture+phong-fragmentshader.glsl"), "shader_texturephong");
    shaders.addShader(new myShader("shaders/skybox-vertexshader.glsl", "shaders/skybox-fragmentshader.glsl"), "shader_skybox");
    //shaders.addShader(new myShader("shaders/water-vertexshader.glsl", "shaders/water-fragmentshader.glsl"), "shader_water");
    //shaders.addShader(new myShader("shaders/waterReflectRefrac-vertexshader.glsl", "shaders/waterReflectRefrac-fragmentshader.glsl"), "shader_advanced_water");
    myShader* curr_shader;

    int sleep_time = 0;

    atomic_bool playSound = false;
    atomic_bool playKeys = false;

    std::thread Soundplayer = std::thread([&playSound, &splayer]() {
        while (!playSound)
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        splayer.play();

    });
    std::thread Keyplayer = std::thread([&playKeys, &splayer2]() {
        while (!playKeys)
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        splayer2.play();

    });

    DWORD next_game_tick = GetTickCount();
    DWORD start_render = GetTickCount();


    // display loop
    while (!quit)
    {
        if (!playSound)
        {
            if (GetTickCount() - start_render > 6950) // 1 second of delay
            {
                playSound = true;// very broad sense of synchronization (none). We expect that it runs more or less synchron
            }
        }
        if (!playKeys)
        {
            if (GetTickCount() - start_render > 6000)
            {
                playKeys = true; // very broad sense of synchronization (none). We expect that it runs more or less synchron
            }
        }

        // move camera:
        if (automoveCamera)
        {
            // Move in circle around the piano:
            cam1->camera_eye = cRotMat * cam1->camera_eye;
            auto shouldForward = glm::normalize(glm::vec3(0.f, 0.5f, 0.f) - cam1->camera_eye);
            auto ddiff = shouldForward - cam1->camera_forward;
            cam1->camera_forward += ddiff * 0.01f; // slowly get back to focusing the center

            if (cam1->camera_eye[0] < -0.1)
            {
                // it would get stuck if we manually move behind the scene:
                cam1->camera_eye[0] = -cam1->camera_eye[0];
                cRotMat = glm::inverse(cRotMat);
            }

        }

        if (moveLights)
        {
            /// Lanterns MOVE!
            for (size_t l = 0; l < lanternSize; ++l)
            {
                lanterns[l]->translate(lantern_tvec[l]);
                scene.lights->lights[l]->position += lantern_tvec[l];

                // update direction:
                lantern_tvec[l] += glm::vec3(((std::rand() % 200) - 100) * 0.0000001, ((std::rand() % 200) - 100) * 0.0000001, ((std::rand() % 200) - 100) * 0.0000001);
            }
        }

        if (windowsize_changed)
        {
            SDL_GetWindowSize(window, &cam1->window_width, &cam1->window_height);
            windowsize_changed = false;

            if (waterFBOs) delete waterFBOs;
            waterFBOs = new myWaterFBOs();
            waterFBOs->initWaterFBOs(cam1->window_width, cam1->window_height);
            //scene["plane1"]->setTexture(waterFBOs->reflectionFrameBuffer->colortexture, mySubObject::COLORMAP);
            //scene["plane2"]->setTexture(waterFBOs->refractionFrameBuffer->colortexture, mySubObject::COLORMAP);
            //scene["waterAdvanced"]->setTexture(waterFBOs->reflectionFrameBuffer->colortexture, mySubObject::REFLECTIONMAP);
            //scene["waterAdvanced"]->setTexture(waterFBOs->refractionFrameBuffer->colortexture, mySubObject::REFRACTIONMAP);
            //scene["lake"]->setTexture(waterFBOs->reflectionFrameBuffer->colortexture, mySubObject::REFLECTIONMAP);
            //scene["lake"]->setTexture(waterFBOs->refractionFrameBuffer->colortexture, mySubObject::REFRACTIONMAP);
            //scene["plane2"]->setTexture(fbo->colortexture, mySubObject::COLORMAP);
        }

        //Computing transformation matrices. Note that model_matrix will be computed and set in the displayScene function for each object separately
        glViewport(0, 0, cam1->window_width, cam1->window_height);
        glm::mat4 projection_matrix = cam1->projectionMatrix();
        glm::mat4 view_matrix = cam1->viewMatrix();
        glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(view_matrix)));

        //Setting uniform variables for each shader
        for (unsigned int i = 0; i < shaders.size(); i++)
        {
            curr_shader = shaders[i];
            curr_shader->start();
            curr_shader->setUniform("myprojection_matrix", projection_matrix);
            curr_shader->setUniform("myview_matrix", view_matrix);
            curr_shader->setUniform("mynormal_matrix", normal_matrix);
            scene.lights->setUniform(curr_shader, "Lights");
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        curr_shader = shaders["shader_texturephong"];
        curr_shader->start();
        for (auto& o : scene.all_objects)
            o->displayObjects(curr_shader, view_matrix);

        for (auto& ov : spheres)
            for (auto& o : ov)
                o->displayObjects(curr_shader, view_matrix);

        curr_shader = shaders["shader_skybox"];
        curr_shader->start();

        // skybox->rotate();
        ((mySkybox*)scene["skybox"])->displayObjects(curr_shader, view_matrix, 0.0001f);


        /*-----------------------*/
        // update all moving objects
        {
            // lock_guard<mutex> lock(g_keys_mutex); const access is safe (C++ 11 standard)
            for (const auto& k : g_keys)
            {
                k.second->Update(); // current frequency of call
            }
        }

        SDL_GL_SwapWindow(window);

        SDL_Event current_event;
        while (SDL_PollEvent(&current_event) != 0)
            processEvents(current_event);

        next_game_tick += SKIP_TICKS;
        sleep_time = next_game_tick - GetTickCount();

        if (automoveCameraTimer > 0)
        {
            automoveCameraTimer -= SKIP_TICKS;
            if (automoveCameraTimer <= 0)
                automoveCamera = true;
        }


        if (sleep_time >= 0) {
            //cout << "Left " << sleep_time << " ms" << endl;
            this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
        }
        else {
            //cout << "Run slower than fixed fpw!";
        }
    }

    // Freeing resources before exiting.

    // Destroy window
    if (glContext) SDL_GL_DeleteContext(glContext);
    if (window) SDL_DestroyWindow(window);

    // Quit SDL subsystems
    SDL_Quit();

    return 0;
}
