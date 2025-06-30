#include "Guitar3D.h"
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Guitar3D::Guitar3D(int windowWidth, int windowHeight, AudioManager *audioManager)
    : audioManager_(audioManager), shaderProgram_(0), lightPos_(2.0f, 2.0f, 2.0f), lightColor_(1.0f, 1.0f, 1.0f)
{

    // Initialize camera
    camera_ = std::make_unique<Camera>((float)windowWidth, (float)windowHeight);
    camera_->setPosition(glm::vec3(0.0f, 2.0f, 3.0f)); // Position camera to look at the guitar from an angle
    camera_->setTarget(glm::vec3(0.0f, 0.0f, 0.0f));

    // Initialize string frequencies (same as before)
    stringBaseFrequencies_ = {
        82.41f,  // Low E (6th string)
        110.0f,  // A (5th string)
        146.83f, // D (4th string)
        196.0f,  // G (3rd string)
        246.94f, // B (2nd string)
        329.63f  // High E (1st string)
    };

    // Initialize model loader
    modelLoader_ = std::make_unique<GLBLoader>();
}

Guitar3D::~Guitar3D()
{
    if (shaderProgram_)
    {
        glDeleteProgram(shaderProgram_);
    }
}

bool Guitar3D::initialize()
{
    // Initialize GLEW
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }

    // Load shaders
    std::cout << "Loading shaders..." << std::endl;
    shaderProgram_ = loadShader("shaders/vertex.glsl", "shaders/fragment.glsl");
    if (shaderProgram_ == 0)
    {
        std::cerr << "Failed to load shaders" << std::endl;
        return false;
    }
    std::cout << "Shaders loaded successfully" << std::endl;

    // Load guitar model
    std::cout << "Loading guitar model..." << std::endl;
    if (!modelLoader_->loadModel("gibson_les_paul_standard_guitar.glb"))
    {
        std::cerr << "Failed to load guitar model" << std::endl;
        return false;
    }
    std::cout << "Guitar model loaded successfully" << std::endl;

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    std::cout << "Guitar3D initialized successfully" << std::endl;
    return true;
}

void Guitar3D::render()
{
    // Clear screen with different color to test
    glClearColor(0.2f, 0.3f, 0.4f, 1.0f); // Blue-ish background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use shader program
    glUseProgram(shaderProgram_);

    // Set matrices
    model_ = glm::mat4(1.0f);
    model_ = glm::rotate(model_, glm::radians(-75.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Tilt it forward
    model_ = glm::rotate(model_, glm::radians(15.0f), glm::vec3(0.0f, 0.0f, 1.0f));  // A slight rotation for a better view
    glm::mat4 view = camera_->getViewMatrix();
    glm::mat4 projection = camera_->getProjectionMatrix();

    // Set uniforms
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram_, "model"), 1, GL_FALSE, glm::value_ptr(model_));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram_, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram_, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Set lighting uniforms
    glUniform3fv(glGetUniformLocation(shaderProgram_, "lightPos"), 1, glm::value_ptr(lightPos_));
    glUniform3fv(glGetUniformLocation(shaderProgram_, "lightColor"), 1, glm::value_ptr(lightColor_));
    glUniform3f(glGetUniformLocation(shaderProgram_, "objectColor"), 0.8f, 0.4f, 0.2f); // Wood color
    glUniform3fv(glGetUniformLocation(shaderProgram_, "viewPos"), 1, glm::value_ptr(camera_->getPosition()));

    // Render guitar model
    modelLoader_->render();
}

void Guitar3D::handleClick(int x, int y, int windowWidth, int windowHeight)
{
    // Get ray from camera through click position
    glm::vec3 rayDir = camera_->screenToWorldRay(x, y, windowWidth, windowHeight);
    glm::vec3 rayOrigin = camera_->getPosition();

    // Transform ray to model space
    glm::mat4 inverseModel = glm::inverse(model_);
    glm::vec4 rayOriginModelSpace = inverseModel * glm::vec4(rayOrigin, 1.0f);
    glm::vec4 rayDirModelSpace = inverseModel * glm::vec4(rayDir, 0.0f);

    // Check for intersection with guitar
    glm::vec3 hitPoint;
    if (modelLoader_->checkGuitarHit(glm::vec3(rayOriginModelSpace), glm::vec3(rayDirModelSpace), hitPoint))
    {
        int stringIndex = modelLoader_->getStringFromHit(hitPoint);
        int fretNumber = modelLoader_->getFretFromHit(hitPoint);

        // Clamp to valid ranges
        stringIndex = std::max(0, std::min(stringIndex, 5));
        fretNumber = std::max(0, std::min(fretNumber, 12));

        // Calculate frequency
        float baseFreq = stringBaseFrequencies_[stringIndex];
        float frequency = calculateFretFrequency(baseFreq, fretNumber);
        std::string noteName = getNoteName(frequency);

        std::cout << "Hit guitar! String: " << (stringIndex + 1)
                  << ", Fret: " << fretNumber
                  << ", Note: " << noteName
                  << " (" << frequency << " Hz)" << std::endl;

        // Play the note
        audioManager_->playNote(frequency);
    }
}

void Guitar3D::handleMouseMotion(int deltaX, int deltaY)
{
    camera_->handleMouseMotion(deltaX, deltaY);
}

void Guitar3D::handleMouseWheel(int delta)
{
    camera_->handleMouseWheel(delta);
}

void Guitar3D::resize(int width, int height)
{
    glViewport(0, 0, width, height);
    camera_->updateAspectRatio((float)width, (float)height);
}

unsigned int Guitar3D::loadShader(const std::string &vertexPath, const std::string &fragmentPath)
{
    std::string vertexSource = loadShaderSource(vertexPath);
    std::string fragmentSource = loadShaderSource(fragmentPath);

    if (vertexSource.empty() || fragmentSource.empty())
    {
        return 0;
    }

    unsigned int vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);

    if (vertexShader == 0 || fragmentShader == 0)
    {
        return 0;
    }

    // Link shaders
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // Check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
        return 0;
    }

    // Delete shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

unsigned int Guitar3D::compileShader(const std::string &source, unsigned int type)
{
    unsigned int shader = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    // Check for compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
        return 0;
    }

    return shader;
}

std::string Guitar3D::loadShaderSource(const std::string &filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open shader file: " << filepath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

float Guitar3D::calculateFretFrequency(float baseFreq, int fretNumber)
{
    return baseFreq * std::pow(2.0f, fretNumber / 12.0f);
}

std::string Guitar3D::getNoteName(float frequency)
{
    std::vector<std::string> noteNames = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

    float a4 = 440.0f;
    float noteNumber = 12 * std::log2(frequency / a4) + 69;
    int noteIndex = ((int)noteNumber) % 12;
    if (noteIndex < 0)
        noteIndex += 12;

    int octave = ((int)noteNumber + 8) / 12;

    return noteNames[noteIndex] + std::to_string(octave);
}