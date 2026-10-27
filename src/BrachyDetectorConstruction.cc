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
// --------------------------------------------------------------
//                 GEANT 4 - Brachytherapy example
// --------------------------------------------------------------
//
// Code developed by:
// S. Agostinelli, F. Foppiano, S. Garelli , M. Tropeano, S.Guatelli
//
//
//    ****************************************
//    *                                      *
//    *    BrachyDetectorConstruction.cc     *
//    *                                      *
//    ****************************************
//
#include "G4SystemOfUnits.hh"
#include "G4CSGSolid.hh"
#include "G4MaterialPropertyVector.hh"
#include "G4SDManager.hh"
#include "G4RunManager.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4PVPlacement.hh"
#include "globals.hh"
#include "G4MaterialTable.hh"
#include "G4TransportationManager.hh"
#include "G4Colour.hh"
#include "G4UserLimits.hh"
#include "G4VisAttributes.hh"
#include "BrachyMaterial.hh"
#include "BrachyFactoryLeipzig.hh"
#include "BrachyFactoryIr.hh"
#include "BrachyFactoryI.hh"
#include "BrachyDetectorMessenger.hh"
#include "BrachyDetectorConstruction.hh"

G4double BrachyDetectorConstruction::phantomDensity=1.0;//solved
G4double BrachyDetectorConstruction::initialSourceActivity=16.0e6 * becquerel;//16 MBq
G4double BrachyDetectorConstruction::exposureTime=-3600.0 * second;
//->Claude ai
G4double BrachyDetectorConstruction::sourceHalfLife=73.831*24.0*3600.0*second;//Ir-192 = sursa implicita //600.0 * second;//solved
/*
Claude Good catch -> photonsPerDecay should not be condidered 1!!!! e.g. 60Co emits 2 photons with yields almost 1 at 1173 keV and 1332 keV so ~ 2 photons per a single decay!!!!
Therefore photonsPerDecay should be the SUM of all photons yields!!!!!!! 
CORRECT!
*/
G4double BrachyDetectorConstruction::photonsPerDecay=2.2;//Ir-192 (sursa implicita)

G4int BrachyDetectorConstruction::nbEventInRun=1;

BrachyDetectorConstruction::BrachyDetectorConstruction()
: detectorChoice(0), factory(0),
  World(0), WorldLog(0), WorldPhys(0),
  Phantom(0), PhantomLog(0), PhantomPhys(0),
  phantomAbsorberMaterial(0)
{
  // Define half size of the phantom along the x, y, z axis
  /*phantomSizeX = 5.*cm ;
  phantomSizeY = 5.*cm;
  phantomSizeZ = 5.*cm;*/

  phantomSizeX = 5.*cm ;//to match the mesh!!!!!see mac file!...or automatically later
  phantomSizeY = 5.*cm;
  phantomSizeZ = 5.*cm;
  //meshSizeX = 0.0;
  //meshSizeY = 0.0;
  //meshSizeZ = 0.0;
  
  // Define the sizes of the World volume containing the phantom
  worldSizeX = 4.0*m;
  worldSizeY = 4.0*m;
  worldSizeZ = 4.0*m;

  phantomMass = 1.0;//init
  // Define the messenger of the Detector component
  // It is possible to modify geometrical parameters through UI
  detectorMessenger = new BrachyDetectorMessenger(this);
  
  // Define the Iridium source as default source modelled in the geometry
  factory = new BrachyFactoryIr();

  // BrachyMaterial defined the all the materials necessary
  // for the experimental set-up 
  pMaterial = new BrachyMaterial();
}

BrachyDetectorConstruction::~BrachyDetectorConstruction()
{ 
  delete pMaterial;
  delete factory;
  delete detectorMessenger;
}

/*
run.mac files should be like this:
# 1. Creezi mesh-ul de scoring si ii definesti dimensiunile dorite
/score/create/boxMesh boxMesh_1
/score/mesh/boxSize 15. 15. 15. cm
/score/mesh/nBin 300 300 300

# 2. Apelezi comanda ta custom. 
# In acest moment se declanseaza C++-ul modificat: ia cei 15 cm si ii scrie in detector!
/score/quantity/doseDeposit2 myDoseGy Gy

# 3. Inchizi mesh-ul de scoring
/score/close

# 4. FORTEZI RECONSTRUCTIA GEOMETRIEI FIZICE
# Geant4 executa din nou functia Construct() cu noile dimensiuni de 15 cm
/run/initialize...........NOPE...no need for this!!!!!run.mac remain as they were....just /other/
if not removed....well yields error. 
Instead of deleting it, to assure compatbility with sprinboot server which write run.mac as it is...
just dummy it...do nothing!!!!
*/
void BrachyDetectorConstruction::UpdatePhantomSize(G4double hx, G4double hy, G4double hz)
{
	if (!Phantom) return;                       // geometria nu e construita inca
	if (hx > worldSizeX || hy > worldSizeY || hz > worldSizeZ)
    G4Exception("DetectorConstruction::UpdatePhantomSize()", "Etherapy001", FatalException,
                "The phantom (mesh) does not fit in the world volume.");

    // Actualizam variabilele interne cu dimensiunile primite de la mesh
    phantomSizeX = hx;
    phantomSizeY = hy;
    phantomSizeZ = hz;

	Phantom->SetXHalfLength(hx);                // solidul real
	Phantom->SetYHalfLength(hy);
	Phantom->SetZHalfLength(hz);
	phantomMass = phantomDensity * 8.0*hx*hy*hz;

    // NOTIFICARE CRITICA: Ii spunem lui Geant4 ca geometria fizica s-a modificat
    G4RunManager::GetRunManager()->GeometryHasBeenModified();
	//G4RunManager::GetRunManager()->ReinitializeGeometry();
    G4cout << "--> Phantom geometry updated!!!" << G4endl;
}

G4VPhysicalVolume* BrachyDetectorConstruction::Construct()
{
  pMaterial -> DefineMaterials();
 
  // Model the phantom (water box)  
  ConstructPhantom();

  // Model the source in the phantom
  factory -> CreateSource(PhantomPhys); 

  return WorldPhys;
}

void BrachyDetectorConstruction::SwitchBrachytherapicSeed()
{
  // Change the source in the water phantom
  factory -> CleanSource();
  delete factory;

  switch(detectorChoice)
  { 
    case 1:
      factory = new BrachyFactoryI();
      break;
    case 2:
      factory = new BrachyFactoryLeipzig();
      break;
    case 3:
      factory = new BrachyFactoryIr();
      break;   
    default:
      factory = new BrachyFactoryIr();
      break;
  }

  factory -> CreateSource(PhantomPhys);

  // Notify run manager that the new geometry has been built
  G4RunManager::GetRunManager() -> DefineWorldVolume( WorldPhys );
}

void BrachyDetectorConstruction::SelectBrachytherapicSeed(G4String val)
{
  if(val == "Iodium") 
  {
   detectorChoice = 1;
   sourceHalfLife=59.402*24.0*3600.0*second;//59.402 days I125
   photonsPerDecay=1.6;//0.0668/0.045671;//=1.463; yield gama 35.5 keV / probabilitatea ei normalizata din generator
   //1.598....nuclide exposure -> sum of all yields..more exact!
  }
  else
  {
    if(val=="Leipzig")
    {
      detectorChoice = 2;
	  sourceHalfLife=73.831*24.0*3600.0*second;//73.831 days Ir192
	  photonsPerDecay=2.2;//Ir192 nuclide exposure -> sum of all yields
    }
    else
    {
      if(val=="Iridium")
      {
        detectorChoice = 3;
		sourceHalfLife=73.831*24.0*3600.0*second;//73.831 days Ir192
		photonsPerDecay=2.2;//Ir192 nuclide exposure -> sum of all yields
      }
    }
  }

  G4cout << "Now the source is " << val << G4endl;
}

void BrachyDetectorConstruction::ConstructPhantom()
{
  // Model the water phantom 
  
  // Define the light blue color
  G4Colour  lblue   (0.0, 0.0, .75);

  G4Material* air = pMaterial -> GetMat("Air") ;
  G4Material* water = pMaterial -> GetMat("Water");
  phantomDensity = water->GetDensity();

  // World volume
  World = new G4Box("World",worldSizeX,worldSizeY,worldSizeZ);
  WorldLog = new G4LogicalVolume(World,air,"WorldLog",0,0,0);
  WorldPhys = new G4PVPlacement(0,G4ThreeVector(),
                                "WorldPhys",WorldLog,0,false,0);

  // Water Box
  Phantom = new G4Box("Phantom",phantomSizeX,phantomSizeY,
                       phantomSizeZ);

  // Logical volume
  PhantomLog = new G4LogicalVolume(Phantom,water,"PhantomLog",0,0,0);

  // Physical volume
  PhantomPhys = new G4PVPlacement(0,G4ThreeVector(), // Position: rotation and translation
                                  "PhantomPhys", // Name
				  PhantomLog, // Associated logical volume
                                  WorldPhys, // Mother volume
				  false,0); 

  G4double phantomVolume = PhantomLog->GetSolid()->GetCubicVolume();
  phantomMass = phantomDensity * phantomVolume;
  //WorldLog -> SetVisAttributes (G4VisAttributes::Invisible);

  // Visualization attributes of the phantom
  G4VisAttributes* simpleBoxVisAtt = new G4VisAttributes(lblue);
  simpleBoxVisAtt -> SetVisibility(true);
  simpleBoxVisAtt -> SetForceWireframe(true);
  PhantomLog -> SetVisAttributes(simpleBoxVisAtt);
}


void BrachyDetectorConstruction::PrintDetectorParameters()
{
  G4cout << "-----------------------------------------------------------------------"
         << G4endl
         << "the phantom is a box whose size is: " << G4endl
         << phantomSizeX *2./cm
         << " cm * "
         << phantomSizeY *2./cm
         << " cm * "
         << phantomSizeZ *2./cm
         << " cm" << G4endl
         << "The phantom is made of "
         << phantomAbsorberMaterial -> GetName() <<G4endl
		 << "Phantom density [g/cm3] is: "<<phantomDensity*cm3/g<<G4endl
         //<< "the source is at the center of the phantom" << G4endl
         << "-------------------------------------------------------------------------"
         << G4endl;
}

void BrachyDetectorConstruction::SetPhantomMaterial(G4String materialChoice)
{
  // It is possible to change the material of the phantom
  // interactively

  // Search the material by its name   
  G4Material* pttoMaterial = G4Material::GetMaterial(materialChoice);     
  if (pttoMaterial)
  {
    phantomAbsorberMaterial = pttoMaterial;
	phantomDensity = pttoMaterial->GetDensity();
    PhantomLog -> SetMaterial(pttoMaterial); 
    //PrintDetectorParameters();
	G4double phantomVolume = PhantomLog->GetSolid()->GetCubicVolume();
    phantomMass = phantomDensity * phantomVolume;
  } 
  else
    G4cout << "WARNING: material '" << materialChoice
           << "' not available!" << G4endl;            
}
