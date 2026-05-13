#pragma once
#include "MP.h"

class SpaceBody {
public:
    MP Mp;
    double Radius;

    SpaceBody() : Radius(0) {}

    Vector get_V() { return Mp.V; }
    void set_V(Vector value) { Mp.V = value; }

    Vector get_A() { return Mp.A; }
    void set_A(Vector value) { Mp.A = value; }

    double get_M() { return Mp.M; }
    void set_M(double value) { Mp.M = value; }

    Vector get_R() { return Mp.R; }
    void set_R(Vector value) { Mp.R = value; }

    Vector get_NextR() { return Mp.NextR; }
};