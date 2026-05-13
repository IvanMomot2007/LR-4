#pragma once
#include "Vector.h"

class MP {
public:
    Vector R, A, V, NextR;
    double Radius;
    double M;

    MP() : M(0), Radius(0) {}

    void Move(double dt) {
        V = V + A * dt;
        R = R + V * dt;
        NextR = R + (V + A) * dt;
    }

    void Accelerate(Vector F) {
        if (M != 0) A = F / M;
    }
};