#include <GL/glut.h>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include "2005110_classes.h"
#include "bitmap_image.hpp"

static int slices = 16;
static int stacks = 16;

// Use original global vectors as expected by ray tracing logic
extern vector<Object *> Objects;
extern vector<PointLight *> pointLights;
extern vector<SpotLight *> spotlights;
extern int level_recursion;

// External texture declarations
extern TextureData floorTexture;
extern bool loadTexture(const char* filename, TextureData& texture);
extern bool useTextureMode;
int pixels;

double mainCameraHeight;
double mainCameraAngle;
int drawGridFlag;
int drawAxesFlag;
double animationAngle;

int mainWindowWidth = 500;
int mainWindowHeight = 500;

int outputImageCount = 11;

Point cameraEye;
Point cameraUp, cameraRight, cameraLook;
double cameraMoveAngle;

double globalRx = 10;
double globalRy = 10;
double globalRz = 10;
double globalRadius = 6;

// Helper: Calculate rotated coordinate for a single axis
void rotateSingleAxisComponent(double& primary, double& secondary, double angleRad) {
    double temp = primary * cos(angleRad) + secondary * sin(angleRad);
    secondary = secondary * cos(angleRad) - primary * sin(angleRad);
    primary = temp;
}

// Refactored rotate_axis function
void rotate_axis(Point* primaryAxis, Point* secondaryAxis)
{
    double radianAngle = cameraMoveAngle * PI / 180;
    rotateSingleAxisComponent(primaryAxis->xCoord, secondaryAxis->xCoord, radianAngle);
    rotateSingleAxisComponent(primaryAxis->yCoord, secondaryAxis->yCoord, radianAngle);
    rotateSingleAxisComponent(primaryAxis->zCoord, secondaryAxis->zCoord, radianAngle);
}

// Helper: Set color for axes
void setAxesColor() {
    glColor3f(1.0, 1.0, 1.0);
}

// Helper: Draw a single axis line
void drawAxisLine(float x1, float y1, float z1, float x2, float y2, float z2) {
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
}

void drawAxes()
{
    if (drawAxesFlag == 1)
    {
        setAxesColor();
        glBegin(GL_LINES);
        {
            // X axis
            drawAxisLine(100, 0, 0, -100, 0, 0);
            // Y axis
            drawAxisLine(0, -100, 0, 0, 100, 0);
            // Z axis
            drawAxisLine(0, 0, 100, 0, 0, -100);
        }
        glEnd();
    }
}

// Helper: Set color for grid
void setGridColor() {
    glColor3f(0.6, 0.6, 0.6); // grey
}

// Helper: Draw a single grid line
void drawGridLine(float x1, float y1, float z1, float x2, float y2, float z2) {
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
}

void drawGrid()
{
    if (drawGridFlag == 1)
    {
        setGridColor();
        glBegin(GL_LINES);
        {
            for (int gridIndex = -8; gridIndex <= 8; gridIndex++)
            {
                if (gridIndex == 0)
                    continue; // SKIP the MAIN axes
                // lines parallel to Y-axis
                drawGridLine(gridIndex * 10, -90, 0, gridIndex * 10, 90, 0);
                // lines parallel to X-axis
                drawGridLine(-90, gridIndex * 10, 0, 90, gridIndex * 10, 0);
            }
        }
        glEnd();
    }
}

// Helper: Set OpenGL viewport
void setViewport(int windowWidth, int windowHeight) {
    glViewport(0, 0, windowWidth, windowHeight);
}

// Helper: Set OpenGL projection matrix
void setProjectionMatrix(float aspectRatio) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-aspectRatio, aspectRatio, -1.0, 1.0, 2.0, 100.0);
}

// Helper: Set OpenGL modelview matrix
void setModelViewMatrix() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

static void resize(int windowWidth, int windowHeight)
{
    float aspectRatio = (float)windowWidth / (float)windowHeight;
    setViewport(windowWidth, windowHeight);
    setProjectionMatrix(aspectRatio);
    setModelViewMatrix();
}

// Helper: Handle exit key
void handleExitKey() {
    exit(0);
}

// Helper: Handle incrementing slices and stacks
void handleIncrementSlicesStacks() {
    slices++;
    stacks++;
}

// Helper: Handle decrementing slices and stacks
void handleDecrementSlicesStacks() {
    if (slices > 3 && stacks > 3)
    {
        slices--;
        stacks--;
    }
}

static void key(unsigned char pressedKey, int mouseX, int mouseY)
{
    switch (pressedKey)
    {
    case 27:
    case 'q':
        handleExitKey();
        break;
    case '+':
        handleIncrementSlicesStacks();
        break;
    case '-':
        handleDecrementSlicesStacks();
        break;
    }
    glutPostRedisplay();
}

static void idle(void)
{
    glutPostRedisplay();
}

const GLfloat light_ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
const GLfloat light_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
const GLfloat light_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
const GLfloat light_position[] = {2.0f, 5.0f, 5.0f, 0.0f};

const GLfloat mat_ambient[] = {0.7f, 0.7f, 0.7f, 1.0f};
const GLfloat mat_diffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
const GLfloat mat_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
const GLfloat high_shininess[] = {100.0f};

// Helper: Read a triangle object from file
Object* readTriangleObject(std::ifstream& sceneFile) {
    Point vertices[3];
    for (int j = 0; j < 3; j++) {
        sceneFile >> vertices[j].xCoord >> vertices[j].yCoord >> vertices[j].zCoord;
    }
    Object* triangle = new Triangle(vertices[0], vertices[1], vertices[2]);
    double color[3];
    double coeff[4];
    int shine;
    sceneFile >> color[0] >> color[1] >> color[2];
    triangle->setColor(color);
    sceneFile >> coeff[0] >> coeff[1] >> coeff[2] >> coeff[3];
    triangle->setCoefficients(coeff);
    sceneFile >> shine;
    triangle->setShine(shine);
    return triangle;
}

// Helper: Read a sphere object from file
Object* readSphereObject(std::ifstream& sceneFile) {
    Point center;
    sceneFile >> center.xCoord >> center.yCoord >> center.zCoord;
    double radius;
    sceneFile >> radius;
    Object* sphere = new Sphere(center, radius);
    double color[3];
    double coeff[4];
    int shine;
    sceneFile >> color[0] >> color[1] >> color[2];
    sphere->setColor(color);
    sceneFile >> coeff[0] >> coeff[1] >> coeff[2] >> coeff[3];
    sphere->setCoefficients(coeff);
    sceneFile >> shine;
    sphere->setShine(shine);
    return sphere;
}

// Helper: Read a general object from file
Object* readGeneralObject(std::ifstream& sceneFile) {
    std::string line;
    std::getline(sceneFile, line);
    std::getline(sceneFile, line);
    std::istringstream ss(line);
    std::string token;
    std::vector<double> degree_coeff;
    while (std::getline(ss, token, ' ')) {
        degree_coeff.push_back(std::stod(token));
    }
    Object* general = new General(degree_coeff);
    Point ref_point;
    double length, width, height;
    double color[3];
    double coeff[4];
    int shine;
    sceneFile >> ref_point.xCoord >> ref_point.yCoord >> ref_point.zCoord >> length >> width >> height;
    sceneFile >> color[0] >> color[1] >> color[2];
    sceneFile >> coeff[0] >> coeff[1] >> coeff[2] >> coeff[3];
    sceneFile >> shine;
    general->objectReferencePoint = ref_point;
    general->objectHeight = height;
    general->objectWidth = width;
    general->objectLength = length;
    general->setColor(color);
    general->setCoefficients(coeff);
    general->setShine(shine);
    return general;
}

// Helper: Read point lights from file
void readPointLights(std::ifstream& sceneFile, int pointLightCount) {
    for (int j = 0; j < pointLightCount; j++) {
        Point pos;
        sceneFile >> pos.xCoord >> pos.yCoord >> pos.zCoord;
        double color[3];
        sceneFile >> color[0] >> color[1] >> color[2];
        PointLight* pl = new PointLight(pos);
        pl->setColor(color);
        pointLights.push_back(pl);
    }
}

// Helper: Read spot lights from file
void readSpotLights(std::ifstream& sceneFile, int spotLightCount) {
    for (int i = 0; i < spotLightCount; i++) {
        Point pos, dir;
        double color[3];
        double angle;
        sceneFile >> pos.xCoord >> pos.yCoord >> pos.zCoord;
        sceneFile >> color[0] >> color[1] >> color[2];
        sceneFile >> dir.xCoord >> dir.yCoord >> dir.zCoord;
        sceneFile >> angle;
        PointLight pl(pos);
        pl.setColor(color);
        SpotLight* sl = new SpotLight(pl, dir, angle);
        spotlights.push_back(sl);
    }
}

// Helper: Load floor texture
void loadFloorTexture() {
    if (!loadTexture("texture.jpg", floorTexture)) {
        std::cout << "Warning: Could not load floor texture, will use checkerboard when in texture mode" << std::endl;
    }
    std::cout << "Floor rendering modes available: Press 'T' to toggle between TEXTURE and CHECKERBOARD" << std::endl;
}

// Helper: Add floor object
void addFloorObject() {
    Object* floor_tile = new Floor(1000, 20); // 1000 - floorWidth, 20 - tileWidth
    double color[3] = {1, 1, 1};
    floor_tile->setColor(color);
    Objects.push_back(floor_tile);
}

void loadData()
{
    std::cout << "Starting to load data..." << std::endl;
    std::ifstream sceneFile;
    sceneFile.open("scene.txt");
    sceneFile >> level_recursion;
    sceneFile >> pixels;
    int objectCount;
    sceneFile >> objectCount;
    for (int i = 0; i < objectCount; i++) {
        std::string objectType;
        sceneFile >> objectType;
        std::cout << objectType << std::endl;
        if (objectType == "triangle") {
            Objects.push_back(readTriangleObject(sceneFile));
        } else if (objectType == "sphere") {
            Objects.push_back(readSphereObject(sceneFile));
        } else if (objectType == "general") {
            Objects.push_back(readGeneralObject(sceneFile));
        }
    }
    int pointLightCount = 0;
    sceneFile >> pointLightCount;
    std::cout << pointLightCount << std::endl;
    readPointLights(sceneFile, pointLightCount);
    int spotLightCount = 0;
    sceneFile >> spotLightCount;
    std::cout << spotLightCount << std::endl;
    readSpotLights(sceneFile, spotLightCount);
    sceneFile.close();
    loadFloorTexture();
    addFloorObject();
}

// Helper: Print a single object
void printSingleObject(Object* obj) {
    obj->print_object();
}

void printObjects()
{
    for (auto& obj : Objects)
    {
        printSingleObject(obj);
    }
}

// Helper: Print a single point light
void printSinglePointLight(PointLight* pl) {
    pl->PrintLight();
}

// Helper: Print a single spot light
void printSingleSpotLight(SpotLight* sl) {
    sl->PrintLight();
}

void printLights()
{
    for (auto& pl : pointLights)
    {
        printSinglePointLight(pl);
    }
    for (auto& sl : spotlights)
    {
        printSingleSpotLight(sl);
    }
}

// Helper: Initialize the image to black
void initializeImage(bitmap_image& image, int width, int height) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            image.set_pixel(i, j, 0, 0, 0);
        }
    }
}

// Helper: Calculate the top-left corner of the view plane
Point calculateTopLeftCorner(const Point& eye, const Point& l, const Point& r, const Point& u, double planeDistance, double windowWidth, double windowHeight) {
    Point topLeft;
    topLeft.xCoord = eye.xCoord + (l.xCoord * planeDistance) - (r.xCoord * windowWidth / 2) + (u.xCoord * windowHeight / 2);
    topLeft.yCoord = eye.yCoord + (l.yCoord * planeDistance) - (r.yCoord * windowWidth / 2) + (u.yCoord * windowHeight / 2);
    topLeft.zCoord = eye.zCoord + (l.zCoord * planeDistance) - (r.zCoord * windowWidth / 2) + (u.zCoord * windowHeight / 2);
    return topLeft;
}

// Helper: Offset the top-left corner for pixel center alignment
void offsetTopLeftForPixelCenter(Point& topLeft, const Point& r, const Point& u, double du, double dv) {
    topLeft.xCoord = topLeft.xCoord + (r.xCoord) * (0.5 * du) - (u.xCoord) * (0.5 * dv);
    topLeft.yCoord = topLeft.yCoord + (r.yCoord) * (0.5 * du) - (u.yCoord) * (0.5 * dv);
    topLeft.zCoord = topLeft.zCoord + (r.zCoord) * (0.5 * du) - (u.zCoord) * (0.5 * dv);
}

// Helper: Trace a single pixel and return the color
void tracePixel(int i, int j, const Point& topLeft, const Point& r, const Point& u, double du, double dv, const Point& eye, bitmap_image& image) {
    Point curPixel;
    curPixel.xCoord = topLeft.xCoord + (i * r.xCoord * du) - (j * u.xCoord * dv);
    curPixel.yCoord = topLeft.yCoord + (i * r.yCoord * du) - (j * u.yCoord * dv);
    curPixel.zCoord = topLeft.zCoord + (i * r.zCoord * du) - (j * u.zCoord * dv);

    Ray* ray = new Ray(eye, curPixel);
    double* color_ray = new double[3];

    Object* nearest = NULL;
    int near_idx;
    double t_min = INT_MAX;
    int count = 0;
    double epsilon = 0.000001;

    for (auto& element : Objects) {
        double* color_temp = new double[3];
        double t = element->intersect(ray, color_temp, 0);
        if ((t < t_min) && (t > epsilon)) {
            t_min = t;
            near_idx = count;
        }
        delete[] color_temp; // Fix memory leak
        count += 1;
    }

    if (t_min != INT_MAX) {
        nearest = Objects[near_idx];
        t_min = nearest->intersect(ray, color_ray, 1);
        image.set_pixel(i, j, round(color_ray[0] * 255), round(color_ray[1] * 255), round(color_ray[2] * 255));
    }
    
    // Clean up allocated memory
    delete ray;
    delete[] color_ray;
}

// Helper: Save the image
void saveImage(bitmap_image& image, int& imageCount) {
    image.save_image("Output_" + std::to_string(imageCount) + ".bmp");
    imageCount += 1;
}

void Capture()
{
    std::cout << "Capturing image..." << std::endl;
    bitmap_image image(pixels, pixels);
    initializeImage(image, pixels, pixels);

    int imageWidth = pixels;
    int imageHeight = pixels;

    double planeDistance = (mainWindowHeight / 2.0) / (tan(mainCameraAngle * PI / (2.0 * 180)));
    Point topLeft = calculateTopLeftCorner(cameraEye, cameraLook, cameraRight, cameraUp, planeDistance, mainWindowWidth, mainWindowHeight);

    double du = mainWindowWidth * 1.0 / imageWidth;
    double dv = mainWindowHeight * 1.0 / imageHeight;

    offsetTopLeftForPixelCenter(topLeft, cameraRight, cameraUp, du, dv);

    std::cout << "Starting ray tracing for " << imageWidth << "x" << imageHeight << " image..." << std::endl;
    int progressStep = max(1, (imageWidth * imageHeight) / 100); // Report progress every 1%
    int pixelCount = 0;

    for (int i = 0; i < imageWidth; i++) {
        for (int j = 0; j < imageHeight; j++) {
            tracePixel(i, j, topLeft, cameraRight, cameraUp, du, dv, cameraEye, image);
            pixelCount++;
            if (pixelCount % progressStep == 0) {
                std::cout << "Progress: " << (pixelCount * 100) / (imageWidth * imageHeight) << "%" << std::endl;
            }
        }
    }

    saveImage(image, outputImageCount);
    std::cout << "Image captured successfully" << std::endl;
}

// Helper: Draw a single scene object
void drawSingleSceneObject(Object* sceneObject) {
    glPushMatrix();
    glTranslatef(sceneObject->objectReferencePoint.xCoord, sceneObject->objectReferencePoint.yCoord, sceneObject->objectReferencePoint.zCoord);
    sceneObject->draw();
    glPopMatrix();
}

// Helper: Draw a single point light
void drawSinglePointLight(PointLight* pointLight) {
    glPushMatrix();
    pointLight->draw();
    glPopMatrix();
}

// Helper: Draw a single spot light
void drawSingleSpotLight(SpotLight* spotLight) {
    glPushMatrix();
    spotLight->draw();
    glPopMatrix();
}

void drawSS()
{
    for (auto& sceneObject : Objects)
    {
        drawSingleSceneObject(sceneObject);
    }
    for (auto& pointLight : pointLights)
    {
        drawSinglePointLight(pointLight);
    }
    for (auto& spotLight : spotlights)
    {
        drawSingleSpotLight(spotLight);
    }
}

// Helper: Handle camera capture
void handleCameraCapture() {
    Capture();
}

// Helper: Handle texture mode toggle
void handleTextureModeToggle() {
    useTextureMode = !useTextureMode;
    if (useTextureMode) {
        std::cout << "Switched to TEXTURE mode" << std::endl;
    } else {
        std::cout << "Switched to CHECKERBOARD mode" << std::endl;
    }
}

// Helper: Handle camera movement
void handleCameraMovement(int key) {
    switch (key)
    {
    case '1':
        cameraMoveAngle = -cameraMoveAngle;
        rotate_axis(&cameraLook, &cameraRight);
        cameraMoveAngle = -cameraMoveAngle;
        break;
    case '2':
        rotate_axis(&cameraLook, &cameraRight);
        break;
    case '3':
        rotate_axis(&cameraLook, &cameraUp);
        break;
    case '4':
        cameraMoveAngle = -cameraMoveAngle;
        rotate_axis(&cameraLook, &cameraUp);
        cameraMoveAngle = -cameraMoveAngle;
        break;
    case '5':
        rotate_axis(&cameraUp, &cameraRight);
        break;
    case '6':
        cameraMoveAngle = -cameraMoveAngle;
        rotate_axis(&cameraUp, &cameraRight);
        cameraMoveAngle = -cameraMoveAngle;
        break;
    default:
        break;
    }
}

void keyboardListener(unsigned char pressedKey, int mouseX, int mouseY)
{
    switch (pressedKey)
    {
    case '0':
        handleCameraCapture();
        break;
    case 'T':
    case 't':
        handleTextureModeToggle();
        break;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
        handleCameraMovement(pressedKey);
        break;
    default:
        break;
    }
}

// Helper: Handle camera movement for special keys
void handleSpecialCameraMovement(int specialKey) {
    switch (specialKey)
    {
    case GLUT_KEY_DOWN: // down arrow key
        mainCameraHeight -= 3.0;
        cameraEye.xCoord -= 5.0 * cameraLook.xCoord;
        cameraEye.yCoord -= 5.0 * cameraLook.yCoord;
        cameraEye.zCoord -= 5.0 * cameraLook.zCoord;
        break;
    case GLUT_KEY_UP: // up arrow key
        mainCameraHeight += 3.0;
        cameraEye.xCoord += 5.0 * cameraLook.xCoord;
        cameraEye.yCoord += 5.0 * cameraLook.yCoord;
        cameraEye.zCoord += 5.0 * cameraLook.zCoord;
        break;
    case GLUT_KEY_RIGHT:
        mainCameraAngle += 0.03;
        cameraEye.xCoord += 5.0 * cameraRight.xCoord;
        cameraEye.yCoord += 5.0 * cameraRight.yCoord;
        cameraEye.zCoord += 5.0 * cameraRight.zCoord;
        break;
    case GLUT_KEY_LEFT:
        mainCameraAngle -= 0.03;
        cameraEye.xCoord -= 5.0 * cameraRight.xCoord;
        cameraEye.yCoord -= 5.0 * cameraRight.yCoord;
        cameraEye.zCoord -= 5.0 * cameraRight.zCoord;
        break;
    case GLUT_KEY_PAGE_UP:
        cameraEye.xCoord += 5.0 * cameraUp.xCoord;
        cameraEye.yCoord += 5.0 * cameraUp.yCoord;
        cameraEye.zCoord += 5.0 * cameraUp.zCoord;
        break;
    case GLUT_KEY_PAGE_DOWN:
        cameraEye.xCoord -= 5.0 * cameraUp.xCoord;
        cameraEye.yCoord -= 5.0 * cameraUp.yCoord;
        cameraEye.zCoord -= 5.0 * cameraUp.zCoord;
        break;
    case GLUT_KEY_INSERT:
        break;
    case GLUT_KEY_HOME:
        if (globalRx >= 1)
        {
            globalRadius += 1;
            globalRx -= 1;
            globalRy -= 1;
            globalRz -= 1;
        }
        break;
    case GLUT_KEY_END:
        if (globalRadius >= 1)
        {
            globalRadius -= 1;
            globalRx += 1;
            globalRy += 1;
            globalRz += 1;
        }
        break;
    default:
        break;
    }
}

void specialKeyListener(int specialKey, int mouseX, int mouseY)
{
    handleSpecialCameraMovement(specialKey);
}

// Helper: Handle mouse button actions
void handleMouseButton(int button, int state) {
    switch (button)
    {
    case GLUT_LEFT_BUTTON:
        if (state == GLUT_DOWN)
        {
            drawAxesFlag = 1 - drawAxesFlag;
        }
        break;
    case GLUT_RIGHT_BUTTON:
        // Reserved for future actions
        break;
    case GLUT_MIDDLE_BUTTON:
        // Reserved for future actions
        break;
    default:
        break;
    }
}

void mouseListener(int button, int state, int mouseX, int mouseY)
{
    handleMouseButton(button, state);
}

// Helper: Set up OpenGL for display
void setupOpenGLDisplay() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0, 0, 0, 0); // color black
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Helper: Set up camera view
void setupCameraView() {
    gluLookAt(cameraEye.xCoord, cameraEye.yCoord, cameraEye.zCoord, cameraEye.xCoord + cameraLook.xCoord, cameraEye.yCoord + cameraLook.yCoord, cameraEye.zCoord + cameraLook.zCoord, cameraUp.xCoord, cameraUp.yCoord, cameraUp.zCoord);
    glMatrixMode(GL_MODELVIEW);
}

void display()
{
    setupOpenGLDisplay();
    setupCameraView();
    drawAxes();
    drawGrid();
    drawSS();
    glutSwapBuffers();
}

// Helper: Perform animation step
void performAnimationStep() {
    animationAngle += 0.05;
    glutPostRedisplay();
}

void animate()
{
    performAnimationStep();
}

// Helper: Initialize camera
void initializeCamera() {
    mainCameraHeight = 150.0;
    mainCameraAngle = 80.0;
    animationAngle = 0;
    cameraUp.xCoord = 0;
    cameraUp.yCoord = 0;
    cameraUp.zCoord = 1;
    cameraRight.xCoord = -0.70710678118;
    cameraRight.yCoord = 0.70710678118;
    cameraRight.zCoord = 0;
    cameraLook.xCoord = -0.70710678118;
    cameraLook.yCoord = -0.70710678118;
    cameraLook.zCoord = 0;
    cameraEye.xCoord = 100;
    cameraEye.yCoord = 100;
    cameraEye.zCoord = 50;
    cameraMoveAngle = 5.0;
}

// Helper: Initialize OpenGL state
void initializeOpenGLState() {
    glClearColor(0, 0, 0, 0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(mainCameraAngle, 1, 1, 1000.0);
}

void init()
{
    drawGridFlag = 0;
    drawAxesFlag = 1;
    initializeCamera();
    initializeOpenGLState();
}

// Helper: Free scene objects
void freeSceneObjects() {
    for (int i = 0; i < Objects.size(); ++i)
    {
        delete Objects[i];
    }
    Objects.clear();
}

// Helper: Free point lights
void freePointLights() {
    for (int i = 0; i < pointLights.size(); ++i)
    {
        delete pointLights[i];
    }
    pointLights.clear();
}

// Helper: Free spot lights
void freeSpotLights() {
    for (int i = 0; i < spotlights.size(); ++i)
    {
        delete spotlights[i];
    }
    spotlights.clear();
}

void freeMemory()
{
    freeSceneObjects();
    freePointLights();
    freeSpotLights();
    std::cout << "Freeing Memory " << std::endl;
}

// Helper: Set up GLUT and OpenGL
void setupGLUTandOpenGL(int& argCount, char** argValues) {
    glutInit(&argCount, argValues);
    glutInitWindowSize(mainWindowWidth, mainWindowHeight);
    glutInitWindowPosition(0, 0);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
    glutCreateWindow("My OpenGL Program");
    glEnable(GL_DEPTH_TEST);
}

// Helper: Register GLUT callbacks
void registerGLUTCallbacks() {
    glutDisplayFunc(display);
    glutIdleFunc(animate);
    glutKeyboardFunc(keyboardListener);
    glutSpecialFunc(specialKeyListener);
    glutMouseFunc(mouseListener);
}

// Helper: Initial scene setup and printing
void initialSceneSetupAndPrint() {
    loadData();
    printObjects();
    printLights();
}

int main(int argCount, char* argValues[])
{
    atexit(freeMemory);
    initialSceneSetupAndPrint();
    setupGLUTandOpenGL(argCount, argValues);
    init();
    registerGLUTCallbacks();
    glutMainLoop();
    return EXIT_SUCCESS;
}
