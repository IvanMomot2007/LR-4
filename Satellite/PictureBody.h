#pragma once
#include "SpaceBody.h"
#include "PlanetsData.h"

using namespace System;
using namespace System::Drawing;

public ref class PictureBody {
public:
    SpaceBody* BodyObj;
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

    int ExplodeX, ExplodeY, ExplodeSize;

    PictureBody(SpaceBody* ball, double scale) {
        BodyObj = ball;
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
        catch (...) {
            PlanetImage = nullptr;
        }
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
        if (IsDead) return;

        if (IsExploding) {
            float ratio = 1.0f - ((float)CollisionTimer / MaxCollisionTime);
            int expSize = ExplodeSize + (int)(ExplodeSize * ratio * 2.5f);
            int alpha = 255 - (int)(255 * ratio);
            if (alpha < 0) alpha = 0;

            int exX = ExplodeX + ExplodeSize / 2 - expSize / 2;
            int exY = ExplodeY + ExplodeSize / 2 - expSize / 2;

            SolidBrush^ fireBrush = gcnew SolidBrush(Color::FromArgb(alpha, 255, 69, 0));
            SolidBrush^ coreBrush = gcnew SolidBrush(Color::FromArgb(alpha, 255, 215, 0));

            g->FillEllipse(fireBrush, exX, exY, expSize, expSize);
            g->FillEllipse(coreBrush, exX + expSize / 4, exY + expSize / 4, expSize / 2, expSize / 2);

            delete fireBrush;
            delete coreBrush;
        }
        else if (BodyObj != nullptr) {
            double k = 20;
            int height = (int)(BodyObj->Radius * Scale * k);
            if (height < 10) height = 10;
            int width = (int)(height * AspectRatio);

            int x = p0.X + (int)(BodyObj->get_R().X * Scale) - width / 2;
            int y = p0.Y - (int)(BodyObj->get_R().Y * Scale) - height / 2;

            VisualBounds = Rectangle(x, y, width, height);

            if (PlanetImage != nullptr) {
                g->DrawImage(PlanetImage, VisualBounds);
            }
        }
    }

    void StartCollision() {
        if (!IsExploding && !IsDead && BodyObj != nullptr) {
            IsExploding = true;
            CollisionTimer = MaxCollisionTime;

            double k = 20;
            int height = (int)(BodyObj->Radius * Scale * k);
            if (height < 10) height = 10;
            int width = (int)(height * AspectRatio);

            ExplodeX = p0.X + (int)(BodyObj->get_R().X * Scale) - width / 2;
            ExplodeY = p0.Y - (int)(BodyObj->get_R().Y * Scale) - height / 2;
            ExplodeSize = height;
        }
    }
};