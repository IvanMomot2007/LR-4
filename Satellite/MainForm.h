#pragma once

#include "PointD.h"
#include "Vector.h"
#include "MP.h"
#include "SpaceBody.h"
#include "CustomList.h"
#include "PhysicsModel.h"
#include "PictureBody.h"
#include "PlanetsData.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections::Generic;
using namespace System::Windows::Forms;
using namespace System::Drawing;

namespace Satellite {

    public ref class MainForm : public System::Windows::Forms::Form
    {
    public:
        MainForm(void)
        {
            InitializeComponent();
            physicsModel = new PhysicsModel();
            pbs = gcnew List<PictureBody^>();
            InitializeSystem();
        }

    protected:
        ~MainForm()
        {
            if (components) delete components;
        }

    private:
        PhysicsModel* physicsModel;
        PictureBody^ pbPlanet;
        List<PictureBody^>^ pbs;
        Point p0;
        double scale;

        SpaceBody* activeSatellite;

        double baseMass = 7.3177e22;
        double baseOrbit = 384408e3;
        double baseV = 1023;
        double baseRadius = 1737400;
        double baseScale;

        void InitializeSystem()
        {
            p0 = Point(ClientSize.Width / 2, ClientSize.Height / 2);
            baseScale = ClientSize.Height / baseOrbit * 0.8;
            scale = baseScale;

            PlanetsData::Init();
            SpaceBody* planet = PlanetsData::GetPlanet("Earth");
            physicsModel->Planet = planet;
            pbPlanet = gcnew PictureBody(planet, this, scale);
            pbPlanet->p0 = p0;
            pbPlanet->SetSettings("Earth");
            pbPlanet->Pb->Visible = false;
            pbPlanet->Update();
            pbs->Add(pbPlanet);
            cbPlanets->Items->AddRange(PlanetsData::GetAllNames());
            cbPlanets->SelectedIndex = 2;
        }

        void comboBox1_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e)
        {
            String^ name = cbPlanets->SelectedItem->ToString();
            SpaceBody* planet = PlanetsData::GetPlanet(name);
            physicsModel->Planet = planet;
            pbPlanet->BallObj = planet;
            pbPlanet->SetSettings(name);
        }

        void NewSatellite()
        {
            tbMass->Value = 0;
            tbOrbit->Value = 0;
            tbSpeed->Value = 0;
            activeSatellite = physicsModel->AddSatellite(baseV, baseMass, baseOrbit, baseRadius);

            PictureBody^ pbBall = gcnew PictureBody(activeSatellite, this, scale);
            pbBall->p0 = p0;

            String^ imagePath = System::IO::Path::Combine(System::AppDomain::CurrentDomain->BaseDirectory, "ImagesSatellite", "Satellite.jpg");
            try {
                if (System::IO::File::Exists(imagePath)) {
                    Image^ img = Image::FromFile(imagePath);
                    pbBall->Pb->Image = img;
                    pbBall->OriginalImage = img;
                    pbBall->AspectRatio = (double)img->Width / img->Height;
                }
            }
            catch (...) {}

            // Если симуляция уже запущена, показываем спутник сразу
            if (simulationStarted) {
                pbBall->Pb->Visible = true;
            }
            else {
                pbBall->Pb->Visible = false;
            }

            pbBall->Update();
            pbs->Add(pbBall);
            pbBall->Pb->BringToFront();

            UpdateListSattelites();
            cbSatellites->SelectedIndex = cbSatellites->Items->Count - 1;
        }

        void UpdateListSattelites()
        {
            cbSatellites->Items->Clear();
            for (int i = 0; i < physicsModel->Satellites.Count(); i++) {
                cbSatellites->Items->Add("Satellite " + i);
            }
        }

        void bStart_Click(System::Object^ sender, System::EventArgs^ e)
        {
            if (!simulationStarted) {
                pbPlanet->Pb->Visible = true;
                for (int i = 1; i < pbs->Count; i++) {
                    pbs[i]->Pb->Visible = true;
                }
                tPhisics->Start();
                label1->Visible = true;
                simulationStarted = true;
                bStart->Text = "Pause";
            }
            else {
                if (tPhisics->Enabled) {
                    tPhisics->Stop();
                    bStart->Text = "Resume";
                }
                else {
                    tPhisics->Start();
                    bStart->Text = "Pause";
                }
            }
        }

        void bAddSatellite_Click(System::Object^ sender, System::EventArgs^ e)
        {
            NewSatellite();
            tbSpeed->Enabled = true;
            tbMass->Enabled = true;
            tbOrbit->Enabled = true;
        }

        void tbSpeed_Scroll(System::Object^ sender, System::EventArgs^ e)
        {
            SetSatelliteSettings();
        }

        void SetSatelliteSettings()
        {
            double m = baseMass * (1 + tbMass->Value / 10.0);
            double orbit = baseOrbit * (1 + tbOrbit->Value / 10.0);
            double v = baseV * (1 + tbSpeed->Value / 10.0);

            physicsModel->ChangeV(activeSatellite, v);
            physicsModel->ChangeM(activeSatellite, m);
            physicsModel->ChangeR(activeSatellite, orbit);
        }

        void tPhisics_Tick(System::Object^ sender, System::EventArgs^ e)
        {
            physicsModel->Update((double)(tPhisics->Interval / 0.0010));

            // Проверяем столкновения
            CustomList<int> collidedIndices;
            physicsModel->CheckAllCollisions(collidedIndices);

            // Запускаем анимацию столкновения для столкнувшихся спутников
            // Не трогаем планету (индекс 0 в pbs)
            for (int i = 0; i < collidedIndices.Count(); i++) {
                int satIndex = collidedIndices[i];
                int pbsIndex = satIndex + 1;  // спутник 0 в physicsModel -> индекс 1 в pbs (планета на 0)
                if (pbsIndex >= 1 && pbsIndex < pbs->Count) {
                    pbs[pbsIndex]->StartCollision();
                }
            }

            for (int i = 0; i < pbs->Count; i++) {
                pbs[i]->Update();
            }

            if (pbs->Count > 1) {
                label1->Text = pbs[1]->Pb->Location.X + ", " + pbs[1]->Pb->Location.Y + "\n" + pbs[1]->BallObj->get_V().X;
            }
        }

        void tbScale_Scroll(System::Object^ sender, System::EventArgs^ e)
        {
            if (tbScale->Value < 0) scale = baseScale / -tbScale->Value;
            else scale = baseScale * tbScale->Value;

            for (int i = 0; i < pbs->Count; i++) {
                pbs[i]->Scale = scale;
            }
        }

        void cbSatellites_SelectedIndexChanged_1(System::Object^ sender, System::EventArgs^ e)
        {
            int idx = cbSatellites->SelectedIndex;
            if (idx >= 0 && idx < physicsModel->Satellites.Count()) {
                activeSatellite = physicsModel->Satellites[idx];
            }
        }

    private:
        System::ComponentModel::IContainer^ components;
        System::Windows::Forms::Timer^ tPhisics;
        System::Windows::Forms::Button^ bStart;
        System::Windows::Forms::Label^ lblPlanets;
        System::Windows::Forms::ComboBox^ cbPlanets;
        System::Windows::Forms::Button^ bAddSatellite;
        System::Windows::Forms::TrackBar^ tbScale;
        System::Windows::Forms::ComboBox^ cbSatellites;
        System::Windows::Forms::Label^ lblSatelites;
        System::Windows::Forms::Label^ lblScale;
        System::Windows::Forms::Label^ lblSpeed;
        System::Windows::Forms::TrackBar^ tbSpeed;
        System::Windows::Forms::Label^ lblMass;
        System::Windows::Forms::TrackBar^ tbMass;
        System::Windows::Forms::Label^ lblOrbit;
        System::Windows::Forms::TrackBar^ tbOrbit;
        System::Windows::Forms::Label^ label1;
        bool simulationStarted = false;

        void InitializeComponent(void)
        {
            this->components = (gcnew System::ComponentModel::Container());
            System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(MainForm::typeid));
            this->tPhisics = (gcnew System::Windows::Forms::Timer(this->components));
            this->bStart = (gcnew System::Windows::Forms::Button());
            this->lblPlanets = (gcnew System::Windows::Forms::Label());
            this->cbPlanets = (gcnew System::Windows::Forms::ComboBox());
            this->bAddSatellite = (gcnew System::Windows::Forms::Button());
            this->tbScale = (gcnew System::Windows::Forms::TrackBar());
            this->cbSatellites = (gcnew System::Windows::Forms::ComboBox());
            this->lblSatelites = (gcnew System::Windows::Forms::Label());
            this->lblScale = (gcnew System::Windows::Forms::Label());
            this->lblSpeed = (gcnew System::Windows::Forms::Label());
            this->tbSpeed = (gcnew System::Windows::Forms::TrackBar());
            this->lblMass = (gcnew System::Windows::Forms::Label());
            this->tbMass = (gcnew System::Windows::Forms::TrackBar());
            this->lblOrbit = (gcnew System::Windows::Forms::Label());
            this->tbOrbit = (gcnew System::Windows::Forms::TrackBar());
            this->label1 = (gcnew System::Windows::Forms::Label());
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->tbScale))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->tbSpeed))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->tbMass))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->tbOrbit))->BeginInit();
            this->SuspendLayout();
            // 
            // tPhisics
            // 
            this->tPhisics->Interval = 16;
            this->tPhisics->Tick += gcnew System::EventHandler(this, &MainForm::tPhisics_Tick);
            // 
            // bStart
            // 
            this->bStart->BackColor = System::Drawing::SystemColors::Control;
            this->bStart->Font = (gcnew System::Drawing::Font(L"Bookman Old Style", 20.25F));
            this->bStart->Location = System::Drawing::Point(827, 12);
            this->bStart->Name = L"bStart";
            this->bStart->Size = System::Drawing::Size(121, 40);
            this->bStart->TabIndex = 14;
            this->bStart->Text = L"Start";
            this->bStart->UseVisualStyleBackColor = false;
            this->bStart->Click += gcnew System::EventHandler(this, &MainForm::bStart_Click);
            // 
            // lblPlanets
            // 
            this->lblPlanets->ForeColor = System::Drawing::Color::White;
            this->lblPlanets->Location = System::Drawing::Point(777, 81);
            this->lblPlanets->Name = L"lblPlanets";
            this->lblPlanets->Size = System::Drawing::Size(48, 15);
            this->lblPlanets->TabIndex = 13;
            this->lblPlanets->Text = L"Planets:";
            // 
            // cbPlanets
            // 
            this->cbPlanets->Location = System::Drawing::Point(827, 73);
            this->cbPlanets->Name = L"cbPlanets";
            this->cbPlanets->Size = System::Drawing::Size(121, 21);
            this->cbPlanets->TabIndex = 12;
            this->cbPlanets->SelectedIndexChanged += gcnew System::EventHandler(this, &MainForm::comboBox1_SelectedIndexChanged);
            // 
            // bAddSatellite
            // 
            this->bAddSatellite->BackColor = System::Drawing::SystemColors::Control;
            this->bAddSatellite->Font = (gcnew System::Drawing::Font(L"Bookman Old Style", 12));
            this->bAddSatellite->Location = System::Drawing::Point(827, 466);
            this->bAddSatellite->Name = L"bAddSatellite";
            this->bAddSatellite->Size = System::Drawing::Size(121, 37);
            this->bAddSatellite->TabIndex = 11;
            this->bAddSatellite->Text = L"Add Satellite";
            this->bAddSatellite->UseVisualStyleBackColor = false;
            this->bAddSatellite->Click += gcnew System::EventHandler(this, &MainForm::bAddSatellite_Click);
            // 
            // tbScale
            // 
            this->tbScale->Location = System::Drawing::Point(827, 126);
            this->tbScale->Maximum = 20;
            this->tbScale->Minimum = -20;
            this->tbScale->Name = L"tbScale";
            this->tbScale->Size = System::Drawing::Size(121, 45);
            this->tbScale->TabIndex = 10;
            this->tbScale->Scroll += gcnew System::EventHandler(this, &MainForm::tbScale_Scroll);
            // 
            // cbSatellites
            // 
            this->cbSatellites->Location = System::Drawing::Point(827, 270);
            this->cbSatellites->Name = L"cbSatellites";
            this->cbSatellites->Size = System::Drawing::Size(121, 21);
            this->cbSatellites->TabIndex = 8;
            this->cbSatellites->SelectedIndexChanged += gcnew System::EventHandler(this, &MainForm::cbSatellites_SelectedIndexChanged_1);
            // 
            // lblSatelites
            // 
            this->lblSatelites->ForeColor = System::Drawing::Color::White;
            this->lblSatelites->Location = System::Drawing::Point(772, 273);
            this->lblSatelites->Name = L"lblSatelites";
            this->lblSatelites->Size = System::Drawing::Size(53, 15);
            this->lblSatelites->TabIndex = 9;
            this->lblSatelites->Text = L"Satelites:";
            // 
            // lblScale
            // 
            this->lblScale->ForeColor = System::Drawing::Color::White;
            this->lblScale->Location = System::Drawing::Point(777, 137);
            this->lblScale->Name = L"lblScale";
            this->lblScale->Size = System::Drawing::Size(34, 15);
            this->lblScale->TabIndex = 7;
            this->lblScale->Text = L"Scale";
            // 
            // lblSpeed
            // 
            this->lblSpeed->ForeColor = System::Drawing::Color::White;
            this->lblSpeed->Location = System::Drawing::Point(777, 327);
            this->lblSpeed->Name = L"lblSpeed";
            this->lblSpeed->Size = System::Drawing::Size(42, 15);
            this->lblSpeed->TabIndex = 5;
            this->lblSpeed->Text = L"Speed:";
            // 
            // tbSpeed
            // 
            this->tbSpeed->Enabled = false;
            this->tbSpeed->Location = System::Drawing::Point(827, 318);
            this->tbSpeed->Maximum = 20;
            this->tbSpeed->Name = L"tbSpeed";
            this->tbSpeed->Size = System::Drawing::Size(121, 45);
            this->tbSpeed->TabIndex = 6;
            this->tbSpeed->Scroll += gcnew System::EventHandler(this, &MainForm::tbSpeed_Scroll);
            // 
            // lblMass
            // 
            this->lblMass->ForeColor = System::Drawing::Color::White;
            this->lblMass->Location = System::Drawing::Point(777, 364);
            this->lblMass->Name = L"lblMass";
            this->lblMass->Size = System::Drawing::Size(37, 15);
            this->lblMass->TabIndex = 3;
            this->lblMass->Text = L"Mass:";
            // 
            // tbMass
            // 
            this->tbMass->Enabled = false;
            this->tbMass->Location = System::Drawing::Point(827, 364);
            this->tbMass->Maximum = 20;
            this->tbMass->Name = L"tbMass";
            this->tbMass->Size = System::Drawing::Size(121, 45);
            this->tbMass->TabIndex = 4;
            this->tbMass->Scroll += gcnew System::EventHandler(this, &MainForm::tbSpeed_Scroll);
            // 
            // lblOrbit
            // 
            this->lblOrbit->ForeColor = System::Drawing::Color::White;
            this->lblOrbit->Location = System::Drawing::Point(780, 425);
            this->lblOrbit->Name = L"lblOrbit";
            this->lblOrbit->Size = System::Drawing::Size(37, 15);
            this->lblOrbit->TabIndex = 1;
            this->lblOrbit->Text = L"Orbit:";
            // 
            // tbOrbit
            // 
            this->tbOrbit->Enabled = false;
            this->tbOrbit->Location = System::Drawing::Point(827, 415);
            this->tbOrbit->Maximum = 20;
            this->tbOrbit->Name = L"tbOrbit";
            this->tbOrbit->Size = System::Drawing::Size(121, 45);
            this->tbOrbit->TabIndex = 2;
            this->tbOrbit->Scroll += gcnew System::EventHandler(this, &MainForm::tbSpeed_Scroll);
            // 
            // label1
            // 
            this->label1->ForeColor = System::Drawing::Color::White;
            this->label1->Location = System::Drawing::Point(744, 201);
            this->label1->Name = L"label1";
            this->label1->Size = System::Drawing::Size(38, 15);
            this->label1->TabIndex = 0;
            this->label1->Text = L"label1";
            this->label1->Visible = false;
            // 
            // MainForm
            // 
            this->BackColor = System::Drawing::Color::Black;
            this->ClientSize = System::Drawing::Size(960, 515);
            this->Controls->Add(this->label1);
            this->Controls->Add(this->lblOrbit);
            this->Controls->Add(this->tbOrbit);
            this->Controls->Add(this->lblMass);
            this->Controls->Add(this->tbMass);
            this->Controls->Add(this->lblSpeed);
            this->Controls->Add(this->tbSpeed);
            this->Controls->Add(this->lblScale);
            this->Controls->Add(this->cbSatellites);
            this->Controls->Add(this->lblSatelites);
            this->Controls->Add(this->tbScale);
            this->Controls->Add(this->bAddSatellite);
            this->Controls->Add(this->cbPlanets);
            this->Controls->Add(this->lblPlanets);
            this->Controls->Add(this->bStart);
            this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
            this->Name = L"MainForm";
            this->Text = L"Satellite Simullation";
            this->Load += gcnew System::EventHandler(this, &MainForm::MainForm_Load);
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->tbScale))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->tbSpeed))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->tbMass))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->tbOrbit))->EndInit();
            this->ResumeLayout(false);
            this->PerformLayout();

        }
    private: System::Void MainForm_Load(System::Object^ sender, System::EventArgs^ e) {
    }
};
}