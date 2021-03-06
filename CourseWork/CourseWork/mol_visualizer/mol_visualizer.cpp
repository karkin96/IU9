#include <iostream>
#include <fstream>

#include <GL/glew.h>

#include <GL/glu.h>
#include <GL/gl.h>
#include <GL/glut.h>


#include <GLFW/glfw3.h>

#include <boost/tuple/tuple.hpp>
#include <glm/vec3.hpp>
#include <mol.h>

using namespace std;

#define MAX_SCALE 3.275
bool draw_cube = false;
bool vdw_radius = false;
float user_scale = 2.5f, rotate_x = 0, rotate_y = 0;
vector<glm::vec3> cubePoints;
string WORK_DIR = "/home/alex/dev/src/cpp/CourseWork/";
pmols::Molecule mol(WORK_DIR + "resources/Structure3D_CID_6212.sdf");
glm::vec3 appos_point;


void packMoleculeToCube() {
    //boost::tuple<glm::vec3, boost::tuple<float, float, float>> rect_shell = mol.GetRectangularShell();
    std::tuple<float, float, float> size;
    std::tie(appos_point, size) = mol.GetRectangularShell();
    float a, b, c;
    std::tie(a, b, c) = size;
    std::cout << "shell sizes: " << a << "x" << b << "x" << c << std::endl;

    cubePoints.emplace_back(appos_point);
    cubePoints.emplace_back(appos_point.x, appos_point.y + b, appos_point.z);
    cubePoints.emplace_back(appos_point.x + a, appos_point.y + b, appos_point.z);
    cubePoints.emplace_back(appos_point.x + a, appos_point.y, appos_point.z);

    cubePoints.emplace_back(appos_point.x + a, appos_point.y, appos_point.z + c);
    cubePoints.emplace_back(appos_point.x + a, appos_point.y + b, appos_point.z + c);
    cubePoints.emplace_back(appos_point.x, appos_point.y + b, appos_point.z + c);
    cubePoints.emplace_back(appos_point.x, appos_point.y, appos_point.z + c);

    std::cout << "cube coordinates: " << std::endl;
    for(int i = 0; i < cubePoints.size(); i++) {
        std::cout << "\t" << i <<": " << vec_to_string(cubePoints[i]) << std::endl;
    }
}

void setMaterial(pmols::Colorf ambient,
                 pmols::Colorf diffuse = pmols::Colorf(1.0f, 1.0f, 1.0f),
                 pmols::Colorf specular = pmols::Colorf(1.0f, 1.0f, 1.0f),
                 float shininess=70.0f) {
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, &ambient.red);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &diffuse.red);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &specular.red);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &shininess);
}

// draws cube by points stored in shellPoints vector
void drawCube() {
    //glDepthMask(GL_TRUE);
    //glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    pmols::Colorf cube_color(0.4, 0.4, 0.6, 0.4);

    glm::vec3 N_back(glm::cross(cubePoints[1] - cubePoints[0], cubePoints[2] - cubePoints[0]));
    glm::vec3 N_left(glm::cross(cubePoints[1] - cubePoints[0], cubePoints[6] - cubePoints[0]));
    glm::vec3 N_bottom(glm::cross(cubePoints[3] - cubePoints[0], cubePoints[7] - cubePoints[0]));

    glm::vec3 N_right = - N_left;
    glm::vec3 N_top = - N_bottom;
    glm::vec3 N_front = -N_back;

    setMaterial(cube_color, cube_color);


    glBegin(GL_QUADS);
        // back face
        glNormal3fv(&N_back.x);

        glVertex3fv(&cubePoints[0].x);
        glVertex3fv(&cubePoints[1].x);
        glVertex3fv(&cubePoints[2].x);
        glVertex3fv(&cubePoints[3].x);

        // left face
        glNormal3fv(&N_left.x);

        glVertex3fv(&cubePoints[0].x);
        glVertex3fv(&cubePoints[1].x);
        glVertex3fv(&cubePoints[6].x);
        glVertex3fv(&cubePoints[7].x);

        // bottom face
        glNormal3fv(&N_bottom.x);

        glVertex3fv(&cubePoints[0].x);
        glVertex3fv(&cubePoints[3].x);
        glVertex3fv(&cubePoints[4].x);
        glVertex3fv(&cubePoints[7].x);

        // top face
        glNormal3fv(&N_top.x);

        glVertex3fv(&cubePoints[1].x);
        glVertex3fv(&cubePoints[2].x);
        glVertex3fv(&cubePoints[5].x);
        glVertex3fv(&cubePoints[6].x);

        // right face
        glNormal3fv(&N_right.x);

        glVertex3fv(&cubePoints[2].x);
        glVertex3fv(&cubePoints[3].x);
        glVertex3fv(&cubePoints[4].x);
        glVertex3fv(&cubePoints[5].x);

        // front face
        glNormal3fv(&N_front.x);

        glVertex3fv(&cubePoints[4].x);
        glVertex3fv(&cubePoints[5].x);
        glVertex3fv(&cubePoints[6].x);
        glVertex3fv(&cubePoints[7].x);
    glEnd();

    glDisable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void drawSphere(glm::vec3 center, float radius, int numStacks, int numSides, pmols::Colorf color)
{
    float x0 = center.x, y0 = center.y, z0 = center.z;
    GLfloat curRadius, curTheta, curRho, deltaTheta, deltaRho, curX,curY,curZ;
    int curStack, curSlice, numVerts = (numStacks-1)*numSides;
    glm::vec3 points[numVerts];
    int curVert = 0;
    int t;

    deltaTheta = (float)(2*M_PI) / numSides;
    deltaRho = (float)M_PI / numStacks;

    for (curStack=1; curStack<numStacks; curStack++)
    {
        curRho = (float)(M_PI/2.0f) - curStack*deltaRho;
        curY = y0 + sin(curRho) * radius;
        curRadius = cos(curRho) * radius;
        for (curSlice=0; curSlice<numSides; curSlice++)
        {
            curTheta = curSlice * deltaTheta;
            curX = x0 + curRadius * cos(curTheta);
            curZ = z0 - curRadius * sin(curTheta);
            points[curVert++] = glm::vec3(curX,curY,curZ);
        }
    }

    setMaterial(color, color);

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(x0, y0 + 1.0f, z0);
    glVertex3f(x0, y0+radius, z0);
    for (t=0; t<numSides; t++)
    {
        curX = points[t].x;
        curY = points[t].y;
        curZ = points[t].z;
        glNormal3f(curX, curY, curZ);
        glVertex3f(curX, curY, curZ);
    }
    curX = points[0].x;
    curY = points[0].y;
    curZ = points[0].z;
    glNormal3f(curX, curY, curZ);
    glVertex3f(curX, curY, curZ);
    glEnd();

    int vertIndex;
    glBegin(GL_QUAD_STRIP);
    for (curStack=0; curStack<numStacks-2; curStack++)
    {
        vertIndex = curStack * numSides;
        for (curSlice=0; curSlice<numSides; curSlice++)
        {
            glNormal3f(points[vertIndex+curSlice].x, points[vertIndex+curSlice].y, points[vertIndex+curSlice].z);
            glVertex3f(points[vertIndex+curSlice].x, points[vertIndex+curSlice].y, points[vertIndex+curSlice].z);

            glNormal3f(points[vertIndex+numSides+curSlice].x, points[vertIndex+numSides+curSlice].y, points[vertIndex+numSides+curSlice].z);
            glVertex3f(points[vertIndex+numSides+curSlice].x, points[vertIndex+numSides+curSlice].y, points[vertIndex+numSides+curSlice].z);
        }
        glNormal3f(points[vertIndex].x, points[vertIndex].y, points[vertIndex].z);
        glVertex3f(points[vertIndex].x, points[vertIndex].y, points[vertIndex].z);
        glNormal3f(points[vertIndex+numSides].x, points[vertIndex+numSides].y, points[vertIndex+numSides].z);
        glVertex3f(points[vertIndex+numSides].x, points[vertIndex+numSides].y, points[vertIndex+numSides].z);
    }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(x0, y0 - 1.0f, z0);
    glVertex3f(x0, y0-radius, z0);
    for (t=0; t<numSides-1; t++)
    {
        curX = points[numVerts-1-t].x;
        curY = points[numVerts-1-t].y;
        curZ = points[numVerts-1-t].z;
        glNormal3f(curX, curY, curZ);
        glVertex3f(curX, curY, curZ);
    }
    curX = points[numVerts-1].x;
    curY = points[numVerts-1].y;
    curZ = points[numVerts-1].z;
    glNormal3f(curX, curY, curZ);
    glVertex3f(curX, curY, curZ);
    glEnd();
}

void drawTube(glm::vec3 basePoint, glm::vec3 cupPoint, float radius, int partition, pmols::Colorf color) {
    float dphi = 2 * (float)M_PI / partition;
    float phi;

    setMaterial(color, color);
    glBegin(GL_QUADS);
    for (int i = 1; i <= partition; ++i) {
        phi = (i - 1) * dphi;

        glm::vec3 p1(basePoint.x + radius * cos(phi), basePoint.y, basePoint.z + radius * sin(phi));
        glm::vec3 p2(cupPoint.x + radius * cos(phi), cupPoint.y, cupPoint.z + radius * sin(phi));

        phi = i % partition * dphi;
        glm::vec3 p3(cupPoint.x + radius * cos(phi), cupPoint.y, cupPoint.z + radius * sin(phi));

        float x, dx = (cupPoint.x - basePoint.x) / partition;
        float y, dy = (cupPoint.y - basePoint.y) / partition;
        float z, dz = (cupPoint.z - basePoint.z) / partition;
        glm::vec3 N = -glm::cross(p3 - p2, p2 - p1);
        glNormal3f(N.x, N.y, N.z);

        for (int j = 0; j < partition; ++j) {
            x = basePoint.x + j * dx;
            y = basePoint.y + j * dy;
            z = basePoint.z + j * dz;

            phi = (i - 1) * dphi;
            glm::vec3 p1(x + radius * cos(phi), y, z + radius * sin(phi));
            glm::vec3 p2(x + dx + radius * cos(phi), y + dy, z + dz + radius * sin(phi));

            phi = i % partition * dphi;
            glm::vec3 p3(x + dx + radius * cos(phi), y + dy, z + dz + radius * sin(phi));
            glm::vec3 p4(x + radius * cos(phi), y, z + radius * sin(phi));

            glVertex3fv(&p1.x);
            glVertex3fv(&p2.x);
            glVertex3fv(&p3.x);
            glVertex3fv(&p4.x);
        }
    }
    glEnd();
}

void drawMolecule(pmols::Molecule &mol, bool vdw_radii) {
    if (!vdw_radii) {
        for (int i = 0; i < mol.BondsCount(); i++) {
            pmols::Atom *begin_atom = mol.GetBond(i).begin;
            pmols::Atom *end_atom = mol.GetBond(i).end;

            glm::vec3 begin_coord = begin_atom->coord;
            glm::vec3 end_coord = end_atom->coord;
            glm::vec3 middle_coord((begin_coord.x + end_coord.x) / 2, (begin_coord.y + end_coord.y) / 2,
                                   (begin_coord.z + end_coord.z) / 2);

            pmols::Colorf begin_color = begin_atom->color;
            pmols::Colorf end_color = end_atom->color;


            drawTube(begin_coord, middle_coord, 0.1, 30, begin_color);
            drawTube(middle_coord, end_coord, 0.1, 30, end_color);
        }
    }

    float radius = 0;

    for(int i=0; i < mol.AtomsCount(); i++) {
        pmols::Atom &atom = mol.GetAtom(i);
        if(vdw_radii)
            radius = atom.vdw_radius;
        else
            radius = atom.radius;

        drawSphere(atom.coord, radius, 30, 30, atom.color);
    }
}

#define RADPERDEG 0.0174533

void drawArrow(GLdouble x1, GLdouble y1, GLdouble z1, GLdouble x2, GLdouble y2, GLdouble z2, GLdouble D)
{
    double x=x2-x1;
    double y=y2-y1;
    double z=z2-z1;
    double L=sqrt(x*x+y*y+z*z);

    GLUquadricObj *quadObj;

    glPushMatrix ();

    glTranslated(x1,y1,z1);

    if((x!=0.)||(y!=0.)) {
        glRotated(atan2(y,x)/RADPERDEG,0.,0.,1.);
        glRotated(atan2(sqrt(x*x+y*y),z)/RADPERDEG,0.,1.,0.);
    } else if (z<0){
        glRotated(180,1.,0.,0.);
    }

    glTranslatef(0,0,L-4*D);

    quadObj = gluNewQuadric ();
    gluQuadricDrawStyle (quadObj, GLU_FILL);
    gluQuadricNormals (quadObj, GLU_SMOOTH);
    gluCylinder(quadObj, 2*D, 0.0, 4*D, 32, 1);
    gluDeleteQuadric(quadObj);

    quadObj = gluNewQuadric ();
    gluQuadricDrawStyle (quadObj, GLU_FILL);
    gluQuadricNormals (quadObj, GLU_SMOOTH);
    gluDisk(quadObj, 0.0, 2*D, 32, 1);
    gluDeleteQuadric(quadObj);

    glTranslatef(0,0,-L+4*D);

    quadObj = gluNewQuadric ();
    gluQuadricDrawStyle (quadObj, GLU_FILL);
    gluQuadricNormals (quadObj, GLU_SMOOTH);
    gluCylinder(quadObj, D, D, L-4*D, 32, 1);
    gluDeleteQuadric(quadObj);

    quadObj = gluNewQuadric ();
    gluQuadricDrawStyle (quadObj, GLU_FILL);
    gluQuadricNormals (quadObj, GLU_SMOOTH);
    gluDisk(quadObj, 0.0, D, 32, 1);
    gluDeleteQuadric(quadObj);

    glPopMatrix ();

}

void drawAxes(GLdouble length)
{
    pmols::Colorf red(1.0, 0.0, 0.0);
    pmols::Colorf green(0.0, 1.0, 0.0);
    pmols::Colorf blue(0.0, 0.0, 1.0);

    glPushMatrix();
    glTranslatef(-length,-length,-length);
    setMaterial(red, red);
    drawArrow(0, 0, 0, 2 * length, 0, 0, 0.05);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-length,-length,-length);
    setMaterial(blue, blue);
    drawArrow(0, 0, 0, 0, 2 * length, 0, 0.05);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-length,-length,-length);
    setMaterial(green, green);
    drawArrow(0, 0, 0, 0, 0, 2 * length, 0.05);
    glPopMatrix();
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void init(GLFWwindow* window, int width, int height) {
    float ambient[4] = {0.2, 0.2, 0.2, 1.0};
    float lightPos[4] = {0.0, 1.0, 15.0, 1.0};

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glOrtho(-(GLfloat)width / 64.0f, (GLfloat)width / 64.0f, -(GLfloat)height / 64.0f, (GLfloat)height / 64.0f, -10, 10);
    glShadeModel(GL_SMOOTH);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, true);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 90.0);
}


static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS)
        return;
    switch (key) {
        case GLFW_KEY_UP:
            rotate_x += 10;
            //mol.RotateX(0.2f);
            break;
        case GLFW_KEY_DOWN:
            rotate_x -= 10;
            //mol.RotateX(-0.2f);
            break;
        case GLFW_KEY_LEFT:
            //mol.rotateY(-0.2f);
            rotate_y -= 10;
            break;
        case GLFW_KEY_RIGHT:
            //mol.rotateY(0.2f);
            rotate_y += 10;
            break;
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        case GLFW_KEY_P:
            if (user_scale*1.1f<MAX_SCALE)
                user_scale *= 1.1f;
            break;
        case GLFW_KEY_X:
            mol.RotateX(0.2f);
            break;
        case GLFW_KEY_Y:
            mol.RotateY(0.2f);
            break;
        case GLFW_KEY_Z:
            mol.RotateZ(0.2f);
            break;
        case GLFW_KEY_M:
            user_scale /= 1.1f;
            break;
        case GLFW_KEY_W:
            mol.Translate(glm::vec3(0, 0.2f, 0));
            break;
        case GLFW_KEY_S:
            mol.Translate(glm::vec3(0, -0.2f, 0));
            break;
        case GLFW_KEY_A:
            mol.Translate(glm::vec3(-0.2f, 0, 0));
            break;
        case GLFW_KEY_D:
            mol.Translate(glm::vec3(0.2f, 0, 0));
            break;
        case GLFW_KEY_PAGE_UP:
            mol.Translate(glm::vec3(0, 0, 0.2f));
            break;
        case GLFW_KEY_PAGE_DOWN:
            mol.Translate(glm::vec3(0, 0, -0.2f));
            break;
        case GLFW_KEY_C:
            packMoleculeToCube();
            draw_cube = !draw_cube;
            break;
        case GLFW_KEY_V:
            vdw_radius = !vdw_radius;
            break;
        default:
            break;
    }
}


int main(void) {
    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }

    window = glfwCreateWindow(640, 480, "Lab 6", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "Failed to open GLFW window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glEnable(GL_DEPTH_TEST);

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, init);

    init(window, 640, 480);



    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);

        glLoadIdentity();
        glScalef(user_scale, user_scale, user_scale);
        glRotatef(rotate_x, 1.0, 0.0, 0.0);
        glRotatef(rotate_y, 0.0, 1.0, 0.0);

        drawMolecule(mol, vdw_radius);

        drawSphere(appos_point, 0.05, 20, 20, pmols::Colorf(0.0, 1.0, 0.0));

        if (draw_cube) {
            drawCube();
        }
        drawAxes(1.9);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

}
