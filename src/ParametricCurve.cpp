/**
 * Author:	Manorie Vachon
 * Course:	CPSC 587 Fundamentals of Computer Animation
 */

#include "ParametricCurve.h"

ParametricCurve::ParametricCurve() {
};

void ParametricCurve::setCurve(std::vector<Vec3f> contPoints) {
	N = 150;
	deltaU = 0.0001f;
	initialCurve = contPoints;
	numBezierCurves = floor(initialCurve.size()/3); // Assuming that the input control points are an
																					 				// even multiple of 3 which would make a close loop
	setTotalArcLength();
	deltaS = totalArcLength/N;
	arcLengthParameterization();
	setHighestPoint();
}

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

	Vec3f output = (pow((1 - distBez), 3) * P1) +
								 (3 * pow((1 - distBez), 2) * distBez * P2) +
								 (3 * (1 - distBez) * pow(distBez, 2) * P3) +
								 (pow(distBez, 3) * P4);

	return output;
}

void ParametricCurve::setTotalArcLength() {
	float L = 0.f; // Total arc length
	float u = 0.f;
	Vec3f p = getCurvePoint(u); // Point holder

	while (u <= 1.f) {
		u = u + deltaU;

		L = L + p.distance(getCurvePoint(u));

		p = getCurvePoint(u);
	}
	totalArcLength = L;
}

float ParametricCurve::getTotalArcLength() {
	return totalArcLength;
}


void ParametricCurve::arcLengthParameterization() {
	int i = 0;
	float currS = 0;
	float uh = 0;
	float ul = 0;
	Vec3f currPos;

	while (uh <= 1.f) {
		currPos = getCurvePoint(uh);
		uh = uh + deltaU;
		if ((currS + currPos.distance(getCurvePoint(uh))) > deltaS) {
			ul = uh - deltaU;
			uValues[i] = bisectionRefinement(ul, uh, deltaS, currS, currPos);
		  uh = uValues[i];
			i = i + 1;

			currS = 0;
		}
		else {
			currS = currS + currPos.distance(getCurvePoint(uh));
		}
	}
}

float ParametricCurve::bisectionRefinement(float ul, float uh, float deltaS, float currS, Vec3f currPos) {
	int i = 0;
	float um = 0.f;
	Vec3f posm;
	float deltaSm;

	while (i <= 100) {
			um = ((ul + uh)/2);
			posm = getCurvePoint(um);
			deltaSm = currS + currPos.distance(posm);
			if ((abs(deltaSm - deltaS) < 0.001) || (((uh - uh)/2) < 0.001)) {
				return um;
			}
			if (deltaSm < deltaS) {
				ul = um;
			}
			else {
				uh = um;
			}
	}
	return um;
}

void ParametricCurve::setHighestPoint() {
	float i = 0.f;
	highestPoint = getCurvePoint(i).y();
	printf("highest first point: %f\n", highestPoint);
	printf("point y: %f\n", getCurvePoint(i).y());

	while (i <= 1.f) {
		if (getCurvePoint(i).y() > highestPoint) {
			highestPoint = getCurvePoint(i).y();
			printf("high each time: %f\n", highestPoint);

		}
		i = i + deltaU;
	}
	printf("heighest point: %f\n", highestPoint);

}

float ParametricCurve::getVelocity(float s) {
	float v = 0.f;
	float vMin = 2.f;
	Vec3f currPos = getPosition(s);
	v = sqrt(2.f * 9.81f * (highestPoint - currPos.y()));
	printf("curr y: %f\n", currPos.y());
	printf("v: %f\n", v);
	if (s >= 0.f && s <= 21.05f) { // lifting stage
			v = vMin;
	}
	return v;
}

Vec3f ParametricCurve::getPosition(float distance) {
	int newIndex = floor(distance/deltaS);
	newIndex = newIndex % (N-1);
	printf("newindex: %d\n", newIndex);
printf("distance: %f\n", distance);
	float withinIndex = modDist((distance - (((float)newIndex)*deltaS)) / deltaS);

	float newPos = ((1.f - withinIndex) * uValues[(int)newIndex]) +
								 (withinIndex * uValues[(int)newIndex + 1]);

 	//printf("deltaS: %f\n", deltaS);
	printf("within: %f\n", withinIndex);
	printf("newPos: %f\n", newPos);


	return getCurvePoint(newPos);
}

float ParametricCurve::modDist(float position) {
	float original = position;
	float remain = position;
	if (position > (N-1)) {
		int division = position / (N-1);
		remain = (original - (division * (N-1)));
	}
	return remain;
}

float ParametricCurve::wrap(float s) {
	if (s > totalArcLength) {
		s = s - totalArcLength;
	}
	if (s < 0) {
		s = s + totalArcLength;
	}
	return s;
}

Vec3f ParametricCurve::tangent(float s) {
	return (getPosition(s + deltaU) - getPosition(s)) / deltaU;
}

Vec3f ParametricCurve::curvature(float s) {
	return (getPosition(s + deltaU) - 2*getPosition(s) + getPosition(s - deltaU)) / (deltaU*deltaU);
}

float ParametricCurve::tanAcc(float s, float dt) {
	return (getVelocity(s + getVelocity(s)*dt) - getVelocity(s)) / dt;
}
