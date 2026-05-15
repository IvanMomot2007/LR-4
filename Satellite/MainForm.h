#pragma once

#include "PhysicsModel.h"
#include "PictureBody.h"
#include "PlanetsData.h"

namespace Satellite {

    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Collections::Generic;
    using namespace System::Windows::Forms;
    using namespace System::Drawing;

    public ref class MainForm : public System::Windows::Forms::Form
    {
    public:
        MainForm(void)
        {
            InitializeComponent();
            this->DoubleBuffered = true;
            physicsModel = new PhysicsModel();
            pbs = gcnew List<PictureBody^>();
            InitializeSystem();
        }

    protected:
        ~MainForm()
        {
            if (components) delete components;
        }

        virtual void OnPaint(PaintEventArgs^ e) override {
            Form::OnPaint(e);
            Graphics^ g = e->Graphics;
            g->SmoothingMode = System::Drawing::Drawing2D::SmoothingMode::AntiAlias;
            g->Clear(Color::Black);

            if (pbs != nullptr) {
                for (int i = 0; i < pbs->Count; i++) {
                    pbs[i]->Draw(g);
                }
            }
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
            baseScale = ClientSize.Height / baseOrbit * 0.5;
            scale = baseScale;

            PlanetsData::Init();
            physicsModel->Planet = PlanetsData::GetPlanet("Earth");
            pbPlanet = gcnew PictureBody(physicsModel->Planet, scale);
            pbPlanet->p0 = p0;
            pbPlanet->SetSettings("Earth");
            pbs->Add(pbPlanet);

            cbPlanets->Items->AddRange(PlanetsData::GetAllNames());
            cbPlanets->SelectedIndex = 2;
        }

        void tPhisics_Tick(System::Object^ sender, System::EventArgs^ e)
        {
            physicsModel->Update((double)(tPhisics->Interval / 0.0010));
            p0 = Point(ClientSize.Width / 2, ClientSize.Height / 2);

            bool needsListUpdate = false;

            for (int i = pbs->Count - 1; i >= 0; i--) {
                pbs[i]->p0 = p0;
                pbs[i]->UpdateLogic();

                if (i > 0 && pbs[i]->IsDead) {
                    for (int k = 0; k < physicsModel->Satellites.Count(); k++) {
                        if (physicsModel->Satellites[k] == pbs[i]->BodyObj) {
                            physicsModel->Satellites.RemoveAt(k);
                            break;
                        }
                    }
                    pbs->RemoveAt(i);
                    needsListUpdate = true;
                    continue;
                }
            }

            for (int i = 0; i < pbs->Count; i++) {
                for (int j = i + 1; j < pbs->Count; j++) {
                    if (!pbs[i]->IsExploding && !pbs[j]->IsExploding &&
                        !pbs[i]->IsDead && !pbs[j]->IsDead &&
                        pbs[i]->BodyObj != nullptr && pbs[j]->BodyObj != nullptr)
                    {
                        double dist = (pbs[i]->BodyObj->get_R() - pbs[j]->BodyObj->get_R()).GetLength();

                        double k = 20.0;
                        double visualRadius1 = pbs[i]->BodyObj->Radius * k;
                        double visualRadius2 = pbs[j]->BodyObj->Radius * k;

                        double collisionThreshold = (visualRadius1 + visualRadius2) * 0.95;

                        if (dist <= collisionThreshold) {
                            if (i != 0) pbs[i]->StartCollision();
                            if (j != 0) pbs[j]->StartCollision();
                            needsListUpdate = true;
                        }
                    }
                }
            }

            if (needsListUpdate) {
                cbSatellites->Items->Clear();
                for (int k = 0; k < physicsModel->Satellites.Count(); k++)
                    cbSatellites->Items->Add("Satellite " + k);
            }

            // ПРАВИЛЬНЫЙ ВЫВОД ТЕЛЕМЕТРИИ
            if (activeSatellite != nullptr && pbs->Count > 1) {
                Vector heightVec = activeSatellite->get_R() - physicsModel->Planet->get_R();
                double height = heightVec.GetLength() - physicsModel->Planet->Radius;
                double velocity = activeSatellite->get_V().GetLength();

                String^ velStr = velocity.ToString("F2");
                String^ heightStr = (height / 1000.0).ToString("F0");

                label1->Text = "V: " + velStr + " m/s\r\nH: " + heightStr + " km";
            }

            this->Invalidate();
        }

        void bAddSatellite_Click(System::Object^ sender, System::EventArgs^ e)
        {
            activeSatellite = physicsModel->AddSatellite(baseV, baseMass, baseOrbit, baseRadius);
            PictureBody^ pbBody = gcnew PictureBody(activeSatellite, scale);
            pbBody->p0 = p0;
            pbBody->SetSettings("Satellite");
            pbs->Add(pbBody);

            cbSatellites->Items->Clear();
            for (int i = 0; i < physicsModel->Satellites.Count(); i++)
                cbSatellites->Items->Add("Satellite " + i);
            cbSatellites->SelectedIndex = cbSatellites->Items->Count - 1;

            tbSpeed->Enabled = true; tbMass->Enabled = true; tbOrbit->Enabled = true;
        }

        void bStart_Click(System::Object^ sender, System::EventArgs^ e) { tPhisics->Start(); label1->Visible = true; }

        void comboBox1_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) {
            String^ name = cbPlanets->SelectedItem->ToString();
            physicsModel->Planet = PlanetsData::GetPlanet(name);
            pbPlanet->BodyObj = physicsModel->Planet;
            pbPlanet->SetSettings(name);
            this->Invalidate();
        }

        void tbSpeed_Scroll(System::Object^ sender, System::EventArgs^ e) {
            if (activeSatellite == nullptr) return;
            physicsModel->ChangeV(activeSatellite, baseV * (1 + tbSpeed->Value / 10.0));
            physicsModel->ChangeM(activeSatellite, baseMass * (1 + tbMass->Value / 10.0));
            physicsModel->ChangeR(activeSatellite, baseOrbit * (1 + tbOrbit->Value / 10.0));
        }

        void tbScale_Scroll(System::Object^ sender, System::EventArgs^ e) {
            scale = (tbScale->Value < 0) ? baseScale / -tbScale->Value : baseScale * (tbScale->Value == 0 ? 1 : tbScale->Value);
            for (int i = 0; i < pbs->Count; i++) pbs[i]->Scale = scale;
            this->Invalidate();
        }

        void cbSatellites_SelectedIndexChanged_1(System::Object^ sender, System::EventArgs^ e) {
            int idx = cbSatellites->SelectedIndex;
            if (idx >= 0 && idx < physicsModel->Satellites.Count()) activeSatellite = physicsModel->Satellites[idx];
        }

    private:
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
        System::ComponentModel::IContainer^ components;

        void InitializeComponent(void)
        {
            this->components = (gcnew System::ComponentModel::Container());
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

            this->tPhisics->Interval = 16;
            this->tPhisics->Tick += gcnew System::EventHandler(this, &MainForm::tPhisics_Tick);

            this->bStart->BackColor = System::Drawing::SystemColors::Control;
            this->bStart->Font = (gcnew System::Drawing::Font(L"Bookman Old Style", 20.25f));
            this->bStart->Location = System::Drawing::Point(827, 12);
            this->bStart->Name = L"bStart";
            this->bStart->Size = System::Drawing::Size(121, 40);
            this->bStart->Text = L"Start";
            this->bStart->UseVisualStyleBackColor = false;
            this->bStart->Click += gcnew System::EventHandler(this, &MainForm::bStart_Click);

            this->lblPlanets->ForeColor = System::Drawing::Color::White;
            this->lblPlanets->Location = System::Drawing::Point(777, 81);
            this->lblPlanets->Name = L"lblPlanets";
            this->lblPlanets->Size = System::Drawing::Size(48, 15);
            this->lblPlanets->Text = L"Planets:";

            this->cbPlanets->Location = System::Drawing::Point(827, 73);
            this->cbPlanets->Name = L"cbPlanets";
            this->cbPlanets->Size = System::Drawing::Size(121, 21);
            this->cbPlanets->SelectedIndexChanged += gcnew System::EventHandler(this, &MainForm::comboBox1_SelectedIndexChanged);

            this->bAddSatellite->BackColor = System::Drawing::SystemColors::Control;
            this->bAddSatellite->Font = (gcnew System::Drawing::Font(L"Bookman Old Style", 12.0f));
            this->bAddSatellite->Location = System::Drawing::Point(827, 466);
            this->bAddSatellite->Name = L"bAddSatellite";
            this->bAddSatellite->Size = System::Drawing::Size(121, 37);
            this->bAddSatellite->Text = L"Add Satellite";
            this->bAddSatellite->UseVisualStyleBackColor = false;
            this->bAddSatellite->Click += gcnew System::EventHandler(this, &MainForm::bAddSatellite_Click);

            this->tbScale->Location = System::Drawing::Point(827, 126);
            this->tbScale->Maximum = 20;
            this->tbScale->Minimum = -20;
            this->tbScale->Name = L"tbScale";
            this->tbScale->Size = System::Drawing::Size(121, 45);
            this->tbScale->Scroll += gcnew System::EventHandler(this, &MainForm::tbScale_Scroll);

            this->cbSatellites->Location = System::Drawing::Point(827, 270);
            this->cbSatellites->Name = L"cbSatellites";
            this->cbSatellites->Size = System::Drawing::Size(121, 21);
            this->cbSatellites->SelectedIndexChanged += gcnew System::EventHandler(this, &MainForm::cbSatellites_SelectedIndexChanged_1);

            this->lblSatelites->ForeColor = System::Drawing::Color::White;
            this->lblSatelites->Location = System::Drawing::Point(772, 273);
            this->lblSatelites->Name = L"lblSatelites";
            this->lblSatelites->Size = System::Drawing::Size(53, 15);
            this->lblSatelites->Text = L"Satelites:";

            this->lblScale->ForeColor = System::Drawing::Color::White;
            this->lblScale->Location = System::Drawing::Point(777, 137);
            this->lblScale->Name = L"lblScale";
            this->lblScale->Size = System::Drawing::Size(34, 15);
            this->lblScale->Text = L"Scale";

            this->lblSpeed->ForeColor = System::Drawing::Color::White;
            this->lblSpeed->Location = System::Drawing::Point(777, 327);
            this->lblSpeed->Name = L"lblSpeed";
            this->lblSpeed->Size = System::Drawing::Size(42, 15);
            this->lblSpeed->Text = L"Speed:";

            this->tbSpeed->Enabled = false;
            this->tbSpeed->Location = System::Drawing::Point(827, 318);
            this->tbSpeed->Maximum = 20;
            this->tbSpeed->Name = L"tbSpeed";
            this->tbSpeed->Size = System::Drawing::Size(121, 45);
            this->tbSpeed->Scroll += gcnew System::EventHandler(this, &MainForm::tbSpeed_Scroll);

            this->lblMass->ForeColor = System::Drawing::Color::White;
            this->lblMass->Location = System::Drawing::Point(777, 364);
            this->lblMass->Name = L"lblMass";
            this->lblMass->Size = System::Drawing::Size(37, 15);
            this->lblMass->Text = L"Mass:";

            this->tbMass->Enabled = false;
            this->tbMass->Location = System::Drawing::Point(827, 364);
            this->tbMass->Maximum = 20;
            this->tbMass->Name = L"tbMass";
            this->tbMass->Size = System::Drawing::Size(121, 45);
            this->tbMass->Scroll += gcnew System::EventHandler(this, &MainForm::tbSpeed_Scroll);

            this->lblOrbit->ForeColor = System::Drawing::Color::White;
            this->lblOrbit->Location = System::Drawing::Point(780, 425);
            this->lblOrbit->Name = L"lblOrbit";
            this->lblOrbit->Size = System::Drawing::Size(37, 15);
            this->lblOrbit->Text = L"Orbit:";

            this->tbOrbit->Enabled = false;
            this->tbOrbit->Location = System::Drawing::Point(827, 415);
            this->tbOrbit->Maximum = 20;
            this->tbOrbit->Name = L"tbOrbit";
            this->tbOrbit->Size = System::Drawing::Size(121, 45);
            this->tbOrbit->Scroll += gcnew System::EventHandler(this, &MainForm::tbSpeed_Scroll);

            this->label1->ForeColor = System::Drawing::Color::White;
            this->label1->Location = System::Drawing::Point(744, 180);
            this->label1->Name = L"label1";
            this->label1->Size = System::Drawing::Size(180, 40);
            this->label1->Text = L"V: 0 m/s  H: 0 km";
            this->label1->Visible = false;

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
            this->Name = L"MainForm";
            this->Text = L"SpaceX Simulation";
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->tbScale))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->tbSpeed))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->tbMass))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->tbOrbit))->EndInit();
            this->ResumeLayout(false);
            this->PerformLayout();
        }
    };
}