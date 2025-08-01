#include <bits/stdc++.h>
#include "bitmap_image.hpp"
using namespace std;


struct Point
{
    double x, y, z, w;
    Point(double x = 0, double y = 0, double z = 0, double w = 1) : x(x), y(y), z(z), w(w) {}
    void normalize()
    {
        x /= w;
        y /= w;
        z /= w;
        w = 1;
    }
};

struct Color
{
    int r, g, b;
    Color(int r = 0, int g = 0, int b = 0) : r(r), g(g), b(b) {}
};

struct Triangle
{
    Point points[3];
    Color color;
};

// ==== Matrix Utilities ====

typedef vector<vector<double>> Matrix;

Matrix identityMatrix()
{
    Matrix I(4, vector<double>(4, 0));
    for (int i = 0; i < 4; i++)
        I[i][i] = 1;
    return I;
}

Point multiply(const Matrix &m, const Point &p)
{
    vector<double> vec = {p.x, p.y, p.z, p.w};
    vector<double> res(4, 0);
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            res[i] += m[i][j] * vec[j];
    return Point(res[0], res[1], res[2], res[3]);
}

Matrix multiply(const Matrix &a, const Matrix &b)
{
    Matrix res(4, vector<double>(4, 0));
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            for (int k = 0; k < 4; k++)
                res[i][j] += a[i][k] * b[k][j];
    return res;
}

Matrix translationMatrix(double tx, double ty, double tz)
{
    Matrix T = identityMatrix();
    T[0][3] = tx;
    T[1][3] = ty;
    T[2][3] = tz;
    return T;
}

Matrix scalingMatrix(double sx, double sy, double sz)
{
    Matrix S = identityMatrix();
    S[0][0] = sx;
    S[1][1] = sy;
    S[2][2] = sz;
    return S;
}

Point cross(const Point &a, const Point &b)
{
    return Point(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x);
}

double dot(const Point &a, const Point &b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Point normalize(const Point &a)
{
    double mag = sqrt(dot(a, a));
    return Point(a.x / mag, a.y / mag, a.z / mag);
}

Point rodrigues(const Point &x, const Point &a, double angle)
{
    Point k = normalize(a);
    double rad = angle * M_PI / 180.0;
    Point term1 = Point(x.x * cos(rad), x.y * cos(rad), x.z * cos(rad));
    Point term2 = Point(k.x * dot(k, x) * (1 - cos(rad)),
                        k.y * dot(k, x) * (1 - cos(rad)),
                        k.z * dot(k, x) * (1 - cos(rad)));
    Point term3 = cross(k, x);
    term3 = Point(term3.x * sin(rad), term3.y * sin(rad), term3.z * sin(rad));
    return Point(term1.x + term2.x + term3.x,
                 term1.y + term2.y + term3.y,
                 term1.z + term2.z + term3.z);
}

Matrix rotationMatrix(double angle, double ax, double ay, double az)
{
    Point i(1, 0, 0), j(0, 1, 0), k(0, 0, 1), a(ax, ay, az);
    Point c1 = rodrigues(i, a, angle);
    Point c2 = rodrigues(j, a, angle);
    Point c3 = rodrigues(k, a, angle);
    Matrix R = identityMatrix();
    R[0][0] = c1.x;
    R[0][1] = c2.x;
    R[0][2] = c3.x;
    R[1][0] = c1.y;
    R[1][1] = c2.y;
    R[1][2] = c3.y;
    R[2][0] = c1.z;
    R[2][1] = c2.z;
    R[2][2] = c3.z;
    return R;
}

// ==== Stage 1: Modeling Transformation ====
void stage1()
{
    ifstream in("scene.txt");
    ofstream out("stage1.txt");
    string cmd;
    stack<Matrix> S;
    S.push(identityMatrix());
    while (in >> cmd)
    {
        if (cmd == "triangle")
        {
            Point p[3];
            for (int i = 0; i < 3; i++)
            {
                in >> p[i].x >> p[i].y >> p[i].z;
                p[i].w = 1;
                p[i] = multiply(S.top(), p[i]);
                p[i].normalize();
                out << fixed << setprecision(7)
                    << p[i].x << " " << p[i].y << " " << p[i].z << "\n";
            }
            out << "\n";
        }
        else if (cmd == "translate")
        {
            double tx, ty, tz;
            in >> tx >> ty >> tz;
            Matrix T = translationMatrix(tx, ty, tz);
            S.top() = multiply(S.top(), T);
        }
        else if (cmd == "scale")
        {
            double sx, sy, sz;
            in >> sx >> sy >> sz;
            Matrix S_new = scalingMatrix(sx, sy, sz);
            S.top() = multiply(S.top(), S_new);
        }
        else if (cmd == "rotate")
        {
            double angle, ax, ay, az;
            in >> angle >> ax >> ay >> az;
            Matrix R = rotationMatrix(angle, ax, ay, az);
            S.top() = multiply(S.top(), R);
        }
        else if (cmd == "push")
        {
            S.push(S.top());
        }
        else if (cmd == "pop")
        {
            S.pop();
        }
        else if (cmd == "end")
        {
            break;
        }
    }
    in.close();
    out.close();
}

// ==== Stage 2: View Transformation ====
void stage2()
{
    ifstream config("scene.txt");
    ifstream in("stage1.txt");
    ofstream out("stage2.txt");

    Point eye, look, up;
    config >> eye.x >> eye.y >> eye.z;
    config >> look.x >> look.y >> look.z;
    config >> up.x >> up.y >> up.z;

    Point l = normalize(Point(look.x - eye.x, look.y - eye.y, look.z - eye.z));
    Point r = normalize(cross(l, up));
    Point u = cross(r, l);

    Matrix T = identityMatrix();
    T[0][3] = -eye.x;
    T[1][3] = -eye.y;
    T[2][3] = -eye.z;

    Matrix R = identityMatrix();
    R[0][0] = r.x;
    R[0][1] = r.y;
    R[0][2] = r.z;
    R[1][0] = u.x;
    R[1][1] = u.y;
    R[1][2] = u.z;
    R[2][0] = -l.x;
    R[2][1] = -l.y;
    R[2][2] = -l.z;

    Matrix V = multiply(R, T);

    while (true)
    {
        Point p[3];
        for (int i = 0; i < 3; i++)
        {
            if (!(in >> p[i].x >> p[i].y >> p[i].z))
                return;
            p[i].w = 1;
            p[i] = multiply(V, p[i]);
            p[i].normalize();
            out << fixed << setprecision(7) << p[i].x << " " << p[i].y << " " << p[i].z << "\n";
        }
        out << "\n";
    }

    config.close();
    in.close();
    out.close();
}

// ==== Stage 3: Projection Transformation ====
void stage3()
{
    ifstream config("scene.txt");
    ifstream in("stage2.txt");
    ofstream out("stage3.txt");

    double trash;
    for (int i = 0; i < 3 * 3; i++)
        config >> trash; // skip eye, look, up

    double fovY, aspectRatio, near, far;
    config >> fovY >> aspectRatio >> near >> far;

    double fovX = fovY * aspectRatio;
    double t = near * tan((fovY * M_PI / 180.0) / 2.0);
    double r = near * tan((fovX * M_PI / 180.0) / 2.0);

    Matrix P(4, vector<double>(4, 0));
    P[0][0] = near / r;
    P[1][1] = near / t;
    P[2][2] = -(far + near) / (far - near);
    P[2][3] = -(2 * far * near) / (far - near);
    P[3][2] = -1;

    while (true)
    {
        Point p[3];
        for (int i = 0; i < 3; i++)
        {
            if (!(in >> p[i].x >> p[i].y >> p[i].z))
                return;
            p[i].w = 1;
            p[i] = multiply(P, p[i]);
            p[i].normalize();
            out << fixed << setprecision(7) << p[i].x << " " << p[i].y << " " << p[i].z << "\n";
        }
        out << "\n";
    }

    config.close();
    in.close();
    out.close();
}

// ==== Stage 4: Z-Buffer and Scan Conversion ====
void stage4()
{
    ifstream config("config.txt");
    ifstream in("stage3.txt");
    ofstream zout("z-buffer.txt");

    int screenWidth, screenHeight;
    double x_left, y_bottom, z_front, z_rear;
    config >> screenWidth >> screenHeight;
    config >> x_left;
    double x_right = -x_left;
    config >> y_bottom;
    double y_top = -y_bottom;
    config >> z_front >> z_rear;

    double dx = (x_right - x_left) / screenWidth;
    double dy = (y_top - y_bottom) / screenHeight;
    double topY = y_top - dy / 2;
    double leftX = x_left + dx / 2;

    vector<vector<double>> zBuffer(screenHeight, vector<double>(screenWidth, z_rear));
    bitmap_image image(screenWidth, screenHeight);
    image.set_all_channels(0, 0, 0);

    vector<Triangle> triangles;
    Point p;
    while (in >> p.x >> p.y >> p.z)
    {
        Point a = p;
        in >> p.x >> p.y >> p.z;
        Point b = p;
        in >> p.x >> p.y >> p.z;
        Point c = p;

        Triangle tri;
        tri.points[0] = a;
        tri.points[1] = b;
        tri.points[2] = c;
        tri.color = Color(rand() % 256, rand() % 256, rand() % 256);
        triangles.push_back(tri);
    }

    for (auto &tri : triangles)
    {
        double minY = min({tri.points[0].y, tri.points[1].y, tri.points[2].y});
        double maxY = max({tri.points[0].y, tri.points[1].y, tri.points[2].y});

        int topScan = max(0, (int)ceil((topY - maxY) / dy));
        int bottomScan = min(screenHeight - 1, (int)floor((topY - minY) / dy));

        for (int row = topScan; row <= bottomScan; row++)
        {
            double scanY = topY - row * dy;

            vector<double> xints;
            vector<double> zvals;
            for (int i = 0; i < 3; i++)
            {
                Point p1 = tri.points[i];
                Point p2 = tri.points[(i + 1) % 3];
                if ((p1.y <= scanY && p2.y >= scanY) || (p2.y <= scanY && p1.y >= scanY))
                {
                    if (p1.y != p2.y)
                    {
                        double x = p1.x + (scanY - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
                        double z = p1.z + (scanY - p1.y) * (p2.z - p1.z) / (p2.y - p1.y);
                        xints.push_back(x);
                        zvals.push_back(z);
                    }
                }
            }

            if (xints.size() < 2)
                continue;
            double xl = min(xints[0], xints[1]);
            double xr = max(xints[0], xints[1]);
            double zl = xints[0] < xints[1] ? zvals[0] : zvals[1];
            double zr = xints[0] < xints[1] ? zvals[1] : zvals[0];

            int leftCol = max(0, (int)ceil((xl - leftX) / dx));
            int rightCol = min(screenWidth - 1, (int)floor((xr - leftX) / dx));

            for (int col = leftCol; col <= rightCol; col++)
            {
                double scanX = leftX + col * dx;
                double z = zl + (scanX - xl) * (zr - zl) / (xr - xl);

                if (z >= z_front && z < zBuffer[row][col])
                {
                    zBuffer[row][col] = z;
                    image.set_pixel(col, row, tri.color.r, tri.color.g, tri.color.b);
                }
            }
        }
    }

    for (int i = 0; i < screenHeight; i++) {
        bool first = true;
        for (int j = 0; j < screenWidth; j++) {
            if (zBuffer[i][j] < z_rear) {
                if (!first) zout << "\t";
                zout << fixed << setprecision(6) << zBuffer[i][j];
                first = false;
            }
        }
        zout << "\n";
    }

    image.save_image("out.bmp");
    config.close();
    in.close();
    zout.close();
}

// ==== Main Function ====
int main()
{
    stage1();
    stage2();
    stage3();
    stage4();
    return 0;
}
