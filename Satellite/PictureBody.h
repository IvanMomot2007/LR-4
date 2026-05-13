#pragma once
#include "SpaceBody.h"
#include "PlanetsData.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;

public ref class PictureBody {
public:
    SpaceBody* BallObj;
    double Scale;
    String^ Name;
    Point p0;
    double AspectRatio;

    bool IsExploding;
    bool IsDead;
    int CollisionTimer;
    int MaxCollisionTime;

    Image^ PlanetImage;
    Rectangle VisualBounds;

    PictureBody(SpaceBody* ball, double scale) {
        BallObj = ball;
        Scale = scale;
        p0 = Point(0, 0);
        AspectRatio = 1.0;

        IsExploding = false;
        IsDead = false;
        MaxCollisionTime = 60;
        CollisionTimer = 0;
        PlanetImage = nullptr;
    }

    void SetSettings(String^ name) {
        String^ imagePath = System::IO::Path::Combine(System::AppDomain::CurrentDomain->BaseDirectory, "ImagesSatellite", name + ".jpg");
        try {
            if (System::IO::File::Exists(imagePath)) {
                PlanetImage = Image::FromFile(imagePath);
                AspectRatio = (double)PlanetImage->Width / PlanetImage->Height;
            }
        }
        catch (...) {}
        BallObj = PlanetsData::GetPlanet(name);
    }

    void UpdateLogic() {
        if (IsExploding && CollisionTimer > 0) {
            CollisionTimer--;
        }
        else if (IsExploding && CollisionTimer <= 0) {
            IsExploding = false;
            IsDead = true;
        }
    }

    void Draw(Graphics^ g) {
        if (BallObj == nullptr || IsDead) return;

        double k = 20;
        int height = (int)(BallObj->Radius * Scale * k);
        if (height < 10) height = 10;
        int width = (int)(height * AspectRatio);

        int x = p0.X + (int)(BallObj->get_R().X * Scale) - width / 2;
        int y = p0.Y - (int)(BallObj->get_R().Y * Scale) - height / 2;

        VisualBounds = Rectangle(x, y, width, height);

        if (IsExploding) {
            float ratio = 1.0f - ((float)CollisionTimer / MaxCollisionTime);

            int expSize = height + (int)(height * ratio * 2.5f); 
            int alpha = 255 - (int)(255 * ratio);
            if (alpha < 0) alpha = 0;

            int exX = p0.X + (int)(BallObj->get_R().X * Scale) - expSize / 2;
            int exY = p0.Y - (int)(BallObj->get_R().Y * Scale) - expSize / 2;

            Color fireColor = Color::FromArgb(alpha, 255, 69, 0);
            Color coreColor = Color::FromArgb(alpha, 255, 215, 0);

            SolidBrush^ brush1 = gcnew SolidBrush(fireColor);
            SolidBrush^ brush2 = gcnew SolidBrush(coreColor);

            g->FillEllipse(brush1, exX, exY, expSize, expSize);
            g->FillEllipse(brush2, exX + expSize / 4, exY + expSize / 4, expSize / 2, expSize / 2);

            delete brush1;
            delete brush2;
        }
        else {
            if (PlanetImage != nullptr) {
                g->DrawImage(PlanetImage, VisualBounds);
            }
        }
    }

    void StartCollision() {
        if (!IsExploding && !IsDead) {
            IsExploding = true;
            CollisionTimer = MaxCollisionTime;
        }
    }
};