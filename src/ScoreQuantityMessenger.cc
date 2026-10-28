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
// Authors: D. Fulea, National Institute of Public Health Bucharest, Cluj-Napoca Regional Center, Romania
//
#include "ScoreQuantityMessenger.hh"
#include "ScoringManager.hh"
#include "G4ScoringManager.hh"
#include "PSDoseDeposition2.hh"
#include "BrachyUserScoreWriter.hh"

#include "G4RunManager.hh"
#include "BrachyDetectorConstruction.hh"

ScoreQuantityMessenger::ScoreQuantityMessenger(G4ScoringManager* SManager)
:G4ScoreQuantityMessenger(SManager),fSMan(SManager)//call superclass constructor and set fsMan = SManager
{
   G4UIparameter* param;

  qdoseDep2Cmd = new G4UIcommand("/score/quantity/doseDeposit2",this);
  qdoseDep2Cmd->SetGuidance("Custom dose deposit scorer.");
  qdoseDep2Cmd->
  SetGuidance("[usage] /score/quantiy/doseDeposit2 qname unit");
  qdoseDep2Cmd->SetGuidance("  qname  :(String) scorer name");
  qdoseDep2Cmd->SetGuidance("  unit   :(String) unit");
  param = new G4UIparameter("qname",'s',false);
  qdoseDep2Cmd->SetParameter(param);
  param = new G4UIparameter("unit",'s',true);
  param->SetDefaultValue("Gy");
  qdoseDep2Cmd->SetParameter(param);

  mBinCmd = new G4UIcommand("/other/mesh/nBin",this);
  mBinCmd->SetGuidance("Define segments of the scoring mesh.");
  param = new G4UIparameter("Ni",'i',false);
  param->SetDefaultValue("1");
  param->SetParameterRange("Ni>0");
  mBinCmd->SetParameter(param);
  param = new G4UIparameter("Nj",'i',false);
  param->SetDefaultValue("1");
  param->SetParameterRange("Nj>0");
  mBinCmd->SetParameter(param);
  param = new G4UIparameter("Nk",'i',false);
  param->SetDefaultValue("1");
  mBinCmd->SetParameter(param);
  param->SetParameterRange("Nk>0");
 
}

ScoreQuantityMessenger::~ScoreQuantityMessenger()
{
  delete    qdoseDep2Cmd;
  delete    mBinCmd;
}

//SetNewValue is override of SUPERCLASS and handled internaly by GEANT4!!!!
//my superclass is G4ScoreQuantityMessenger  so only commands that are overriden are those of form:
// /score/quantity/, /score/filter/....
//Other commands like /score/mesh/ are specific to other class, G4ScoringMessenger with is own SetNewValue
void ScoreQuantityMessenger::SetNewValue(G4UIcommand * command,G4String newVal)
{
	// comenzile care nu sunt ale noastre: le preia clasa de baza
	if (command != qdoseDep2Cmd && command != mBinCmd) {
		G4ScoreQuantityMessenger::SetNewValue(command, newVal);
		return;
	}

	G4VScoringMesh* mesh = fSMan->GetCurrentMesh();
      if(!mesh)
      {
        G4cerr << "ERROR : No mesh is currently open. Open/create a mesh first. Command ignored." << G4endl;
        return;
      }
      // Tokens..for instance /score/quantity/doseDeposit2 myDoseGy Gy->
	  //token[0]=myDoseGy; token[1]=Gy
      G4TokenVec token;
      FillTokenVec(newVal,token);
      
      // Commands for Current Mesh
	  if(command== qdoseDep2Cmd) {//command :/score/quantity/doseDeposit2 eDep2
		if ( CheckMeshPS(mesh,token[0]) ){//safe function probably to avoid duplicates!!!
           if( mesh->GetShape()==boxMesh ) {
			   //G4cout<<"ANA ARE MERE!"<<G4endl;
				
			    PSDoseDeposition2* ps = new PSDoseDeposition2(token[0]);
				ps->SetUnit(token[1]);
				mesh->SetPrimitiveScorer(ps);

				//similar cu codul de la cilindru:
				//dimensiunea (semi-lungimi) si numarul de segmente se citesc direct din mesh
				G4ThreeVector msize = mesh->GetSize();
				BrachyUserScoreWriter::setMeshHalfSize(msize.x(), msize.y(), msize.z());

				G4int nSeg[3];
				mesh->GetNumberOfSegments(nSeg);
								
				BrachyUserScoreWriter::setMeshSegment_X(nSeg[0]);
				BrachyUserScoreWriter::setMeshSegment_Y(nSeg[1]);
				BrachyUserScoreWriter::setMeshSegment_Z(nSeg[2]);

				//also update the phantom:
				// === CODUL NOU DE CONECTARE CU DETECTORUL ===
				G4RunManager* runManager = G4RunManager::GetRunManager();
				BrachyDetectorConstruction* detector = (BrachyDetectorConstruction*)(runManager->GetUserDetectorConstruction());

				if(detector) {
					// Trimitem aceleasi dimensiuni din mesh direct in phantomul fizic
					detector->UpdatePhantomSize(msize.x(), msize.y(), msize.z());
				 }
				 
				
            } 
			else if( mesh->GetShape()==cylinderMesh ) {
              //G4PSDoseDepositForCylinder3D* ps = 
			  //new G4PSDoseDepositForCylinder3D(token[0]);
              //ps->SetUnit(token[1]);
	          //G4ThreeVector msize = mesh->GetSize(); // gevin in R Z N/A
              //ps->SetCylinderSize(msize[0],msize[1]); // given in dr dz
              //G4int nSeg[3];
              //mesh->GetNumberOfSegments(nSeg);
              //ps->SetNumberOfSegments(nSeg);
	          //mesh->SetPrimitiveScorer(ps);
            }
		}
      } else if(command==mBinCmd) {
	      MeshBinCommand(mesh,token);
	  } 
	  
}

void ScoreQuantityMessenger::MeshBinCommand(G4VScoringMesh* mesh, G4TokenVec& token){
    
	//NOT NEEDED ANYMORE...just for compatibility with spring boot server!!!!
	//this does nothing
	
	/*G4int Ni = StoI(token[0]);
    G4int Nj = StoI(token[1]);
    G4int Nk = StoI(token[2]);

	G4cout<<"Messenger: nX = "<<Ni<<" ; nY= "<<Nj<<" ; nZ= "<<Nk<<G4endl;

	BrachyUserScoreWriter::setMeshSegment_X(Ni);
	BrachyUserScoreWriter::setMeshSegment_Y(Nj);
	BrachyUserScoreWriter::setMeshSegment_Z(Nk);*/
}
