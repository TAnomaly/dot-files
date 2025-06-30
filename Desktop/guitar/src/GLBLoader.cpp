#include "GLBLoader.h"
#include <GL/glew.h>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

// Define these only in one cpp file
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../third_party/tinygltf/tiny_gltf.h"

GLBLoader::GLBLoader() : guitarLength_(8.0f), guitarWidth_(1.2f) {
    neckStart_ = glm::vec3(-4.0f, 0.0f, 0.0f);
    neckEnd_ = glm::vec3(4.0f, 0.0f, 0.0f);
}

GLBLoader::~GLBLoader() {
    for (auto& mesh : meshes_) {
        glDeleteVertexArrays(1, &mesh.VAO);
        glDeleteBuffers(1, &mesh.VBO);
        glDeleteBuffers(1, &mesh.EBO);
    }
}

bool GLBLoader::loadModel(const std::string& filename) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool res = loader.LoadBinaryFromFile(&model, &err, &warn, filename);
    if (!warn.empty()) {
        std::cout << "WARN: " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cerr << "ERR: " << err << std::endl;
    }
    if (!res) {
        std::cerr << "Failed to load glTF: " << filename << std::endl;
        return false;
    }

    const tinygltf::Scene& scene = model.scenes[model.defaultScene > -1 ? model.defaultScene : 0];
    for (size_t i = 0; i < scene.nodes.size(); i++) {
        processNode(model.nodes[scene.nodes[i]], model);
    }

    return true;
}

void GLBLoader::processNode(const tinygltf::Node& node, const tinygltf::Model& model) {
    if (node.mesh > -1) {
        processMesh(model.meshes[node.mesh], model);
    }
    for (size_t i = 0; i < node.children.size(); i++) {
        processNode(model.nodes[node.children[i]], model);
    }
}

void GLBLoader::processMesh(const tinygltf::Mesh& mesh, const tinygltf::Model& model) {
    for (size_t i = 0; i < mesh.primitives.size(); ++i) {
        Mesh newMesh;
        const tinygltf::Primitive& primitive = mesh.primitives[i];

        // Positions
        const float* positions = nullptr;
        if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
            const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.at("POSITION")];
            const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
            positions = reinterpret_cast<const float*>(&model.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset]);
        }

        // Normals
        const float* normals = nullptr;
        if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
            const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.at("NORMAL")];
            const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
            normals = reinterpret_cast<const float*>(&model.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset]);
        }

        // Texture Coordinates
        const float* texCoords = nullptr;
        if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
            const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
            const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
            texCoords = reinterpret_cast<const float*>(&model.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset]);
        }

        const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
        newMesh.vertices.resize(posAccessor.count);
        for (size_t v = 0; v < posAccessor.count; ++v) {
            if (positions) {
                newMesh.vertices[v].position = glm::make_vec3(&positions[v * 3]);
            }
            if (normals) {
                newMesh.vertices[v].normal = glm::make_vec3(&normals[v * 3]);
            }
            if (texCoords) {
                newMesh.vertices[v].texCoords = glm::make_vec2(&texCoords[v * 2]);
            }
        }

        // Indices
        if (primitive.indices > -1) {
            const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
            const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
            const void* data_ptr = &buffer.data[bufferView.byteOffset + accessor.byteOffset];

            newMesh.indices.resize(accessor.count);
            switch (accessor.componentType) {
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                    for (size_t j = 0; j < accessor.count; ++j) newMesh.indices[j] = reinterpret_cast<const uint8_t*>(data_ptr)[j];
                    break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                    for (size_t j = 0; j < accessor.count; ++j) newMesh.indices[j] = reinterpret_cast<const uint16_t*>(data_ptr)[j];
                    break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                    for (size_t j = 0; j < accessor.count; ++j) newMesh.indices[j] = reinterpret_cast<const uint32_t*>(data_ptr)[j];
                    break;
                default:
                    break;
            }
        }

        newMesh.setupMesh();
        meshes_.push_back(newMesh);
    }
}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // Vertex Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // Vertex Texture Coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

    glBindVertexArray(0);
}

void GLBLoader::render() {
    for (auto& mesh : meshes_) {
        mesh.draw();
    }
}

void Mesh::draw() {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// Ray-Triangle intersection
bool rayIntersectsTriangle(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float& t) {
    const float EPSILON = 0.000001f;
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 h = glm::cross(rayDirection, edge2);
    float a = glm::dot(edge1, h);
    if (a > -EPSILON && a < EPSILON) return false; // Ray is parallel to the triangle

    float f = 1.0f / a;
    glm::vec3 s = rayOrigin - v0;
    float u = f * glm::dot(s, h);
    if (u < 0.0f || u > 1.0f) return false;

    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(rayDirection, q);
    if (v < 0.0f || u + v > 1.0f) return false;

    t = f * glm::dot(edge2, q);
    return t > EPSILON;
}

bool GLBLoader::checkGuitarHit(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, glm::vec3& hitPoint) {
    float closest_t = std::numeric_limits<float>::max();
    bool hit = false;

    for (const auto& mesh : meshes_) {
        for (size_t i = 0; i < mesh.indices.size(); i += 3) {
            const Vertex& v0 = mesh.vertices[mesh.indices[i]];
            const Vertex& v1 = mesh.vertices[mesh.indices[i+1]];
            const Vertex& v2 = mesh.vertices[mesh.indices[i+2]];

            float t;
            if (rayIntersectsTriangle(rayOrigin, rayDirection, v0.position, v1.position, v2.position, t)) {
                if (t < closest_t) {
                    closest_t = t;
                    hitPoint = rayOrigin + t * rayDirection;
                    hit = true;
                }
            }
        }
    }

    return hit;
}

int GLBLoader::getStringFromHit(const glm::vec3& hitPoint) {
    float normalizedY = (hitPoint.y + guitarWidth_ / 2) / guitarWidth_;
    int string = static_cast<int>(normalizedY * NUM_STRINGS);
    return std::max(0, std::min(string, NUM_STRINGS - 1));
}

int GLBLoader::getFretFromHit(const glm::vec3& hitPoint) {
    float normalizedX = (hitPoint.x - neckStart_.x) / (neckEnd_.x - neckStart_.x);
    int fret = static_cast<int>(normalizedX * NUM_FRETS);
    return std::max(0, std::min(fret, NUM_FRETS));
}
