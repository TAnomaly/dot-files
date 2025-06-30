#pragma once
#include <memory>
#include "GLBLoader.h"
#include "Camera.h"
#include "AudioManager.h"

class Guitar3D
{
private:
    std::unique_ptr<GLBLoader> modelLoader_;
    std::unique_ptr<Camera> camera_;
    AudioManager *audioManager_;

    unsigned int shaderProgram_;

    // Guitar string frequencies (same as before)
    std::vector<float> stringBaseFrequencies_;

    // Lighting
    glm::vec3 lightPos_;
    glm::vec3 lightColor_;
    glm::mat4 model_;

    // Shader utility functions
    unsigned int loadShader(const std::string &vertexPath, const std::string &fragmentPath);
    unsigned int compileShader(const std::string &source, unsigned int type);
    std::string loadShaderSource(const std::string &filepath);

    // Guitar calculations
    float calculateFretFrequency(float baseFreq, int fretNumber);
    std::string getNoteName(float frequency);

public:
    Guitar3D(int windowWidth, int windowHeight, AudioManager *audioManager);
    ~Guitar3D();

    bool initialize();
    void render();
    void handleClick(int x, int y, int windowWidth, int windowHeight);
    void handleMouseMotion(int deltaX, int deltaY);
    void handleMouseWheel(int delta);
    void resize(int width, int height);
};