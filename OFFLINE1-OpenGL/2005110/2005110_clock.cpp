#include <GL/glut.h>
#include <cmath>
#include <chrono>

#define PI 3.14159265358979323846

void drawCircle(float radius) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; ++i) {
        float angle = i * PI / 180;
        glVertex2f(radius * cos(angle), radius * sin(angle));
    }
    glEnd();
}

void drawTicks() {
    for (int i = 0; i < 60; ++i) {
        float angle = i * 6 * PI / 180;
        float xStart = cos(angle) * 0.9;
        float yStart = sin(angle) * 0.9;
        float xEnd = cos(angle) * (i % 5 == 0 ? 0.8 : 0.85);
        float yEnd = sin(angle) * (i % 5 == 0 ? 0.8 : 0.85);
        glLineWidth(i % 5 == 0 ? 3 : 1);
        glBegin(GL_LINES);
        glVertex2f(xStart, yStart);
        glVertex2f(xEnd, yEnd);
        glEnd();
    }
}

void drawHand(float length, float angleDeg, float width, float r, float g, float b, bool drawTipSquare = false) {
    float angleRad = (90 - angleDeg) * PI / 180.0; 
    float x = length * cos(angleRad);
    float y = length * sin(angleRad);

    glColor3f(r, g, b);
    glLineWidth(width);
    glBegin(GL_LINES);
    glVertex2f(0, 0);
    glVertex2f(x, y);
    glEnd();

    if (drawTipSquare) {
        glBegin(GL_QUADS);
        glVertex2f(x - 0.02, y - 0.02);
        glVertex2f(x + 0.02, y - 0.02);
        glVertex2f(x + 0.02, y + 0.02);
        glVertex2f(x - 0.02, y + 0.02);
        glEnd();
    }
}


void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // Get very accurate time
    using namespace std::chrono;
    auto now = system_clock::now();
    auto now_ms = time_point_cast<milliseconds>(now);
    auto epoch = now_ms.time_since_epoch();
    long long totalMilliseconds = epoch.count();

    int totalSeconds = totalMilliseconds / 1000;
    int milliseconds = totalMilliseconds % 1000;

    time_t tt = system_clock::to_time_t(now);
    struct tm* localTime = localtime(&tt);

    int hours = localTime->tm_hour % 12;
    int minutes = localTime->tm_min;
    int seconds = localTime->tm_sec;

    // Now, calculate precise seconds, minutes, hours
    double preciseSecond = seconds + milliseconds / 1000.0;
    double preciseMinute = minutes + preciseSecond / 60.0;
    double preciseHour = hours + preciseMinute / 60.0;

    // Now calculate angles
    float secAngle = -preciseSecond * 6.0f;
    float minAngle = -preciseMinute * 6.0f;
    float hourAngle = -preciseHour * 30.0f;

    glColor3f(1, 1, 1);
    drawCircle(1.0f); // Clock border

    drawTicks(); // Hour and minute ticks

    drawHand(0.5f, -hourAngle, 5, 1, 1, 1);               // Hour hand (short and thick)
    drawHand(0.7f, -minAngle, 3, 1, 1, 1);                // Minute hand (longer and thinner)
    drawHand(0.85f, -secAngle, 1, 1, 0, 0, true);         // Second hand (red, with square tip)

    glutSwapBuffers();
}

void update(int value) {
    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // ~60 FPS
}

void init() {
    glClearColor(0.0, 0.0, 0.0, 1.0); // Black background
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.2, 1.2, -1.2, 1.2); // Orthographic 2D view
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Analog Clock");

    init();
    glutDisplayFunc(display);
    glutTimerFunc(0, update, 0);
    glutMainLoop();

    return 0;
}
