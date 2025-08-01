#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

#define PI (2 * acos(0.0))
#define DEG2RAD(deg) (deg * PI / 180)

struct Point
{
    double x, y, z;
    Point(double _x = 0, double _y = 0, double _z = 0) : x(_x), y(_y), z(_z) {}
    Point operator+(const Point &p) const { return Point(x + p.x, y + p.y, z + p.z); }
    Point operator-(const Point &p) const { return Point(x - p.x, y - p.y, z - p.z); }
    Point operator*(double s) const { return Point(x * s, y * s, z * s); }
};

Point pos(150, 150, 150);
Point u(0, 0, 1);
Point r(-1 / sqrt(2), -1 / sqrt(2), 0);
Point l(-1 / sqrt(2), -1 / sqrt(2), -1 / sqrt(3));

const double CAMERA_SPEED = 2.0;
const double ROTATE_ANGLE = 0.03;
const float CUBE_SIZE = 100.0f;
const float BALL_RADIUS = 4.0f;

struct Ball
{
    Point pos, vel, spin;
    bool active = false;
    float initialSpeed = 20.0f;
};

Ball ball;
bool showArrow = true;
float restitution = 0.75f;
float gravity = -9.8f;
float timestep = 0.016f;

void rotate(Point &v, Point &axis, double angle)
{
    Point temp = v;
    v = temp * cos(angle) + axis * (1 - cos(angle)) * (temp.x * axis.x + temp.y * axis.y + temp.z * axis.z) + Point(axis.y * temp.z - axis.z * temp.y, axis.z * temp.x - axis.x * temp.z, axis.x * temp.y - axis.y * temp.x) * sin(angle);
}

void drawArrow(Point base, Point dir)
{
    if (!showArrow)
        return;
    Point end = base + dir * 5;
    glColor3f(1, 1, 0);
    glBegin(GL_LINES);
    glVertex3f(base.x, base.y, base.z);
    glVertex3f(end.x, end.y, end.z);
    glEnd();
}

void drawCheckeredFloor()
{
    int tiles = 10;
    float size = CUBE_SIZE / tiles;
    for (int i = 0; i < tiles; i++)
    {
        for (int j = 0; j < tiles; j++)
        {
            if ((i + j) % 2 == 0)
                glColor3f(1, 1, 1);
            else
                glColor3f(0, 0, 0);
            glBegin(GL_QUADS);
            glVertex3f(i * size, j * size, 0);
            glVertex3f((i + 1) * size, j * size, 0);
            glVertex3f((i + 1) * size, (j + 1) * size, 0);
            glVertex3f(i * size, (j + 1) * size, 0);
            glEnd();
        }
    }
}

void drawCube()
{
    float s = CUBE_SIZE;
    float x = 0, y = 0, z = 0;
    drawCheckeredFloor();
    glBegin(GL_QUADS);
    glColor3f(0.7, 0.2, 0.2);
    glVertex3f(x, y, z);
    glVertex3f(x, y, z + s);
    glVertex3f(x, y + s, z + s);
    glVertex3f(x, y + s, z);
    glColor3f(0.2, 0.7, 0.2);
    glVertex3f(x + s, y, z);
    glVertex3f(x + s, y, z + s);
    glVertex3f(x + s, y + s, z + s);
    glVertex3f(x + s, y + s, z);
    glColor3f(0.2, 0.2, 0.7);
    glVertex3f(x, y, z);
    glVertex3f(x + s, y, z);
    glVertex3f(x + s, y, z + s);
    glVertex3f(x, y, z + s);
    glColor3f(0.6, 0.6, 0.2);
    glVertex3f(x, y + s, z);
    glVertex3f(x + s, y + s, z);
    glVertex3f(x + s, y + s, z + s);
    glVertex3f(x, y + s, z + s);
    glColor3f(0.4, 0.4, 0.4);
    glVertex3f(x, y, z + s);
    glVertex3f(x + s, y, z + s);
    glVertex3f(x + s, y + s, z + s);
    glVertex3f(x, y + s, z + s);
    glEnd();
}

void drawBall()
{
    glPushMatrix();
    glTranslatef(ball.pos.x, ball.pos.y, ball.pos.z);
    glRotatef(ball.spin.x, 1, 0, 0);
    glRotatef(ball.spin.y, 0, 1, 0);
    glRotatef(ball.spin.z, 0, 0, 1);
    glColor3f(0.2, 0.4, 0.9);
    glutSolidSphere(BALL_RADIUS, 20, 20);
    glPopMatrix();
    drawArrow(ball.pos, ball.vel);
}

void updateBall()
{
    if (!ball.active)
        return;
    ball.vel.z += gravity * timestep;
    ball.pos = ball.pos + ball.vel * timestep;
    ball.spin = ball.vel * 1.5; // simple spin based on movement

    // Collisions with all 6 walls
    if (ball.pos.z - BALL_RADIUS < 0)
    {
        ball.pos.z = BALL_RADIUS;
        if (fabs(ball.vel.z) < 1.0)
        {
            ball.vel = Point(0, 0, 0);
            ball.active = false;
            return;
        }
        ball.vel.z *= -restitution;
    }
    if (ball.pos.z + BALL_RADIUS > CUBE_SIZE)
    {
        ball.pos.z = CUBE_SIZE - BALL_RADIUS;
        ball.vel.z *= -restitution;
    }
    if (ball.pos.x - BALL_RADIUS < 0)
    {
        ball.pos.x = BALL_RADIUS;
        ball.vel.x *= -restitution;
    }
    if (ball.pos.x + BALL_RADIUS > CUBE_SIZE)
    {
        ball.pos.x = CUBE_SIZE - BALL_RADIUS;
        ball.vel.x *= -restitution;
    }
    if (ball.pos.y - BALL_RADIUS < 0)
    {
        ball.pos.y = BALL_RADIUS;
        ball.vel.y *= -restitution;
    }
    if (ball.pos.y + BALL_RADIUS > CUBE_SIZE)
    {
        ball.pos.y = CUBE_SIZE - BALL_RADIUS;
        ball.vel.y *= -restitution;
    }
}

void specialKeyListener(int key, int x, int y)
{
    if (key == GLUT_KEY_UP)
        pos = pos + l * CAMERA_SPEED;
    if (key == GLUT_KEY_DOWN)
        pos = pos - l * CAMERA_SPEED;
    if (key == GLUT_KEY_LEFT)
        pos = pos - r * CAMERA_SPEED;
    if (key == GLUT_KEY_RIGHT)
        pos = pos + r * CAMERA_SPEED;
    if (key == GLUT_KEY_PAGE_UP)
        pos = pos + u * CAMERA_SPEED;
    if (key == GLUT_KEY_PAGE_DOWN)
        pos = pos - u * CAMERA_SPEED;
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(pos.x, pos.y, pos.z, pos.x + l.x, pos.y + l.y, pos.z + l.z, u.x, u.y, u.z);

    drawCube();
    drawBall();

    glutSwapBuffers();
}

void animate()
{
    updateBall();
    glutPostRedisplay();
}

void init()
{
    glClearColor(0, 0, 0, 0);
    glEnable(GL_DEPTH_TEST);
    ball.pos = Point(CUBE_SIZE / 2, CUBE_SIZE / 2, BALL_RADIUS + 5);
    ball.vel = Point(0, 0, ball.initialSpeed);
    srand(time(0));
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(80, (double)w / h, 1, 1000);
}

void keyboardListener(unsigned char key, int x, int y)
{
    if (key == ' ')
        ball.active = !ball.active;
    if (key == 'r')
    {
        float vx = ((rand() % 20) - 10) / 5.0f;
        float vy = ((rand() % 20) - 10) / 5.0f;
        ball.pos = Point(CUBE_SIZE / 2, CUBE_SIZE / 2, BALL_RADIUS + 5);
        ball.vel = Point(vx, vy, ball.initialSpeed);
        ball.active = false;
    }
    if (key == '+' && !ball.active)
    {
        ball.initialSpeed += 2;
        printf("Initial speed: %.2f\n", ball.initialSpeed);
    }
    if (key == '-' && !ball.active)
    {
        ball.initialSpeed -= 2;
        if (ball.initialSpeed < 0)
            ball.initialSpeed = 0;
        printf("Initial speed: %.2f\n", ball.initialSpeed);
    }
    if (key == 'v')
        showArrow = !showArrow;

    if (key == '1')
        rotate(l, u, -ROTATE_ANGLE), rotate(r, u, -ROTATE_ANGLE);
    if (key == '2')
        rotate(l, u, ROTATE_ANGLE), rotate(r, u, ROTATE_ANGLE);
    if (key == '3')
        rotate(l, r, -ROTATE_ANGLE), rotate(u, r, -ROTATE_ANGLE);
    if (key == '4')
        rotate(l, r, ROTATE_ANGLE), rotate(u, r, ROTATE_ANGLE);
    if (key == '5')
        rotate(r, l, -ROTATE_ANGLE), rotate(u, l, -ROTATE_ANGLE);
    if (key == '6')
        rotate(r, l, ROTATE_ANGLE), rotate(u, l, ROTATE_ANGLE);
    if (key == 'w')
        pos = pos + u * CAMERA_SPEED;
    if (key == 's')
        pos = pos - u * CAMERA_SPEED;
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Task 1 + Task 3: Camera + Bouncing Ball in Full Cube");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboardListener);
    glutSpecialFunc(specialKeyListener);
    glutIdleFunc(animate);
    glutReshapeFunc(reshape);
    glutMainLoop();
    return 0;
}
