#pragma once
#include "SpaceBody.h"
#include "CustomList.h"

class PhysicsModel {
public:
    SpaceBody* Planet;
    CustomList<SpaceBody*> Satellites;
    int steps = 50;
    double G = 6.67e-11;

    PhysicsModel() {
        Planet = new SpaceBody();
    }

    void ChangeV(SpaceBody* satellite, double newV) {
        satellite->Mp.V = satellite->Mp.V.GetE() * newV;
    }

    void ChangeM(SpaceBody* satellite, double newM) {
        satellite->Mp.M = newM;
    }

    void ChangeR(SpaceBody* satellite, double newR) {
        Vector orbit = satellite->Mp.R - Planet->Mp.R;
        orbit = orbit.GetE() * newR;
        satellite->Mp.R = Planet->Mp.R + orbit;
    }

    SpaceBody* AddSatellite(double v, double M, double radiusOrbite, double radius) {
        SpaceBody* ball = new SpaceBody();
        ball->Mp.V = Vector(0, -v);
        ball->Mp.R = Planet->Mp.R + Vector(radiusOrbite, 0);
        ball->Mp.M = M;
        ball->Radius = radius;
        Satellites.Add(ball);
        return ball;
    }

    void Update(double interval) {
        double dt = interval / steps;
        for (int i = 0; i < steps; i++) {
            MoveSatellite(dt);
        }
    }

    void MoveSatellite(double dt) {
        for (int i = 0; i < Satellites.Count(); i++) {
            SpaceBody* satellite = Satellites[i];

            if (satellite->get_V().GetLength() == 0) {
                continue;
            }

            Vector r = Planet->Mp.R - satellite->Mp.R;
            double sqLen = r.SqLength();

            if (sqLen > 0) {
                Vector F = r.GetE() * (G * Planet->Mp.M * satellite->Mp.M / sqLen);
                satellite->Mp.Accelerate(F);
            }
            satellite->Mp.Move(dt);
        }
    }
};