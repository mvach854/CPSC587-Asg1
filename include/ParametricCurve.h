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
	int getTotalArcLength();

private:
	std::vector<Vec3f> initialCurve;
	float numBezierCurves;
};

#endif // PARAMETRIC_CURVE_H
