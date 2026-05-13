#pragma once
#include "SpaceBody.h"
#include "PlanetsData.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;

public ref class PictureBody {
public:
    SpaceBody* BallObj;
    PictureBox^ Pb;
    double Scale;
    String^ Name;
    Point p0;
    double AspectRatio;
    bool IsCollided;
    int CollisionTimer;
    Image^ OriginalImage;

    PictureBody(SpaceBody* ball, Control^ parent, double scale) {
        BallObj = ball;
        Scale = scale;
        p0 = Point(0, 0);
        AspectRatio = 1.0;
        IsCollided = false;
        CollisionTimer = 0;
        OriginalImage = nullptr;
        Pb = CreatePb(parent, 50);
    }

    void SetSettings(String^ name) {
        String^ imagePath = System::IO::Path::Combine(System::AppDomain::CurrentDomain->BaseDirectory, "ImagesSatellite", name + ".jpg");
        try {
            if (System::IO::File::Exists(imagePath)) {
                Image^ img = Image::FromFile(imagePath);
                Pb->Image = img;
                OriginalImage = img;
                AspectRatio = (double)img->Width / img->Height;
            }
        }
        catch (...) {}
        BallObj = PlanetsData::GetPlanet(name);
    }

    void Update() {
        double k = 20;
        int height = (int)(BallObj->Radius * Scale * k);
        if (height < 10) height = 10;
        int width = (int)(height * AspectRatio);

        Pb->Size = Drawing::Size(width, height);
        Pb->Location = Point(
            p0.X + (int)(BallObj->get_R().X * Scale) - Pb->Width / 2,
            p0.Y - (int)(BallObj->get_R().Y * Scale) - Pb->Height / 2
        );

        UpdateCollisionAnimation();
    }

    void StartCollision() {
        IsCollided = true;
        CollisionTimer = 32;
    }

    void UpdateCollisionAnimation() {
        if (IsCollided && CollisionTimer > 0) {
            CollisionTimer--;

            if (CollisionTimer % 4 < 2) {
                if (OriginalImage != nullptr) {
                    Pb->Image = OriginalImage;
                    Pb->BackColor = Color::Transparent;
                }
            }
            else {
                Pb->BackColor = Color::OrangeRed;
                Pb->Image = nullptr;
            }
        }
        else if (IsCollided && CollisionTimer <= 0) {
            IsCollided = false;
            Pb->Visible = false;
            Pb->BackColor = Color::Transparent;
        }
    }

    static PictureBox^ CreatePb(Control^ parent, int size) {
        PictureBox^ pb = gcnew PictureBox();
        pb->Size = Drawing::Size(size, size);
        pb->SizeMode = PictureBoxSizeMode::Zoom;
        pb->BackColor = Color::Transparent;
        parent->Controls->Add(pb);
        return pb;
    }
};
