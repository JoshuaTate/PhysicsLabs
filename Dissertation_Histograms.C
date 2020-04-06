#include "TFile.h"
#include "TChain.h"
#include "TH1F.h"
#include "TH3F.h"
#include "TMath.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTreeReaderArray.h"
#include <sstream>
#include <cmath>
#include "string.h"
// I use a 'main()' script to print of multiple hists if I need to, so I need to prototype the code first
int processFiles_Variance_Fin(TString fileSaveName = "output_e-_3+eV_Umbrella15cm2mm1430V_anywhere_radial_100000", TString Choicevar = "FinElecCosTheta",
			      int Theta_space = 25, int Radius_space = 75, int x_space = 90, int y_space = 180);
int main()
{                                                    // Here I can pass whatever arguments I want, as many times
  processFiles_Variance_Fin("output_e-_3+eV_Umbrella15cm2mm1430V_anywhere_radial_100000");
  // as I want to generate multiple histograms
  processFiles_Variance_Fin("output_e-_3+eV_Umbrella15cm2mm1430V_anywhere_radial_100000", "FinElecCosTheta");
  processFiles_Variance_Fin("output_e-_3+eV_Segmented3090Lengthave15cm2mm1430V_anywhere_radial_100000");
  processFiles_Variance_Fin("output_e-_3+eV_Segmented3090Lengthave15cm2mm1430V_anywhere_radial_100000", "FinElecCosTheta");
  return 0;
}
int processFiles_Variance_Fin(TString fileSaveName = "output_e-_3+eV_Umbrella15cm2mm1430V_anywhere_radial_100000", TString Choicevar = "Risetime",
			      int Theta_space = 25, int Radius_space = 75, int x_space = 90, int y_space = 180)
{
  TChain * simul = new TChain("simul");
  TCanvas * c1 = new TCanvas("c1","c1", 600, 600);
  // These are the files to analyse; I haven't found a way to reliably automatically select one yet so I just
  // Comment out the one I don't want to analyse
  
  //simul->Add("/disk/moose/general/prk/anywhere3/output_e-_3+eV_Segmented3090Lengthave15cm2mm1430V_anywhere_radial_100000.root");
  
  TString dir = ("/disk/moose/general/prk/anywhere3/");
  //TString dir = ("/disk/moose/general/prk/bgdlProcessed/ua30n000/");
  //  simul->Add("/disk/moose/general/prk/anywhere3/output_e-_3+eV_Umbrella15cm2mm1430V_anywhere_radial_100000.root");    
  simul->Add(dir+fileSaveName+".root");
  
  TFile * m_outFile = TFile::Open("Variance", "RECREATE");
  gStyle->SetOptStat(0);           // I use my own legends for more customisation so I disabled the default stats box
  TTreeReader myReader(simul);     // Variables to fill the various histograms this macro can produce
  TTreeReaderArray<double> Variable(myReader, Choicevar);
  TTreeReaderArray<double> InitElecCosTheta(myReader, "InitElecCosTheta");
  TTreeReaderArray<double> InitialParticleRadius(myReader, "InitialParticleRadius");
  TTreeReaderArray<double> InitialParticleCosTheta(myReader, "InitialParticleCosTheta");
  TTreeReaderArray<int> NumInitElecs(myReader, "NumInitElecs");
  TTreeReaderArray<double> Amplitude(myReader, "Amplitude");
  TTreeReaderArray<double> Risetime(myReader, "Risetime");
  TTreeReaderArray<double> FinElecRadius(myReader, "FinElecRadius");
  TTreeReaderArray<double> FinElecCosTheta(myReader, "FinElecCosTheta");
  
  std::vector<double> x_sum;   // all the variables used to calculate the mean and standard devs
  double stds_uh = 0;          // detector is split into upper hemisphere (uh) and lower hemisphere (lh) by default
  double stds_lh = 0;          // although it can be split over whatever ranges of cos(theta) you want
  double var_sum_uh = 0;
  double var_sum_lh = 0;
  double Amp_Max = 0;
  double Amp_Min = 999999;
  double RT_Max = 0;
  double RT_Min = 999999;
  std::vector<double> vars;
  std::vector<double> icosthetas;
  int i = 0;
  int test = 0;     // Random variables I use to test my code, particularly 'if' statement iterations
  int entries_uh = 0;
  int entries_lh = 0;
  int ii = 0;
  int ij = 0;
  std::vector<double> diagnosis;
  // Defining all histograms I might want to draw - I only really use xyVariable and xyVariance though
  TH2F * cosThetaRadiusVariable = new TH2F("cosThetaRadiusVariable",
      "cosThetaRadiusElecDrift; cos(#theta); Radius [cm]; Elec Drift [ns];",
      Theta_space, -1, 1,
      Radius_space, 0, 16);
  TH2F * cosThetaRadiusN = new TH2F("cosThetaRadiusN",
      "cosThetaRadiusN; cos(#theta); Radius [cm]; Events;",
      Theta_space, -1, 1,
      Radius_space, 0, 16);
  
  TH2D * cosThetaRadiusVariance = new TH2D("cosThetaRadiusVariance",
      "cosThetaRadiusVariance; cos(#theta); Radius [cm]; Variance",
      Theta_space, -1, 1,
      Radius_space, 0, 16);
  TH2F * xyVariable = new TH2F("xyVariable",
      "Choicevar; x [cm]; y [cm]; ",
      x_space, 0, 16,
      y_space, -16, 16);
  // This one is just used to map the detector and let me split it into regions and has no data analysis value
  TH2F * InitElecCosThetaVar = new TH2F("InitElecCosThetaVar",
      "CosTheta; x [cm]; y [cm]; ",
      x_space, 0, 16,
      y_space, -16, 16);
  TH2F * xyN = new TH2F("xyN",
      "xyN; x [cm]; y [cm]; Events ",
      x_space, 0, 16,
      y_space, -16, 16);			  
  TH2D * xyVariance = new TH2D("xyVariance",
      "Variance; x [cm]; y [cm]; Std. Dev",
      x_space, 0, 16,
      y_space, -16, 16);
  // I use this method for automatically naming my hists; probably better ways of doing it but this is the first thing
  // I found on stack overflow and it seems to work pretty well
  char buffer[256];
  strncpy(buffer, Choicevar, sizeof(buffer));
  strncat(buffer, " Histogram", sizeof(buffer));
  xyVariable->SetTitle(buffer);
  
  char buffer2[256];
  strncpy(buffer2, Choicevar, sizeof(buffer2));
  strncat(buffer2, " Standard deviation", sizeof(buffer2));
  xyVariance->SetTitle(buffer2);
  
  gPad->SetRightMargin(0.13); // Required or the canvas cuts off the values written on the Y axis
  gPad->SetLeftMargin(0.13);
  
  while(myReader.Next())
  {
    if(Variable.GetSize() < 1)  // The ROOT bug where dodgy some events simply are null in that the variables
      {             // correspond to completely empty arrays (e.g. 0 size) that will crash ROOT if you pass them
        continue;   // so this is needed to filter them out to at least make use of the good data points
      }
    
    if(FinElecRadius[0] > 0.5)
      {
       continue;
       }
       /*if(FinElecCosTheta[0] > -0.5)
      {
       continue;
       }*/
    
    double R = InitialParticleRadius[0];
    double y = R*InitialParticleCosTheta[0];
    double x = TMath::Sqrt(1 - InitialParticleCosTheta[0]*InitialParticleCosTheta[0])*R;
    // Apparently it's more accurate on computers to do this rather than just R*sin(theta)
    
    cosThetaRadiusVariable->Fill(InitialParticleCosTheta[0], InitialParticleRadius[0], Variable[0]);
    cosThetaRadiusVariance->Fill(InitialParticleCosTheta[0], InitialParticleRadius[0], 0);
    cosThetaRadiusN->Fill(InitialParticleCosTheta[0], InitialParticleRadius[0]);
    xyVariable->Fill(x,y,Variable[0]);
    InitElecCosThetaVar->Fill(x,y,InitElecCosTheta[0]);
    xyVariance->Fill(x,y,0);
    xyN->Fill(x,y);
    if(Amplitude[0] > Amp_Max)
      {
	Amp_Max = Amplitude[0];
      }
    if(Amplitude[0] < Amp_Min)
      {
	Amp_Min = Amplitude[0];
      }
    if(Risetime[0] > RT_Max)
      {
	RT_Max = Risetime[0];
      }
    if(Risetime[0] < RT_Min)
      {
	RT_Min = Risetime[0];
	}
    test++;
    //std::cout << Amplitude[0] <<"  "<< Risetime[0] <<"  "<< test << std::endl;
    if(InitElecCosTheta[0] > 0)    // Counts how many entries are in upper or lower half of detector
      {
	entries_uh++;    
      }
    if(InitElecCosTheta[0] <= 0)   // You could change the 0's to some other value to evaluate specific segments of interest
      {
	entries_lh++;
      }
  }
  
  cosThetaRadiusVariable->Divide(cosThetaRadiusN);
  cosThetaRadiusVariance->Divide(cosThetaRadiusN);
  xyVariable->Divide(xyN);
  cosThetaRadiusVariable->Write();
  cosThetaRadiusN->Write();
  xyVariable->Write();
  xyN->Write();
  int bins_xy = xyVariable->GetNbinsX()*xyVariable->GetNbinsY();
  int bins_xy_uh = 0;
  int bins_xy_lh = 0;
  int bins_rc = cosThetaRadiusVariable->GetNbinsX()*cosThetaRadiusVariable->GetNbinsY();
  
  /*TH2D * Amp_RT = new TH2D("Amp_RT",
      "Amplitude vs Risetime; Amplitude; Risetime; Events ",
        100, Amp_Min/1.1, Amp_Max*1.1,
        100, RT_Min/1.1, RT_Max*1.1);*/
  TH2D * Amp_RT = new TH2D("Amp_RT",
      "Amplitude vs Risetime; Amplitude; Risetime; Events ",
        100, 0, 1.5,
        100, 2.25, 8.25);
  
  myReader.Restart();
  
  while(myReader.Next())
    {
      if(Variable.GetSize() < 1)
	{                   
          continue;
	}
      if(FinElecRadius[0] > 0.5)
      {
       continue;
       }
      /*if(FinElecCosTheta[0] > -0.5)
      {
       continue;
       }*/
      
      Amp_RT->Fill(Amplitude[0],Risetime[0]);
      
      if(ii < (bins_xy + 1))
	  {
	    double errorxy = sqrt(xyVariable->GetBinError(ii));
	    xyVariance->SetBinContent(ii,errorxy);
	    
	    if(InitElecCosThetaVar->GetBinContent(ii) > 0)
	      {
		if(xyVariable->GetBinContent(ii) > 0.1)
		  {
		    stds_uh = stds_uh + errorxy;
		    var_sum_uh = var_sum_uh + xyVariable->GetBinContent(ii);
		    bins_xy_uh++;
		    std::cout<<ii<<" "<<xyVariable->GetBinContent(ii)<<std::endl;
		  }
		else
		  {
		    std::cout<<bins_xy_uh<<" "<<var_sum_uh<<std::endl;
		  }
	      }
	    if(InitElecCosThetaVar->GetBinContent(ii) <= 0)
	      {
		if(xyVariable->GetBinContent(ii) > 0.1)
		  {
		    stds_lh = stds_lh + errorxy;
		    var_sum_lh = var_sum_lh + xyVariable->GetBinContent(ii);
		    bins_xy_lh++;
		  }
		else
		  {
		    int j = 0;
		    //std::cout<<bins_xy_lh<<" "<<var_sum_lh<<std::endl;
		  }
	      }
	    ii++;
	  }
      if(ij < (bins_rc + 1))
	  {
	    double errorrc = sqrt(cosThetaRadiusVariable->GetBinError(ij));
	    cosThetaRadiusVariance->SetBinContent(ij,errorrc);
	    ij++;
	  }
    }
    
  cosThetaRadiusVariance->Write();
  xyVariance->Write();
   
  // draw the histogram(s)
  c1->SetFrameFillColor(0);
  xyVariable->Draw("colz>>c1");  // to see another histogram do NameOfHistogram->Draw("colz>>c1") in terminal after running macro
  double var_mean_uh = var_sum_uh / (bins_xy_uh);  // bins_xy is the no. bins in the square canvas, not the elliptical region
  double var_stds_uh = stds_uh / (bins_xy_uh);     // area of an ellipse would be semi-minor axis * semi-major axis * pi
  double var_mean_lh = var_sum_lh / (bins_xy_lh);  // in terms on binx_xy the no, bins in each quarter-ellipse is bins_xy * pi / 8
  double var_stds_lh = stds_lh / (bins_xy_lh);     // C++ doesn't have a constant for pi so I just included it manually (0.3927 to 4 s.f.)
  c1->Update();
  auto legend_uh = new TLegend(0.53,0.78,0.9,0.87);
  legend_uh->SetBorderSize(0);
  legend_uh->SetNColumns(1);
  legend_uh->SetFillStyle(0);
  legend_uh->SetTextSize(0.034);
  
  auto legend_lh = new TLegend(0.53,0.13,0.9,0.22);
  legend_lh->SetBorderSize(0);
  legend_lh->SetNColumns(1);
  legend_lh->SetFillStyle(0);
  legend_lh->SetTextSize(0.034);
  char legmean_uh[256];
  stringstream lm_uh;
  lm_uh << var_mean_uh;
  const char* str_m_uh = lm_uh.str().c_str();
  strncpy(legmean_uh, "mean =", sizeof(buffer));
  strncat(legmean_uh, str_m_uh, sizeof(buffer));
  char legmean_lh[256];
  stringstream lm_lh;
  lm_lh << var_mean_lh;
  const char* str_m_lh = lm_lh.str().c_str();
  strncpy(legmean_lh, "mean =", sizeof(buffer));
  //strncat(legmean_lh, str_m_lh, sizeof(buffer));
  strncat(legmean_lh, str_m_lh, sizeof(buffer));
  
  
  char legentries_uh[256];
  stringstream le_uh;
  le_uh << entries_uh;
  const char* str_e_uh = le_uh.str().c_str();
  strncpy(legentries_uh, "entries =", sizeof(buffer));
  strncat(legentries_uh, str_e_uh, sizeof(buffer));
  char legentries_lh[256];
  stringstream le_lh;
  le_lh << entries_lh;
  const char* str_e_lh = le_lh.str().c_str();
  strncpy(legentries_lh, "entries =", sizeof(buffer));
  strncat(legentries_lh, str_e_lh, sizeof(buffer));
  char legvariance_uh[256];
  stringstream lv_uh;
  lv_uh << var_stds_uh;
  const char* str_s_uh = lv_uh.str().c_str();
  strncpy(legvariance_uh, "std. dev =", sizeof(buffer));
  strncat(legvariance_uh, str_s_uh, sizeof(buffer));
  char legvariance_lh[256];
  stringstream lv_lh;
  lv_lh << var_stds_lh;
  const char* str_s_lh = lv_lh.str().c_str();
  strncpy(legvariance_lh, "std. dev =", sizeof(buffer));
  strncat(legvariance_lh, str_s_lh, sizeof(buffer));
  
  legend_uh->AddEntry(c1, legmean_uh, "");
  legend_uh->AddEntry(c1, legentries_uh, "");
  legend_uh->AddEntry(c1, legvariance_uh, "");
  legend_uh->Draw();
  
  legend_lh->AddEntry(c1, legmean_lh, "");
  legend_lh->AddEntry(c1, legentries_lh, "");
  legend_lh->AddEntry(c1, legvariance_lh, "");
  legend_lh->Draw();
  
  c1->Update();
  c1->Print(Choicevar+"_"+fileSaveName+".png");
  return 0;
}
/*
~Variables!~ (so I don't have to do simul->Print() everytime I want to see what variables I can test)
InitialParticleType, InitialParticleEnergy, InitialParticleRadius, InitialParticleCosTheta, InitialParticlePhi
InitialParticleTime, InitialParticleDirRad, InitialParticleDirCosTheta, InitialParticleDirPhi
InitialParticleDirX, InitialParticleDirY, InitialParticleDirZ, InitElecRadius, InitElecPhi, InitElecCosTheta
InitElecTime, InitIonRadius, InitIonPhi, InitIonCosTheta, InitIonTime, FinElecRadius, FinElecPhi, FinElecCosTheta
FinElecTime, FinIonRadius, FinIonPhi, FinIonCosTheta, FinIonTime, IonCharge, AvalancheSize, Signal, Signalt
Pulse, Pulset, ElecDriftTime, IonDriftTime, NumInitElecs, AverageGain, IntegratedTownsend, PrimaryProcess
PrimaryDeltaEnergy, PrimaryCosTheta, PrimaryPhi, PrimaryVolume, SecondaryProcess, SecondaryParent, SecondaryID
SecondaryDeltaEnergy, SecondaryRadius, SecondaryCosTheta, SecondaryPhi
For the new ROOT files: (can see these by doing root filename, new TBrowser in console then double clicking filename_all_processed.root, SphPulsePros;1. You want to analyse the variables starting with "my"
evID, rawbaseline, rawtime1, rawtime2, rawnoise, rawamplitude, rawrisetime, datapoints, tmicro, time, timeCorrected, mybaseline, mynoise, myrisetime, mywidth, myamplitude, peaks, mytimes, mytimesDif, integral, cumulativeIntegral, pulse, dpulse, delay, saturated, longeurIA, longuerIAW (these last two are like FWHM alternatives)
*/
