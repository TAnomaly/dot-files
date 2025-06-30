#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

// Vertex shader source code
const char *vertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "out vec2 TexCoords;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(aPos, 1.0);\n"
    "    TexCoords = aTexCoord;\n"
    "}\n";

// Function to read shader file
char *readFile(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        printf("Failed to open file: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(length + 1);
    if (!buffer)
    {
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

// Function to compile shader
GLuint compileShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Shader compilation failed: %s\n", infoLog);
        return 0;
    }
    return shader;
}

// Forward declare the main function
int main(int argc, char *argv[]);

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    int argc = __argc;
    char **argv = __argv;
    return main(argc, argv);
}
#endif

// Function to initialize video encoder
AVCodecContext *init_video_encoder(int width, int height, int fps, const char *filename,
                                   AVFormatContext **output_format_context)
{
    const AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec)
    {
        printf("Could not find H.264 encoder\n");
        return NULL;
    }

    AVCodecContext *codec_context = avcodec_alloc_context3(codec);
    if (!codec_context)
    {
        printf("Could not allocate encoder context\n");
        return NULL;
    }

    codec_context->bit_rate = 2000000;
    codec_context->width = width;
    codec_context->height = height;
    codec_context->time_base = (AVRational){1, fps};
    codec_context->framerate = (AVRational){fps, 1};
    codec_context->gop_size = 10;
    codec_context->max_b_frames = 1;
    codec_context->pix_fmt = AV_PIX_FMT_YUV420P;

    int ret = avcodec_open2(codec_context, codec, NULL);
    if (ret < 0)
    {
        printf("Could not open codec: %d\n", ret);
        avcodec_free_context(&codec_context);
        return NULL;
    }

    ret = avformat_alloc_output_context2(output_format_context, NULL, NULL, filename);
    if (ret < 0)
    {
        printf("Could not allocate output context: %d\n", ret);
        avcodec_free_context(&codec_context);
        return NULL;
    }

    AVStream *stream = avformat_new_stream(*output_format_context, codec);
    if (!stream)
    {
        printf("Could not create output stream\n");
        avcodec_free_context(&codec_context);
        avformat_free_context(*output_format_context);
        return NULL;
    }

    stream->time_base = codec_context->time_base;
    ret = avcodec_parameters_from_context(stream->codecpar, codec_context);
    if (ret < 0)
    {
        printf("Could not copy codec params: %d\n", ret);
        avcodec_free_context(&codec_context);
        avformat_free_context(*output_format_context);
        return NULL;
    }

    ret = avio_open(&(*output_format_context)->pb, filename, AVIO_FLAG_WRITE);
    if (ret < 0)
    {
        printf("Could not open output file: %d\n", ret);
        avcodec_free_context(&codec_context);
        avformat_free_context(*output_format_context);
        return NULL;
    }

    ret = avformat_write_header(*output_format_context, NULL);
    if (ret < 0)
    {
        printf("Could not write header: %d\n", ret);
        avcodec_free_context(&codec_context);
        avio_closep(&(*output_format_context)->pb);
        avformat_free_context(*output_format_context);
        return NULL;
    }

    return codec_context;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <input_video>\n", argv[0]);
        return 1;
    }

    printf("Initializing SDL...\n");
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    printf("Creating window...\n");
    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Create window
    SDL_Window *window = SDL_CreateWindow(
        "Video Shader",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    if (!window)
    {
        printf("Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    printf("Creating OpenGL context...\n");
    // Create OpenGL context
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext)
    {
        printf("OpenGL context creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    printf("Initializing GLEW...\n");
    // Initialize GLEW
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK)
    {
        printf("GLEW initialization failed: %s\n", glewGetErrorString(glewError));
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    printf("Opening video file: %s\n", argv[1]);
    // Initialize FFmpeg
    AVFormatContext *formatContext = NULL;
    if (avformat_open_input(&formatContext, argv[1], NULL, NULL) != 0)
    {
        printf("Could not open video file: %s\n", argv[1]);
        return 1;
    }

    printf("Finding stream info...\n");
    if (avformat_find_stream_info(formatContext, NULL) < 0)
    {
        printf("Could not find stream info\n");
        return 1;
    }

    // Find video stream
    int videoStream = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; i++)
    {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
            break;
        }
    }

    if (videoStream == -1)
    {
        printf("Could not find video stream\n");
        return 1;
    }

    printf("Setting up video codec...\n");
    // Get codec parameters
    AVCodecParameters *codecParams = formatContext->streams[videoStream]->codecpar;
    const AVCodec *codec = avcodec_find_decoder(codecParams->codec_id);
    if (!codec)
    {
        printf("Unsupported codec!\n");
        return 1;
    }

    AVCodecContext *codecContext = avcodec_alloc_context3(codec);
    if (!codecContext)
    {
        printf("Failed to allocate codec context!\n");
        return 1;
    }

    if (avcodec_parameters_to_context(codecContext, codecParams) < 0)
    {
        printf("Failed to copy codec parameters to context!\n");
        return 1;
    }

    if (avcodec_open2(codecContext, codec, NULL) < 0)
    {
        printf("Failed to open codec!\n");
        return 1;
    }

    printf("Loading shaders...\n");
    // Create and compile shaders
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    char *fragmentSource = readFile("video_shader.glsl");
    if (!fragmentSource)
    {
        printf("Failed to read fragment shader file: video_shader.glsl\n");
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    free(fragmentSource);

    if (!vertexShader || !fragmentShader)
    {
        printf("Shader compilation failed!\n");
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Create shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("Shader program linking failed: %s\n", infoLog);
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Create fullscreen quad
    float vertices[] = {
        // positions        // texture coords
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f};

    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3};

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Create texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Get uniform locations
    GLint timeLocation = glGetUniformLocation(shaderProgram, "iTime");
    GLint resolutionLocation = glGetUniformLocation(shaderProgram, "iResolution");

    // Initialize video encoder
    AVFormatContext *output_format_context = NULL;
    AVCodecContext *encoder_context = init_video_encoder(
        codecContext->width, codecContext->height, 30, "video2.mp4",
        &output_format_context);

    if (!encoder_context || !output_format_context)
    {
        printf("Failed to initialize video encoder\n");
        // Cleanup
        if (codecContext)
            avcodec_free_context(&codecContext);
        if (formatContext)
            avformat_close_input(&formatContext);
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Allocate frame buffer for capturing OpenGL output
    uint8_t *frame_buffer = (uint8_t *)malloc(
        codecContext->width * codecContext->height * 4);

    // Allocate output frame
    AVFrame *output_frame = av_frame_alloc();
    output_frame->format = encoder_context->pix_fmt;
    output_frame->width = encoder_context->width;
    output_frame->height = encoder_context->height;
    av_frame_get_buffer(output_frame, 0);

    // Create RGB to YUV converter
    struct SwsContext *sws_context = sws_getContext(
        codecContext->width, codecContext->height, AV_PIX_FMT_RGBA,
        codecContext->width, codecContext->height, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, NULL, NULL, NULL);

    printf("Starting main loop...\n");
    SDL_Event event;
    int running = 1;
    Uint32 startTime = SDL_GetTicks();
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    struct SwsContext *swsContext = NULL;
    int frame_index = 0;

    // Main loop
    while (running && av_read_frame(formatContext, packet) >= 0)
    {
        if (packet->stream_index == videoStream)
        {
            if (avcodec_send_packet(codecContext, packet) >= 0)
            {
                while (avcodec_receive_frame(codecContext, frame) >= 0)
                {
                    // Initialize swsContext if not done yet
                    if (!swsContext)
                    {
                        swsContext = sws_getContext(
                            frame->width, frame->height, codecContext->pix_fmt,
                            frame->width, frame->height, AV_PIX_FMT_RGB24,
                            SWS_BILINEAR, NULL, NULL, NULL);
                        printf("Video dimensions: %dx%d\n", frame->width, frame->height);
                    }

                    // Convert frame to RGB
                    uint8_t *rgb_data[4];
                    int rgb_linesize[4];
                    av_image_alloc(rgb_data, rgb_linesize,
                                   frame->width, frame->height,
                                   AV_PIX_FMT_RGB24, 1);

                    sws_scale(swsContext,
                              (const uint8_t *const *)frame->data, frame->linesize,
                              0, frame->height,
                              rgb_data, rgb_linesize);

                    // Update texture
                    glBindTexture(GL_TEXTURE_2D, texture);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                                 frame->width, frame->height, 0,
                                 GL_RGB, GL_UNSIGNED_BYTE, rgb_data[0]);

                    // Handle events
                    while (SDL_PollEvent(&event))
                    {
                        if (event.type == SDL_QUIT ||
                            (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
                        {
                            running = 0;
                            break;
                        }
                    }

                    // Clear screen
                    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
                    glClear(GL_COLOR_BUFFER_BIT);

                    // Use shader and update uniforms
                    glUseProgram(shaderProgram);
                    float currentTime = (SDL_GetTicks() - startTime) / 1000.0f;
                    glUniform1f(timeLocation, currentTime);
                    glUniform2f(resolutionLocation, (float)frame->width, (float)frame->height);

                    // Draw fullscreen quad
                    glBindVertexArray(VAO);
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                    // Read pixels from OpenGL framebuffer
                    glReadBuffer(GL_BACK);
                    glReadPixels(0, 0, frame->width, frame->height,
                                 GL_RGBA, GL_UNSIGNED_BYTE, frame_buffer);

                    // Convert RGBA to YUV420P for video encoding
                    const uint8_t *rgba_data[4] = {frame_buffer, NULL, NULL, NULL};
                    int rgba_linesize[4] = {frame->width * 4, 0, 0, 0};
                    sws_scale(sws_context,
                              rgba_data, rgba_linesize,
                              0, frame->height,
                              output_frame->data, output_frame->linesize);

                    // Encode frame
                    output_frame->pts = frame_index++;
                    int ret = avcodec_send_frame(encoder_context, output_frame);
                    if (ret < 0)
                    {
                        printf("Error sending frame for encoding: %d\n", ret);
                        break;
                    }

                    // Get encoded packets and write them
                    AVPacket *out_packet = av_packet_alloc();
                    while (ret >= 0)
                    {
                        ret = avcodec_receive_packet(encoder_context, out_packet);
                        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                        {
                            break;
                        }
                        else if (ret < 0)
                        {
                            printf("Error receiving encoded packet: %d\n", ret);
                            break;
                        }

                        // Write packet to file
                        ret = av_interleaved_write_frame(output_format_context, out_packet);
                        if (ret < 0)
                        {
                            printf("Error writing packet: %d\n", ret);
                            break;
                        }
                    }
                    av_packet_free(&out_packet);

                    // Display frame
                    SDL_GL_SwapWindow(window);

                    // Free RGB buffer
                    av_freep(&rgb_data[0]);

                    // Add small delay to control playback speed
                    SDL_Delay(1000 / 30); // Cap at 30 FPS
                }
            }
        }
        av_packet_unref(packet);
    }

    printf("Finalizing video encoding...\n");

    // Flush encoder
    avcodec_send_frame(encoder_context, NULL);
    AVPacket *out_packet = av_packet_alloc();
    int ret;
    while (1)
    {
        ret = avcodec_receive_packet(encoder_context, out_packet);
        if (ret == AVERROR_EOF)
        {
            break;
        }
        else if (ret < 0)
        {
            printf("Error flushing encoder: %d\n", ret);
            break;
        }
        av_interleaved_write_frame(output_format_context, out_packet);
        av_packet_unref(out_packet);
    }
    av_packet_free(&out_packet);

    // Write trailer and close output file
    av_write_trailer(output_format_context);
    avio_closep(&output_format_context->pb);

    printf("Video encoding completed.\n");

    // Cleanup
    free(frame_buffer);
    av_frame_free(&output_frame);
    avcodec_free_context(&encoder_context);
    avformat_free_context(output_format_context);
    sws_freeContext(sws_context);

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glDeleteTextures(1, &texture);

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    printf("Video saved as video2.mp4\n");
    return 0;
}