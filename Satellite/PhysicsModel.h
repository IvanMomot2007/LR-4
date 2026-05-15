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

            if (satellite->get_V().GetLength() == 0 && satellite->get_A().GetLength() < 0.01) {
                satellite->Mp.Move(dt);
                continue;
            }

            Vector r = Planet->Mp.R - satellite->Mp.R;

            double sqLen = r.SqLength();
            if (sqLen != 0) {
                Vector F = r.GetE() * (G * Planet->Mp.M * satellite->Mp.M / sqLen);
                satellite->Mp.Accelerate(F);
            }
            satellite->Mp.Move(dt);
        }
    }

    bool CheckCollisionWithPlanet(int satelliteIndex) {
        if (satelliteIndex < 0 || satelliteIndex >= Satellites.Count()) return false;

        SpaceBody* satellite = Satellites[satelliteIndex];
        Vector distVector = satellite->Mp.R - Planet->Mp.R;
        double distance = distVector.GetLength();
        double minDistance = Planet->Radius + satellite->Radius;

        return distance <= minDistance;
    }

    bool CheckCollisionBetweenSatellites(int sat1Index, int sat2Index) {
        if (sat1Index < 0 || sat1Index >= Satellites.Count()) return false;
        if (sat2Index < 0 || sat2Index >= Satellites.Count()) return false;
        if (sat1Index == sat2Index) return false;

        SpaceBody* sat1 = Satellites[sat1Index];
        SpaceBody* sat2 = Satellites[sat2Index];
        Vector distVector = sat1->Mp.R - sat2->Mp.R;
        double distance = distVector.GetLength();
        double minDistance = sat1->Radius + sat2->Radius;

        return distance <= minDistance;
    }

    void CheckAllCollisions(CustomList<int>& collidedIndices) {
        for (int i = 0; i < Satellites.Count(); i++) {
            if (CheckCollisionWithPlanet(i)) {
                collidedIndices.Add(i);
            }
        }

        for (int i = 0; i < Satellites.Count(); i++) {
            for (int j = i + 1; j < Satellites.Count(); j++) {
                if (CheckCollisionBetweenSatellites(i, j)) {
                    bool iAlreadyAdded = false, jAlreadyAdded = false;
                    for (int k = 0; k < collidedIndices.Count(); k++) {
                        if (collidedIndices[k] == i) iAlreadyAdded = true;
                        if (collidedIndices[k] == j) jAlreadyAdded = true;
                    }
                    if (!iAlreadyAdded) collidedIndices.Add(i);
                    if (!jAlreadyAdded) collidedIndices.Add(j);
                }
            }
        }
    }
};