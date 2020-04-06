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

int ProcessDetectorFiles(string fileSaveName = "ub28n005_processed", string CSVfile = "") //u28n0001_processed for root file, u28n005_2020228 for pressure csv, longRunPico for temp csv
{

  if(fileSaveName == "")
    {
      std::cout<<"No ROOT file selected"<<std::endl;
    }
  else
    {
      TChain * simul = new TChain("SphPulsePros");
      TCanvas * c1 = new TCanvas("c1","c1", 600, 600);
 
      TString dir = ("/disk/moose/general/prk/BGDL_processed/");
      simul->Add(dir+fileSaveName+".root");


      TTreeReader myReader(simul);
      TTreeReaderArray<double> timeCorrected(myReader, "timeCorrected");
      //TTreeReaderArray<double> mybaseline(myReader, "mybaseline");
      //TTreeReaderArray<double> mynoise(myReader, "mynoise");
      TTreeReaderArray<double> myrisetime(myReader, "myrisetime");
      TTreeReaderArray<double> mywidth(myReader, "mywidth");
      TTreeReaderArray<double> myamplitude(myReader, "myamplitude");
      TTreeReaderArray<double> rawamplitude(myReader, "rawamplitude");
      TTreeReaderArray<int> mytimes(myReader, "mytimes");
      TTreeReaderArray<int> mytimesDif(myReader, "mytimesDif");
      TTreeReaderArray<double> integral(myReader, "integral");
      TTreeReaderArray<int> time(myReader, "time");
  
      double Amp_Max = 0;
      double Amp_Min = 999999;
      double RT_Max = 0;
      double RT_Min = 999999;
      double Date_Max = 0;
      double Date_Min = 999999;

      int test = 0; 

      TH2D * xyVariance = new TH2D("xyVariance",
				   "Variance; x [cm]; y [cm]; Std. Dev",
				   200, 0, 16,
				   200, -16, 16);
  
      gPad->SetRightMargin(0.13);
      gPad->SetLeftMargin(0.13);
  

      while(myReader.Next())
	{
	  if(myamplitude[0] <= 0)  
	    {            
	      continue;
	    }
	  if(myamplitude[0] > Amp_Max)
	    {
	      Amp_Max = myamplitude[0];
	    }
	  if(myamplitude[0] < Amp_Min)
	    {
	      Amp_Min = myamplitude[0];
	    }
	  if(myrisetime[0] > RT_Max)
	    {
	      RT_Max = myrisetime[0];
	    }
	  if(myrisetime[0] < RT_Min)
	    {
	      RT_Min = myrisetime[0];
	    }
	  if(timeCorrected[0] < Date_Min)
	    {
	      Date_Min = timeCorrected[0];
	    }
	  if(timeCorrected[0] > Date_Max)
	    {
	      timeCorrected[0] = Date_Max;
	    }

	  
	}
      test++;
      //std::cout << Amplitude[0] <<"  "<< Risetime[0] <<"  "<< test << std::endl;
      std::cout<<"max date = "<<Date_Max<<" "<<"min date =  "<<Date_Min<<std::endl;

      TH2D * Amp_RT = new TH2D("Amp_RT",
			       "Amplitude vs Risetime; Amplitude; Risetime; Events ",
			       100, Amp_Min/1.1, Amp_Max*1.1,
			       100, RT_Min/1.1, RT_Max*1.1);

      TH2D * Amp_date = new TH2D("Amp_date",
				 "Amplitude vs Date; Date; Amplitude; Events ",
				 Date_Min/10, 0, Date_Max,
				 100, Amp_Min/1,1, Amp_Max*1.1);

  
      myReader.Restart();
  
      while(myReader.Next())
	{
	  if(myamplitude[0] <= 0)
	    {                   
	      continue;
	    }
      
	  Amp_RT->Fill(myamplitude[0],myrisetime[0]);
	  Amp_date->Fill(timeCorrected[0],rawamplitude[0]);
	}
   
      c1->SetFrameFillColor(0);
      Amp_date->Draw("colz>>c1");
      c1->Update();

}


  if(CSVfile == "")
    {
      std::cout << "no CSV file passed" << std::endl;
    }
  else
    {
      
      std::ifstream csv("longRunPico.csv");
      std::ifstream csv_temp(CSVfile);
      std::string line;
      std::vector<string> time; // first column
      std::vector<double> temp; // 2nd column = ave. temp
      std::vector<string> Vhumidity; // 3rd column = ave. voltage for humidity
      std::vector<string> Vsupply; // 4th column = ave. supply voltage
      std::vector<double> humidity; // 5th column is uncorrected humidity ave.
      std::vector<double> humidity60;
      double pressureMin = 9999.999;
      double pressureMax = 0.0001;
      int missingData=0;
      int iterator=0;
      
      while(std::getline(csv, line, '\n'))
	{
	  std::istringstream iss{line};
	  std::vector<std::string> tokens;
	  std::string token;
	  //std::cout<<"\n\n\nLINE: "<<line<<"\n";
	  //std::cout<<"tokens size:"<<tokens.size()<<"\n";
	  while(std::getline(iss, token, ',')) //typically delimiter is , for picolog and ; for pressure
	    {
	      tokens.push_back(token);
	      //std::cout<<"tokens size:"<<tokens.size()<<"\n";
	      //std::cout <<"token:" << token << std::endl;
	    }
	  //std::cout<<tokens.size()<<"\n\n\n";
	  if(tokens.size()==9)  //5 for test data, 9 for picolog and 8 for pressure
	    {
	      std::string::size_type sz;
	      //time.push_back(tokens[0]);
	      temp.push_back(std::stod(tokens[2],&sz));
	      //std::cout<<tokens[2]<<std::endl;
	    }
	  else
	    {
	      //std::cout<<"YOU WERE MISSING SOME DATA HERE\n";
	      missingData++;
	  //std::cout << time[0] << " " << temp[0] << std::endl;
	  //std::cout << tokens[1580390] << " " << tokens[1975495] << std::endl;
	  //std::cout << tokens.size() << std::endl;
	    }
	}

      while(std::getline(csv_temp, line, '\n'))
	{
	  std::istringstream iss2{line};
	  std::vector<std::string> tokens2;
	  std::string token2;
	  //std::cout<<"\n\n\nLINE: "<<line<<"\n";
	  //std::cout<<"tokens size:"<<tokens.size()<<"\n";
	  while(std::getline(iss2, token2, ';')) //typically delimiter is , for picolog and ; for pressure
	    {
	      tokens2.push_back(token2);
	      //std::cout<<"tokens size:"<<tokens.size()<<"\n";
	      //std::cout <<"token:" << token << std::endl;
	    
	  //std::cout<<tokens.size()<<"\n\n\n";
	  if(tokens2.size()==8)  //5 for test data, 9 for picolog and 8 for pressure
	    {
	      std::string::size_type sz;
	      //time.push_back(tokens[0]);
		  humidity.push_back(std::stod(tokens2[1],&sz));
		  std::cout<<tokens2[0]<<std::endl;
		  if(std::stod(tokens2[1],&sz) > pressureMax)
		    {
		      pressureMax = std::stod(tokens2[1],&sz);
		    }
		  if(std::stod(tokens2[1],&sz) < pressureMin)
		    {
		      pressureMin = std::stod(tokens2[1],&sz);
		    }
	    }
	      else
	     {
		 missingData++;
	      }
	    
	    }
	}

      std::cout<<"THE RESULTS: "<<temp.size()<<" With this many missing: "<<missingData<<"\n\n\n";

      
      
      Int_t n = humidity.size();
      Double_t temp_coords[n], x[n], humid_coords[n];
      for (Int_t i=0; i<n; i++)
	{
        x[i] = i;
	temp_coords[i] = humidity[i];
	//double difference = pressureMax - pressureMin;
	//double factor = 6/difference;
	//int sixti = 60*i;
	//humid_coords[i] = (humidity[sixti]-pressureMin+3*difference)*factor;
	//std::cout<<"diff = "<<difference<<" "<<"pMin = "<<pressureMin<<" "<<"pMax = "<<pressureMax<<" "<<"factor: "<<factor<<std::endl;
	}

      
      TGraph *gr1 = new TGraph (n, x, temp_coords);
      TGraph *gr2 = new TGraph (n, x, humid_coords);
      TMultiGraph *mg = new TMultiGraph();

      gr1->SetLineColor(kRed);
      //gr2->SetLineColor(kRed);
      gr1->SetTitle("Pressure");
      // gr2->SetTitle("Temperature");
      mg->SetTitle("Pressure  vs Time; Time; Pressure (bar)");

      mg->Add(gr1);
      // mg->Add(gr2);

      mg->Draw("a");
      
    }
  
  return 0;

}

/*
  ~Variables!~ (so I don't have to do simul->Print() everytime I want to see what variables I can test)

  For the new ROOT files: (can see these by doing root filename, new TBrowser in console then double clicking filename_all_processed.root, SphPulsePros;1. You want to analyse the variables starting with "my"

  evID, rawbaseline, rawtime1, rawtime2, rawnoise, rawamplitude, rawrisetime, datapoints, tmicro, time, timeCorrected, mybaseline, mynoise, myrisetime, mywidth, myamplitude, peaks, mytimes, mytimesDif, integral, cumulativeIntegral, pulse, dpulse, delay, saturated, longeurIA, longuerIAW (these last two are like FWHM alternatives)

*/ 
