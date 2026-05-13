#pragma once
#include "PointD.h"
#include <cmath>

class Vector {
public:
    double X, Y;

    Vector(double x = 0, double y = 0) : X(x), Y(y) {}
    Vector(PointD p) : X(p.X), Y(p.Y) {}
    Vector(PointD p1, PointD p2) : X(p2.X - p1.X), Y(p2.Y - p1.Y) {}

    double SqLength() { return X * X + Y * Y; }
    double GetLength() { return std::sqrt(SqLength()); }
    Vector Normal() { return Vector(-Y, X); }

    Vector operator+(Vector b) { return Vector(X + b.X, Y + b.Y); }
    Vector operator-() { return Vector(-X, -Y); }
    Vector operator-(Vector b) { return *this + (-b); }
    Vector operator*(double c) { return Vector(X * c, Y * c); }
    Vector operator/(double c) { return *this * (1.0 / c); }

    double operator*(Vector b) { return DotProduct(*this, b); }

    Vector GetE() { return *this / GetLength(); }
    Vector Projection(Vector OnVector) { return OnVector * ((*this * OnVector) / OnVector.SqLength()); }
    Vector Mirror(Vector v) { return *this - Projection(v.Normal()) * 2; }

    void HorizontalBounce() { Y = -Y; }
    void VerticalBounce() { X = -X; }

    static double DotProduct(Vector a, Vector b) { return a.X * b.X + a.Y * b.Y; }
};

inline Vector operator*(double c, Vector v) { return Vector(c * v.X, c * v.Y); }