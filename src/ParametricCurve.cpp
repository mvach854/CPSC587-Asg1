/**
 * Author:	Manorie Vachon
 * Course:	CPSC 587 Fundamentals of Computer Animation
 */

#include "ParametricCurve.h"

ParametricCurve::ParametricCurve() {};

void ParametricCurve::setCurve(std::vector<Vec3f> contPoints) {
	initialCurve = contPoints;
	numBezierCurves = floor(initialCurve.size()/3); // Assuming that the input control points are an
}																					 // even multiple of 3 which would make a close loop

Vec3f ParametricCurve::getCurvePoint(float distAlongTrack) {
	float bezierSeg = floor(numBezierCurves * distAlongTrack); // Finds which Bexier segment the point is in
	// Special case with the last point in the curve
	if (bezierSeg == numBezierCurves) {
		bezierSeg = bezierSeg - 1;
	}

	// Convert to the corresponding distance along Bezier segment
	float startOfSeg = ((bezierSeg)/numBezierCurves);
	float distBez = ((distAlongTrack-startOfSeg) / (1/numBezierCurves));

	// Put through Bezier equation
	int firstPoint = static_cast<int>(startOfSeg * initialCurve.size());
	Vec3f P1 = initialCurve[firstPoint];
	Vec3f P2 = initialCurve[firstPoint + 1];
	Vec3f P3 = initialCurve[firstPoint + 2];
	Vec3f P4 = initialCurve[(firstPoint + 3)%initialCurve.size()];
/*	printf("distAlongTrack = %f\n", distAlongTrack);
	printf("bezierSeg** = %f\n", bezierSeg);
	printf("startOfSeg** = %f\n", startOfSeg);
	printf("distBez** = %f\n", distBez);
	int i = (firstPoint + 3)%initialCurve.size();
	printf("first point** = %d\n", firstPoint);
	printf("last point** = %d\n", i);*/
	Vec3f output = (pow((1 - distBez), 3) * P1) +
								 (3 * pow((1 - distBez), 2) * distBez * P2) +
								 (3 * (1 - distBez) * pow(distBez, 2) * P3) +
								 (pow(distBez, 3) * P4);

/* 	std::cout << "P1: " << P1 << "\n";
	std::cout << "P2: " << P2 << "\n";
	std::cout << "P3: " << P3 << "\n";
	std::cout << "P4: " << P4 << "\n";

	std::cout << "output point: " << output << "\n";*/

	return output;
}

float ParametricCurve::getTotalArcLength() {
	float deltaU = 0.001f;
	float L = 0.f; // Total arc length
	float u = 0.f;
	Vec3f p = getCurvePoint(u); // Point holder
//	printf("point = %f\n", p);
	while (u <= 1.f) {
		u = u + deltaU;
//		printf("u = %f\n", u);
//		printf("Lb = %f\n", L);
	//	std::cout << getCurvePoint(u) << "\n";
		L = L + p.distance(getCurvePoint(u));
//		printf("La = %f\n", L);
//		printf("distance = %f\n", p.distance(getCurvePoint(u)));
//		printf("current L = %d\n", L);
	//	printf("new point = %f\n", p);
		p = getCurvePoint(u);
	}
	return L;
}

float ParametricCurve::getVelocity(float currHeight) {
	float v = 0.f;

	return v;
}

Vec3f ParametricCurve::getPosition(float deltaS) {
	Vec3f pos;

	return pos;
}
