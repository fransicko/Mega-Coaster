#ifdef __APPLE__ // if compiling on Mac OS
#include <OpenGL/gl.h>
#else // if compiling on Linux or Windows OS
#include <GL/gl.h>
#endif

#include <GLFW/glfw3.h> // include GLFW framework header

#include <CSCI441/objects.hpp> // for our 3D objects

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <math.h>   // for cos(), sin() functionality
#include <stdio.h>  // for printf functionality
#include <stdlib.h> // for exit functionality
#include <time.h>   // for time() functionality

#include <fstream> // for file I/O
#include <vector>  // for vector
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

glm::vec3 carPosM = glm::vec3(0, 1, 0); // the initial car position in cartesian coords
float carTheta = 0;                     // car DIRECTION in spherical coordinates
float wheelAngle = 0;

vector<glm::vec3> controlPointsM;
int numSegments = 0;

vector<glm::vec3> controlPath; // This will be the the path that the mascot will take
glm::vec3 controlPoint;        // This will b the point on the path of the curve
static int path = 0;
bool hideControlPoint = false;
bool hideControlCage = false;
bool hideControlPath = false;

bool loadControlPointsMV(char *filename)
{
    // TODO #02: read in control points from file.  Make sure the file can be
    // opened and handle it appropriately.
    std::ifstream data(filename);
    std::string line;

    //This is is to read in the number of points
    data >> numSegments;
    //cout << numSegments << endl;
    std::getline(data, line); // read to the next line
    for (int i = 0; i < numSegments; ++i)
    {
        std::getline(data, line); // read in the line
        std::stringstream ss(line);
        std::string cell;

        vector<int> values;
        for (int j = 0; j < 3; ++j)
        {
            std::getline(ss, cell, ',');
            //cout << cell << endl;
            values.push_back(atoi(cell.c_str()));
        }

        controlPointsM.push_back(glm::vec3(values.at(0), values.at(1), values.at(2)));
    }
    controlPath.push_back(controlPointsM.at(0));
    return true;
}

// Lab03
// evaluateBezierCurve() ////////////////////////////////////////////////////////
//
// Computes a location along a Bezier Curve.
//
////////////////////////////////////////////////////////////////////////////////
glm::vec3 evaluateBezierCurveM(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t)
{
    glm::vec3 point(0, 0, 0);

    // TODO #06: Compute a point along a Bezier curve
    point = pow((1 - t), 3) * p0 + 3 * pow((1 - t), 2) * t * p1 + 3 * (1 - t) * pow(t, 2) * p2 + pow(t, 3) * p3;

    return point;
}

// renderBezierCurve() //////////////////////////////////////////////////////////
//
// Responsible for drawing a Bezier Curve as defined by four control points.
//  Breaks the curve into n segments as specified by the resolution.
//
////////////////////////////////////////////////////////////////////////////////
void renderBezierCurveM(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, int resolution)
{
    // TODO #05: Draw the Bezier Curve!
    float step = (float)1 / (float)resolution;
    if (hideControlPath)
    {
        glColor3f(0, 0, 1);
        glLineWidth(6.0);
        glBegin(GL_LINE_STRIP);
        {
            for (float i = 0; i <= 1.0f + step; i += step)
            {
                glm::vec3 point = evaluateBezierCurve(p0, p1, p2, p3, i);
                glVertex3f(point.x, point.y, point.z);

                if (i != 0)
                    controlPath.push_back(point); // the point in the curve
            }
        };
        glEnd();
    }
    else
    {
        for (float i = 0; i <= 1.0f + step; i += step)
        {
            glm::vec3 point = evaluateBezierCurve(p0, p1, p2, p3, i);

            if (i != 0)
                controlPath.push_back(point); // the point in the curve
        }
    }
}

void drawWheelsM()
{
    // draw the wheels
    // front right
    glm::mat4 transCube = glm::translate(glm::mat4(), glm::vec3(1, -0.8, 1));
    glMultMatrixf(&transCube[0][0]);
    {
        glm::mat4 scaleTri = glm::scale(glm::mat4(), glm::vec3(1.0f, 1, 1.0f));
        glMultMatrixf(&scaleTri[0][0]);
        {
            glm::mat4 rotTri = glm::rotate(glm::mat4(), (float)(M_PI / 2), glm::vec3(0.0f, 1.0f, 0.0f));
            glMultMatrixf(&rotTri[0][0]);
            {
                glm::mat4 rotTri = glm::rotate(glm::mat4(), wheelAngle, glm::vec3(0.0f, 0.0f, 1.0f));
                glMultMatrixf(&rotTri[0][0]);
                {
                    // and then scale it 10X in x and 10X in y
                    glColor3f(0.545, 0.271, 0.075);
                    CSCI441::drawSolidTorus(0.15, 0.25, 4, 4);
                };
                glMultMatrixf(&(glm::inverse(rotTri))[0][0]);
            };
            glMultMatrixf(&(glm::inverse(rotTri))[0][0]);
        };
        glMultMatrixf(&(glm::inverse(scaleTri))[0][0]);
    };
    glMultMatrixf(&(glm::inverse(transCube))[0][0]);

    // back wheel
    transCube = glm::translate(glm::mat4(), glm::vec3(-1, -0.8, -1));
    glMultMatrixf(&transCube[0][0]);
    {
        glm::mat4 scaleTri = glm::scale(glm::mat4(), glm::vec3(1.0f, 1, 1.0f));
        glMultMatrixf(&scaleTri[0][0]);
        {
            glm::mat4 rotTri = glm::rotate(glm::mat4(), (float)(M_PI / 2), glm::vec3(0.0f, 1.0f, 0.0f));
            glMultMatrixf(&rotTri[0][0]);
            {
                glm::mat4 rotTri = glm::rotate(glm::mat4(), wheelAngle, glm::vec3(0.0f, 0.0f, 1.0f));
                glMultMatrixf(&rotTri[0][0]);
                {
                    // and then scale it 10X in x and 10X in y
                    glColor3f(0.545, 0.271, 0.075);
                    CSCI441::drawSolidTorus(0.15, 0.25, 4, 4);
                };
                glMultMatrixf(&(glm::inverse(rotTri))[0][0]);
            };
            glMultMatrixf(&(glm::inverse(rotTri))[0][0]);
        };
        glMultMatrixf(&(glm::inverse(scaleTri))[0][0]);
    };
    glMultMatrixf(&(glm::inverse(transCube))[0][0]);

    transCube = glm::translate(glm::mat4(), glm::vec3(1, -0.8, -1));
    glMultMatrixf(&transCube[0][0]);
    {
        glm::mat4 scaleTri = glm::scale(glm::mat4(), glm::vec3(1.0f, 1, 1.0f));
        glMultMatrixf(&scaleTri[0][0]);
        {
            glm::mat4 rotTri = glm::rotate(glm::mat4(), (float)(M_PI / 2), glm::vec3(0.0f, 1.0f, 0.0f));
            glMultMatrixf(&rotTri[0][0]);
            {
                glm::mat4 rotTri = glm::rotate(glm::mat4(), wheelAngle, glm::vec3(0.0f, 0.0f, 1.0f));
                glMultMatrixf(&rotTri[0][0]);
                {
                    // and then scale it 10X in x and 10X in y
                    glColor3f(0.545, 0.271, 0.075);
                    CSCI441::drawSolidTorus(0.15, 0.25, 4, 4);
                };
                glMultMatrixf(&(glm::inverse(rotTri))[0][0]);
            };
            glMultMatrixf(&(glm::inverse(rotTri))[0][0]);
        };
        glMultMatrixf(&(glm::inverse(scaleTri))[0][0]);
    };
    glMultMatrixf(&(glm::inverse(transCube))[0][0]);

    transCube = glm::translate(glm::mat4(), glm::vec3(-1, -0.8, 1));
    glMultMatrixf(&transCube[0][0]);
    {
        glm::mat4 scaleTri = glm::scale(glm::mat4(), glm::vec3(1.0f, 1, 1.0f));
        glMultMatrixf(&scaleTri[0][0]);
        {
            glm::mat4 rotTri = glm::rotate(glm::mat4(), (float)(M_PI / 2), glm::vec3(0.0f, 1.0f, 0.0f));
            glMultMatrixf(&rotTri[0][0]);
            {
                glm::mat4 rotTri = glm::rotate(glm::mat4(), wheelAngle, glm::vec3(0.0f, 0.0f, 1.0f));
                glMultMatrixf(&rotTri[0][0]);
                {
                    // and then scale it 10X in x and 10X in y
                    glColor3f(0.545, 0.271, 0.075);
                    CSCI441::drawSolidTorus(0.15, 0.25, 4, 4);
                };
                glMultMatrixf(&(glm::inverse(rotTri))[0][0]);
            };
            glMultMatrixf(&(glm::inverse(rotTri))[0][0]);
        };
        glMultMatrixf(&(glm::inverse(scaleTri))[0][0]);
    };
    glMultMatrixf(&(glm::inverse(transCube))[0][0]);
}

// draw spikes
void drawSpikes()
{
    // top spike
    glm::mat4 transCube = glm::translate(glm::mat4(), glm::vec3(0, 1, 0));
    glMultMatrixf(&transCube[0][0]);
    {
        glm::mat4 scaleTri = glm::scale(glm::mat4(), glm::vec3(1.0f, 1.0f, 1.0f));
        glMultMatrixf(&scaleTri[0][0]);
        {
            glColor3f(1, 0, 0);
            CSCI441::drawSolidCone(1, 1, 2, 6);
        };
        glMultMatrixf(&(glm::inverse(scaleTri))[0][0]);
    };
    glMultMatrixf(&(glm::inverse(transCube))[0][0]);

    // left side spike
    transCube = glm::translate(glm::mat4(), glm::vec3(1, 0, 0));
    glMultMatrixf(&transCube[0][0]);
    {
        glm::mat4 scaleTri = glm::scale(glm::mat4(), glm::vec3(1.0f, 1, 1.0f));
        glMultMatrixf(&scaleTri[0][0]);
        {
            glm::mat4 rotTri = glm::rotate(glm::mat4(), (float)M_PI * 3.0f / 2.0f, glm::vec3(0.0f, 0.0f, 1.0f));
            glMultMatrixf(&rotTri[0][0]);
            {
                // and then scale it 10X in x and 10X in y
                glColor3f(1, 0, 0);
                CSCI441::drawSolidCone(1, 1, 2, 6);
            };
            glMultMatrixf(&(glm::inverse(rotTri))[0][0]);
        };
        glMultMatrixf(&(glm::inverse(scaleTri))[0][0]);
    };
    glMultMatrixf(&(glm::inverse(transCube))[0][0]);

    // right side spike
    transCube = glm::translate(glm::mat4(), glm::vec3(-1, 0, 0));
    glMultMatrixf(&transCube[0][0]);
    {
        glm::mat4 scaleTri = glm::scale(glm::mat4(), glm::vec3(1.0f, 1, 1.0f));
        glMultMatrixf(&scaleTri[0][0]);
        {
            glm::mat4 rotTri = glm::rotate(glm::mat4(), (float)M_PI / 2.0f, glm::vec3(0.0f, 0.0f, 1.0f));
            glMultMatrixf(&rotTri[0][0]);
            {
                // and then scale it 10X in x and 10X in y
                glColor3f(1, 0, 0);
                CSCI441::drawSolidCone(1, 1, 2, 6);
            };
            glMultMatrixf(&(glm::inverse(rotTri))[0][0]);
        };
        glMultMatrixf(&(glm::inverse(scaleTri))[0][0]);
    };
    glMultMatrixf(&(glm::inverse(transCube))[0][0]);

    // front spike
    transCube = glm::translate(glm::mat4(), glm::vec3(0, 0, 1));
    glMultMatrixf(&transCube[0][0]);
    {
        glm::mat4 scaleTri = glm::scale(glm::mat4(), glm::vec3(1.0f, 1, 1.0f));
        glMultMatrixf(&scaleTri[0][0]);
        {
            glm::mat4 rotTri = glm::rotate(glm::mat4(), (float)M_PI / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
            glMultMatrixf(&rotTri[0][0]);
            {
                // and then scale it 10X in x and 10X in y
                glColor3f(1, 0, 0);
                CSCI441::drawSolidCone(1, 1, 2, 6);
            };
            glMultMatrixf(&(glm::inverse(rotTri))[0][0]);
        };
        glMultMatrixf(&(glm::inverse(scaleTri))[0][0]);
    };
    glMultMatrixf(&(glm::inverse(transCube))[0][0]);

    // back spike
    transCube = glm::translate(glm::mat4(), glm::vec3(0, 0, -1));
    glMultMatrixf(&transCube[0][0]);
    {
        glm::mat4 scaleTri = glm::scale(glm::mat4(), glm::vec3(1.0f, 1, 1.0f));
        glMultMatrixf(&scaleTri[0][0]);
        {
            glm::mat4 rotTri = glm::rotate(glm::mat4(), (float)M_PI * 3.0f / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
            glMultMatrixf(&rotTri[0][0]);
            {
                // and then scale it 10X in x and 10X in y
                glColor3f(1, 0, 0);
                CSCI441::drawSolidCone(1, 1, 2, 6);
            };
            glMultMatrixf(&(glm::inverse(rotTri))[0][0]);
        };
        glMultMatrixf(&(glm::inverse(scaleTri))[0][0]);
    };
    glMultMatrixf(&(glm::inverse(transCube))[0][0]);
}

void drawCube()
{
    glm::mat4 transCube = glm::translate(glm::mat4(), glm::vec3(0, 0, 0));
    glMultMatrixf(&transCube[0][0]);
    {
        glm::mat4 scaleTri = glm::scale(glm::mat4(), glm::vec3(1.0f, 1, 1.0f));
        glMultMatrixf(&scaleTri[0][0]);
        {
            // and then scale it 10X in x and 10X in y
            glColor3f(0.184, 0.310, 0.310);
            CSCI441::drawSolidCube(2);

            // draw the wheels
            drawWheels();

            // draw the spikes
            drawSpikes();
        };
        glMultMatrixf(&(glm::inverse(scaleTri))[0][0]);
    };
    glMultMatrixf(&(glm::inverse(transCube))[0][0]);

    // draw the spikes
}

// draw all the different eyeballs
void drawFace()
{
    // draw the center eyeballs
    glm::mat4 transCube = glm::translate(glm::mat4(), glm::vec3(-1.5, 0, 0));
    glMultMatrixf(&transCube[0][0]);
    {
        // draw body
        glColor3f(0.4941, 0.9843, 1);
        CSCI441::drawSolidSphere(0.75, 8, 8);
    };
    glMultMatrixf(&(glm::inverse(transCube))[0][0]);

    // Now we need the 6 eyes surounding the big eyeball

    // Middle left and right
    transCube = glm::translate(glm::mat4(), glm::vec3(-1.25, 0, -1.1)); // more z
    glMultMatrixf(&transCube[0][0]);
    {
        // draw eye
        glColor3f(0.4941, 0.9843, 1);
        CSCI441::drawSolidSphere(0.40, 8, 8);
    };
    glMultMatrixf(&(glm::inverse(transCube))[0][0]);
    transCube = glm::translate(glm::mat4(), glm::vec3(-1.25, 0, 1.1)); // more z
    glMultMatrixf(&transCube[0][0]);
    {
        // draw eye
        glColor3f(0.4941, 0.9843, 1);
        CSCI441::drawSolidSphere(0.40, 8, 8);
    };
    glMultMatrixf(&(glm::inverse(transCube))[0][0]);

    // Top left and right
    transCube = glm::translate(glm::mat4(), glm::vec3(-1.1, 0.75, -1.1)); // more z
    glMultMatrixf(&transCube[0][0]);
    {
        // draw eye
        glColor3f(0.4941, 0.9843, 1);
        CSCI441::drawSolidSphere(0.40, 8, 8);
    };
    glMultMatrixf(&(glm::inverse(transCube))[0][0]);
    transCube = glm::translate(glm::mat4(), glm::vec3(-1.1q, 0.75, 1.1)); // more z
    glMultMatrixf(&transCube[0][0]);
    {
        // draw eye
        glColor3f(0.4941, 0.9843, 1);
        CSCI441::drawSolidSphere(0.40, 8, 8);
    };
    glMultMatrixf(&(glm::inverse(transCube))[0][0]);

    // Bottom left and Right
    // Top left and right
    transCube = glm::translate(glm::mat4(), glm::vec3(-1.1, -0.75, -1.1)); // more z
    glMultMatrixf(&transCube[0][0]);
    {
        // draw eye
        glColor3f(0.4941, 0.9843, 1);
        CSCI441::drawSolidSphere(0.40, 8, 8);
    };
    glMultMatrixf(&(glm::inverse(transCube))[0][0]);
    transCube = glm::translate(glm::mat4(), glm::vec3(-1.1q, -0.75, 1.1)); // more z
    glMultMatrixf(&transCube[0][0]);
    {
        // draw eye
        glColor3f(0.4941, 0.9843, 1);
        CSCI441::drawSolidSphere(0.40, 8, 8);
    };
    glMultMatrixf(&(glm::inverse(transCube))[0][0]);
}

// draw arms
void drawArms()
{
    // draw front tentacles left and right tantecle
    glm::mat4 transCube = glm::translate(glm::mat4(), glm::vec3(-0.6, -0.5, 0.8));
    glMultMatrixf(&transCube[0][0]);
    {
        glm::mat4 scaleTri = glm::scale(glm::mat4(), glm::vec3(0.5f, 0.5f, 0.5f));
        glMultMatrixf(&scaleTri[0][0]);
        {
            glm::mat4 rotTri = glm::rotate(glm::mat4(), (float)M_PI, glm::vec3(1.0f, 0.0f, 0.0f));
            glMultMatrixf(&rotTri[0][0]);
            {
                // and then scale it 10X in x and 10X in y
                glColor3f(57.0 / 255, 86.0 / 255, 94.0 / 255);
                CSCI441::drawSolidCone(1.5, 5, 2, 6);
            };
            glMultMatrixf(&(glm::inverse(rotTri))[0][0]);
        };
        glMultMatrixf(&(glm::inverse(scaleTri))[0][0]);
    };
    glMultMatrixf(&(glm::inverse(transCube))[0][0]);

    transCube = glm::translate(glm::mat4(), glm::vec3(-0.6, -0.5, -0.8));
    glMultMatrixf(&transCube[0][0]);
    {
        glm::mat4 scaleTri = glm::scale(glm::mat4(), glm::vec3(0.5f, 0.5f, 0.5f));
        glMultMatrixf(&scaleTri[0][0]);
        {
            glm::mat4 rotTri = glm::rotate(glm::mat4(), (float)M_PI, glm::vec3(1.0f, 0.0f, 0.0f));
            glMultMatrixf(&rotTri[0][0]);
            {
                // and then scale it 10X in x and 10X in y
                glColor3f(57.0 / 255, 86.0 / 255, 94.0 / 255);
                CSCI441::drawSolidCone(1.5, 5, 2, 6);
            };
            glMultMatrixf(&(glm::inverse(rotTri))[0][0]);
        };
        glMultMatrixf(&(glm::inverse(scaleTri))[0][0]);
    };
    glMultMatrixf(&(glm::inverse(transCube))[0][0]);

    // back tentacles
    transCube = glm::translate(glm::mat4(), glm::vec3(0.7, -0.0, 0.9));
    glMultMatrixf(&transCube[0][0]);
    {
        glm::mat4 scaleTri = glm::scale(glm::mat4(), glm::vec3(0.5f, 0.5f, 0.5f));
        glMultMatrixf(&scaleTri[0][0]);
        {
            glm::mat4 rotTri = glm::rotate(glm::mat4(), (float)M_PI, glm::vec3(1.0f, 0.0f, 0.0f));
            glMultMatrixf(&rotTri[0][0]);
            {
                // and then scale it 10X in x and 10X in y
                glColor3f(57.0 / 255, 86.0 / 255, 94.0 / 255);
                CSCI441::drawSolidCone(1.5, 5, 2, 6);
            };
            glMultMatrixf(&(glm::inverse(rotTri))[0][0]);
        };
        glMultMatrixf(&(glm::inverse(scaleTri))[0][0]);
    };
    glMultMatrixf(&(glm::inverse(transCube))[0][0]);

    transCube = glm::translate(glm::mat4(), glm::vec3(0.7, -0.0, -0.9));
    glMultMatrixf(&transCube[0][0]);
    {
        glm::mat4 scaleTri = glm::scale(glm::mat4(), glm::vec3(0.5f, 0.5f, 0.5f));
        glMultMatrixf(&scaleTri[0][0]);
        {
            glm::mat4 rotTri = glm::rotate(glm::mat4(), (float)M_PI, glm::vec3(1.0f, 0.0f, 0.0f));
            glMultMatrixf(&rotTri[0][0]);
            {
                // and then scale it 10X in x and 10X in y
                glColor3f(57.0 / 255, 86.0 / 255, 94.0 / 255);
                CSCI441::drawSolidCone(1.5, 5, 2, 6);
            };
            glMultMatrixf(&(glm::inverse(rotTri))[0][0]);
        };
        glMultMatrixf(&(glm::inverse(scaleTri))[0][0]);
    };
    glMultMatrixf(&(glm::inverse(transCube))[0][0]);
}

// draw the mascot
void drawMascotM()
{
    glm::mat4 transCube = glm::translate(glm::mat4(), glm::vec3(0, 0, 0));
    glMultMatrixf(&transCube[0][0]);
    {
        // draw body
        glColor3f(0.2324, 0.4196, 0.5098);
        CSCI441::drawSolidSphere(2, 8, 8);

        // draw facet
        drawFace();

        // draw arms
        drawArms();
    };
    glMultMatrixf(&(glm::inverse(transCube))[0][0]);
}

// draw curve
void drawCurveM()
{
    // TODO #03: Draw our control points
    if (hideControlPoint)
    {
        for (std::size_t i = 0; i < controlPointsM.size(); ++i)
        {
            glm::mat4 transCube = glm::translate(glm::mat4(), controlPointsM.at(i));
            glMultMatrixf(&transCube[0][0]);
            {
                glColor3f(0, 1, 0);
                CSCI441::drawSolidSphere(0.5, 4, 5);
            };
            glMultMatrixf(&(glm::inverse(transCube))[0][0]);
        }
    }

    // TODO #04: Connect our control points
    if (hideControlCage)
    {
        glColor3f(1.000, 1.000, 0.000);
        glLineWidth(6.0);
        glDisable(GL_LIGHTING);
        glBegin(GL_LINE_STRIP);
        {
            for (int i = 0; i < numSegments; ++i)
            {
                glVertex3f(controlPointsM.at(i).x, controlPointsM.at(i).y, controlPointsM.at(i).z);
            }
        };
        glEnd();
        glLineWidth(1.0);
    }

    // TODO #05: Draw the Bezier Curve!
    if (hideControlPath)
    {
        glDisable(GL_LIGHTING);
        glLineWidth(6.0);
        glColor3f(0, 0, 1);
        for (int i = 0; i < numSegments - 1; i += 3)
        {
            renderBezierCurveM(controlPointsM.at(i), controlPointsM.at(i + 1), controlPointsM.at(i + 2),
                               controlPointsM.at(i + 3), 100);
        }
        glEnable(GL_LIGHTING);
        glLineWidth(1.0);
    }
    else
    { // This is to tape the hole in the code, IT will move only if the curve was drawn. It was never drawn whe nthey don't show.
        for (int i = 0; i < numSegments - 1; i += 3)
        {
            renderBezierCurveM(controlPointsM.at(i), controlPointsM.at(i + 1), controlPointsM.at(i + 2),
                               controlPointsM.at(i + 3), 100);
        }
    }
}

// These functions will draw the car
void drawCarM()
{
    glm::mat4 transCube = glm::translate(glm::mat4(), carPosM);
    glMultMatrixf(&transCube[0][0]);
    {
        glm::mat4 scaleTri = glm::scale(glm::mat4(), glm::vec3(1.0f, 1, 1.0f));
        glMultMatrixf(&scaleTri[0][0]);
        {
            glm::mat4 rotTri = glm::rotate(glm::mat4(), carTheta, glm::vec3(0.0f, 1.0f, 0.0f));
            glMultMatrixf(&rotTri[0][0]);
            {
                // and then scale it 10X in x and 10X in y
                drawCube();

                // The curve
                drawCurveM();

                // the mascot
                glm::mat4 transMas = glm::translate(glm::mat4(), controlPoint);
                glMultMatrixf(&transMas[0][0]);
                {
                    glm::mat4 rotTri = glm::rotate(glm::mat4(), (float)M_PI * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
                    glMultMatrixf(&rotTri[0][0]);
                    {
                        glm::mat4 scaleTri = glm::scale(glm::mat4(), glm::vec3(0.5f, 0.5f, 0.5f));
                        glMultMatrixf(&scaleTri[0][0]);
                        {
                            drawMascotM();
                        };
                        glMultMatrixf(&(glm::inverse(scaleTri))[0][0]);
                    };
                    glMultMatrixf(&(glm::inverse(rotTri))[0][0]);
                };
                glMultMatrixf(&(glm::inverse(transMas))[0][0]);
            };
            glMultMatrixf(&(glm::inverse(rotTri))[0][0]);
        };
        glMultMatrixf(&(glm::inverse(scaleTri))[0][0]);
    };
    glMultMatrixf(&(glm::inverse(transCube))[0][0]);
}

void drawMV()
{
    if (path == (int)controlPath.size())
    {
        cout << "control path size: " << (int)controlPath.size() << endl;
        path = 0;
    }
    controlPoint = controlPath.at(path);
    drawCarM();
}