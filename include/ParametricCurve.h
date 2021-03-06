/**
 * Author:	Manorie Vachon
 * Course:	CPSC 587 Fundamentals of Computer Animation
 */

#ifndef PARAMETRIC_CURVE_H
#define PARAMETRIC_CURVE_H

#include <iostream>
#include <vector>
#include <cmath>
#include <GLFW/glfw3.h>
#include "Vec3f.h"

class ParametricCurve {
public:
	ParametricCurve();
	void setCurve(std::vector<Vec3f> contPoints);
	Vec3f getCurvePoint(float distAlongTrack);
	float getVelocity(float s);
	Vec3f getPosition(float distance);
	float getTotalArcLength();
	float wrap(float s);
	Vec3f tangent(float s);
	Vec3f curvature(float s);
	float tanAcc(float s, float dt);

private:
	std::vector<Vec3f> initialCurve;
	float numBezierCurves;
	int N;
	float uValues[150]; // 150 represents N values that the curve will be split into
	float totalArcLength;
	float deltaS; // arc length subdivision size
	float deltaU;
	float highestPoint;

	void arcLengthParameterization();
	void setTotalArcLength();
	float bisectionRefinement(float ul, float uh, float deltaS, float currS, Vec3f currPos);
	void setHighestPoint();
	float modDist(float position);
};

#endif // PARAMETRIC_CURVE_H
