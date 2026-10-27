//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
// --------------------------------------------------------------
//                 GEANT 4 - Brachytherapy example
// --------------------------------------------------------------
//
// Code developed by:
// S.Guatelli
//
//    ********************************************
//    *                                          *
//    *    BrachyPrimaryGeneratorActionI.cc      *
//    *                                          *
//    ********************************************
//
// $Id$
//
#include "BrachyPrimaryGeneratorActionI.hh"
#include "globals.hh"
#include "Randomize.hh"  
#include "G4SystemOfUnits.hh"
#include "G4ParticleTable.hh"
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4UImanager.hh"
#include "G4RunManager.hh"

BrachyPrimaryGeneratorActionI::BrachyPrimaryGeneratorActionI()
{
  G4int numberParticles = 1;
 
  particleGun = new G4ParticleGun(numberParticles);
  
  // Gamma energy spectrum ...
  // Fill a vector with the energy probabilities
  // always we have X-rays even no "real" gamma emmission. 
  /*
  Using NuckudeExplorer=>get Energies and radiation yields, i.e. probability emmission of that quanta.
  Here we simulate a gun shooting only 1 particle at time=>we want probability emmision PER DECAY.
  It is computed in SIMPLE way, by dividing energy yield to SUM of all yields!

  This method is good for severeal quantas, i.e. <15
  For more....use alias-sampling like in DHPRE!!!!
  */
  energySpectrum.push_back(0.783913);//x-ray
  energySpectrum.push_back(0.170416);//x-ray
  energySpectrum.push_back(0.045671); //main gamma with yield of 0.0668 (6.68%)..see nuclide explorer 0.045671 = 0.0668/SUM_all_yield
}

BrachyPrimaryGeneratorActionI::~BrachyPrimaryGeneratorActionI()
{
 if(particleGun)
	delete particleGun;
}

void BrachyPrimaryGeneratorActionI::GeneratePrimaries(G4Event* anEvent)
{
  // Define the primary particle type
  G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
  G4String ParticleName = "gamma";
  G4ParticleDefinition* particle = particleTable -> FindParticle(ParticleName);

  particleGun -> SetParticleDefinition(particle);

  //  Random generation of gamma source point inside the Iodium core ...
  //G4double x,y,z;
   //G4double radiuMax = 0.30*mm;
  //G4double radiusMin = 0.085*mm;

  // Miez ICore: r in [0.085, 0.35] mm, semilungime 1.75 mm (la fel ca in geometrie)
  const G4double coreRadiusMin  = 0.085*mm;
  const G4double coreRadiusMax  = 0.35*mm;
  const G4double coreHalfLength = 1.75*mm;
  G4double x, y, r2;
   
  do{
     //x = (G4UniformRand()-0.5)*(radiuMax)/0.5;
     //y = (G4UniformRand()-0.5)*(radiuMax)/0.5;
	  x  = (2.0*G4UniformRand() - 1.0)*coreRadiusMax;
	  y  = (2.0*G4UniformRand() - 1.0)*coreRadiusMax;
      r2 = x*x + y*y;
  }//while(((x*x+y*y )> (radiuMax*radiuMax))||((x*x+y*y)<(radiusMin*radiusMin)));
  while (r2 > coreRadiusMax*coreRadiusMax || r2 < coreRadiusMin*coreRadiusMin);
 
  //z = (G4UniformRand()-0.5)*1.75*mm/0.5 ;
   const G4double z = (2.0*G4UniformRand() - 1.0)*coreHalfLength;

  G4ThreeVector position(x,y,z);
  particleGun -> SetParticlePosition(position);

  // Random generation of the impulse direction of primary particles ...
  G4double a,b,c;
  G4double n;
  do{
    a = (G4UniformRand()-0.5)/0.5;//gives [-1,1]
    b = (G4UniformRand()-0.5)/0.5;//gives [-1,1] 
    c = (G4UniformRand()-0.5)/0.5;//gives [-1,1]
    n = a*a+b*b+c*c;
  }while(n > 1 || n == 0.0);
  n = std::sqrt(n);
  a /= n;
  b /= n;
  c /= n;

  G4ThreeVector direction(a,b,c);
  particleGun -> SetParticleMomentumDirection(direction);//uniform in all directions! correct
 
  //selectia energiei nu mai poate citi în afara vectorului.
  //if sum of probab is not 1, but 0.999999 and random gives 0.99999999999999 greater->infinite loop!!!
  /*// Generate the primary particles with a defined energy spectrum
  G4double random = G4UniformRand();
  G4double sum = 0;
  G4int i = 0;
  while(sum<random){sum+=energySpectrum[i];//add probab elem i
  i++;//imediat increment!?? daca prima probab energySpectrum[0]>random exit loop but i value is 1 not 0!!!!!
  }

  // energy spectrum
  if(i==1){primaryParticleEnergy = 27.4*keV;}
  else{ 
    if(i==2){primaryParticleEnergy = 31.4*keV;}
    else {primaryParticleEnergy = 35.5*keV;}}//if more than 3 energies -> always 3!!! bad logic

  particleGun -> SetParticleEnergy(primaryParticleEnergy);

  */
  static const G4double energies[3] = {27.4*keV, 31.4*keV, 35.5*keV};//eith energy spectrum frequencies 0,7839 / 0,1703 / 0,0457
  const G4double random = G4UniformRand();
  size_t i = 0;//size_t is a type of data ...can store 0 and positive numbers only!
  G4double cumulative = energySpectrum[0];
  while (i < energySpectrum.size() - 1 && random > cumulative) {
    ++i;
    cumulative += energySpectrum[i];
  }
  primaryParticleEnergy = energies[i];
  //Frecventele reproduc 0,7839 / 0,1703 / 0,0457 în testul pe 2 milioane de esantioane.
  particleGun -> SetParticleEnergy(primaryParticleEnergy);

  // generate primary particle
  particleGun->GeneratePrimaryVertex(anEvent);
}
 
G4double  BrachyPrimaryGeneratorActionI::GetEnergy()
{ 
  return primaryParticleEnergy;
}

