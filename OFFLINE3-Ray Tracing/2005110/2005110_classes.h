#include <iostream>
#include <vector>
#include <math.h>

#define PI (2 * acos(0.0))

using namespace std;

// Include STB image implementation
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Texture data structure
struct TextureData {
    unsigned char* data;
    int width;
    int height;
    int channels;
    
    TextureData() : data(nullptr), width(0), height(0), channels(0) {}
    
    ~TextureData() {
        if (data) {
            stbi_image_free(data);
        }
    }
};

// Global texture data
TextureData floorTexture;

// Floor rendering mode: true = texture, false = checkerboard
bool useTextureMode = true;

// Function to load texture
bool loadTexture(const char* filename, TextureData& texture) {
    texture.data = stbi_load(filename, &texture.width, &texture.height, &texture.channels, 0);
    if (!texture.data) {
        cout << "Failed to load texture: " << filename << endl;
        return false;
    }
    cout << "Loaded texture: " << filename << " (" << texture.width << "x" << texture.height << ", " << texture.channels << " channels)" << endl;
    return true;
}

// Function to sample texture with checkerboard fallback
void sampleTexture(const TextureData& texture, double u, double v, double* color) {
    if (!useTextureMode || !texture.data) {
        // Use checkerboard pattern, per tile
        if (u < 0) u += 1.0;
        if (v < 0) v += 1.0;
        int check_u = (int)(u);
        int check_v = (int)(v);
        if ((check_u + check_v) % 2 == 0) {
            color[0] = 1.0; color[1] = 1.0; color[2] = 1.0; // White
        } else {
            color[0] = 0.0; color[1] = 0.0; color[2] = 0.0; // Black
        }
        return;
    }
    
    // Clamp u and v to [0, 1] range
    u = fmod(u, 1.0);
    v = fmod(v, 1.0);
    if (u < 0) u += 1.0;
    if (v < 0) v += 1.0;
    
    // Convert to pixel coordinates
    int x = (int)(u * (texture.width - 1));
    int y = (int)(v * (texture.height - 1));
    
    // Clamp to valid range
    x = max(0, min(x, texture.width - 1));
    y = max(0, min(y, texture.height - 1));
    
    // Get pixel data with bounds checking
    int index = (y * texture.width + x) * texture.channels;
    
    // Additional bounds checking
    if (index < 0 || index >= texture.width * texture.height * texture.channels) {
        // Fallback to checkerboard if index is out of bounds
        if (((int)(u * 8) + (int)(v * 8)) % 2 == 0) {
            color[0] = 1.0; color[1] = 1.0; color[2] = 1.0;
        } else {
            color[0] = 0.0; color[1] = 0.0; color[2] = 0.0;
        }
        return;
    }
    
    if (texture.channels >= 3) {
        color[0] = texture.data[index] / 255.0;
        color[1] = texture.data[index + 1] / 255.0;
        color[2] = texture.data[index + 2] / 255.0;
    } else if (texture.channels == 1) {
        // Grayscale
        double gray = texture.data[index] / 255.0;
        color[0] = color[1] = color[2] = gray;
    } else {
        color[0] = 1.0; color[1] = 1.0; color[2] = 1.0;
    }
}

// Point class
class Point
{
public:
    double xCoord;
    double yCoord;
    double zCoord;

    Point() {}

    Point(double x, double y, double z)
    {
        this->xCoord = x;
        this->yCoord = y;
        this->zCoord = z;
    }

    // Calculate squared length to avoid unnecessary sqrt
    double calculateSquaredLength() 
    {
        return pow(xCoord, 2) + pow(yCoord, 2) + pow(zCoord, 2);
    }

    // Calculate actual length using squared length
    double length()
    {
        return sqrt(calculateSquaredLength());
    }

    // Scale a point by a factor
    void scaleByFactor(double factor)
    {
        this->xCoord *= factor;
        this->yCoord *= factor;
        this->zCoord *= factor;
    }

    // Normalize the point and return original length
    double normalize()
    {
        double len = length();
        if (len > 0) {
            double scaleFactor = 1.0 / len;
            scaleByFactor(scaleFactor);
        }
        return len;
    }

    void printPoint()
    {
        cout << " Point : " << xCoord << " " << yCoord << " " << zCoord << endl;
        return;
    }
};

// Ray class
class Ray
{
public:
    Point rayStart;
    Point rayDirection;

    Ray(Point eye, Point cur_pixel)
    {
        rayStart = eye;
        calculateDirectionFromPoints(eye, cur_pixel);
    }

    Ray() {}

    // Calculate direction vector from two points
    void calculateDirectionFromPoints(Point startPoint, Point endPoint)
    {
        rayDirection.xCoord = endPoint.xCoord - startPoint.xCoord;
        rayDirection.yCoord = endPoint.yCoord - startPoint.yCoord;
        rayDirection.zCoord = endPoint.zCoord - startPoint.zCoord;
        normalizeDirection();
    }

    // Calculate the magnitude of the direction vector
    double calculateDirectionMagnitude()
    {
        return sqrt((rayDirection.xCoord * rayDirection.xCoord) + 
                   (rayDirection.yCoord * rayDirection.yCoord) + 
                   (rayDirection.zCoord * rayDirection.zCoord));
    }

    // Normalize the direction vector
    void normalizeDirection()
    {
        double magnitude = calculateDirectionMagnitude();
        if (magnitude > 0) {
            rayDirection.xCoord = rayDirection.xCoord / magnitude;
            rayDirection.yCoord = rayDirection.yCoord / magnitude;
            rayDirection.zCoord = rayDirection.zCoord / magnitude;
        }
    }

    // Set direction vector and normalize it
    void setDir(Point direction)
    {
        rayDirection = direction;
        normalizeDirection();
    }

    // Set the starting point of the ray
    void setStart(Point startPoint)
    {
        rayStart = startPoint;
    }

    void printRay()
    {
        cout << "Ray start  " << endl;
        rayStart.printPoint();
        cout << "Ray dir  " << endl;
        rayDirection.printPoint();
    }
};

// PointLight class
class PointLight
{
public:
    Point lightPosition;
    double lightColor[3];

    PointLight(Point pos)
    {
        lightPosition = pos;
        initializeDefaultColor();
    }

    // Initialize default color values
    void initializeDefaultColor()
    {
        lightColor[0] = 0.0;
        lightColor[1] = 0.0;
        lightColor[2] = 0.0;
    }

    // Set the light color from an array
    void setColor(double colorArray[])
    {
        copyColorValues(colorArray);
    }

    // Copy color values from source array
    void copyColorValues(double sourceColor[])
    {
        this->lightColor[0] = sourceColor[0];
        this->lightColor[1] = sourceColor[1];
        this->lightColor[2] = sourceColor[2];
    }

    // Get individual color component
    double getColorComponent(int index)
    {
        if (index >= 0 && index < 3) {
            return lightColor[index];
        }
        return 0.0;
    }

    // Set individual color component
    void setColorComponent(int index, double value)
    {
        if (index >= 0 && index < 3) {
            lightColor[index] = value;
        }
    }

    // Draw the light representation in OpenGL
    void draw()
    {
        setOpenGLColor();
        drawLightQuad();
    }

    // Set OpenGL color for rendering
    void setOpenGLColor()
    {
        glColor3f(lightColor[0], lightColor[1], lightColor[2]);
    }

    // Draw the light quad representation
    void drawLightQuad()
    {
        glBegin(GL_QUADS);
        {
            // upper hemisphere
            glVertex3f(lightPosition.xCoord, lightPosition.yCoord, lightPosition.zCoord);
            glVertex3f(lightPosition.xCoord + 5, lightPosition.yCoord, lightPosition.zCoord);
            glVertex3f(lightPosition.xCoord + 5, lightPosition.yCoord + 5, lightPosition.zCoord);
            glVertex3f(lightPosition.xCoord, lightPosition.yCoord + 5, lightPosition.zCoord);
        }
        glEnd();
    }

    void PrintLight()
    {
        cout << "PointLight " << endl;
        lightPosition.printPoint();
        cout << "Color : " << lightColor[0] << " " << lightColor[1] << " " << lightColor[2] << endl;
    }
};

// SpotLight class
class SpotLight
{
public:
    PointLight *pointLight;
    Point spotDirection;
    double cutoffAngle;

    SpotLight(PointLight pl, Point light_dir, double angle)
    {
        createPointLightCopy(pl);
        setSpotDirection(light_dir);
        setCutoffAngle(angle);
    }

    // Create a copy of the point light
    void createPointLightCopy(PointLight sourceLight)
    {
        this->pointLight = new PointLight(sourceLight.lightPosition);
        this->pointLight->setColor(sourceLight.lightColor);
    }

    // Set the spot light direction
    void setSpotDirection(Point direction)
    {
        this->spotDirection = direction;
    }

    // Set the cutoff angle for the spot light
    void setCutoffAngle(double angle)
    {
        this->cutoffAngle = angle;
    }

    // Get the cutoff angle
    double getCutoffAngle()
    {
        return cutoffAngle;
    }

    // Get the spot direction
    Point getSpotDirection()
    {
        return spotDirection;
    }

    // Get the point light component
    PointLight* getPointLight()
    {
        return pointLight;
    }

    // Check if a ray is within the spot light cone
    bool isRayInCone(Ray* ray)
    {
        double angle = calculateAngleWithDirection(ray);
        return angle <= cutoffAngle;
    }

    // Calculate angle between ray direction and spot direction
    double calculateAngleWithDirection(Ray* ray)
    {
        double dotProduct = calculateDotProduct(ray->rayDirection, spotDirection);
        double lightDirLength = spotDirection.length();
        double rayLength = ray->rayDirection.length();
        
        if (lightDirLength > 0 && rayLength > 0) {
            double cosAngle = dotProduct / (lightDirLength * rayLength);
            return acos(cosAngle) * 180.0 / PI;
        }
        return 180.0; // Return maximum angle if vectors are invalid
    }

    // Calculate dot product between two points
    double calculateDotProduct(Point first, Point second)
    {
        return (first.xCoord * second.xCoord) + 
               (first.yCoord * second.yCoord) + 
               (first.zCoord * second.zCoord);
    }

    // Draw the spot light representation
    void draw()
    {
        drawPointLightComponent();
    }

    // Draw the point light component
    void drawPointLightComponent()
    {
        pointLight->draw();
    }

    // Print spot light information
    void PrintLight()
    {
        printSpotLightHeader();
        printPointLightInfo();
        printDirectionInfo();
        printCutoffInfo();
    }

    // Print spot light header
    void printSpotLightHeader()
    {
        cout << "Spotlight " << endl;
    }

    // Print point light information
    void printPointLightInfo()
    {
        pointLight->PrintLight();
    }

    // Print direction information
    void printDirectionInfo()
    {
        spotDirection.printPoint();
    }

    // Print cutoff angle information
    void printCutoffInfo()
    {
        cout << "Cutoff Angle : " << cutoffAngle << endl;
    }
};

// Object class
class Object
{
public:
    Point objectReferencePoint;
    double objectHeight, objectWidth, objectLength;
    double objectColor[3];
    double materialCoefficients[4];
    double materialShine;

    Object()
    {
        initializeDefaultValues();
    }

    // Initialize default values for object properties
    void initializeDefaultValues()
    {
        objectHeight = 0.0;
        objectWidth = 0.0;
        objectLength = 0.0;
        materialShine = 0.0;
        
        for(int i = 0; i < 3; i++) {
            objectColor[i] = 0.0;
        }
        
        for(int i = 0; i < 4; i++) {
            materialCoefficients[i] = 0.0;
        }
    }

    // Virtual method for drawing the object
    virtual void draw() {}

    // Virtual method for ray intersection
    virtual double intersect(Ray *r, double *color, int level)
    {
        return INT_MAX;
    }

    // Set the lighting color based on ambient coefficient
    void setLightingColor(double *colorArray)
    {
        calculateAmbientColor(colorArray);
    }

    // Calculate ambient color component
    void calculateAmbientColor(double *colorArray)
    {
        colorArray[0] = objectColor[0] * materialCoefficients[0];
        colorArray[1] = objectColor[1] * materialCoefficients[0];
        colorArray[2] = objectColor[2] * materialCoefficients[0];
    }

    // Set the object color from an array
    void setColor(double colorArray[])
    {
        copyColorValues(colorArray);
    }

    // Copy color values from source array
    void copyColorValues(double sourceColor[])
    {
        this->objectColor[0] = sourceColor[0];
        this->objectColor[1] = sourceColor[1];
        this->objectColor[2] = sourceColor[2];
    }

    // Get individual color component
    double getColorComponent(int index)
    {
        if (index >= 0 && index < 3) {
            return objectColor[index];
        }
        return 0.0;
    }

    // Set individual color component
    void setColorComponent(int index, double value)
    {
        if (index >= 0 && index < 3) {
            objectColor[index] = value;
        }
    }

    // Set the material shine value
    void setShine(double shineValue)
    {
        this->materialShine = shineValue;
    }

    // Get the material shine value
    double getShine()
    {
        return materialShine;
    }

    // Set the material coefficients from an array
    void setCoefficients(double coefficientArray[])
    {
        copyCoefficientValues(coefficientArray);
    }

    // Copy coefficient values from source array
    void copyCoefficientValues(double sourceCoefficients[])
    {
        this->materialCoefficients[0] = sourceCoefficients[0];
        this->materialCoefficients[1] = sourceCoefficients[1];
        this->materialCoefficients[2] = sourceCoefficients[2];
        this->materialCoefficients[3] = sourceCoefficients[3];
    }

    // Get individual coefficient value
    double getCoefficient(int index)
    {
        if (index >= 0 && index < 4) {
            return materialCoefficients[index];
        }
        return 0.0;
    }

    // Set individual coefficient value
    void setCoefficient(int index, double value)
    {
        if (index >= 0 && index < 4) {
            materialCoefficients[index] = value;
        }
    }

    // Calculate dot product between two points
    double dot_product(Point first, Point second)
    {
        double result = (first.xCoord * second.xCoord) + 
                       (first.yCoord * second.yCoord) + 
                       (first.zCoord * second.zCoord);
        return result;
    }

    // Calculate reflection contribution
    void calculateReflection(double *colorRay, double *color_in)
    {
        addReflectionContribution(colorRay, color_in);
        clampColorValues(color_in);
    }

    // Add reflection contribution to color
    void addReflectionContribution(double *colorRay, double *color_in)
    {
        color_in[0] += colorRay[0] * materialCoefficients[3];
        color_in[1] += colorRay[1] * materialCoefficients[3];
        color_in[2] += colorRay[2] * materialCoefficients[3];
    }

    // Clamp color values to valid range [0, 1]
    void clampColorValues(double *colorArray)
    {
        for (int i = 0; i < 3; i++) {
            if (colorArray[i] > 1.0) {
                colorArray[i] = 1.0;
            }
        }
    }

    // Calculate specular and diffuse lighting
    void calculateSpecularDiffuse(Point normal, Ray *ray_point_light, double *color_in, 
                                 Point intersection_point, Ray *r, PointLight *pl)
    {
        double cosTheta = calculateCosineTheta(normal, ray_point_light);
        double lambertValue = calculateLambertValue(cosTheta);
        
        Point reflectedRayDir = calculateReflectedRayDirection(normal, ray_point_light, cosTheta);
        Ray reflectedRay = createReflectedRay(intersection_point, reflectedRayDir);
        
        double phongValue = calculatePhongValue(reflectedRay, r);
        
        double constDiffuse = calculateDiffuseConstant(lambertValue);
        double constSpecular = calculateSpecularConstant(phongValue);
        
        applyDiffuseLighting(color_in, pl, constDiffuse);
        applySpecularLighting(color_in, pl, constSpecular);
        clampColorValues(color_in);
    }

    // Calculate cosine of angle between normal and light ray
    double calculateCosineTheta(Point normal, Ray *ray_point_light)
    {
        return dot_product(normal, ray_point_light->rayDirection);
    }

    // Calculate Lambert value for diffuse lighting
    double calculateLambertValue(double cosTheta)
    {
        return -1.0 * cosTheta;
    }

    // Calculate reflected ray direction
    Point calculateReflectedRayDirection(Point normal, Ray *ray_point_light, double cosTheta)
    {
        Point reflectedRayDir(
            -(2.0 * cosTheta * normal.xCoord) + ray_point_light->rayDirection.xCoord,
            -(2.0 * cosTheta * normal.yCoord) + ray_point_light->rayDirection.yCoord,
            -(2.0 * cosTheta * normal.zCoord) + ray_point_light->rayDirection.zCoord
        );
        reflectedRayDir.normalize();
        return reflectedRayDir;
    }

    // Create reflected ray
    Ray createReflectedRay(Point intersection_point, Point reflectedRayDir)
    {
        Ray reflectedRay;
        reflectedRay.setStart(intersection_point);
        reflectedRay.setDir(reflectedRayDir);
        return reflectedRay;
    }

    // Calculate Phong value for specular lighting
    double calculatePhongValue(Ray reflectedRay, Ray *r)
    {
        return -1.0 * dot_product(reflectedRay.rayDirection, r->rayDirection);
    }

    // Calculate diffuse constant
    double calculateDiffuseConstant(double lambertValue)
    {
        return materialCoefficients[1] * max(lambertValue, 0.0);
    }

    // Calculate specular constant
    double calculateSpecularConstant(double phongValue)
    {
        return materialCoefficients[2] * pow(max(phongValue, 0.0), materialShine);
    }

    // Apply diffuse lighting to color
    void applyDiffuseLighting(double *color_in, PointLight *pl, double constDiffuse)
    {
        color_in[0] += objectColor[0] * pl->lightColor[0] * constDiffuse;
        color_in[1] += objectColor[1] * pl->lightColor[1] * constDiffuse;
        color_in[2] += objectColor[2] * pl->lightColor[2] * constDiffuse;
    }

    // Apply specular lighting to color
    void applySpecularLighting(double *color_in, PointLight *pl, double constSpecular)
    {
        color_in[0] += objectColor[0] * pl->lightColor[0] * constSpecular;
        color_in[1] += objectColor[1] * pl->lightColor[1] * constSpecular;
        color_in[2] += objectColor[2] * pl->lightColor[2] * constSpecular;
    }

    // Print object information
    virtual void print_object()
    {
        printColorInfo();
        printCoefficientInfo();
        printShineInfo();
    }

    // Print color information
    void printColorInfo()
    {
        cout << "Color : " << objectColor[0] << " " << objectColor[1] << " " << objectColor[2] << endl;
    }

    // Print coefficient information
    void printCoefficientInfo()
    {
        cout << "Coefficients: " << materialCoefficients[0] << " " << materialCoefficients[1] 
             << " " << materialCoefficients[2] << " " << materialCoefficients[3] << endl;
    }

    // Print shine information
    void printShineInfo()
    {
        cout << "Shine : " << materialShine << endl;
    }
};

vector<Object *> Objects;
vector<PointLight *> pointLights;
vector<SpotLight *> spotlights;
int level_recursion;

class Sphere : public Object
{
public:
    Sphere(Point center, double radius)
    {
        objectReferencePoint = center;
        objectLength = radius;
    }

    // Draw the sphere using OpenGL
    void draw()
    {
        Point spherePoints[100][100];
        generateSpherePoints(spherePoints);
        renderSphereGeometry(spherePoints);
    }

    // Generate sphere points for rendering
    void generateSpherePoints(Point spherePoints[100][100])
    {
        int slices = 12;
        int stacks = 20;
        double radius = objectLength;

        for (int i = 0; i <= stacks; i++)
        {
            double h = calculateHeightCoordinate(i, stacks, radius);
            double r = calculateRadiusCoordinate(i, stacks, radius);
            
            for (int j = 0; j <= slices; j++)
            {
                spherePoints[i][j] = calculateSpherePoint(i, j, slices, r, h);
            }
        }
    }

    // Calculate height coordinate for sphere point
    double calculateHeightCoordinate(int stackIndex, int totalStacks, double radius)
    {
        return radius * sin(((double)stackIndex / (double)totalStacks) * (PI / 2));
    }

    // Calculate radius coordinate for sphere point
    double calculateRadiusCoordinate(int stackIndex, int totalStacks, double radius)
    {
        return radius * cos(((double)stackIndex / (double)totalStacks) * (PI / 2));
    }

    // Calculate individual sphere point coordinates
    Point calculateSpherePoint(int stackIndex, int sliceIndex, int totalSlices, double radius, double height)
    {
        Point point;
        point.xCoord = radius * cos(((double)sliceIndex / (double)totalSlices) * 2 * PI);
        point.yCoord = radius * sin(((double)sliceIndex / (double)totalSlices) * 2 * PI);
        point.zCoord = height;
        return point;
    }

    // Render sphere geometry using generated points
    void renderSphereGeometry(Point spherePoints[100][100])
    {
        int slices = 12;
        int stacks = 20;

        for (int i = 0; i < stacks; i++)
        {
            setSphereColor();
            for (int j = 0; j < slices; j++)
            {
                renderSphereQuad(spherePoints, i, j);
            }
        }
    }

    // Set the sphere color for rendering
    void setSphereColor()
    {
        glColor3f(objectColor[0], objectColor[1], objectColor[2]);
    }

    // Render a single quad of the sphere
    void renderSphereQuad(Point spherePoints[100][100], int stackIndex, int sliceIndex)
    {
        glBegin(GL_QUADS);
        {
            renderUpperHemisphere(spherePoints, stackIndex, sliceIndex);
            renderLowerHemisphere(spherePoints, stackIndex, sliceIndex);
        }
        glEnd();
    }

    // Render upper hemisphere quad
    void renderUpperHemisphere(Point spherePoints[100][100], int stackIndex, int sliceIndex)
    {
        glVertex3f(spherePoints[stackIndex][sliceIndex].xCoord, spherePoints[stackIndex][sliceIndex].yCoord, spherePoints[stackIndex][sliceIndex].zCoord);
        glVertex3f(spherePoints[stackIndex][sliceIndex + 1].xCoord, spherePoints[stackIndex][sliceIndex + 1].yCoord, spherePoints[stackIndex][sliceIndex + 1].zCoord);
        glVertex3f(spherePoints[stackIndex + 1][sliceIndex + 1].xCoord, spherePoints[stackIndex + 1][sliceIndex + 1].yCoord, spherePoints[stackIndex + 1][sliceIndex + 1].zCoord);
        glVertex3f(spherePoints[stackIndex + 1][sliceIndex].xCoord, spherePoints[stackIndex + 1][sliceIndex].yCoord, spherePoints[stackIndex + 1][sliceIndex].zCoord);
    }

    // Render lower hemisphere quad
    void renderLowerHemisphere(Point spherePoints[100][100], int stackIndex, int sliceIndex)
    {
        glVertex3f(spherePoints[stackIndex][sliceIndex].xCoord, spherePoints[stackIndex][sliceIndex].yCoord, -spherePoints[stackIndex][sliceIndex].zCoord);
        glVertex3f(spherePoints[stackIndex][sliceIndex + 1].xCoord, spherePoints[stackIndex][sliceIndex + 1].yCoord, -spherePoints[stackIndex][sliceIndex + 1].zCoord);
        glVertex3f(spherePoints[stackIndex + 1][sliceIndex + 1].xCoord, spherePoints[stackIndex + 1][sliceIndex + 1].yCoord, -spherePoints[stackIndex + 1][sliceIndex + 1].zCoord);
        glVertex3f(spherePoints[stackIndex + 1][sliceIndex].xCoord, spherePoints[stackIndex + 1][sliceIndex].yCoord, -spherePoints[stackIndex + 1][sliceIndex].zCoord);
    }

    // Calculate sphere-ray intersection
    double intersect(Ray *r, double *color_in, int level)
    {
        double epsilon = 0.0000001;
        Point translatedRayStart = calculateTranslatedRayStart(r);
        
        double projectionDistance = calculateProjectionDistance(r, translatedRayStart);
        if (projectionDistance < 0)
        {
            return INT_MAX;
        }

        double rayStartDistance = calculateRayStartDistance(translatedRayStart);
        double perpendicularDistance = calculatePerpendicularDistance(rayStartDistance, projectionDistance);
        
        if (perpendicularDistance > (objectLength * objectLength))
        {
            return INT_MAX;
        }

        double intersectionDistance = calculateIntersectionDistance(perpendicularDistance);
        double t1 = projectionDistance + intersectionDistance;
        double t2 = projectionDistance - intersectionDistance;

        double t = determineIntersectionParameter(rayStartDistance, t1, t2);

        if (level == 0)
        {
            return t;
        }

        processLightingAndReflection(r, color_in, level, t);
        return t;
    }

    // Calculate translated ray start point
    Point calculateTranslatedRayStart(Ray *r)
    {
        return Point(r->rayStart.xCoord - objectReferencePoint.xCoord, 
                    r->rayStart.yCoord - objectReferencePoint.yCoord, 
                    r->rayStart.zCoord - objectReferencePoint.zCoord);
    }

    // Calculate projection distance of ray onto sphere center
    double calculateProjectionDistance(Ray *r, Point translatedRayStart)
    {
        return -(r->rayDirection.xCoord * translatedRayStart.xCoord) - 
               (r->rayDirection.yCoord * translatedRayStart.yCoord) - 
               (r->rayDirection.zCoord * translatedRayStart.zCoord);
    }

    // Calculate distance from ray start to sphere center
    double calculateRayStartDistance(Point translatedRayStart)
    {
        return (translatedRayStart.xCoord * translatedRayStart.xCoord) + 
               (translatedRayStart.yCoord * translatedRayStart.yCoord) + 
               (translatedRayStart.zCoord * translatedRayStart.zCoord);
    }

    // Calculate perpendicular distance from ray to sphere center
    double calculatePerpendicularDistance(double rayStartDistance, double projectionDistance)
    {
        return rayStartDistance - (projectionDistance * projectionDistance);
    }

    // Calculate intersection distance along ray
    double calculateIntersectionDistance(double perpendicularDistance)
    {
        return sqrt((objectLength * objectLength) - perpendicularDistance);
    }

    // Determine which intersection parameter to use
    double determineIntersectionParameter(double rayStartDistance, double t1, double t2)
    {
        double t = -1.0;
        double sphereRadiusSquared = objectLength * objectLength;

        if (rayStartDistance < sphereRadiusSquared)
        {
            t = t1;
        }
        else if (rayStartDistance > sphereRadiusSquared)
        {
            t = t2;
        }
        else if (t == -1.0)
        {
            t = min(t1, t2);
        }

        return t;
    }

    // Process lighting and reflection calculations
    void processLightingAndReflection(Ray *r, double *color_in, int level, double t)
    {
        setLightingColor(color_in);

        Point intersectPoint = calculateIntersectionPoint(r, t);
        Point normal = calculateSphereNormal(intersectPoint);
        normal.normalize();

        vector<PointLight *> allPointLights = pointLights;
        processSpotLights(allPointLights, intersectPoint);
        processPointLights(allPointLights, intersectPoint, normal, r, color_in);

        if (level >= level_recursion)
            return;

        processReflection(r, color_in, level, intersectPoint, normal);
    }

    // Calculate intersection point
    Point calculateIntersectionPoint(Ray *r, double t)
    {
        return Point(r->rayStart.xCoord + (t * r->rayDirection.xCoord),
                    r->rayStart.yCoord + (t * r->rayDirection.yCoord),
                    r->rayStart.zCoord + (t * r->rayDirection.zCoord));
    }

    // Calculate sphere normal at intersection point
    Point calculateSphereNormal(Point intersectPoint)
    {
        return Point(intersectPoint.xCoord - objectReferencePoint.xCoord,
                    intersectPoint.yCoord - objectReferencePoint.yCoord,
                    intersectPoint.zCoord - objectReferencePoint.zCoord);
    }

    // Process spot lights for the sphere
    void processSpotLights(vector<PointLight *> &allPointLights, Point intersectPoint)
    {
        for (auto &sl : spotlights)
        {
            PointLight *pl = sl->pointLight;
            Ray *ray_point_light = new Ray(pl->lightPosition, intersectPoint);

            double alpha = calculateSpotLightAngle(ray_point_light, sl);
            if (alpha <= sl->cutoffAngle)
            {
                allPointLights.push_back(pl);
            }
        }
    }

    // Calculate angle between spot light ray and direction
    double calculateSpotLightAngle(Ray *ray_point_light, SpotLight *sl)
    {
        double dotProduct = dot_product(ray_point_light->rayDirection, sl->spotDirection);
        double lightDirLength = sl->spotDirection.length();
        double rayLength = ray_point_light->rayDirection.length();
        return acos(dotProduct * 1.0 / (lightDirLength * rayLength)) * 180.0 / PI;
    }

    // Process point lights for the sphere
    void processPointLights(vector<PointLight *> allPointLights, Point intersectPoint, 
                           Point normal, Ray *r, double *color_in)
    {
        for (auto &pl : allPointLights)
        {
            Ray *ray_point_light = new Ray(pl->lightPosition, intersectPoint);
            double min_t_pl = INT_MAX;
            
            for (auto &obj : Objects)
            {
                double *dummyColor = new double[3];
                double t_pl = obj->intersect(ray_point_light, dummyColor, 0);
                if ((t_pl > 0.0000001) && (t_pl < min_t_pl))
                {
                    min_t_pl = t_pl;
                }
            }
            
            if (min_t_pl != INT_MAX)
            {
                processShadowCalculation(ray_point_light, min_t_pl, intersectPoint, normal, r, pl, color_in);
            }
        }
    }

    // Process shadow calculation for point light
    void processShadowCalculation(Ray *ray_point_light, double min_t_pl, Point intersectPoint,
                                 Point normal, Ray *r, PointLight *pl, double *color_in)
    {
        Point shadowIntersectPoint = calculateShadowIntersectionPoint(ray_point_light, min_t_pl);
        
        double shadowDistance = calculateDistance(ray_point_light->rayStart, shadowIntersectPoint);
        double lightDistance = calculateDistance(ray_point_light->rayStart, intersectPoint);
        
        if (shadowDistance >= lightDistance - 0.0000001)
        {
            calculateSpecularDiffuse(normal, ray_point_light, color_in, intersectPoint, r, pl);
        }
    }

    // Calculate shadow intersection point
    Point calculateShadowIntersectionPoint(Ray *ray_point_light, double min_t_pl)
    {
        return Point(ray_point_light->rayStart.xCoord + (min_t_pl * ray_point_light->rayDirection.xCoord),
                    ray_point_light->rayStart.yCoord + (min_t_pl * ray_point_light->rayDirection.yCoord),
                    ray_point_light->rayStart.zCoord + (min_t_pl * ray_point_light->rayDirection.zCoord));
    }

    // Calculate distance between two points
    double calculateDistance(Point start, Point end)
    {
        return sqrt(pow(end.xCoord - start.xCoord, 2) + 
                   pow(end.yCoord - start.yCoord, 2) + 
                   pow(end.zCoord - start.zCoord, 2));
    }

    // Process reflection for the sphere
    void processReflection(Ray *r, double *color_in, int level, Point intersectPoint, Point normal)
    {
        double dot_ray_n = dot_product(normal, r->rayDirection);
        Point reflectedRayDir = calculateReflectedRayDirection(normal, r, dot_ray_n);
        reflectedRayDir.normalize();

        Point reflectInitial = calculateReflectionStartPoint(intersectPoint, reflectedRayDir);
        Ray *reflectedRay = createReflectedRay(reflectInitial, reflectedRayDir);

        double *color_ray = new double[3];
        Object *nearest = findNearestObject(reflectedRay, color_ray, level);
        
        if (nearest != NULL)
        {
            nearest->intersect(reflectedRay, color_ray, level + 1);
        }

        calculateReflection(color_ray, color_in);
    }

    // Calculate reflected ray direction
    Point calculateReflectedRayDirection(Point normal, Ray *r, double dot_ray_n)
    {
        return Point(-(2.0 * dot_ray_n * normal.xCoord) + r->rayDirection.xCoord,
                    -(2.0 * dot_ray_n * normal.yCoord) + r->rayDirection.yCoord,
                    -(2.0 * dot_ray_n * normal.zCoord) + r->rayDirection.zCoord);
    }

    // Calculate reflection start point
    Point calculateReflectionStartPoint(Point intersectPoint, Point reflectedRayDir)
    {
        return Point(intersectPoint.xCoord + reflectedRayDir.xCoord,
                    intersectPoint.yCoord + reflectedRayDir.yCoord,
                    intersectPoint.zCoord + reflectedRayDir.zCoord);
    }

    // Create reflected ray
    Ray* createReflectedRay(Point reflectInitial, Point reflectedRayDir)
    {
        Ray *reflectedRay = new Ray();
        reflectedRay->setStart(reflectInitial);
        reflectedRay->setDir(reflectedRayDir);
        return reflectedRay;
    }

    // Find nearest object for reflection
    Object* findNearestObject(Ray *reflectedRay, double *color_ray, int level)
    {
        Object *nearest = NULL;
        int near_idx;
        double t_min = INT_MAX;
        int count = 0;
        double epsilon = 0.0000001;

        for (auto &element : Objects)
        {
            double *color_temp = new double[3];
            double t = element->intersect(reflectedRay, color_temp, 0);
            if ((t < t_min) && (t > epsilon))
            {
                t_min = t;
                near_idx = count;
            }
            count += 1;
        }

        if (t_min != INT_MAX)
        {
            nearest = Objects[near_idx];
        }

        return nearest;
    }

    // Print sphere information
    void print_object()
    {
        printSphereHeader();
        printSphereProperties();
        Object::print_object();
    }

    // Print sphere header
    void printSphereHeader()
    {
        cout << "Sphere " << endl;
    }

    // Print sphere properties
    void printSphereProperties()
    {
        cout << "Radius : " << objectLength << endl;
        cout << "Center : " << objectReferencePoint.xCoord << " " << objectReferencePoint.yCoord << " " << objectReferencePoint.zCoord << endl;
    }
};

class Triangle : public Object
{
public:
    Point firstVertex, secondVertex, thirdVertex;

    Triangle(Point first, Point second, Point third)
    {
        this->firstVertex = first;
        this->secondVertex = second;
        this->thirdVertex = third;
    }

    void draw()
    {
        glColor3f(objectColor[0], objectColor[1], objectColor[2]);
        glBegin(GL_TRIANGLES);
        {
            glVertex3f(firstVertex.xCoord, firstVertex.yCoord, firstVertex.zCoord);
            glVertex3f(secondVertex.xCoord, secondVertex.yCoord, secondVertex.zCoord);
            glVertex3f(thirdVertex.xCoord, thirdVertex.yCoord, thirdVertex.zCoord);
        }
        glEnd();
    }

    void print_object()
    {
        cout << "First : " << firstVertex.xCoord << " " << firstVertex.yCoord << " " << firstVertex.zCoord << endl;
        cout << "Second : " << secondVertex.xCoord << " " << secondVertex.yCoord << " " << secondVertex.zCoord << endl;
        cout << "Third : " << thirdVertex.xCoord << " " << thirdVertex.yCoord << " " << thirdVertex.zCoord << endl;
        Object::print_object();
    }

    // Helper to compute determinants for intersection
    void computeDeterminants(const Ray* r, double& D, double& D1, double& D2, double& D3, double& a1, double& a2, double& a3, double& b1, double& b2, double& b3, double& c1, double& c2, double& c3, double& d1, double& d2, double& d3) {
        a1 = firstVertex.xCoord - secondVertex.xCoord;
        a2 = firstVertex.yCoord - secondVertex.yCoord;
        a3 = firstVertex.zCoord - secondVertex.zCoord;
        b1 = thirdVertex.xCoord - secondVertex.xCoord;
        b2 = thirdVertex.yCoord - secondVertex.yCoord;
        b3 = thirdVertex.zCoord - secondVertex.zCoord;
        c1 = -r->rayDirection.xCoord;
        c2 = -r->rayDirection.yCoord;
        c3 = -r->rayDirection.zCoord;
        d1 = r->rayStart.xCoord - secondVertex.xCoord;
        d2 = r->rayStart.yCoord - secondVertex.yCoord;
        d3 = r->rayStart.zCoord - secondVertex.zCoord;
        D = a1 * (b2 * c3 - c2 * b3) + b1 * (c2 * a3 - c3 * a2) + c1 * (a2 * b3 - a3 * b2);
        D1 = d1 * (b2 * c3 - c2 * b3) + b1 * (c2 * d3 - c3 * d2) + c1 * (d2 * b3 - d3 * b2);
        D2 = a1 * (d2 * c3 - c2 * d3) + d1 * (c2 * a3 - c3 * a2) + c1 * (a2 * d3 - a3 * d2);
        D3 = a1 * (b2 * d3 - d2 * b3) + b1 * (d2 * a3 - d3 * a2) + d1 * (a2 * b3 - a3 * b2);
    }

    // Helper to compute intersection point and normal
    void computeIntersectionAndNormal(const Ray* r, double t, double a1, double a2, double a3, double b1, double b2, double b3, Point& intersectionPoint, Point& normal) {
        intersectionPoint = Point(
            r->rayStart.xCoord + (t * r->rayDirection.xCoord),
            r->rayStart.yCoord + (t * r->rayDirection.yCoord),
            r->rayStart.zCoord + (t * r->rayDirection.zCoord)
        );
        normal.xCoord = (a2 * b3) - (b2 * a3);
        normal.yCoord = (a3 * b1) - (a1 * b3);
        normal.zCoord = (a1 * b2) - (a2 * b1);
        normal.normalize();
    }

    // Helper to process spot lights
    void processSpotLightsForTriangle(vector<PointLight*>& allPointLights, Point& intersectionPoint) {
        for (auto& sl : spotlights) {
            PointLight* pl = sl->pointLight;
            Ray* ray_point_light = new Ray(pl->lightPosition, intersectionPoint);
            double alpha = dot_product(ray_point_light->rayDirection, sl->spotDirection);
            double light_dir_len = sl->spotDirection.length();
            double ray_len = ray_point_light->rayDirection.length();
            alpha = acos(alpha * 1.0 / (light_dir_len * ray_len)) * 180.0 / PI;
            if (alpha <= sl->cutoffAngle) {
                allPointLights.push_back(pl);
            }
        }
    }

    // Helper to process point lights
    void processPointLightsForTriangle(vector<PointLight*>& allPointLights, Point& intersectionPoint, Point& normal, Ray* r, double* color_in) {
        double epsilon = 0.0000001;
        for (auto& pl : allPointLights) {
            Ray* ray_point_light = new Ray(pl->lightPosition, intersectionPoint);
            double min_t_pl = INT_MAX;
            for (auto& obj : Objects) {
                double* dummyColor = new double[3];
                double t_pl = obj->intersect(ray_point_light, dummyColor, 0);
                if ((t_pl > epsilon) && (t_pl < min_t_pl)) {
                    min_t_pl = t_pl;
                }
            }
            if (min_t_pl != INT_MAX) {
                Point shadow_intersect_point(
                    ray_point_light->rayStart.xCoord + (min_t_pl * ray_point_light->rayDirection.xCoord),
                    ray_point_light->rayStart.yCoord + (min_t_pl * ray_point_light->rayDirection.yCoord),
                    ray_point_light->rayStart.zCoord + (min_t_pl * ray_point_light->rayDirection.zCoord)
                );
                double dis_shadow_ip = sqrt(pow(shadow_intersect_point.xCoord - ray_point_light->rayStart.xCoord, 2) + pow(shadow_intersect_point.yCoord - ray_point_light->rayStart.yCoord, 2) + pow(shadow_intersect_point.zCoord - ray_point_light->rayStart.zCoord, 2));
                double dis_ip = sqrt(pow(intersectionPoint.xCoord - ray_point_light->rayStart.xCoord, 2) + pow(intersectionPoint.yCoord - ray_point_light->rayStart.yCoord, 2) + pow(intersectionPoint.zCoord - ray_point_light->rayStart.zCoord, 2));
                if (dis_shadow_ip >= dis_ip - epsilon) {
                    calculateSpecularDiffuse(normal, ray_point_light, color_in, intersectionPoint, r, pl);
                }
            }
        }
    }

    // Helper to process reflection
    void processReflectionForTriangle(Point& normal, Ray* r, Point& intersectionPoint, double* color_in, int level) {
        double dot_ray_n = dot_product(normal, r->rayDirection);
        Point reflectedRayDir(
            -(2.0 * dot_ray_n * normal.xCoord) + r->rayDirection.xCoord,
            -(2.0 * dot_ray_n * normal.yCoord) + r->rayDirection.yCoord,
            -(2.0 * dot_ray_n * normal.zCoord) + r->rayDirection.zCoord
        );
        reflectedRayDir.normalize();
        Point reflect_initial(
            intersectionPoint.xCoord + reflectedRayDir.xCoord,
            intersectionPoint.yCoord + reflectedRayDir.yCoord,
            intersectionPoint.zCoord + reflectedRayDir.zCoord
        );
        Ray* reflectedRay = new Ray();
        reflectedRay->setStart(reflect_initial);
        reflectedRay->setDir(reflectedRayDir);
        double* color_ray = new double[3];
        Object* nearest = NULL;
        int near_idx;
        double t_min = INT_MAX;
        int count = 0;
        double epsilon = 0.0000001;
        for (auto& element : Objects) {
            double* color_temp = new double[3];
            double t = element->intersect(reflectedRay, color_temp, 0);
            if ((t < t_min) && (t > epsilon)) {
                t_min = t;
                near_idx = count;
            }
            count += 1;
        }
        if (t_min != INT_MAX) {
            nearest = Objects[near_idx];
            t_min = nearest->intersect(reflectedRay, color_ray, level + 1);
        }
        calculateReflection(color_ray, color_in);
    }

    double intersect(Ray* r, double* color_in, int level)
    {
        double epsilon = 0.0000001;
        setLightingColor(color_in);
        double a1, a2, a3, b1, b2, b3, c1, c2, c3, d1, d2, d3, D, D1, D2, D3;
        computeDeterminants(r, D, D1, D2, D3, a1, a2, a3, b1, b2, b3, c1, c2, c3, d1, d2, d3);
        if (D != 0) {
            double k1 = D1 / D;
            double k2 = D2 / D;
            double t = D3 / D;
            if ((k1 > 0) && (k2 > 0) && (k1 + k2 <= 1)) {
                if (level == 0)
                    return t;
                Point intersectionPoint, normal;
                computeIntersectionAndNormal(r, t, a1, a2, a3, b1, b2, b3, intersectionPoint, normal);
                vector<PointLight*> allPointLights = pointLights;
                processSpotLightsForTriangle(allPointLights, intersectionPoint);
                processPointLightsForTriangle(allPointLights, intersectionPoint, normal, r, color_in);
                if (level >= level_recursion)
                    return t;
                processReflectionForTriangle(normal, r, intersectionPoint, color_in, level);
                return t;
            }
        }
        return INT_MAX;
    }
};

class General : public Object
{
public:
    vector<double> polynomialCoefficients;

    General(vector<double> coeff)
    {
        this->polynomialCoefficients = coeff;
    }

    void draw()
    {
    }

    // Helper: Calculate quadratic coefficients a, b, c
    void calculateQuadraticCoefficients(const Ray* r, Point& rayStart, double& a, double& b, double& c) {
        a = (polynomialCoefficients[0] * pow(r->rayDirection.xCoord, 2)) +
            (polynomialCoefficients[1] * pow(r->rayDirection.yCoord, 2)) +
            (polynomialCoefficients[2] * pow(r->rayDirection.zCoord, 2)) +
            (polynomialCoefficients[3] * r->rayDirection.xCoord * r->rayDirection.yCoord) +
            (polynomialCoefficients[4] * r->rayDirection.yCoord * r->rayDirection.zCoord) +
            (polynomialCoefficients[5] * r->rayDirection.xCoord * r->rayDirection.zCoord);
        b = (2 * polynomialCoefficients[0] * rayStart.xCoord * r->rayDirection.xCoord) +
            (2 * polynomialCoefficients[1] * rayStart.yCoord * r->rayDirection.yCoord) +
            (2 * polynomialCoefficients[2] * rayStart.zCoord * r->rayDirection.zCoord) +
            (polynomialCoefficients[3] * ((r->rayDirection.xCoord * rayStart.yCoord) + (rayStart.xCoord * r->rayDirection.yCoord))) +
            (polynomialCoefficients[4] * ((r->rayDirection.yCoord * rayStart.zCoord) + (rayStart.yCoord * r->rayDirection.zCoord))) +
            (polynomialCoefficients[5] * ((r->rayDirection.zCoord * rayStart.xCoord) + (rayStart.zCoord * r->rayDirection.xCoord))) +
            (polynomialCoefficients[6] * r->rayDirection.xCoord) +
            (polynomialCoefficients[7] * r->rayDirection.yCoord) +
            (polynomialCoefficients[8] * r->rayDirection.zCoord);
        c = (polynomialCoefficients[0] * pow(rayStart.xCoord, 2)) +
            (polynomialCoefficients[1] * pow(rayStart.yCoord, 2)) +
            (polynomialCoefficients[2] * pow(rayStart.zCoord, 2)) +
            (polynomialCoefficients[3] * rayStart.xCoord * rayStart.yCoord) +
            (polynomialCoefficients[4] * rayStart.yCoord * rayStart.zCoord) +
            (polynomialCoefficients[5] * rayStart.xCoord * rayStart.zCoord) +
            (polynomialCoefficients[6] * rayStart.xCoord) +
            (polynomialCoefficients[7] * rayStart.yCoord) +
            (polynomialCoefficients[8] * rayStart.zCoord) +
            polynomialCoefficients[9];
    }

    // Helper: Check if intersection point is within bounds
    bool isIntersectionValid(Point& intersectionPoint) {
        if (objectLength != 0) {
            if ((intersectionPoint.xCoord < objectReferencePoint.xCoord) || (intersectionPoint.xCoord > (objectReferencePoint.xCoord + objectLength)))
                return false;
        }
        if (objectWidth != 0) {
            if ((intersectionPoint.yCoord < objectReferencePoint.yCoord) || (intersectionPoint.yCoord > (objectReferencePoint.yCoord + objectWidth)))
                return false;
        }
        if (objectHeight != 0) {
            if ((intersectionPoint.zCoord < objectReferencePoint.zCoord) || (intersectionPoint.zCoord > (objectReferencePoint.zCoord + objectHeight)))
                return false;
        }
        return true;
    }

    // Helper: Calculate normal at intersection
    void calculateNormalAtIntersection(Point& intersection, Point& normal) {
        normal.xCoord = (2 * polynomialCoefficients[0] * intersection.xCoord) + (polynomialCoefficients[3] * intersection.yCoord) + (polynomialCoefficients[5] * intersection.zCoord) + polynomialCoefficients[6];
        normal.yCoord = (2 * polynomialCoefficients[1] * intersection.yCoord) + (polynomialCoefficients[3] * intersection.xCoord) + (polynomialCoefficients[4] * intersection.yCoord) + polynomialCoefficients[7];
        normal.zCoord = (2 * polynomialCoefficients[2] * intersection.zCoord) + (polynomialCoefficients[4] * intersection.yCoord) + (polynomialCoefficients[5] * intersection.xCoord) + polynomialCoefficients[8];
        normal.normalize();
    }

    // Helper: Process spot lights
    void processSpotLightsForGeneral(vector<PointLight*>& allPointLights, Point& intersectionPoint) {
        for (auto& sl : spotlights) {
            PointLight* pl = sl->pointLight;
            Ray* ray_point_light = new Ray(pl->lightPosition, intersectionPoint);
            double alpha = dot_product(ray_point_light->rayDirection, sl->spotDirection);
            double light_dir_len = sl->spotDirection.length();
            double ray_len = ray_point_light->rayDirection.length();
            alpha = acos(alpha * 1.0 / (light_dir_len * ray_len)) * 180.0 / PI;
            if (alpha <= sl->cutoffAngle) {
                allPointLights.push_back(pl);
            }
        }
    }

    // Helper: Process point lights
    void processPointLightsForGeneral(vector<PointLight*>& allPointLights, Point& intersectionPoint, Point& normal, Ray* r, double* color_in) {
        double epsilon = 0.0000001;
        for (auto& pl : allPointLights) {
            Ray* ray_point_light = new Ray(pl->lightPosition, intersectionPoint);
            double min_t_pl = INT_MAX;
            for (auto& obj : Objects) {
                double* dummyColor = new double[3];
                double t_pl = obj->intersect(ray_point_light, dummyColor, 0);
                if ((t_pl > epsilon) && (t_pl < min_t_pl)) {
                    min_t_pl = t_pl;
                }
            }
            if (min_t_pl != INT_MAX) {
                Point shadow_intersect_point(
                    ray_point_light->rayStart.xCoord + (min_t_pl * ray_point_light->rayDirection.xCoord),
                    ray_point_light->rayStart.yCoord + (min_t_pl * ray_point_light->rayDirection.yCoord),
                    ray_point_light->rayStart.zCoord + (min_t_pl * ray_point_light->rayDirection.zCoord)
                );
                double dis_shadow_ip = sqrt(pow(shadow_intersect_point.xCoord - ray_point_light->rayStart.xCoord, 2) + pow(shadow_intersect_point.yCoord - ray_point_light->rayStart.yCoord, 2) + pow(shadow_intersect_point.zCoord - ray_point_light->rayStart.zCoord, 2));
                double dis_ip = sqrt(pow(intersectionPoint.xCoord - ray_point_light->rayStart.xCoord, 2) + pow(intersectionPoint.yCoord - ray_point_light->rayStart.yCoord, 2) + pow(intersectionPoint.zCoord - ray_point_light->rayStart.zCoord, 2));
                if (dis_shadow_ip >= dis_ip - epsilon) {
                    calculateSpecularDiffuse(normal, ray_point_light, color_in, intersectionPoint, r, pl);
                }
            }
        }
    }

    // Helper: Process reflection
    void processReflectionForGeneral(Point& normal, Ray* r, Point& intersectionPoint, double* color_in, int level) {
        double dot_ray_n = dot_product(normal, r->rayDirection);
        Point reflectedRayDir(
            -(2.0 * dot_ray_n * normal.xCoord) + r->rayDirection.xCoord,
            -(2.0 * dot_ray_n * normal.yCoord) + r->rayDirection.yCoord,
            -(2.0 * dot_ray_n * normal.zCoord) + r->rayDirection.zCoord
        );
        reflectedRayDir.normalize();
        Point reflect_initial(
            intersectionPoint.xCoord + reflectedRayDir.xCoord,
            intersectionPoint.yCoord + reflectedRayDir.yCoord,
            intersectionPoint.zCoord + reflectedRayDir.zCoord
        );
        Ray* reflectedRay = new Ray();
        reflectedRay->setStart(reflect_initial);
        reflectedRay->setDir(reflectedRayDir);
        double* color_ray = new double[3];
        Object* nearest = NULL;
        int near_idx;
        double t_min = INT_MAX;
        int count = 0;
        double epsilon = 0.0000001;
        for (auto& element : Objects) {
            double* color_temp = new double[3];
            double t = element->intersect(reflectedRay, color_temp, 0);
            if ((t < t_min) && (t > epsilon)) {
                t_min = t;
                near_idx = count;
            }
            count += 1;
        }
        if (t_min != INT_MAX) {
            nearest = Objects[near_idx];
            t_min = nearest->intersect(reflectedRay, color_ray, level + 1);
        }
        calculateReflection(color_ray, color_in);
    }

    double intersect(Ray* r, double* color_in, int level)
    {
        double epsilon = 0.000001;
        setLightingColor(color_in);
        Point rayStart(r->rayStart.xCoord - objectReferencePoint.xCoord, r->rayStart.yCoord - objectReferencePoint.yCoord, r->rayStart.zCoord - objectReferencePoint.zCoord);
        double a, b, c;
        calculateQuadraticCoefficients(r, rayStart, a, b, c);
        double root_squared = (b * b) - (4 * a * c);
        if (root_squared < 0)
            return INT_MAX;
        root_squared = sqrt(root_squared);
        double t1 = (-b + root_squared) / (2 * a);
        double t2 = (-b - root_squared) / (2 * a);
        double t = INT_MAX;
        Point min_intersectionPoint;
        if (t1 > 0) {
            Point intersectionPoint(
                r->rayStart.xCoord + (t1 * r->rayDirection.xCoord),
                r->rayStart.yCoord + (t1 * r->rayDirection.yCoord),
                r->rayStart.zCoord + (t1 * r->rayDirection.zCoord)
            );
            if (isIntersectionValid(intersectionPoint)) {
                min_intersectionPoint = intersectionPoint;
                t = min(t, t1);
            }
        }
        if (t2 > 0) {
            Point intersectionPoint(
                r->rayStart.xCoord + (t2 * r->rayDirection.xCoord),
                r->rayStart.yCoord + (t2 * r->rayDirection.yCoord),
                r->rayStart.zCoord + (t2 * r->rayDirection.zCoord)
            );
            if (isIntersectionValid(intersectionPoint)) {
                if (t != -1.0) {
                    t = min(t, t2);
                    if (t == t2) {
                        min_intersectionPoint = intersectionPoint;
                    }
                } else {
                    min_intersectionPoint = intersectionPoint;
                    t = t2;
                }
            }
        }
        if (level == 0)
            return t;
        Point intersectionPoint = min_intersectionPoint;
        Point normal;
        calculateNormalAtIntersection(min_intersectionPoint, normal);
        vector<PointLight*> allPointLights = pointLights;
        processSpotLightsForGeneral(allPointLights, intersectionPoint);
        processPointLightsForGeneral(allPointLights, intersectionPoint, normal, r, color_in);
        if (level >= level_recursion)
            return t;
        processReflectionForGeneral(normal, r, intersectionPoint, color_in, level);
        return t;
    }
};

// Floor class
class Floor : public Object
{
public:
    double floorWidth;

    Floor(double floorWidth, double tileWidth)
    {
        this->floorWidth = floorWidth;
        objectReferencePoint.xCoord = -floorWidth / 2.0;
        objectReferencePoint.yCoord = -floorWidth / 2.0;
        objectReferencePoint.zCoord = 0;
        objectLength = tileWidth;
        setCoefficients(new double[4]{0.4, 0.2, 0.2, 0.2});
        setShine(0.5);
        setColor(new double[3]{1, 1, 1});
    }

    void print_object()
    {
        cout << "Floor : " << endl;
        cout << "FloorWidth: " << floorWidth << endl;
        cout << "TileWidth : " << objectLength << endl;
        Object::print_object();
    }

    // Helper: Draw a single tile
    void drawTile(Point& curPoint, double* color) {
        glColor3f(color[0], color[1], color[2]);
        glBegin(GL_QUADS);
        {
            glVertex3f(curPoint.xCoord, curPoint.yCoord, 0);
            glVertex3f(curPoint.xCoord + objectLength, curPoint.yCoord, 0);
            glVertex3f(curPoint.xCoord + objectLength, curPoint.yCoord + objectLength, 0);
            glVertex3f(curPoint.xCoord, curPoint.yCoord + objectLength, 0);
        }
        glEnd();
    }

    // Helper: Toggle color for checkerboard
    void toggleColor(double* color) {
        color[0] = 1 - color[0];
        color[1] = 1 - color[1];
        color[2] = 1 - color[2];
    }

    void draw()
    {
        Point curPoint(0, 0, 0);
        double rowColor[3] = {1, 1, 1};
        while (curPoint.yCoord <= floorWidth)
        {
            double colColor[3] = {rowColor[0], rowColor[1], rowColor[2]};
            while (curPoint.xCoord <= floorWidth)
            {
                drawTile(curPoint, colColor);
                curPoint.xCoord = curPoint.xCoord + objectLength;
                toggleColor(colColor);
            }
            curPoint.yCoord = curPoint.yCoord + objectLength;
            curPoint.xCoord = 0;
            toggleColor(rowColor);
        }
    }

    // Helper: Check if intersection is within floor bounds
    bool isWithinFloorBounds(Point& intersectionPoint) {
        return (intersectionPoint.yCoord > objectReferencePoint.yCoord) &&
               (intersectionPoint.yCoord < (objectReferencePoint.yCoord + floorWidth)) &&
               (intersectionPoint.xCoord > objectReferencePoint.xCoord) &&
               (intersectionPoint.xCoord < (objectReferencePoint.xCoord + floorWidth));
    }

    // Helper: Calculate intersection t for ray-plane
    double calculateIntersectionT(Ray* r, Point& normal) {
        double dot_d_n = dot_product(normal, r->rayDirection);
        if (dot_d_n == 0)
            return INT_MAX;
        double dot_p_n = objectReferencePoint.zCoord * r->rayDirection.zCoord;
        double dot_r0_n = dot_product(r->rayStart, normal);
        return (dot_p_n - dot_r0_n) * 1.0 / dot_d_n;
    }

    // Helper: Process spot lights
    void processSpotLightsForFloor(vector<PointLight*>& allPointLights, Point& intersectionPoint) {
        for (auto& sl : spotlights)
        {
            PointLight* pl = sl->pointLight;
            Ray* ray_point_light = new Ray(pl->lightPosition, intersectionPoint);
            double alpha = dot_product(ray_point_light->rayDirection, sl->spotDirection);
            double light_dir_len = sl->spotDirection.length();
            double ray_len = ray_point_light->rayDirection.length();
            alpha = acos(alpha * 1.0 / (light_dir_len * ray_len)) * 180.0 / PI;
            if (alpha <= sl->cutoffAngle)
            {
                allPointLights.push_back(pl);
            }
            delete ray_point_light; // Fix memory leak
        }
    }

    // Helper: Process point lights
    void processPointLightsForFloor(vector<PointLight*>& allPointLights, Point& intersectionPoint, Point& normal, Ray* r, double* color_in) {
        double epsilon = 0.000001;
        for (auto& pl : allPointLights)
        {
            Ray* ray_point_light = new Ray(pl->lightPosition, intersectionPoint);
            if (ray_point_light->rayStart.zCoord < 0.0)
            {
                normal.zCoord = -1.0;
            }
            double min_t_pl = INT_MAX;
            for (auto& obj : Objects)
            {
                double* dummyColor = new double[3];
                double t_pl = obj->intersect(ray_point_light, dummyColor, 0);
                if ((t_pl > epsilon) && (t_pl < min_t_pl))
                {
                    min_t_pl = t_pl;
                }
                delete[] dummyColor; // Fix memory leak
            }
            if (min_t_pl != INT_MAX)
            {
                Point shadow_intersect_point(
                    ray_point_light->rayStart.xCoord + (min_t_pl * ray_point_light->rayDirection.xCoord),
                    ray_point_light->rayStart.yCoord + (min_t_pl * ray_point_light->rayDirection.yCoord),
                    ray_point_light->rayStart.zCoord + (min_t_pl * ray_point_light->rayDirection.zCoord)
                );
                double dis_shadow_ip = sqrt(pow(shadow_intersect_point.xCoord - ray_point_light->rayStart.xCoord, 2) + pow(shadow_intersect_point.yCoord - ray_point_light->rayStart.yCoord, 2) + pow(shadow_intersect_point.zCoord - ray_point_light->rayStart.zCoord, 2));
                double dis_ip = sqrt(pow(intersectionPoint.xCoord - ray_point_light->rayStart.xCoord, 2) + pow(intersectionPoint.yCoord - ray_point_light->rayStart.yCoord, 2) + pow(intersectionPoint.zCoord - ray_point_light->rayStart.zCoord, 2));
                if (dis_shadow_ip >= dis_ip - epsilon)
                {
                    calculateSpecularDiffuse(normal, ray_point_light, color_in, intersectionPoint, r, pl);
                }
            }
            delete ray_point_light; // Fix memory leak
        }
    }

    // Helper: Process reflection
    void processReflectionForFloor(Point& normal, Ray* r, Point& intersectionPoint, double* color_in, int level) {
        double dot_ray_n = dot_product(normal, r->rayDirection);
        Point reflectedRayDir(
            -(2.0 * dot_ray_n * normal.xCoord) + r->rayDirection.xCoord,
            -(2.0 * dot_ray_n * normal.yCoord) + r->rayDirection.yCoord,
            -(2.0 * dot_ray_n * normal.zCoord) + r->rayDirection.zCoord
        );
        reflectedRayDir.normalize();
        Point reflect_initial(
            intersectionPoint.xCoord + reflectedRayDir.xCoord,
            intersectionPoint.yCoord + reflectedRayDir.yCoord,
            intersectionPoint.zCoord + reflectedRayDir.zCoord
        );
        Ray* reflectedRay = new Ray();
        reflectedRay->setStart(reflect_initial);
        reflectedRay->setDir(reflectedRayDir);
        double* color_ray = new double[3];
        Object* nearest = NULL;
        int near_idx;
        double t_min = INT_MAX;
        int count = 0;
        double epsilon = 0.000001;
        for (auto& element : Objects)
        {
            double* color_temp = new double[3];
            double t = element->intersect(reflectedRay, color_temp, 0);
            if ((t < t_min) && (t > epsilon))
            {
                t_min = t;
                near_idx = count;
            }
            delete[] color_temp; // Fix memory leak
            count += 1;
        }
        if (t_min != INT_MAX)
        {
            nearest = Objects[near_idx];
            t_min = nearest->intersect(reflectedRay, color_ray, level + 1);
        }
        calculateReflection(color_ray, color_in);
        delete reflectedRay; // Fix memory leak
        delete[] color_ray; // Fix memory leak
    }

    double intersect(Ray* r, double* color_in, int level)
    {
        Point normal(0.0, 0.0, 1.0);
        if (r->rayStart.zCoord < 0)
        {
            normal.zCoord = -normal.zCoord;
        }
        double t = calculateIntersectionT(r, normal);
        if ((t < 0.0) || (t > INT_MAX))
        {
            return INT_MAX;
        }
        Point intersectionPoint(
            r->rayStart.xCoord + (t * r->rayDirection.xCoord),
            r->rayStart.yCoord + (t * r->rayDirection.yCoord),
            r->rayStart.zCoord + (t * r->rayDirection.zCoord)
        );
        double temp_color[3] = {1, 1, 1};
        if (isWithinFloorBounds(intersectionPoint))
        {
            // Per-tile texture mapping: each tile shows the full texture
            double tileU = fmod(intersectionPoint.xCoord - objectReferencePoint.xCoord, objectLength) / objectLength;
            double tileV = fmod(intersectionPoint.yCoord - objectReferencePoint.yCoord, objectLength) / objectLength;
            if (tileU < 0) tileU += 1.0;
            if (tileV < 0) tileV += 1.0;
            int tile_x = (int)((intersectionPoint.xCoord - objectReferencePoint.xCoord) / objectLength);
            int tile_y = (int)((intersectionPoint.yCoord - objectReferencePoint.yCoord) / objectLength);
            if (!useTextureMode || !floorTexture.data) {
                if ((tile_x + tile_y) % 2 == 0) {
                    temp_color[0] = temp_color[1] = temp_color[2] = 1.0;
                } else {
                    temp_color[0] = temp_color[1] = temp_color[2] = 0.0;
                }
            } else {
                sampleTexture(floorTexture, tileU, tileV, temp_color);
            }
            if (level == 0)
                return t;
            objectColor[0] = temp_color[0];
            objectColor[1] = temp_color[1];
            objectColor[2] = temp_color[2];
            setLightingColor(color_in);
            Point intersect_point = intersectionPoint;
            vector<PointLight*> allPointLights = pointLights;
            processSpotLightsForFloor(allPointLights, intersect_point);
            processPointLightsForFloor(allPointLights, intersect_point, normal, r, color_in);
            if (level >= level_recursion)
                return t;
            processReflectionForFloor(normal, r, intersect_point, color_in, level);
            return t;
        }
        return INT_MAX;
    }
};
