#include <cmath>
#include <gl.h>
#include <freeglut_std.h>

GLfloat secondCircleAngle = 0.0f; // Angle for the second circle's rotation
GLfloat thirdCircleAngle = 0.0f;  // Angle for the third circle's rotation
GLfloat rotationSpeed = 0.5f;     // Speed of rotation

/**
 * Draws a circle at the specified position with a given radius.
 *
 * @param x The x-coordinate of the circle's center
 * @param y The y-coordinate of the circle's center
 * @param radius The radius of the circle
 */
void drawCircle(GLfloat x, GLfloat y, GLfloat radius)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y); // Center of the circle
    for (int i = 0; i <= 360; i++)
    {
        GLfloat angle = i * M_PI / 180.0f;
        glVertex2f(x + cos(angle) * radius, y + sin(angle) * radius);
    }
    glEnd();
}

/**
 * Renders the three circles with the described behavior.
 */
void renderCircles()
{
    // Fixed first circle
    glColor3f(1.0f, 0.0f, 0.0f); // Red color
    drawCircle(0.0f, 0.0f, 0.2f);

    // Rotating second circle
    glPushMatrix();
    glRotatef(secondCircleAngle, 0.0f, 0.0f, 1.0f); // Rotate around the first circle
    glTranslatef(0.5f, 0.0f, 0.0f);                // Move outward from the first circle
    glColor3f(0.0f, 1.0f, 0.0f);                   // Green color
    drawCircle(0.0f, 0.0f, 0.1f);

    // Rotating third circle around the second circle
    glPushMatrix();
    glRotatef(thirdCircleAngle, 0.0f, 0.0f, 1.0f); // Rotate around the second circle
    glTranslatef(0.3f, 0.0f, 0.0f);                // Move outward from the second circle
    glColor3f(0.0f, 0.0f, 1.0f);                   // Blue color
    drawCircle(0.0f, 0.0f, 0.05f);
    glPopMatrix();

    glPopMatrix();
}

/**
 * Updates the rotation angles for the circles.
 */
void updateAnimation()
{
    secondCircleAngle += rotationSpeed; // Rotate the second circle
    thirdCircleAngle += rotationSpeed * 2; // Rotate the third circle faster
    if (secondCircleAngle >= 360.0f)
        secondCircleAngle -= 360.0f;
    if (thirdCircleAngle >= 360.0f)
        thirdCircleAngle -= 360.0f;
}

/**
 * Display callback for rendering the scene.
 */
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Render the circles
    renderCircles();

    glutSwapBuffers();
}

/**
 * Timer function for updating the animation.
 */
void timer(int value)
{
    updateAnimation();
    glutPostRedisplay(); // Request a redraw
    glutTimerFunc(16, timer, 0); // 60 FPS
}

/**
 * Main function.
 */
int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Three Circles Animation");

    // Initialize OpenGL settings
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0); // Set the orthographic projection

    glutDisplayFunc(display);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}