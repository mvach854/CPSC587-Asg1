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
	totalArcLength = getTotalArcLength();
	deltaS = totalArcLength/N;
	printf("arc is: %f\n", totalArcLength);
	printf("arc n before: %d\n", N);

	printf("deltaS is: %f\n", deltaS);
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

void ParametricCurve::arcLengthParameterization() {
	int i = 0;
	float currS = 0;
	float uh = 0;
	float ul = 0;
	Vec3f currPos;
deltaS = 0.387164f;
	while (uh <= 1) {
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

	while (i <= 5) {
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
	while (i <= 1.f) {
		if (getCurvePoint(i).y() > highestPoint) {
			highestPoint = getCurvePoint(i).y();
		}
		i = i + 0.001f;
	}

	printf("highestpoint is : %f\n", highestPoint);
}

float ParametricCurve::getVelocity(float currHeight) {
	float v = 0.f;
	v = sqrt(2.f * 9.81f * (highestPoint - currHeight));
	printf("currHeight: %f\n", currHeight);
	printf("highestPoint: %f\n", highestPoint);

	return v;
}

Vec3f ParametricCurve::getPosition(float distance) {
	// int newIndex = floor(distance/deltaS);
	// printf("newIndexafter floor %d\n", newIndex);
	//
	// newIndex = newIndex * deltaS;
	// printf("newIndexafter delta mult %d\n", newIndex);
	//
	// newIndex = newIndex%(N);
	//
	// distance = fmod(distance, 100);
	// printf("newIndex %d\n", newIndex);
	// float withinIndex = (distance - newIndex) / deltaS;
	// printf("withinIndex %d\n", withinIndex);
	printf("distance before %f\n", distance);
	distance = modDist(distance);
	printf("distance %f\n", distance);

//	printf("deltaS beforeeeee %f\n", deltaS);

	float newIndex = floor(distance/deltaS);
//	printf("newIndexafter floor %f\n", newIndex);

	newIndex = newIndex * deltaS;
//	printf("newIndexafter delta mult %f\n", newIndex);

//	newIndex = newIndex%(N);

//	printf("newIndex %d\n", newIndex);
	float withinIndex = (distance - newIndex) / deltaS;
//	printf("withinIndex %f\n", withinIndex);
//	printf("final index %d\n", (int)newIndex);

	float newPos = ((1.f - withinIndex) * uValues[(int)newIndex]) +
								 (withinIndex * uValues[(int)newIndex + 1]);
//	newPos = modDist(newPos); // to keep it between 0 and 1
//	printf("newPos %f\n", newPos);
	printf("newPos %f\n", newPos);



	return getCurvePoint(newPos);
//std:: cout << "position along curve " << deltaS << " : " << uValues[deltaS] << std::endl;
//	std::cout << "test position: " << getCurvePoint(uValues[deltaS]) << std::endl;
//	return getCurvePoint(uValues[deltaS%N]);
}

float ParametricCurve::modDist(float position) {
	float original = position;
	float remain = position;
	if (position > (N-1)) {
		int division = position / (N-1);
		remain = (original - (division * (N-1)));
		printf("division is  %d\n", division);

	}
	return remain;
}
