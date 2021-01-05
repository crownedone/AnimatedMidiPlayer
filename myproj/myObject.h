#pragma once
#include "myVAO.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include "mySubObject.h"
#include "myShader.h"
#include "myMaterial.h"
#include "myTexture.h"
#include <unordered_map>
#include <mutex>

using string = std::string;
class myObject
{
public:
    myObject();
    ~myObject();
    void clear();

    // create copies from this without vao!
    std::vector<myObject*> createFromThis(size_t num, bool createVAO) const;
    void readMaterials(std::string mtlfilename, std::unordered_map<std::string, myMaterial*>& materials, std::unordered_map<std::string, myTexture*>& textures);
    bool readObjects(std::string filename, bool individualvertices_per_face = true, bool tonormalize = false);
    void normalize();
    void computeNormals();
    void createmyVAO();

    void displayObjects(myShader*, glm::mat4, const std::map<string, bool>& exclude = std::map<string, bool>());
    void displayObjects(myShader* shader, glm::mat4, std::string name);

    void displayNormals(myShader*);

    glm::vec3 closestVertex(glm::vec3 ray, glm::vec3 starting_point);
    glm::vec3 objectAverage() const;
    float closestTriangle(glm::vec3 ray, glm::vec3 origin, size_t& picked_triangle, mySubObject*& picked_object);
    float closestTriangle(glm::vec3 ray, glm::vec3 origin, size_t& picked_triangle);

    void locate(float x, float y, float z);
    void translate(float x, float y, float z);
    void scale(float x, float y, float z);
    void rotate(float axis_x, float axis_y, float axis_z, float angle);

    void locate(glm::vec3);
    void translate(glm::vec3);
    void scale(glm::vec3);
    void rotate(glm::vec3, float);

    void computeTexturecoordinates_plane();
    void computeTexturecoordinates_cylinder();
    void computeTexturecoordinates_sphere();

    void computeTangents();

    void setTexture(myTexture*, mySubObject::TEXTURE_TYPE);

    myVAO* vao;
    std::vector<glm::vec3> vertices;
    std::vector<glm::ivec3> indices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texturecoordinates;
    std::vector<glm::vec3> tangents;

    glm::mat4 model_matrix;
    std::unordered_multimap<std::string, mySubObject*> objects;
    std::string name;
};
