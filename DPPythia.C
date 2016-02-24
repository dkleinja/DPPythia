//Goal is to generate pythia events for Seaquest spectrometer


#include "Pythia8/Pythia.h"

#include "TH1.h"
#include "TH2.h"
#include "TFile.h"
#include "TTree.h"
#include "TVector3.h"
#include "TClonesArray.h"

using namespace Pythia8;

float minEnergy = 5;
float minEta = 2.0;
float maxEta = 5.0;
float minZvtxDecay = 1.35;

int main() {

  // Generator.
  Pythia pythia;

  // Shorthand for the event record in pythia.
  Event& event = pythia.event;

  // Read in commands from external file.
  pythia.readFile("pythia_inclusive.cmnd");

  // Extract settings to be used in the main program.
  int nEvent = pythia.mode("Main:numberOfEvents");
  int nAbort = pythia.mode("Main:timesAllowErrors");

  // Initialize.
  pythia.init();

  // Book histograms.
  TH1F *Hdummy = new TH1F("dummy histo", "dummy histo",10, 0, 1); 
  
  TVector3 *getMomentum = new TVector3();
  TVector3 *getPosition = new TVector3();

  int nParticles;
  int particleID[10000];
  int parentID[10000];
  TClonesArray *fMomentum = new TClonesArray("TVector3");
  fMomentum -> BypassStreamer();
  TClonesArray& Momentum = *fMomentum;
  TClonesArray *fPosition = new TClonesArray("TVector3");
  fPosition -> BypassStreamer();
  TClonesArray& Position = *fPosition;

  //Make TTree
  TTree *pythiatree = new TTree("save", "save");
  pythiatree -> Branch("n", &nParticles, "n/I");
  pythiatree -> Branch("pdg", particleID, "pdg[n]/I"); 
  pythiatree -> Branch("parent", parentID, "parent[n]/I"); 
  pythiatree -> Branch("pos", &fPosition, 256000, 99);
  pythiatree -> Branch("mom", &fMomentum, 256000, 99);


  // Begin event loop.
  int iAbort = 0;
  int nMuons = 0;
  int nPions = 0;
  int nKaons = 0;
  for (int iEvent = 0; iEvent < nEvent; ++iEvent) {

    // Generate events. Quit if many failures.
    if (!pythia.next()) {
      if (++iAbort < nAbort) continue;
      cout << " Event generation aborted prematurely, owing to error!\n";
      break;
    }

    // Loop over final particles in the event.
    for (int i = 0; i < event.size(); ++i){
      //if (event[i].id() == 2212) {
	//cout << iEvent << " We got a proton!  Energy:  " << event[i].e() << "  zProd:  " << event[i].zDec()/1000 << "  Mother1:  " << event[event[i].mother1()].id() << endl;
      //}
      if(event[i].e() > minEnergy){
	if (abs(event[i].id()) == 13) {
	  if(event[i].zProd()/1000 < minZvtxDecay){
	    nMuons++;
	    //cout << iEvent << " We got a muon!  Energy:  " << event[i].e() << "  zProd:  " << event[i].zProd()/1000 << "  Mother1:  " << event[event[i].mother1()].id() << endl;
	    
	    getMomentum -> SetXYZ(event[i].px(), event[i].py(), event[i].pz());
	    getPosition -> SetXYZ(event[i].xProd(), event[i].yProd(), event[i].zProd());	    
	    nParticles++;
	    particleID[nParticles-1] = event[i].id();
	    parentID[nParticles-1] = event[event[i].mother1()].id();
	    new(Momentum[nParticles-1]) TVector3(*getMomentum);
	    new(Position[nParticles-1]) TVector3(*getPosition);
	    //pythiatree -> Fill();
	  }
	}
	/*
	if (abs(event[i].id()) == 211){
	  if(event[i].zProd()/1000 < minZvtxDecay && event[i].zDec()/1000 > minZvtxDecay){
	    //if(event[i].zDec()/1000 > minZvtxDecay){
	    nPions++;
	    //cout << iEvent << " We got a pion!  Energy:  " << event[i].e() << " zDec:  " << event[i].zDec()/1000 << " zDec:  " << event[i].zDec()/1000 << "  Mother1:  " << event[event[i].mother1()].id() << endl;
	    
	    getMomentum -> SetXYZ(event[i].px(), event[i].py(), event[i].pz());
	    getPosition -> SetXYZ(event[i].xProd(), event[i].yProd(), event[i].zProd());	    
	    nParticles++;
	    particleID[nParticles-1] = event[i].id();
	    parentID[nParticles-1] = event[event[i].mother1()].id();
	    new(Momentum[nParticles-1]) TVector3(*getMomentum);
	    new(Position[nParticles-1]) TVector3(*getPosition);
	    //pythiatree -> Fill();
	  }   
	}
	if (abs(event[i].id()) == 321){
	  if(event[i].zProd()/1000 < minZvtxDecay && event[i].zDec()/1000 > minZvtxDecay){
	    //if(event[i].zDec()/1000 > minZvtxDecay){
	    nKaons++;
	    //cout << iEvent << " We got a kaon!  Energy:  " << event[i].e() << "  zDec:  " << event[i].zDec()/1000 << "  Mother1:  " << event[event[i].mother1()].id() << endl;
	    
	    getMomentum -> SetXYZ(event[i].px(), event[i].py(), event[i].pz());
	    getPosition -> SetXYZ(event[i].xProd(), event[i].yProd(), event[i].zProd());	    
	    nParticles++;
	    particleID[nParticles-1] = event[i].id();
	    parentID[nParticles-1] = event[event[i].mother1()].id();
	    new(Momentum[nParticles-1]) TVector3(*getMomentum);
	    new(Position[nParticles-1]) TVector3(*getPosition);
	    //pythiatree -> Fill();
	  }   
	}
	*/
      }
      
      // End of particle loop. Fill global properties. 
    }
    //fill tree, reset counters
    if(nParticles>0)pythiatree -> Fill();
    nParticles = 0.;
    fMomentum->Clear();
    fPosition->Clear();

    // End of event loop.
  }
  cout << "The total number of muons before dump is " << nMuons << endl;
  cout << "The total number of pions is " << nPions << endl;
  cout << "The total number of kaons is " << nKaons << endl;
  // Final statistics. Normalize and output histograms.
  pythia.stat();

  // Create file on which histogram(s) can be saved.
  TFile* outFile = new TFile("pythiatree.root", "RECREATE");
  
  // Save histogram on file and close file.
  pythiatree -> Write();

  outFile -> Close();
  delete outFile;
  delete pythiatree;

  // Done.
  return 0;
}
