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

using namespace std;

int vMove = 0;
int vRot = 0;
float aRot = 0.0f;
glm::vec3 vLoc;
glm::vec3 vLocXYZ;
glm::vec3 vDir;
glm::vec3 vUp(0, 1, 0);
glm::vec3 vAxisRotate;
float vSize = 0.28;
float slow = 20000;
float vAngleRotate;
float vTheta;

glm::vec3 mascotLoc;
int currentPoint = 0;
float extend = 0.0f;
float extDir = 1.0f;

float patchWidth = 30.0f;
float patchLength = 30.0f;

vector<glm::vec3> p;
float trackPointVal = 0.0f;

bool showControl = false;
int numLines;

//Calculate each point to draw on the curve
glm::vec3 evaluateBezierCurveMN(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t)
{
    glm::vec3 a, b, c, point;
    a = -p0 + 3.0f * p1 - 3.0f * p2 + p3;
    b = 3.0f * p0 - 6.0f * p1 + 3.0f * p2;
    c = -3.0f * p0 + 3.0f * p1;
    point = a * glm::pow(t, 3.0f) + b * glm::pow(t, 2.0f) + c * t + p0;
    return point;
}

//Calculate each vertex for the patch
glm::vec3 evaluateBezierPatch(float u, float v)
{
    glm::vec3 point = evaluateBezierCurveMN(
        evaluateBezierCurveMN(p[0], p[1], p[2], p[3], u),
        evaluateBezierCurveMN(p[4], p[5], p[6], p[7], u),
        evaluateBezierCurveMN(p[8], p[9], p[10], p[11], u),
        evaluateBezierCurveMN(p[12], p[13], p[14], p[15], u),
        v);
    return point;
}

//Find d/dt of a Bezier Curve
glm::vec3 dEvaluateBezierCurveMN(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t)
{
    glm::vec3 a, b, c, point;
    a = -p0 + 3.0f * p1 - 3.0f * p2 + p3;
    b = 3.0f * p0 - 6.0f * p1 + 3.0f * p2;
    c = -3.0f * p0 + 3.0f * p1;
    point = a * 3.0f * glm::pow(t, 2.0f) + b * 2.0f * t + c;
    return point;
}

//Find d/du of the patch
glm::vec3 duPatch(float u, float v)
{
    glm::vec3 du = evaluateBezierCurveMN(
        dEvaluateBezierCurveMN(p[0], p[1], p[2], p[3], u),
        dEvaluateBezierCurveMN(p[4], p[5], p[6], p[7], u),
        dEvaluateBezierCurveMN(p[8], p[9], p[10], p[11], u),
        dEvaluateBezierCurveMN(p[12], p[13], p[14], p[15], u),
        v);
    return du;
}

//Find d/dv of the patch
glm::vec3 dvPatch(float u, float v)
{
    glm::vec3 dv = dEvaluateBezierCurveMN(
        evaluateBezierCurveMN(p[0], p[1], p[2], p[3], u),
        evaluateBezierCurveMN(p[4], p[5], p[6], p[7], u),
        evaluateBezierCurveMN(p[8], p[9], p[10], p[11], u),
        evaluateBezierCurveMN(p[12], p[13], p[14], p[15], u),
        v);
    return dv;
}

//Find the normal of a vertex on the patch
glm::vec3 normalPatch(float u, float v)
{
    glm::vec3 dv = dvPatch(u, v);
    glm::vec3 du = duPatch(u, v);
    return glm::normalize(glm::cross(dv, du));
}

//Find the dot product with the normal of the patch at a point
float dotPatch(glm::vec3 otherVector, float u, float v)
{
    return glm::acos(glm::dot(otherVector, normalPatch(u, v)));
}

//Find the cross product with the normal of the patch at a point
glm::vec3 crossPatch(glm::vec3 otherVector, float u, float v)
{
    return glm::cross(otherVector, normalPatch(u, v));
}

// loadControlPoints() /////////////////////////////////////////////////////////
//
//  Load our control points from file and store them in
//	the global variable controlPoints
//
////////////////////////////////////////////////////////////////////////////////
bool loadControlPointsMN(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file != NULL)
    {
        int numPoints;
        fscanf(file, "%i", &numPoints);
        if (numPoints % 16 != 0)
            return false;
        for (int i = 0; i < numPoints * 16; i++)
        {
            glm::vec3 v;
            if (fscanf(file, "%f, %f, %f", &v.x, &v.y, &v.z) == 3)
            {
                p.push_back(v);
            }
            else
                return false;
        }
    }
    else
        return false;
    return true;
}

// renderBezierCurve() //////////////////////////////////////////////////////////
//
// Responsible for drawing a Bezier Curve as defined by four control points.
//  Breaks the curve into n segments as specified by the resolution.
//
////////////////////////////////////////////////////////////////////////////////

//Draw the lines conecting the control points
void drawControl()
{
    glDisable(GL_LIGHTING);
    glColor3f(1, 1, 0);
    glLineWidth(3.0f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < (int)p.size(); i++)
    {
        glVertex3f(p.at(i).x, p.at(i).y, p.at(i).z);
    }
    glEnd();
    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
}

//Draw the points read in from the file
void drawPoint(glm::vec3 v)
{
    glColor3f(0, 1, 0);
    glm::mat4 trans = glm::translate(glm::mat4(), v);
    glMultMatrixf(&trans[0][0]);
    CSCI441::drawSolidSphere(0.3, 16, 16);
    glMultMatrixf(&(glm::inverse(trans))[0][0]);
}

//Draw the rectangle body of the vehicle
void drawBody()
{
    glColor3f(0.545, 0.271, 0.075);
    glm::mat4 scl = glm::scale(glm::mat4(), glm::vec3(1.0, 0.1, 1.5));
    glMultMatrixf(&scl[0][0]);
    CSCI441::drawSolidCube(1);
    glMultMatrixf(&(glm::inverse(scl))[0][0]);
}

//Draw a wheel of the vehicle
void drawWheel()
{
    glColor3f(1.000, 0.714, 0.757);
    glm::mat4 transWheel = glm::translate(glm::mat4(), glm::vec3(0, 0, 0));
    glMultMatrixf(&transWheel[0][0]);
    CSCI441::drawSolidTorus(0.12, 0.2, 8, 8);
    glMultMatrixf(&(glm::inverse(transWheel))[0][0]);
}

//Draw the axle and wheel of the vehicle and rotate when the vehicle is moving
void drawAxle(float loc)
{
    glm::mat4 transAxle = glm::translate(glm::mat4(), glm::vec3(0.6, 0, loc));
    glMultMatrixf(&transAxle[0][0]);
    //Spin the axle while moving
    glm::mat4 spinAxle = glm::rotate(glm::mat4(), aRot, glm::vec3(1, 0, 0));
    glMultMatrixf(&spinAxle[0][0]);
    glm::mat4 rotAxle = glm::rotate(glm::mat4(), (float)M_PI / 2.0f, glm::vec3(0, 0, 1));
    glMultMatrixf(&rotAxle[0][0]);
    CSCI441::drawSolidCylinder(0.125, 0.125, 1.2, 20, 20);
    glm::mat4 rotDisk = glm::rotate(glm::mat4(), (float)M_PI / 2.0f, glm::vec3(1, 0, 0));
    glMultMatrixf(&rotDisk[0][0]);
    glColor3f(0, 0, 0);
    CSCI441::drawSolidDisk(0.001, 0.126, 20, 20);
    drawWheel();
    glm::mat4 tDisk = glm::translate(glm::mat4(), glm::vec3(0, 0, -1.2));
    glMultMatrixf(&tDisk[0][0]);
    glColor3f(0, 0, 0);
    CSCI441::drawSolidDisk(0.001, 0.126, 20, 20);
    drawWheel();
    glMultMatrixf(&(glm::inverse(tDisk))[0][0]);
    glMultMatrixf(&(glm::inverse(rotDisk))[0][0]);
    glMultMatrixf(&(glm::inverse(rotAxle))[0][0]);
    glMultMatrixf(&(glm::inverse(spinAxle))[0][0]);
    glMultMatrixf(&(glm::inverse(transAxle))[0][0]);
}

//Draw the vechicle the user moves around the city
void drawVehicle()
{
    glm::mat4 orient = glm::rotate(glm::mat4(), vAngleRotate, vAxisRotate);
    glMultMatrixf(&orient[0][0]);
    glm::mat4 tranAway = glm::translate(glm::mat4(), glm::vec3(0, vSize, 0));
    glMultMatrixf(&tranAway[0][0]);
    glm::mat4 rot = glm::rotate(glm::mat4(), vTheta, glm::vec3(0, 1, 0));
    glMultMatrixf(&rot[0][0]);
    drawBody();
    glColor3f(0.863, 0.078, 0.235);
    drawAxle(0.75);
    glColor3f(0.333, 0.420, 0.184);
    drawAxle(-0.75);
    glMultMatrixf(&(glm::inverse(rot))[0][0]);
    glMultMatrixf(&(glm::inverse(tranAway))[0][0]);
    glMultMatrixf(&(glm::inverse(orient))[0][0]);
}

//Draws the main ball of the mascot
void drawBall()
{
    glColor3f(1.000, 0.714, 0.757);
    CSCI441::drawSolidSphere(0.5, 15, 15);
}

//Draws the spikes on the mascot
void drawSpike()
{
    glm::mat4 rot = glm::rotate(glm::mat4(), (float)-M_PI / 2.0f, glm::vec3(0, 0, 1));
    glMultMatrixf(&rot[0][0]);
    glColor3f(0.824, 0.412, 0.118);
    CSCI441::drawSolidCone(.1, 0.7, 15, 15);
    glMultMatrixf(&(glm::inverse(rot))[0][0]);
}

//Draws the mascot
void drawMascot()
{
    drawBall();
    for (float i = 0.0f; i < 2.0f * M_PI; i += M_PI / 4.0f)
    {
        glm::mat4 rot = glm::rotate(glm::mat4(), i, glm::vec3(0, 1, 0));
        glMultMatrixf(&rot[0][0]);
        //Move the spikes in and out of the mascot
        glm::mat4 tran = glm::translate(glm::mat4(), glm::vec3(extend, 0, 0));
        glMultMatrixf(&tran[0][0]);
        drawSpike();
        glMultMatrixf(&(glm::inverse(tran))[0][0]);
        glMultMatrixf(&(glm::inverse(rot))[0][0]);
    }
}

//Draw the curve by connecting the points genereated in evaluateBezierCurve()
void drawCurve()
{
    if (showControl)
    {
        // Draw our control points
        for (int i = 0; i < (int)p.size(); i++)
        {
            drawPoint(p.at(i));
        }
        // Connect our control points
        drawControl();

        /* //Draw the Bezier Curve!
        for (int i = 0; i < (int)controlPoints.size() - 1; i += 3)
        {
            renderBezierCurve();
        } */
    }
}

//Move the mascot around the curve
/*  void moveMascot()
{
    if (currentPoint == (int)points.size() - 1)
        currentPoint = 0;
    mascotLoc = points[currentPoint];
    currentPoint++;
    if (extend >= 0.7)
        extDir = -1.0f;
    if (extend <= 0.0)
        extDir = 1.0f;
    extend += 0.01 * extDir;
}  */

//Rotate the curve around the vehicle
/* void rotateCurve()
{
    if (curveRot >= 2.0 * M_PI)
        curveRot = 0;
    curveRot += 0.01;
}  */

void drawMN()
{
    //Move vehicle
    glm::mat4 loc = glm::translate(glm::mat4(), vLocXYZ);
    glMultMatrixf(&loc[0][0]);
    drawVehicle();
    //center the curve's y location
    //  glm::mat4 curveY = glm::translate(glm::mat4(), glm::vec3(0, -center.y, 0));
    //  glMultMatrixf(&curveY[0][0]);
    //Rotate the cureve around the z axis
    //  glm::mat4 rotCurve = glm::rotate(glm::mat4(), curveRot, glm::vec3(0, 0, 1));
    //  glMultMatrixf(&rotCurve[0][0]);
    //Move the mascot along the curve
    // glm::mat4 tran = glm::translate(glm::mat4(), mascotLoc);
    //glMultMatrixf(&tran[0][0]);
    // drawMascot();
    // glMultMatrixf(&(inverse(tran)[0][0]));
    // glMultMatrixf(&(inverse(rotCurve)[0][0]));
    // glMultMatrixf(&(glm::inverse(curveY))[0][0]);
    glMultMatrixf(&(glm::inverse(loc))[0][0]);
}

//Reorient the vehicle
//Used after the vehicle moves
void recomputeVehicleOrientation()
{
    vAxisRotate = crossPatch(vUp, vLoc.x, vLoc.z);
    vAngleRotate = dotPatch(vUp, vLoc.x, vLoc.z);
    vLocXYZ = evaluateBezierPatch(vLoc.x, vLoc.z);
}

//Recompute the direction vector when the A or D key is pressed
void recomputeVehicleDirection()
{
    vDir = glm::vec3(-glm::sin(vTheta), 0, -glm::cos(vTheta));
    //Normalize the direction vector
    vDir = glm::normalize(vDir);
}

//Move the vehicle if the W or S key is pressed
void moveVehicle()
{
    if (vMove != 0)
    {
        glm::vec3 vLocTest;
        vLocTest.x = vLoc.x + (float)vMove * vDir.x * (patchWidth / slow);
        vLocTest.z = vLoc.z + (float)vMove * vDir.z * (patchLength / slow);
        if (vLocTest.x > 0 && vLocTest.x < 1 && vLocTest.z > 0 && vLocTest.z < 1)
        {
            if (aRot > 2.0f * M_PI && vMove > 0)
                aRot = 0.0f;
            if (aRot < -2.0f * M_PI && vMove < 0)
                aRot = 0.0f;
            aRot -= vMove * 0.07;
            vLoc = vLocTest;
            recomputeVehicleOrientation();
        }
    }
}

void MNSetUp()
{

    //Give the vehicle a starting point
    vLoc = glm::vec3(0.5, 0, 0.5);
    vTheta = 0;
    recomputeVehicleDirection();
    recomputeVehicleOrientation();

    //Set the initial mascot location
    //mascotLoc = controlPoints[0];
}