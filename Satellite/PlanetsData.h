#pragma once
#include "SpaceBody.h"

using namespace System;
using namespace System::Collections::Generic;

public ref class PlanetsData {
public:
    static Dictionary<String^, IntPtr>^ Planets = gcnew Dictionary<String^, IntPtr>();

    static void Init() {
        if (Planets->Count > 0) return;

        auto addPlanet = [](String^ name, double M, double radius) {
            SpaceBody* b = new SpaceBody();
            b->set_V(Vector(0, 0));
            b->set_R(Vector(0, 0));
            b->set_M(M);
            b->Radius = radius;
            Planets->Add(name, IntPtr(b));
            };

        addPlanet("Mercury", 0.32868e24, 2439700);
        addPlanet("Venus", 4.81068e24, 6051800);
        addPlanet("Earth", 5.9736e24, 6371000);
        addPlanet("Mars", 0.6418e24, 3389500);
        addPlanet("Jupiter", 1.8986e27, 69911000);
        addPlanet("Saturn", 561.80376e24, 58232000);
        addPlanet("Uran", 86.0544e24, 25362000);
        addPlanet("Neptune", 101.592e24, 24622000);
    }

    static SpaceBody* GetPlanet(String^ name) {
        Init();
        return (SpaceBody*)Planets[name].ToPointer();
    }

    static cli::array<String^>^ GetAllNames() {
        Init();
        cli::array<String^>^ names = gcnew cli::array<String^>(Planets->Count);
        Planets->Keys->CopyTo(names, 0);
        return names;
    }
};