#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

#define PI (2 * acos(0.0))
#define DEG2RAD(deg) (deg * PI / 180)

// Camera parameters
struct Point {
    double x, y, z;
    Point(double _x=0, double _y=0, double _z=0) : x(_x), y(_y), z(_z) {}
    
    Point operator+(const Point& p) const { return Point(x + p.x, y + p.y, z + p.z); }
    Point operator-(const Point& p) const { return Point(x - p.x, y - p.y, z - p.z); }
    Point operator*(double s) const { return Point(x * s, y * s, z * s); }
};

// Camera position and vectors
Point pos(150, 150, 150);     // Camera position moved further out
Point u(0, 0, 1);           // Up vector
Point r(-1/sqrt(2), -1/sqrt(2), 0);          // Right vector
Point l(-1/sqrt(2), -1/sqrt(2), -1/sqrt(3));          // Look vector pointing at cube center

// Constants
const double CAMERA_MOVE_SPEED = 5.0;
const double CAMERA_ROTATION_ANGLE = 0.05;  // radians
const float LENGTH = 48.0f;  // Size of the cube

// Ball properties
struct Ball {
    Point pos;
    Point vel;
    Point rot;  // Rotation angles
    float radius;
    bool isResting;
} ball;

// Physics properties
const float GRAVITY = -9.8f;
const float RESTITUTION = 0.75f;
const float MIN_VELOCITY = 0.1f;
const float TIME_STEP = 0.016f;  // ~60 FPS
float initialSpeed = 5.0f;
bool simulation = false;
bool showVelocityArrow = true;

// Function to normalize a vector
Point normalize(Point p) {
    double mag = sqrt(p.x*p.x + p.y*p.y + p.z*p.z);
    if (mag != 0) {
        p.x /= mag;
        p.y /= mag;
        p.z /= mag;
    }
    return p;
}

// Function to calculate cross product
Point cross(Point a, Point b) {
    return Point(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

// Rotate vectors around an axis
void rotate(Point& v, Point axis, double angle) {
    axis = normalize(axis);
    double c = cos(angle);
    double s = sin(angle);
    double t = 1.0 - c;
    
    double temp_x = v.x * (t * axis.x * axis.x + c) +
                   v.y * (t * axis.x * axis.y - s * axis.z) +
                   v.z * (t * axis.x * axis.z + s * axis.y);
                   
    double temp_y = v.x * (t * axis.x * axis.y + s * axis.z) +
                   v.y * (t * axis.y * axis.y + c) +
                   v.z * (t * axis.y * axis.z - s * axis.x);
                   
    double temp_z = v.x * (t * axis.x * axis.z - s * axis.y) +
                   v.y * (t * axis.y * axis.z + s * axis.x) +
                   v.z * (t * axis.z * axis.z + c);
                   
    v.x = temp_x;
    v.y = temp_y;
    v.z = temp_z;
}

// Function to draw checkered floor
void drawCheckeredFloor() {
    const int TILES = 8;
    const float TILE_SIZE = LENGTH / TILES;
    
    glBegin(GL_QUADS);
    for(int i = 0; i < TILES; i++) {
        for(int j = 0; j < TILES; j++) {
            if((i + j) % 2 == 0)
                glColor3f(1.0f, 1.0f, 1.0f);
            else
                glColor3f(0.3f, 0.3f, 0.3f);
                
            float x1 = -LENGTH/2 + i * TILE_SIZE;
            float x2 = x1 + TILE_SIZE;
            float z1 = -LENGTH/2 + j * TILE_SIZE;
            float z2 = z1 + TILE_SIZE;
            
            glVertex3f(x1, -LENGTH/2, z1);
            glVertex3f(x2, -LENGTH/2, z1);
            glVertex3f(x2, -LENGTH/2, z2);
            glVertex3f(x1, -LENGTH/2, z2);
        }
    }
    glEnd();
}

// Function to draw velocity arrow
void drawVelocityArrow() {
    if (!showVelocityArrow) return;
    
    float speed = sqrt(ball.vel.x * ball.vel.x + 
                      ball.vel.y * ball.vel.y + 
                      ball.vel.z * ball.vel.z);
    if (speed < 0.1f) return;

    Point dir = normalize(ball.vel);
    
    glPushMatrix();
    glTranslatef(ball.pos.x, ball.pos.y, ball.pos.z);
    
    glColor3f(0.0f, 0.0f, 1.0f);  // Blue arrow
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(dir.x * ball.radius * 4, 
               dir.y * ball.radius * 4, 
               dir.z * ball.radius * 4);
    glEnd();
    
    glPopMatrix();
}

// Function to draw the ball
void drawBall() {
    glPushMatrix();
    glTranslatef(ball.pos.x, ball.pos.y, ball.pos.z);
    
    // Apply rotation
    glRotatef(ball.rot.x, 1, 0, 0);
    glRotatef(ball.rot.y, 0, 1, 0);
    glRotatef(ball.rot.z, 0, 0, 1);
    
    // Draw the ball with some surface detail for visible rotation
    glColor3f(0.0f, 0.8f, 0.0f);  // Green ball
    glutSolidSphere(ball.radius, 20, 20);
    
    // Add a stripe for rotation visibility
    glColor3f(1.0f, 1.0f, 1.0f);
    glRotatef(90, 1, 0, 0);
    glutSolidTorus(ball.radius/8, ball.radius, 20, 20);
    
    glPopMatrix();
    
    if (showVelocityArrow) {
        drawVelocityArrow();
    }
}

// Function to draw colored walls
void drawWalls() {
    float L = LENGTH;  // Wall length
    float H = LENGTH;  // Wall height

    glBegin(GL_QUADS);
    // Front wall (red)
    glColor3f(0.8f, 0.2f, 0.2f);
    glVertex3f(-L/2, -L/2, -L/2);
    glVertex3f(L/2, -L/2, -L/2);
    glVertex3f(L/2, L/2, -L/2);
    glVertex3f(-L/2, L/2, -L/2);

    // Back wall (blue)
    glColor3f(0.2f, 0.2f, 0.8f);
    glVertex3f(-L/2, -L/2, L/2);
    glVertex3f(-L/2, L/2, L/2);
    glVertex3f(L/2, L/2, L/2);
    glVertex3f(L/2, -L/2, L/2);

    // Left wall (green)
    glColor3f(0.2f, 0.8f, 0.2f);
    glVertex3f(-L/2, -L/2, -L/2);
    glVertex3f(-L/2, L/2, -L/2);
    glVertex3f(-L/2, L/2, L/2);
    glVertex3f(-L/2, -L/2, L/2);

    // Right wall (yellow)
    glColor3f(0.8f, 0.8f, 0.2f);
    glVertex3f(L/2, -L/2, -L/2);
    glVertex3f(L/2, L/2, -L/2);
    glVertex3f(L/2, L/2, L/2);
    glVertex3f(L/2, -L/2, L/2);

    // Top wall (purple)
    glColor3f(0.8f, 0.2f, 0.8f);
    glVertex3f(-L/2, L/2, -L/2);
    glVertex3f(L/2, L/2, -L/2);
    glVertex3f(L/2, L/2, L/2);
    glVertex3f(-L/2, L/2, L/2);
    glEnd();

    // Draw checkered floor
    drawCheckeredFloor();
}

// Reset ball with random position and upward velocity
void resetBall() {
    ball.pos = Point(
        (float)(rand() % 100 - 50) * LENGTH/200,  // Random position
        (float)(rand() % 100 - 50) * LENGTH/200,
        -LENGTH/4  // Start at quarter height
    );
    
    // Random direction but ensuring upward movement
    float angle = (float)(rand() % 360) * PI / 180.0f;
    ball.vel = Point(
        cos(angle) * initialSpeed * 0.5f,
        sin(angle) * initialSpeed * 0.5f,
        initialSpeed
    );
    
    ball.rot = Point(0, 0, 0);
    ball.isResting = false;
    simulation = false;
}

// Update ball physics
void updateBall() {
    if (!simulation || ball.isResting) return;

    // Update velocity with gravity
    ball.vel.z += GRAVITY * TIME_STEP;
    
    // Store old position for rotation calculation
    Point oldPos = ball.pos;
    
    // Update position
    ball.pos.x += ball.vel.x * TIME_STEP;
    ball.pos.y += ball.vel.y * TIME_STEP;
    ball.pos.z += ball.vel.z * TIME_STEP;
    
    // Calculate displacement for rotation
    Point displacement = ball.pos - oldPos;
    float speed = sqrt(displacement.x * displacement.x + 
                      displacement.y * displacement.y + 
                      displacement.z * displacement.z);
    
    // Update rotation based on displacement
    if (speed > 0.001f) {
        ball.rot.x += displacement.y * 180.0f / (PI * ball.radius);
        ball.rot.y -= displacement.x * 180.0f / (PI * ball.radius);
    }

    // Collision detection and response
    float L = LENGTH/2;
    if (ball.pos.x - ball.radius < -L) {
        ball.pos.x = -L + ball.radius;
        ball.vel.x = -ball.vel.x * RESTITUTION;
    }
    if (ball.pos.x + ball.radius > L) {
        ball.pos.x = L - ball.radius;
        ball.vel.x = -ball.vel.x * RESTITUTION;
    }
    
    if (ball.pos.y - ball.radius < -L) {
        ball.pos.y = -L + ball.radius;
        ball.vel.y = -ball.vel.y * RESTITUTION;
    }
    if (ball.pos.y + ball.radius > L) {
        ball.pos.y = L - ball.radius;
        ball.vel.y = -ball.vel.y * RESTITUTION;
    }
    
    // Floor and ceiling collisions
    if (ball.pos.z - ball.radius < -L) {
        ball.pos.z = -L + ball.radius;
        if (fabs(ball.vel.z) < MIN_VELOCITY) {
            ball.vel.z = 0;
            ball.isResting = true;
        } else {
            ball.vel.z = -ball.vel.z * RESTITUTION;
        }
    }
    if (ball.pos.z + ball.radius > L) {
        ball.pos.z = L - ball.radius;
        ball.vel.z = -ball.vel.z * RESTITUTION;
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    gluLookAt(pos.x, pos.y, pos.z,
              pos.x + l.x, pos.y + l.y, pos.z + l.z,
              u.x, u.y, u.z);

    drawWalls();
    drawBall();

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        // Camera rotation controls
        case '1':  // Look left (yaw)
            rotate(l, u, CAMERA_ROTATION_ANGLE);
            r = cross(l, u);
            break;
        case '2':  // Look right (yaw)
            rotate(l, u, -CAMERA_ROTATION_ANGLE);
            r = cross(l, u);
            break;
        case '3':  // Look up (pitch)
            rotate(l, r, CAMERA_ROTATION_ANGLE);
            u = cross(r, l);
            break;
        case '4':  // Look down (pitch)
            rotate(l, r, -CAMERA_ROTATION_ANGLE);
            u = cross(r, l);
            break;
        case '5':  // Tilt clockwise (roll)
            rotate(u, l, -CAMERA_ROTATION_ANGLE);
            r = cross(l, u);
            break;
        case '6':  // Tilt counterclockwise (roll)
            rotate(u, l, CAMERA_ROTATION_ANGLE);
            r = cross(l, u);
            break;
        case 'w':  // Crane up
            pos.z += CAMERA_MOVE_SPEED;
            break;
        case 's':  // Crane down
            pos.z -= CAMERA_MOVE_SPEED;
            break;
        // Ball controls
        case ' ':  // Toggle simulation
            simulation = !simulation;
            break;
        case 'r':  // Reset ball
            resetBall();
            break;
        case 'v':  // Toggle velocity arrow
            showVelocityArrow = !showVelocityArrow;
            break;
        case '+':  // Increase initial speed
            if (!simulation) initialSpeed += 1.0f;
            break;
        case '-':  // Decrease initial speed
            if (!simulation && initialSpeed > 1.0f) initialSpeed -= 1.0f;
            break;
        case 27:   // ESC key
            exit(0);
            break;
    }
    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    switch(key) {
        case GLUT_KEY_UP:     // Move forward
            pos.x += l.x * CAMERA_MOVE_SPEED;
            pos.y += l.y * CAMERA_MOVE_SPEED;
            pos.z += l.z * CAMERA_MOVE_SPEED;
            break;
        case GLUT_KEY_DOWN:   // Move backward
            pos.x -= l.x * CAMERA_MOVE_SPEED;
            pos.y -= l.y * CAMERA_MOVE_SPEED;
            pos.z -= l.z * CAMERA_MOVE_SPEED;
            break;
        case GLUT_KEY_LEFT:   // Move left
            pos.x -= r.x * CAMERA_MOVE_SPEED;
            pos.y -= r.y * CAMERA_MOVE_SPEED;
            pos.z -= r.z * CAMERA_MOVE_SPEED;
            break;
        case GLUT_KEY_RIGHT:  // Move right
            pos.x += r.x * CAMERA_MOVE_SPEED;
            pos.y += r.y * CAMERA_MOVE_SPEED;
            pos.z += r.z * CAMERA_MOVE_SPEED;
            break;
        case GLUT_KEY_PAGE_UP:    // Move upward
            pos.x += u.x * CAMERA_MOVE_SPEED;
            pos.y += u.y * CAMERA_MOVE_SPEED;
            pos.z += u.z * CAMERA_MOVE_SPEED;
            break;
        case GLUT_KEY_PAGE_DOWN:  // Move downward
            pos.x -= u.x * CAMERA_MOVE_SPEED;
            pos.y -= u.y * CAMERA_MOVE_SPEED;
            pos.z -= u.z * CAMERA_MOVE_SPEED;
            break;
    }
    glutPostRedisplay();
}

void idle() {
    updateBall();
    glutPostRedisplay();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    float ratio = float(w) / float(h);
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, ratio, 1, 1000);
    glMatrixMode(GL_MODELVIEW);
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);  // Dark gray background
    
    // Initialize ball properties
    ball.radius = LENGTH/40;  // Reasonable size relative to cube
    resetBall();
    
    // Initialize random seed
    srand(time(NULL));

    // Initialize camera vectors
    l = normalize(l);  // Look vector
    r = normalize(r);  // Right vector
    u = normalize(u);  // Up vector
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(800, 800);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutCreateWindow("3D Camera Control and Bouncing Ball Demo");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutIdleFunc(idle);

    std::cout << "Camera Controls:\n"
              << "Arrow keys: Move forward/backward/left/right\n"
              << "Page Up/Down: Move up/down\n"
              << "1/2: Yaw left/right\n"
              << "3/4: Pitch up/down\n"
              << "5/6: Roll clockwise/counterclockwise\n"
              << "w/s: Crane up/down\n\n"
              << "Ball Controls:\n"
              << "Space: Toggle simulation\n"
              << "r: Reset ball position\n"
              << "+/-: Adjust initial speed\n"
              << "v: Toggle velocity arrow\n"
              << "ESC: Exit\n\n";

    glutMainLoop();
    return 0;
}
