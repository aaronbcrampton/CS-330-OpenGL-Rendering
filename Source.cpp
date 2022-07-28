//CS330 project backup
#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//math.h for rendering circles
#include <math.h>
#include <corecrt_math_defines.h>
#include "camera.h"
#include "stb_image.h"

using namespace std; // Standard namespace


/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "CS330Project"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    //camera
    Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
    bool firstMouse = true;
    float lastX = WINDOW_WIDTH / 2.0;
    float lastY = WINDOW_HEIGHT / 2.0;

    //timing
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    bool pview = true;

    //Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbo;     // Handles for the vertex buffer objects
        GLuint nVertices;    // Number of indices of the mesh
    };

    struct GLMeshEars
    {
        GLuint vao;
        GLuint vbo;
        GLuint nVertices;
    };

    //Main GLFW window
    GLFWwindow* gWindow = nullptr;
    //Triangle mesh data
    GLMesh gMesh;
    GLMesh gMeshEars;
    //Shader program
    GLuint gProgramId;
    GLuint gProgramId2;
    //Textures
    GLuint gTextureId;
    GLuint glassTextureId;
    glm::vec2 gUVScale(1.0f, 1.0f);
    GLint gTexWrapMode = GL_REPEAT;
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void ProcessInput(GLFWwindow* window);
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
void UCreateMesh(GLMeshEars& meshEars);
void UDestroyMesh(GLMeshEars& meshEars);
void URender();
void URenderEars();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);
void drawCircle(GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides);
void drawPlacemat();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);


/* Default Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
layout(location = 2) in vec2 textureCoordinate;  // Color data from Vertex Attrib Pointer 1

out vec2 vertexTextureCoordinate; // variable to transfer color data to the fragment shader
//Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
    vertexTextureCoordinate = textureCoordinate; // references incoming color data
}
);

///* Glass Vertex Shader Source Code*/
const GLchar* vertexShaderSourceGlass = GLSL(440,
    layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
layout(location = 2) in vec2 textureCoordinate;  // Color data from Vertex Attrib Pointer 1

out vec2 vertexTextureCoordinate; // variable to transfer color data to the fragment shader

//Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
    vertexTextureCoordinate = textureCoordinate; // references incoming color data
}
);


/* Fragment Shader Source Code - default*/
const GLchar* fragmentShaderSource = GLSL(440,
    in vec2 vertexTextureCoordinate; // Variable to hold incoming color data from vertex shader

out vec4 fragmentColor;

uniform sampler2D uTexture;
uniform vec2 uvScale;

void main()
{
    fragmentColor = texture(uTexture, vertexTextureCoordinate * uvScale);
}
);

/* Fragment Shader Source Code - glass*/
const GLchar* fragmentShaderSourceGlass = GLSL(440,
    in vec2 vertexTextureCoordinate; // Variable to hold incoming color data from vertex shader

out vec4 fragmentColor;

uniform sampler2D glassTexture;
uniform vec2 uvScale;

void main()
{
    fragmentColor = texture(glassTexture, vertexTextureCoordinate * uvScale);
}
);


// Creates a perspective projection
//glm::mat4 projection = glm::perspective(45.0f, (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);


int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    //Create the mesh
    UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object

    //turn line mode on and off
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    //Create the shader programs
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;
    cout << "line 189";
    //Create MeshEars
    UCreateMesh(gMeshEars); // Calls the function to create the Vertex Buffer Object
    cout << "line 192";
    if (!UCreateShaderProgram(vertexShaderSourceGlass, fragmentShaderSourceGlass, gProgramId))
        return EXIT_FAILURE;
        cout << "line 195";
    //Flip image vertically to correct positioning if upside down
    stbi_set_flip_vertically_on_load(true);  //this remains set globally
    cout << "line 198";
    //Load textures
    //Default texture
    const char* texFilename = "C:\\Textures\\Blue_iridescent_background.jpg";
    if (!UCreateTexture(texFilename, gTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    //Define texture sampler location
    glUseProgram(gProgramId);
    cout << "line 208";
    //Default texture set to location 0
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

    //Glass texture
    const char* glassTexFilename = "C:\\Textures\\Blue_iridescent_background.jpg";
    if (!UCreateTexture(glassTexFilename, glassTextureId))
    {
        cout << "Failed to load texture " << glassTexFilename << endl;
        return EXIT_FAILURE;
    }
    cout << "line 219";
    //define texture sampler location
    glUseProgram(gProgramId2);
    //Glass texture set to location 1
    glUniform1i(glGetUniformLocation(gProgramId2, "glassTexture"), 1);
    //Enable z-depth glogally
    glEnable(GL_DEPTH_TEST);

    //Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    cout << "line 229";
    //render loop
    while (!glfwWindowShouldClose(gWindow))
    {
        //per frame timing
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        ProcessInput(gWindow);

        // Render this frame
        URender();
 //       URenderEars();

        glfwPollEvents();
    }


    //Release mesh data
    UDestroyMesh(gMesh);
 //   UDestroyMesh(gMeshEars);

    //Release texture data
    UDestroyTexture(gTextureId);
//    UDestroyTexture(glassTextureId);

    //Release shader program
    UDestroyShaderProgram(gProgramId);
    cout << "line 259";
    exit(EXIT_SUCCESS); // Terminates the program successfully
}


//Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    //GLFW: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, mouse_callback);
    glfwSetScrollCallback(*window, scroll_callback);

     //sets GLFW to capture mouse positions
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //GLEW: initialize
    //Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    //Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


//glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    //glOrtho(-4.0f, 4.0f, -4.0f, 4.0f, -0.1f, 1000.0f);
}

//Functions called to render a frame

void URender()
{
    //Enable z-depth
    glEnable(GL_DEPTH_TEST);

    //Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //creates camera view
    glm::mat4 view = camera.GetViewMatrix();

    //1. Scales the object by 2
    glm::mat4 scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
    //2. Rotates shape by 15 degrees in the x axis
    glm::mat4 rotation = glm::rotate(glm::radians(165.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    //3. Place object at the origin
    glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
    //Model matrix: transformations are applied right-to-left order
    glm::mat4 model = translation * rotation * scale;

    //Set the shader to be used
    glUseProgram(gProgramId);

    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    // Draws the triangles

    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices); // Draws the triangle

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}

//void URenderEars()
//{
//
//    //Clear the frame and z buffers
//    //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//    //creates camera view
//    glm::mat4 view = camera.GetViewMatrix();
//
//    //1. Scales the object by 2
//    glm::mat4 scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
//    //2. Rotates shape by 15 degrees in the x axis
//    glm::mat4 rotation = glm::rotate(glm::radians(165.0f), glm::vec3(0.0f, 1.0f, 0.0f));
//    //3. Place object at the origin
//    glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
//    //Model matrix: transformations are applied right-to-left order
//    glm::mat4 model = translation * rotation * scale;
//
//    //Set the shader to be used
//    glUseProgram(gProgramId);
//
//    // Retrieves and passes transform matrices to the Shader program
//    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
//    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
//    GLint projLoc = glGetUniformLocation(gProgramId, "projection");
//
//    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
//    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
//    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
//
//    GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
//    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));
//
//    // Activate the VBOs contained within the mesh's VAO
//    glBindVertexArray(gMeshEars.vao);
//
//    // bind textures on corresponding texture units
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, glassTextureId);
//
//    // Draws the triangles
//    glDrawArrays(GL_TRIANGLES, 0, gMeshEars.nVertices);
//
//    // Deactivate the Vertex Array Object
//    glBindVertexArray(0);
//
//    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
//    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
//}

//Array for drawPlacemat
GLfloat verticesPlacemat[] = { 1, 1, 1,  -1, 1, 1,  -1,-1, 1,      // v0-v1-v2 (front)
                       -1,-1, 1,   1,-1, 1,   1, 1, 1,      // v2-v3-v0

                        1, 1, 1,   1,-1, 1,   1,-1,-1,      // v0-v3-v4 (right)
                        1,-1,-1,   1, 1,-1,   1, 1, 1,      // v4-v5-v0

                        1, 1, 1,   1, 1,-1,  -1, 1,-1,      // v0-v5-v6 (top)
                       -1, 1,-1,  -1, 1, 1,   1, 1, 1,      // v6-v1-v0

                       -1, 1, 1,  -1, 1,-1,  -1,-1,-1,      // v1-v6-v7 (left)
                       -1,-1,-1,  -1,-1, 1,  -1, 1, 1,      // v7-v2-v1

                       -1,-1,-1,   1,-1,-1,   1,-1, 1,      // v7-v4-v3 (bottom)
                        1,-1, 1,  -1,-1, 1,  -1,-1,-1,      // v3-v2-v7

                        1,-1,-1,  -1,-1,-1,  -1, 1,-1,      // v4-v7-v6 (back)
                       -1, 1,-1,   1, 1,-1,   1,-1,-1 };    // v6-v5-v4

// normal array
GLfloat normalsPlacemat[] = { 0, 0, 1,   0, 0, 1,   0, 0, 1,      // v0-v1-v2 (front)
                        0, 0, 1,   0, 0, 1,   0, 0, 1,      // v2-v3-v0

                        1, 0, 0,   1, 0, 0,   1, 0, 0,      // v0-v3-v4 (right)
                        1, 0, 0,   1, 0, 0,   1, 0, 0,      // v4-v5-v0

                        0, 1, 0,   0, 1, 0,   0, 1, 0,      // v0-v5-v6 (top)
                        0, 1, 0,   0, 1, 0,   0, 1, 0,      // v6-v1-v0

                       -1, 0, 0,  -1, 0, 0,  -1, 0, 0,      // v1-v6-v7 (left)
                       -1, 0, 0,  -1, 0, 0,  -1, 0, 0,      // v7-v2-v1

                        0,-1, 0,   0,-1, 0,   0,-1, 0,      // v7-v4-v3 (bottom)
                        0,-1, 0,   0,-1, 0,   0,-1, 0,      // v3-v2-v7

                        0, 0,-1,   0, 0,-1,   0, 0,-1,      // v4-v7-v6 (back)
                        0, 0,-1,   0, 0,-1,   0, 0,-1 };    // v6-v5-v4

// color array
GLfloat colorsPlacemat[] = { 1, 1, 1,   1, 1, 0,   1, 0, 0,      // v0-v1-v2 (front)
                        1, 0, 0,   1, 0, 1,   1, 1, 1,      // v2-v3-v0

                        1, 1, 1,   1, 0, 1,   0, 0, 1,      // v0-v3-v4 (right)
                        0, 0, 1,   0, 1, 1,   1, 1, 1,      // v4-v5-v0

                        1, 1, 1,   0, 1, 1,   0, 1, 0,      // v0-v5-v6 (top)
                        0, 1, 0,   1, 1, 0,   1, 1, 1,      // v6-v1-v0

                        1, 1, 0,   0, 1, 0,   0, 0, 0,      // v1-v6-v7 (left)
                        0, 0, 0,   1, 0, 0,   1, 1, 0,      // v7-v2-v1

                        0, 0, 0,   0, 0, 1,   1, 0, 1,      // v7-v4-v3 (bottom)
                        1, 0, 1,   1, 0, 0,   0, 0, 0,      // v3-v2-v7

                        0, 0, 1,   0, 0, 0,   0, 1, 0,      // v4-v7-v6 (back)
                        0, 1, 0,   0, 1, 1,   0, 0, 1 };    // v6-v5-v4



// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {
        // Vertex Positions         //Texture Coords
        //Left Ear
      //  -0.2f,   -0.1f, -0.025f,   0.0f, 1.0f, // Top Vertex 0
      // -0.175f,  -0.3f, -0.025f,   0.0f, 0.0f, // Bottom Right Vertex 1
      //  -0.1f, -0.225f, -0.025f,   1.0f, 0.0f, // Bottom Left Vertex 2
      //  -0.2f,   -0.1f, -0.025f,   0.0f, 1.0f, // Top Vertex 0
      // -0.15f,  -0.20f,  0.025f,   0.0f, 0.0f, // Bottom Back Vertex 3
      //  -0.1f, -0.225f, -0.025f,   1.0f, 0.0f, // Bottom Left Vertex 2
      //  -0.2f,   -0.1f, -0.025f,   0.0f, 1.0f, // Top Vertex 0
      // -0.15f,  -0.20f,  0.025f,   1.0f, 0.0f, // Bottom Back Vertex 3
      //-0.175f,   -0.3f, -0.025f,   0.0f, 0.0f, // Bottom Right Vertex 1
      //-0.175f,   -0.3f, -0.025f,   0.0f, 0.0f, // Bottom Right Vertex 1
      //  -0.1f, -0.225f, -0.025f,   1.0f, 0.0f, // Bottom Left Vertex 2
      // -0.15f,  -0.20f,  0.025f,   0.0f, 1.0f, // Bottom Back Vertex 3

      //  //Right Ear
      //   0.2f,   -0.1f, -0.025f,   0.0f, 1.0f, // Top Vertex 0
      // 0.175f,   -0.3f, -0.025f,   0.0f, 0.0f, // Bottom Left Vertex 1
      //   0.1f, -0.225f, -0.025f,   1.0f, 0.0f,  // Bottom Right Vertex 2
      //   0.2f,   -0.1f, -0.025f,   0.0f, 1.0f, // Top Vertex 0
      //  0.15f,  -0.20f,  0.025f,   0.0f, 0.0f,  // Bottom Back Vertex 3
      //   0.1f, -0.225f, -0.025f,   1.0f, 0.0f,  // Bottom Right Vertex 2
      //   0.2f,   -0.1f, -0.025f,   0.0f, 1.0f, // Top Vertex 0
      //  0.15f,  -0.20f,  0.025f,   1.0f, 0.0f,  // Bottom Back Vertex 3
      // 0.175f,   -0.3f, -0.025f,   0.0f, 0.0f, // Bottom Left Vertex 1
      // 0.175f,   -0.3f, -0.025f,   0.0f, 0.0f, // Bottom Left Vertex 1
      //   0.1f, -0.225f, -0.025f,   1.0f, 0.0f,  // Bottom Right Vertex 2
      //  0.15f,  -0.20f,  0.025f,   0.0f, 1.0f,  // Bottom Back Vertex 3

   // GLfloat verts1[] = {
            //Cube vase
            0.5f, -0.5f, 0.1f,  1.0f, 0.0f,  //8       //front
            0.1f, -0.5f, 0.1f,  0.0f, 0.0f,  //9       
            0.1f,  0.5f, 0.1f,  0.0f, 1.0f,  //10           
            0.5f, -0.5f, 0.1f,  1.0f, 0.0f,  //8       //8,9,10,8,11,10
            0.5f,  0.5f, 0.1f,  1.0f, 1.0f,  //11
            0.1f,  0.5f, 0.1f,  0.0f, 1.0f,  //10


            0.1f, -0.5f,  0.5f,  0.0f, 0.0f, //12      //back
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f, //13      
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f, //14 
            0.1f, -0.5f,  0.5f,  0.0f, 0.0f, //12
            0.1f,  0.5f,  0.5f,  0.0f, 1.0f, //15       //12,13,14,12,15,14
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f, //14


            0.1f,  0.5f, 0.5f,  1.0f, 1.0f, //16     //left
            0.1f,  0.5f, 0.1f,  1.0f, 0.0f,  //17     
            0.1f, -0.5f, 0.1f,  0.0f, 0.0f,  //18
            0.1f,  0.5f, 0.5f,  1.0f, 1.0f, //16
            0.1f, -0.5f, 0.5f,  0.0f, 1.0f, //19    //16,17,18,16,19,18
            0.1f, -0.5f, 0.1f,  0.0f, 0.0f,  //18


            0.5f,  0.5f, 0.5f,  1.0f, 1.0f,  //20     //right
            0.5f,  0.5f, 0.1f,  1.0f, 0.0f,  //21     
            0.5f, -0.5f, 0.1f,  0.0f, 0.0f,  //22     
            0.5f,  0.5f, 0.5f,  1.0f, 1.0f,  //20
            0.5f, -0.5f, 0.5f,  0.0f, 1.0f,  //23     //20,21,22,20,23,22
            0.5f, -0.5f, 0.1f,  0.0f, 0.0f,  //22


            0.1f, -0.5f, 0.1f,  0.0f, 0.0f, //24     //bottom
            0.5f, -0.5f, 0.1f,  1.0f, 0.0f, //25     
            0.5f, -0.5f, 0.5f,  1.0f, 1.0f, //26  
            0.1f, -0.5f, 0.1f,  0.0f, 0.0f, //24 
            0.1f, -0.5f, 0.5f,  0.0f, 1.0f,  //27     //24,25,26,24,27,26
            0.5f, -0.5f, 0.5f,  1.0f, 1.0f, //26
            };

    //GLfloat verts2[] = {
    //    //Plane
    //     0.8f,   -0.5f, -0.5f,  1.0f, 0.0f, //28       //top
    //    -0.8f,   -0.5f, -0.5f,  0.0f, 0.0f, //29       
    //    -0.8f,   -0.5f,  0.8f,  0.0f, 1.0f, //30
    //     0.8f,   -0.5f, -0.5f,  1.0f, 0.0f, //28 
    //     0.8f,   -0.5f,  0.8f,  1.0f, 1.0f, //31      //28,29,30,28,31,30
    //    -0.8f,   -0.5f,  0.8f,  0.0f, 1.0f, //30

    //     0.8f,   -0.5f, -0.5f,  1.0f, 1.0f, //32       //front
    //    -0.8f,   -0.5f, -0.5f,  0.0f, 1.0f, //33       
    //    -0.8f, -0.505f, -0.5f,  0.0f, 0.0f, //34
    //     0.8f,   -0.5f, -0.5f,  1.0f, 1.0f, //32
    //     0.8f, -0.505f, -0.5f,  1.0f, 0.0f, //35      //32,33,34,32,35,34
    //    -0.8f, -0.505f, -0.5f,  0.0f, 0.0f, //34

    //    -0.8f,   -0.5f,  0.6f,  0.0f, 1.0f, //36      //back
    //     0.8f,   -0.5f,  0.6f,  1.0f, 1.0f, //37      
    //     0.8f, -0.505f,  0.6f,  1.0f, 0.0f, //38  
    //    -0.8f,   -0.5f,  0.6f,  0.0f, 1.0f, //36
    //    -0.8f, -0.505f,  0.6f,  0.0f, 0.0f, //39       //36,37,38,36,39,38
    //     0.8f, -0.505f,  0.6f,  1.0f, 0.0f, //38

    //    -0.8f,   -0.5f, -0.5f,  1.0f, 0.0f, //40     //left
    //    -0.8f, -0.505f,  0.6f,  0.0f, 1.0f, //41     
    //    -0.8f, -0.505f, -0.5f,  0.0f, 0.0f, //42
    //    -0.8f,   -0.5f, -0.5f,  1.0f, 0.0f, //40
    //    -0.8f,   -0.5f,  0.6f,  1.0f, 1.0f, //43    //40,41,42,40,43,41
    //    -0.8f, -0.505f,  0.6f,  0.0f, 1.0f, //41

    //     0.8f,   -0.5f, -0.5f,  1.0f, 0.0f, //44     //right
    //     0.8f,   -0.5f,  0.6f,  1.0f, 1.0f, //45     
    //     0.8f, -0.505f,  0.6f,  0.0f, 1.0f, //46
    //     0.8f,   -0.5f, -0.5f,  1.0f, 0.0f, //44
    //     0.8f, -0.505f, -0.5f,  0.0f, 0.0f, //47     //44,45,46,44,47,46
    //     0.8f, -0.505f,  0.6f,  0.0f, 1.0f, //46

    //    -0.8f, -0.505f,  0.8f,  0.0f, 1.0f, //48     //bottom
    //     0.8f, -0.505f,  0.8f,  1.0f, 1.0f, //49
    //     0.8f, -0.505f, -0.5f,  1.0f, 0.0f, //50
    //    -0.8f, -0.505f,  0.8f,  0.0f, 1.0f, //48
    //    -0.8f, -0.505f, -0.5f,  0.0f, 0.0f,  //51     //48,49,50,48,51,50
    //     0.8f, -0.505f, -0.5f,  1.0f, 0.0f, //50

    //};

    //// Index data to share position data
    //GLushort indices1[] = {
    //    0, 1, 2,  // Triangle 1
    //    0, 3, 2,   // Triangle 2
    //    0, 3, 1,  // Triangle 3
    //    1, 2, 3,  // Triangle 4

    //    4, 5, 6,  // Triangle 1
    //    4, 7, 6,   // Triangle 2
    //    4, 7, 5,  // Triangle 3
    //    5, 6, 7,  // Triangle 4   

    //    //Cube Vase 8-11 and 12-15
    //    8,9,10,8,11,10,     //front red
    //    12,13,14,12,15,14,  //back green
    //    16,17,18,16,19,18,  //left blue
    //    20,21,22,20,23,22,   //right purple
    //    24,25,26,24,27,26,    //bottom gold

    //    //Plane
    //    28,29,30,28,31,30,  //top
    //    32,33,34,32,35,34,  //front
    //    36,37,38,36,39,38,  //back
    //    40,41,42,40,43,41,  //left
    //    44,45,46,44,47,46,  //right
    //    48,49,50,48,51,50   //bottom







    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerUV));
    //mesh.nVertices1 = sizeof(verts1) / (sizeof(verts1[0]) * (floatsPerVertex + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create buffers for vertices
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(2);
}

// Implements the UCreateMesh function
//void UCreateMesh(GLMeshEars& meshEars)
//{
//    // Position and Color data
//    GLfloat verts[] = {
//        // Vertex Positions         //Texture Coords
//        //Left Ear
//        -0.2f,   -0.1f, -0.025f,   0.0f, 1.0f, // Top Vertex 0
//       -0.175f,  -0.3f, -0.025f,   0.0f, 0.0f, // Bottom Right Vertex 1
//        -0.1f, -0.225f, -0.025f,   1.0f, 0.0f, // Bottom Left Vertex 2
//        -0.2f,   -0.1f, -0.025f,   0.0f, 1.0f, // Top Vertex 0
//       -0.15f,  -0.20f,  0.025f,   0.0f, 0.0f, // Bottom Back Vertex 3
//        -0.1f, -0.225f, -0.025f,   1.0f, 0.0f, // Bottom Left Vertex 2
//        -0.2f,   -0.1f, -0.025f,   0.0f, 1.0f, // Top Vertex 0
//       -0.15f,  -0.20f,  0.025f,   1.0f, 0.0f, // Bottom Back Vertex 3
//      -0.175f,   -0.3f, -0.025f,   0.0f, 0.0f, // Bottom Right Vertex 1
//      -0.175f,   -0.3f, -0.025f,   0.0f, 0.0f, // Bottom Right Vertex 1
//        -0.1f, -0.225f, -0.025f,   1.0f, 0.0f, // Bottom Left Vertex 2
//       -0.15f,  -0.20f,  0.025f,   0.0f, 1.0f, // Bottom Back Vertex 3
//
//        //Right Ear
//         0.2f,   -0.1f, -0.025f,   0.0f, 1.0f, // Top Vertex 0
//       0.175f,   -0.3f, -0.025f,   0.0f, 0.0f, // Bottom Left Vertex 1
//         0.1f, -0.225f, -0.025f,   1.0f, 0.0f,  // Bottom Right Vertex 2
//         0.2f,   -0.1f, -0.025f,   0.0f, 1.0f, // Top Vertex 0
//        0.15f,  -0.20f,  0.025f,   0.0f, 0.0f,  // Bottom Back Vertex 3
//         0.1f, -0.225f, -0.025f,   1.0f, 0.0f,  // Bottom Right Vertex 2
//         0.2f,   -0.1f, -0.025f,   0.0f, 1.0f, // Top Vertex 0
//        0.15f,  -0.20f,  0.025f,   1.0f, 0.0f,  // Bottom Back Vertex 3
//       0.175f,   -0.3f, -0.025f,   0.0f, 0.0f, // Bottom Left Vertex 1
//       0.175f,   -0.3f, -0.025f,   0.0f, 0.0f, // Bottom Left Vertex 1
//         0.1f, -0.225f, -0.025f,   1.0f, 0.0f,  // Bottom Right Vertex 2
//        0.15f,  -0.20f,  0.025f,   0.0f, 1.0f  // Bottom Back Vertex 3
//    };
//    //GLfloat verts1[] = {
//    //        //Cube vase
//    //        0.5f, -0.5f, 0.1f,  1.0f, 0.0f,  //8       //front
//    //        0.1f, -0.5f, 0.1f,  0.0f, 0.0f,  //9       
//    //        0.1f,  0.5f, 0.1f,  0.0f, 1.0f,  //10           
//    //        0.5f, -0.5f, 0.1f,  1.0f, 0.0f,  //8       //8,9,10,8,11,10
//    //        0.5f,  0.5f, 0.1f,  1.0f, 1.0f,  //11
//    //        0.1f,  0.5f, 0.1f,  0.0f, 1.0f,  //10
//
//
//    //        0.1f, -0.5f,  0.5f,  0.0f, 0.0f, //12      //back
//    //        0.5f, -0.5f,  0.5f,  1.0f, 0.0f, //13      
//    //        0.5f,  0.5f,  0.5f,  1.0f, 1.0f, //14 
//    //        0.1f, -0.5f,  0.5f,  0.0f, 0.0f, //12
//    //        0.1f,  0.5f,  0.5f,  0.0f, 1.0f, //15       //12,13,14,12,15,14
//    //        0.5f,  0.5f,  0.5f,  1.0f, 1.0f, //14
//
//
//    //        0.1f,  0.5f, 0.5f,  1.0f, 1.0f, //16     //left
//    //        0.1f,  0.5f, 0.1f,  1.0f, 0.0f,  //17     
//    //        0.1f, -0.5f, 0.1f,  0.0f, 0.0f,  //18
//    //        0.1f,  0.5f, 0.5f,  1.0f, 1.0f, //16
//    //        0.1f, -0.5f, 0.5f,  0.0f, 1.0f, //19    //16,17,18,16,19,18
//    //        0.1f, -0.5f, 0.1f,  0.0f, 0.0f,  //18
//
//
//    //        0.5f,  0.5f, 0.5f,  1.0f, 1.0f,  //20     //right
//    //        0.5f,  0.5f, 0.1f,  1.0f, 0.0f,  //21     
//    //        0.5f, -0.5f, 0.1f,  0.0f, 0.0f,  //22     
//    //        0.5f,  0.5f, 0.5f,  1.0f, 1.0f,  //20
//    //        0.5f, -0.5f, 0.5f,  0.0f, 1.0f,  //23     //20,21,22,20,23,22
//    //        0.5f, -0.5f, 0.1f,  0.0f, 0.0f,  //22
//
//
//    //        0.1f, -0.5f, 0.1f,  0.0f, 0.0f, //24     //bottom
//    //        0.5f, -0.5f, 0.1f,  1.0f, 0.0f, //25     
//    //        0.5f, -0.5f, 0.5f,  1.0f, 1.0f, //26  
//    //        0.1f, -0.5f, 0.1f,  0.0f, 0.0f, //24 
//    //        0.1f, -0.5f, 0.5f,  0.0f, 1.0f,  //27     //24,25,26,24,27,26
//    //        0.5f, -0.5f, 0.5f,  1.0f, 1.0f, //26
//    //        };
//
//    //GLfloat verts2[] = {
//    //    //Plane
//    //     0.8f,   -0.5f, -0.5f,  1.0f, 0.0f, //28       //top
//    //    -0.8f,   -0.5f, -0.5f,  0.0f, 0.0f, //29       
//    //    -0.8f,   -0.5f,  0.8f,  0.0f, 1.0f, //30
//    //     0.8f,   -0.5f, -0.5f,  1.0f, 0.0f, //28 
//    //     0.8f,   -0.5f,  0.8f,  1.0f, 1.0f, //31      //28,29,30,28,31,30
//    //    -0.8f,   -0.5f,  0.8f,  0.0f, 1.0f, //30
//
//    //     0.8f,   -0.5f, -0.5f,  1.0f, 1.0f, //32       //front
//    //    -0.8f,   -0.5f, -0.5f,  0.0f, 1.0f, //33       
//    //    -0.8f, -0.505f, -0.5f,  0.0f, 0.0f, //34
//    //     0.8f,   -0.5f, -0.5f,  1.0f, 1.0f, //32
//    //     0.8f, -0.505f, -0.5f,  1.0f, 0.0f, //35      //32,33,34,32,35,34
//    //    -0.8f, -0.505f, -0.5f,  0.0f, 0.0f, //34
//
//    //    -0.8f,   -0.5f,  0.6f,  0.0f, 1.0f, //36      //back
//    //     0.8f,   -0.5f,  0.6f,  1.0f, 1.0f, //37      
//    //     0.8f, -0.505f,  0.6f,  1.0f, 0.0f, //38  
//    //    -0.8f,   -0.5f,  0.6f,  0.0f, 1.0f, //36
//    //    -0.8f, -0.505f,  0.6f,  0.0f, 0.0f, //39       //36,37,38,36,39,38
//    //     0.8f, -0.505f,  0.6f,  1.0f, 0.0f, //38
//
//    //    -0.8f,   -0.5f, -0.5f,  1.0f, 0.0f, //40     //left
//    //    -0.8f, -0.505f,  0.6f,  0.0f, 1.0f, //41     
//    //    -0.8f, -0.505f, -0.5f,  0.0f, 0.0f, //42
//    //    -0.8f,   -0.5f, -0.5f,  1.0f, 0.0f, //40
//    //    -0.8f,   -0.5f,  0.6f,  1.0f, 1.0f, //43    //40,41,42,40,43,41
//    //    -0.8f, -0.505f,  0.6f,  0.0f, 1.0f, //41
//
//    //     0.8f,   -0.5f, -0.5f,  1.0f, 0.0f, //44     //right
//    //     0.8f,   -0.5f,  0.6f,  1.0f, 1.0f, //45     
//    //     0.8f, -0.505f,  0.6f,  0.0f, 1.0f, //46
//    //     0.8f,   -0.5f, -0.5f,  1.0f, 0.0f, //44
//    //     0.8f, -0.505f, -0.5f,  0.0f, 0.0f, //47     //44,45,46,44,47,46
//    //     0.8f, -0.505f,  0.6f,  0.0f, 1.0f, //46
//
//    //    -0.8f, -0.505f,  0.8f,  0.0f, 1.0f, //48     //bottom
//    //     0.8f, -0.505f,  0.8f,  1.0f, 1.0f, //49
//    //     0.8f, -0.505f, -0.5f,  1.0f, 0.0f, //50
//    //    -0.8f, -0.505f,  0.8f,  0.0f, 1.0f, //48
//    //    -0.8f, -0.505f, -0.5f,  0.0f, 0.0f,  //51     //48,49,50,48,51,50
//    //     0.8f, -0.505f, -0.5f,  1.0f, 0.0f, //50
//
//    //};
//
//    //// Index data to share position data
//    //GLushort indices1[] = {
//    //    0, 1, 2,  // Triangle 1
//    //    0, 3, 2,   // Triangle 2
//    //    0, 3, 1,  // Triangle 3
//    //    1, 2, 3,  // Triangle 4
//
//    //    4, 5, 6,  // Triangle 1
//    //    4, 7, 6,   // Triangle 2
//    //    4, 7, 5,  // Triangle 3
//    //    5, 6, 7,  // Triangle 4   
//
//    //    //Cube Vase 8-11 and 12-15
//    //    8,9,10,8,11,10,     //front red
//    //    12,13,14,12,15,14,  //back green
//    //    16,17,18,16,19,18,  //left blue
//    //    20,21,22,20,23,22,   //right purple
//    //    24,25,26,24,27,26,    //bottom gold
//
//    //    //Plane
//    //    28,29,30,28,31,30,  //top
//    //    32,33,34,32,35,34,  //front
//    //    36,37,38,36,39,38,  //back
//    //    40,41,42,40,43,41,  //left
//    //    44,45,46,44,47,46,  //right
//    //    48,49,50,48,51,50   //bottom
//
//
//
//
//
//
//
//    const GLuint floatsPerVertex = 3;
//    const GLuint floatsPerUV = 2;
//
//    meshEars.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerUV));
//    cout << "line 821";
//    glGenVertexArrays(1, &meshEars.vao); // we can also generate multiple VAOs or buffers at the same time
//    glBindVertexArray(meshEars.vao);
//
//    // Create buffers for vertices
//    glGenBuffers(1, &meshEars.vbo);
//    glBindBuffer(GL_ARRAY_BUFFER, meshEars.vbo); // Activates the buffer
//    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU
//
//    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
//    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);// The number of floats before each
//
//    // Create Vertex Attribute Pointers
//    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
//    glEnableVertexAttribArray(0);
//
//    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
//    glEnableVertexAttribArray(2);
//    }

/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}

void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}

void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
}

//void UDestroyMesh(GLMeshEars& meshEars)
//{
//    glDeleteVertexArrays(1, &meshEars.vao);
//    glDeleteBuffers(1, &meshEars.vbo);
//}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}


void ProcessInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && gTexWrapMode != GL_REPEAT)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_REPEAT;

        cout << "Current Texture Wrapping Mode: REPEAT" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && gTexWrapMode != GL_MIRRORED_REPEAT)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_MIRRORED_REPEAT;

        cout << "Current Texture Wrapping Mode: MIRRORED REPEAT" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_EDGE)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_CLAMP_TO_EDGE;

        cout << "Current Texture Wrapping Mode: CLAMP TO EDGE" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_BORDER)
    {
        float color[] = { 1.0f, 0.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_CLAMP_TO_BORDER;

        cout << "Current Texture Wrapping Mode: CLAMP TO BORDER" << endl;
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
    {
        gUVScale += 0.1f;
        cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
    {
        gUVScale -= 0.1f;
        cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
    }

    //change view from perspective to orthographic
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        if (pview) {
            projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, -0.1f, 1000.0f);
            pview = false;
        }
        else {
            projection = glm::perspective(glm::radians(camera.Zoom), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
            pview = true;
        }
    }

    /*
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        mixValue += 0.001f;
        if (mixValue >= 1.0f)
            mixValue = 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        mixValue -= 0.001f;
        if (mixValue <= 0.0f)
            mixValue = 0.0f;
    }
    */

}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }


    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;  //reversed from X positions as y-coords range from bottom to top
    lastX = xpos;
    lastY = ypos;

    /*
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        camera.Pitch =
       // camera.Position = glm::vec3(0, 0, 0);
        //camera.Right = glm::vec3(0.0f, 0.0f, 0.0f);
        //camera.Up = glm::vec3(0.0f, 0.0f, 0.0f);
        camera.Pitch = 0.0;
        camera.Yaw = 0.0;
        camera.Zoom == -.05f;
    }
    */

    camera.ProcessMouseMovement(xoffset, yoffset);

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}


void drawCircle(GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides)
{
    int numberOfVertices = numberOfSides + 2;

    GLfloat twicePi = 2.0f * M_PI;

    GLfloat* circleVerticesX = new GLfloat[numberOfVertices];
    GLfloat* circleVerticesY = new GLfloat[numberOfVertices];
    GLfloat* circleVerticesZ = new GLfloat[numberOfVertices];

    circleVerticesX[0] = x;
    circleVerticesY[0] = y;
    circleVerticesZ[0] = z;

    for (int i = 1; i < numberOfVertices; i++)
    {
        circleVerticesX[i] = x + (radius * cos(i * twicePi / numberOfSides));
        circleVerticesY[i] = y + (radius * sin(i * twicePi / numberOfSides));
        circleVerticesZ[i] = z;
    }

    GLfloat* allCircleVertices = new GLfloat[(numberOfVertices) * 3];

    for (int i = 0; i < numberOfVertices; i++)
    {
        allCircleVertices[i * 3] = circleVerticesX[i];
        allCircleVertices[(i * 3) + 1] = circleVerticesY[i];
        allCircleVertices[(i * 3) + 2] = circleVerticesZ[i];
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, allCircleVertices);
    glDrawArrays(GL_TRIANGLE_FAN, 0, numberOfVertices);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void drawPlacemat()
{

    // enble and specify pointers to vertex arrays
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glNormalPointer(GL_FLOAT, 0, normalsPlacemat);
    glColorPointer(3, GL_FLOAT, 0, colorsPlacemat);
    glVertexPointer(3, GL_FLOAT, 0, verticesPlacemat);

    glPushMatrix();
    glTranslatef(2, 2, 0);                  // move to upper-right corner

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}


