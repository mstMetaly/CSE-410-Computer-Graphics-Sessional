/**
 * Advanced OpenGL Demo Application
 *
 * This program demonstrates basic OpenGL functionality including:
 * - Drawing and transformations
 * - Keyboard input (normal and special keys)
 * - Mouse interaction
 * - Animation using both idle and timer functions
 *
 * Use as a learning template for OpenGL with GLUT
 */

// Platform-specific includes
#ifdef __linux__
#include <GL/glut.h> // For Linux systems
#elif defined(_WIN32) || defined(WIN32)
#include <windows.h>
#include <GL/glut.h> // For Windows systems
#elif defined(__APPLE__)
#include <GLUT/glut.h> // For macOS systems
#else
#include <GL/glut.h> // Default fallback
#endif

#include <cstdio>
#include <cmath>

// Global variables for animation and interaction
GLfloat squareRotation = 0.0f;   // Rotation angle for the first square
GLfloat triangleRotation = 0.0f; // Rotation angle for the triangle
GLfloat xPosition = 0.0f;        // X position for the second square
GLfloat yPosition = 0.0f;        // Y position for the second square
GLfloat moveSpeed = 0.01f;       // Speed of movement

// Animation control flags
bool animateSquare = true;   // Controls square rotation
bool animateTriangle = true; // Controls triangle rotation
int animationSpeed = 30;     // Timer interval in milliseconds

// Mouse tracking
int lastMouseX = 0;
int lastMouseY = 0;
bool mouseLeftDown = false; // Track if left mouse button is pressed

/**
 * Initializes OpenGL settings.
 * Called once at the beginning.
 */
void init()
{
    printf("Initialization complete. Here are the controls:\n");
    printf("----------------------------------------\n");
    printf("KEYBOARD CONTROLS:\n");
    printf("  S/s: Toggle square rotation\n");
    printf("  T/t: Toggle triangle rotation\n");
    printf("  +/-: Increase/decrease animation speed\n");
    printf("  Arrow keys: Move the green square\n");
    printf("  R/r: Reset all animations\n");
    printf("  ESC: Exit the program\n");
    printf("MOUSE CONTROLS:\n");
    printf("  Left click & drag: Rotate view\n");
    printf("  Right click: Reset view\n");
    printf("----------------------------------------\n");

    // Set the background (clear) color to dark blue
    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);

    // Enable depth testing for proper 3D rendering
    glEnable(GL_DEPTH_TEST);
}

/**
 * Handles standard keyboard input (ASCII keys).
 * Triggered when any ASCII key is pressed.
 *
 * @param key The ASCII code of the key pressed
 * @param x The x-coordinate of mouse position when key was pressed
 * @param y The y-coordinate of mouse position when key was pressed
 */
void keyboardListener(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'S':
    case 's':
        // Toggle square rotation
        animateSquare = !animateSquare;
        printf("Square rotation %s\n", animateSquare ? "enabled" : "disabled");
        break;

    case 'T':
    case 't':
        // Toggle triangle rotation
        animateTriangle = !animateTriangle;
        printf("Triangle rotation %s\n", animateTriangle ? "enabled" : "disabled");
        break;

    case '+':
        // Increase animation speed
        animationSpeed = (animationSpeed <= 10) ? 10 : animationSpeed - 5;
        printf("Animation speed increased (timer interval: %d ms)\n", animationSpeed);
        break;

    case '-':
        // Decrease animation speed
        animationSpeed += 5;
        printf("Animation speed decreased (timer interval: %d ms)\n", animationSpeed);
        break;

    case 'R':
    case 'r':
        // Reset animations
        squareRotation = 0.0f;
        triangleRotation = 0.0f;
        xPosition = 0.0f;
        yPosition = 0.0f;
        animateSquare = true;
        animateTriangle = true;
        printf("Animations reset\n");
        break;

    case 27: // ESC key
        printf("Exiting program\n");
        exit(0);
        break;

    default:
        printf("Key pressed: '%c' (ASCII: %d)\n", key, key);
        break;
    }

    // Request a redisplay to update the scene
    glutPostRedisplay();
}

/**
 * Handles special keyboard input (non-ASCII keys like arrows, F1, etc.).
 * Triggered when any special key is pressed.
 *
 * @param key The code of the special key pressed
 * @param x The x-coordinate of mouse position when key was pressed
 * @param y The y-coordinate of mouse position when key was pressed
 */
void specialKeyListener(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_UP:
        // Move the green square up
        yPosition += moveSpeed;
        printf("Moving square up (y = %.2f)\n", yPosition);
        break;

    case GLUT_KEY_DOWN:
        // Move the green square down
        yPosition -= moveSpeed;
        printf("Moving square down (y = %.2f)\n", yPosition);
        break;

    case GLUT_KEY_LEFT:
        // Move the green square left
        xPosition -= moveSpeed;
        printf("Moving square left (x = %.2f)\n", xPosition);
        break;

    case GLUT_KEY_RIGHT:
        // Move the green square right
        xPosition += moveSpeed;
        printf("Moving square right (x = %.2f)\n", xPosition);
        break;

    case GLUT_KEY_F1:
        printf("Help:\n");
        printf("  Use arrow keys to move the green square\n");
        printf("  Press S/s to toggle square rotation\n");
        printf("  Press T/t to toggle triangle rotation\n");
        break;

    default:
        printf("Special key pressed (code: %d)\n", key);
        break;
    }

    // Request a redisplay to update the scene
    glutPostRedisplay();
}

/**
 * Handles mouse button presses.
 *
 * @param button Which mouse button was pressed (GLUT_LEFT_BUTTON, GLUT_RIGHT_BUTTON, etc.)
 * @param state The state of the button (GLUT_DOWN or GLUT_UP)
 * @param x The x-coordinate where the mouse button was pressed
 * @param y The y-coordinate where the mouse button was pressed
 */
void mouseListener(int button, int state, int x, int y)
{
    // Store the mouse position
    lastMouseX = x;
    lastMouseY = y;

    if (button == GLUT_LEFT_BUTTON)
    {
        // Track left mouse button state
        mouseLeftDown = (state == GLUT_DOWN);
        printf("Left mouse button %s at (%d, %d)\n",
               (state == GLUT_DOWN) ? "pressed" : "released", x, y);
    }
    else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        // Reset view on right click
        squareRotation = 0.0f;
        triangleRotation = 0.0f;
        printf("View reset via right mouse button\n");
        glutPostRedisplay();
    }
}

/**
 * Handles mouse movement while a button is pressed.
 *
 * @param x The current x-coordinate of the mouse
 * @param y The current y-coordinate of the mouse
 */
void mouseMotionListener(int x, int y)
{
    if (mouseLeftDown)
    {
        // Calculate the difference from the last position
        int deltaX = x - lastMouseX;

        // Rotate the scene based on mouse movement
        squareRotation += deltaX * 0.5f;

        // Update the stored position
        lastMouseX = x;
        lastMouseY = y;

        // Request a redisplay
        glutPostRedisplay();
    }
}

/**
 * Draws a square centered at the origin in the XY-plane.
 *
 * @param a Half-length of a side of the square
 */
void drawSquare(double a)
{
    glBegin(GL_QUADS);
    {
        glVertex3f(a, a, 0);   // Top-right corner
        glVertex3f(a, -a, 0);  // Bottom-right corner
        glVertex3f(-a, -a, 0); // Bottom-left corner
        glVertex3f(-a, a, 0);  // Top-left corner
    }
    glEnd();
}

/**
 * Draws a triangle centered at the origin in the XY-plane.
 *
 * @param a Half-length of the base of the triangle
 */
void drawTriangle(double a)
{
    glBegin(GL_TRIANGLES);
    {
        glVertex3f(0, a, 0);   // Top vertex
        glVertex3f(a, -a, 0);  // Bottom-right vertex
        glVertex3f(-a, -a, 0); // Bottom-left vertex
    }
    glEnd();
}

/**
 * Draws coordinate axes.
 *
 * @param length The length of each axis
 */
void drawAxes(double length)
{
    glBegin(GL_LINES);
    {
        // X-axis (red)
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(-length, 0.0f, 0.0f);
        glVertex3f(length, 0.0f, 0.0f);

        // Y-axis (green)
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, -length, 0.0f);
        glVertex3f(0.0f, length, 0.0f);

        // Z-axis (blue)
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, -length);
        glVertex3f(0.0f, 0.0f, length);
    }
    glEnd();
}

/**
 * The display callback function.
 * This is where rendering happens. Called whenever the window needs to be redrawn.
 */
void display()
{
    // Clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset transformations
    glLoadIdentity();

    // Set the camera position (simple 2D view for this example)
    gluLookAt(
        0.0, 0.0, 3.0, // Eye position
        0.0, 0.0, 0.0, // Look at point
        0.0, 1.0, 0.0  // Up vector
    );

    // Draw coordinate axes
    drawAxes(1.5);

    // Draw a rotating red square
    glPushMatrix();
    {
        glRotatef(squareRotation, 0, 0, 1); // Rotate around Z-axis
        glTranslatef(0.5, 0.5, 0);          // Move away from center
        glColor3f(1.0f, 0.0f, 0.0f);        // Set color to red
        drawSquare(0.1);                    // Draw a square
    }
    glPopMatrix();

    // Draw a rotating blue triangle
    glPushMatrix();
    {
        glRotatef(-triangleRotation, 0, 0, 1); // Rotate in opposite direction
        glTranslatef(-0.5, 0.5, 0);            // Move to top-left
        glColor3f(0.0f, 0.0f, 1.0f);           // Set color to blue
        drawTriangle(0.15);                    // Draw a triangle
    }
    glPopMatrix();

    // Draw a movable green square (controlled by arrow keys)
    glPushMatrix();
    {
        glTranslatef(xPosition, yPosition, 0); // Position based on keyboard input
        glColor3f(0.0f, 1.0f, 0.0f);           // Set color to green
        drawSquare(0.1);                       // Draw a square
    }
    glPopMatrix();

    // Display the result using double buffering
    glutSwapBuffers();
}

/**
 * Called when the window is resized.
 *
 * @param width The new width of the window
 * @param height The new height of the window
 */
void reshape(int width, int height)
{
    // Prevent division by zero
    if (height == 0)
        height = 1;

    // Set the viewport to the entire window
    glViewport(0, 0, width, height);

    // Set the projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Set perspective projection
    float aspectRatio = (float)width / (float)height;
    gluPerspective(45.0f, aspectRatio, 0.1f, 100.0f);

    // Switch back to modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    printf("Window resized to %d x %d\n", width, height);
}

/**
 * Idle function for continuous animation.
 * This is called when the system is idle, allowing for continuous updates.
 */
void idleFunction()
{
    // Update animation values
    if (animateSquare)
    {
        squareRotation += 0.1f;
        if (squareRotation > 360.0f)
            squareRotation -= 360.0f;
    }

    if (animateTriangle)
    {
        triangleRotation += 0.075f;
        if (triangleRotation > 360.0f)
            triangleRotation -= 360.0f;
    }

    // Request a redisplay
    glutPostRedisplay();
}

/**
 * Timer function for animation.
 * This demonstrates the use of a timer instead of idle function.
 * Timer functions provide better control over animation speed.
 *
 * @param value Value passed to the timer function (not used here)
 */
void timerFunction(int value)
{
    // Update animation values
    if (animateSquare)
    {
        squareRotation += 2.0f;
        if (squareRotation > 360.0f)
            squareRotation -= 360.0f;
    }

    if (animateTriangle)
    {
        triangleRotation += 1.5f;
        if (triangleRotation > 360.0f)
            triangleRotation -= 360.0f;
    }

    // Request a redisplay
    glutPostRedisplay();

    // Register the timer again
    glutTimerFunc(animationSpeed, timerFunction, 0);
}

/**
 * Main function: Entry point of the program.
 */
int main(int argc, char **argv)
{
    // Initialize GLUT
    glutInit(&argc, argv);

    // Set initial window size and position
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);

    // Set display mode: RGB color, double buffering, and depth buffering
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

    // Create a window with the title
    glutCreateWindow("OpenGL Interactive Demo");

    // Register callback functions
    glutDisplayFunc(display);            // Called when screen needs to be redrawn
    glutReshapeFunc(reshape);            // Called when window is resized
    glutKeyboardFunc(keyboardListener);  // Called on ASCII keyboard input
    glutSpecialFunc(specialKeyListener); // Called on special key input (arrows, etc.)
    glutMouseFunc(mouseListener);        // Called on mouse button presses
    glutMotionFunc(mouseMotionListener); // Called on mouse movement with button pressed

    // Register idle function for continuous animation
    // glutIdleFunc(idleFunction); // Uncomment to use idle function for animation

    // Use timer function instead of idle function for better control of animation speed
    glutTimerFunc(animationSpeed, timerFunction, 0);

    // Perform initial setup
    init();

    // Start the GLUT event loop
    printf("Starting GLUT main loop...\n");
    glutMainLoop();

    return 0;
}