#pragma once
#include <vector>
#include <string>
#include <limits> // Required for std::numeric_limits
#include <glm/glm.hpp>
#include "../third_party/tinygltf/tiny_gltf.h" // Include tinygltf header

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO, VBO, EBO;

    void setupMesh();
    void draw();
};

class GLBLoader {
public:
    GLBLoader();
    ~GLBLoader();

    bool loadModel(const std::string& filename);
    void render();

    // Guitar-specific methods
    bool checkGuitarHit(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, glm::vec3& hitPoint);
    int getStringFromHit(const glm::vec3& hitPoint);
    int getFretFromHit(const glm::vec3& hitPoint);

    bool isLoaded() const { return !meshes_.empty(); }

private:
    void processNode(const tinygltf::Node& node, const tinygltf::Model& model);
    void processMesh(const tinygltf::Mesh& mesh, const tinygltf::Model& model);

    std::vector<Mesh> meshes_;

    // Guitar layout constants
    static const int NUM_STRINGS = 6;
    static const int NUM_FRETS = 12;

    // Guitar dimensions (will be calculated from model)
    float guitarLength_;
    float guitarWidth_;
    glm::vec3 neckStart_;
    glm::vec3 neckEnd_;
};