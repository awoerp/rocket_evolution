#include <iostream>
#include <cmath>
#include <fstream>
#include <time.h>
#include <cstdlib>

using namespace std;

const int MAX_TANK_HEIGHT     = 30; // m
const int MAX_TANK_DIAMETER   = 15; // m
const float LOX_DENSITY       = 1141; // kg / m^3
const float RP1_Density       = 1124; // kg / m^3
const float LOX2RP1_RATIO     = 2.56; // tonnes of LOX / RP1
const float STRUCTURE_MASS    = 40.7; // kg / m^2
const float LOX_RATE           = 106;  // kg / s
const float RP1_RATE           =  41;  // kg / s
const float THRUST          = 801000;  // kN

const float G                  =9.81;
const float RESOLUTION          =0.1;
const float PI              = 3.1415;
const int MAX_ROCKETS            =50;
const int MAX_NUM_ENGINES        =30;
const int MAX_ITERATIONS       =5000;

enum fuelType{LOX, RP1};

class Tank
{
private:
   float height; // meters
   float diameter; // meters
   float KgOfFuel;  // metric tons
   float emptyMass;
   bool  empty;
   fuelType fuel;
   float currentMass;

public:
   Tank(float h, float d, float percentFuel, fuelType f)
   {
      height = h;
      diameter  = d;
      fuel = f;
      float volume = PI * pow(d/2,2) * h;
      float surfaceArea = PI * d * h;
      if(f == LOX)
      {
         KgOfFuel = volume * LOX_DENSITY * percentFuel;
         fuel = LOX;
      }
      else if(f == RP1)
      {
         KgOfFuel = volume * RP1_Density * percentFuel;
         fuel = RP1;
      }
      emptyMass = surfaceArea * STRUCTURE_MASS;
      currentMass = emptyMass + KgOfFuel;
      empty = false;
   }

   float GetHeight() const
   {
      return height;
   }

   float GetDiameter() const
   {
      return diameter;
   }

   float GetCurrentFuel() const
   {
      return KgOfFuel;
   }

   float GetCurrentMass() const
   {
      return currentMass;
   }

   float GetEmptyMass() const
   {
      return emptyMass;
   }

   float IsEmpty() const
   {
      return empty;
   }

   void SetCurrenMass(float val)
   {
      currentMass = val;
   }

   void Update(int* numEngines)
   {
      if(fuel == LOX)
         KgOfFuel -= LOX_RATE * (RESOLUTION) * (*numEngines);
      else
         KgOfFuel -= RP1_RATE * (RESOLUTION) * (*numEngines);

      if(KgOfFuel <= 0)
         SetEmpty();
   }

   void SetEmpty()
   {
      empty = true;
   }

   bool IsEmpty()
   {
      return empty;
   }
};


class Rocket
{
private:
   Tank* LoxTank;
   Tank* RP1Tank;
   
   float engineTrust;
   float totalMass;
   float altitude;
   float velocity;

   float lastAltitude;
   float lastVelocity;

   float maxAltitude;
   int numEngines;
   int rocketNum;
   float maxHeight;

public:
   Rocket(Tank* LOX, Tank* RP1)
   {
      LoxTank = LOX;
      RP1Tank = RP1;
      engineTrust = THRUST;
      totalMass = LOX->GetCurrentMass() + RP1->GetCurrentMass();
   }

   Rocket(int rocketNumber)
   {
      float d = (rand() % MAX_TANK_DIAMETER) + 1;
      float h = (rand() % MAX_TANK_HEIGHT) + 1;
      float percentFuel = (rand() % 100) / 100.0;

      LoxTank = new Tank(h, d, percentFuel, LOX);
      h = rand() % MAX_TANK_HEIGHT;
      percentFuel = (rand() % 100) / 100.0;

      RP1Tank = new Tank(h, d, percentFuel, RP1);
      
      numEngines = rand() % MAX_NUM_ENGINES + 1;
      engineTrust = THRUST * numEngines;

      totalMass = LoxTank->GetCurrentMass() + RP1Tank->GetCurrentMass();
      velocity = 0;
      altitude = 0;
      rocketNum = rocketNumber;
   }

   ~Rocket()
   {
      delete LoxTank;
      delete RP1Tank;
   }

   Tank* GetLoxTank() const
   {
      return LoxTank;
   }

   Tank* GetRP1Tank() const
   {
      return RP1Tank;
   }

   int GetRocketNum() const
   {
      return rocketNum;
   }

   float GetForceDrag() const
   {
      float p =  1.2 * exp(-altitude / 7616);
      float A = PI * pow(LoxTank->GetDiameter() / 2, 2);
      return 0.5 * p * pow(velocity, 2) * A;
   }

   float GetVelocity() const
   {
      return velocity;
   }

   float GetAltitude() const
   {
      return altitude;
   }
   
   float GetThrust() const
   {
      return engineTrust;
   }

   float GetCurrentMass() const
   {
      return totalMass;
   }

   int GetNumEngines() const
   {
      return numEngines;
   }

   float GetMaxAltitude() const
   {
      return maxAltitude;
   }

   void SetAltitude(float val)
   {
      altitude = val;
   }

   void SetVelocity(float val)
   {
      velocity = val;
   }

   void SetLastAltitude(float val)
   {
      lastAltitude = val;
   }

   void SetLastVelocity(float val)
   {
      lastVelocity = val;
   }

   void SetMaxAltitude(float val)
   {
      maxAltitude = val;
   }

   void SetRocketNum(int val)
   {
      rocketNum = val;
   }



   void UpdateFuel()
   {
      if(!LoxTank->IsEmpty())
          LoxTank->Update(&numEngines);
         
       if(!RP1Tank->IsEmpty())
          RP1Tank->Update(&numEngines);
   }


};

class Generation
{
private:
   Rocket* generation[MAX_ROCKETS];
   int numRockets;
   ofstream* file;

   void swap(int r1, int r2)
   {
      Rocket* temp = generation[r1];
      generation[r1] = generation[r2];
      generation[r2] = temp;

      generation[r1]->SetRocketNum(r1);
      generation[r2]->SetRocketNum(r2);
   }

public:
   Generation(int num, ofstream* outputFile)
   {
      numRockets = num;
      file = outputFile;
      for(int i = 0; i < MAX_ROCKETS; i++)
         generation[i] = NULL;
   }

   void RandomizeGeneration()
   {
      for(int i = 0; i < numRockets; i++)
      {
         generation[i] = new Rocket(i);
     
      }
   }

   void TestGeneration()
   {
      for(int i = 0; i < numRockets; i++)
         TestRocket(generation[i]);

   }

   void TestRocket(Rocket* r)
   {
      PrintRocket(r);
      float t = 0;
      int count = 0;
      float netForce;
      bool maxIterationFlag = false;
      bool maxAltitudeReached = false;
      bool liftedOff = false;
      while(r->GetAltitude() >= 0 && !maxIterationFlag)
      {
         count++;
         t += RESOLUTION;
         if(!r->GetLoxTank()->IsEmpty() && !r->GetRP1Tank()->IsEmpty())
            netForce = r->GetThrust() - r->GetCurrentMass() * G - r->GetForceDrag();
         else
            netForce =  - r->GetCurrentMass() * G - r->GetForceDrag();

         float Accel = netForce / r->GetCurrentMass();
         float vf = r->GetVelocity() + Accel * RESOLUTION;
         r->SetVelocity(vf);
         if(!liftedOff && Accel > 0)
            liftedOff = true;

         float newAltitude = r->GetAltitude() + vf * RESOLUTION + 0.5 * Accel * pow(RESOLUTION, 2);  // x0 + vt + 0.5at^2

         if(!maxAltitudeReached && (newAltitude - r->GetAltitude()) < 0)
         {
            maxAltitudeReached = true;
            r->SetMaxAltitude(r->GetAltitude());
         }


         r->SetAltitude(newAltitude); 

         if(r->GetAltitude() < 0 && !liftedOff) // if the rocket is still on the ground, don't let gravity pull it down below a altitude of zero.
            r->SetAltitude(0);
         if(r->GetVelocity() < 0 && !liftedOff)
            r->SetVelocity(0);
            
         
         PrintTimeStep(r, &t);

         maxIterationFlag = !(count <= MAX_ITERATIONS);
         if(maxIterationFlag)
            *file << "Maximum Iterations Reached";

         r->UpdateFuel();
         if(r->GetLoxTank()->IsEmpty() || r->GetRP1Tank()->IsEmpty())
         {
            r->GetLoxTank()->SetEmpty();
            r->GetRP1Tank()->SetEmpty();
         }

         
      }
      *file << endl << endl;

   }

   void PrintTimeStep(Rocket* r, float* time)
   {

      *file << r->GetAltitude() << " " << r->GetVelocity() << " " << *time << " " << r->GetLoxTank()->GetCurrentFuel() << " " << r->GetRP1Tank()->GetCurrentFuel() << endl;
   }

   void PrintRocket(Rocket* r)
   {
      Tank* lox = r->GetLoxTank();
      Tank* Rp1 = r->GetRP1Tank();

      *file << "LOX HEIGHT RP1HEIGHT DIAMETER NumEngins" << endl
            << lox->GetHeight() << " " << Rp1->GetHeight() << " " << lox->GetDiameter() << " " << r->GetNumEngines() << endl
            << "Altitude Velocity timestep currentLox currentRP1" << endl;
      
   }

   void PrintMaxAltitudes()
   {
      for(int i = 0; i < numRockets; i++)
      {
         *file << generation[i]->GetMaxAltitude() << endl;
      }
   }

   void SortGeneration()
   {
      int maxInSwap;
      for(int i = 0; i < numRockets - 1; i++)
      {
         maxInSwap = i;
         PrintMaxAltitudes();
         *file << endl;
         for(int j = i + 1; j < numRockets; j++)
         {
            if(generation[j]->GetMaxAltitude() > generation[maxInSwap]->GetMaxAltitude())
               maxInSwap = generation[j]->GetRocketNum();
         }
         if(maxInSwap != i && generation[i]->GetMaxAltitude() < generation[maxInSwap]->GetMaxAltitude())
         {
            swap(generation[maxInSwap]->GetRocketNum(), generation[i]->GetRocketNum());
         }
      }
   }

   void MateGeneration()
   {
        

   }


};

int main()
{
   srand(time(NULL));
   ofstream file;
   file.open("Data.txt");
   Generation sim = Generation(5,&file);
   sim.RandomizeGeneration();
   sim.TestGeneration();
   sim.SortGeneration();
   sim.PrintMaxAltitudes();
   return 0;
}

