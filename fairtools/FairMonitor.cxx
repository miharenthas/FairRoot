/********************************************************************************
 *    Copyright (C) 2014 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH    *
 *                                                                              *
 *              This software is distributed under the terms of the             * 
 *         GNU Lesser General Public Licence version 3 (LGPL) version 3,        *  
 *                  copied verbatim in the file "LICENSE"                       *
 ********************************************************************************/
/*
 * FairMonitor.cxx
 *
 *  Created on: Aug 01, 2015
 *      Author: r.karabowicz
 */

#include "FairMonitor.h"
#include "FairLogger.h"

#include "TArrow.h"
#include "TAxis.h"
#include "TBox.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TLegend.h"
#include "TList.h"
#include "TMath.h"
#include "TNamed.h"
#include "TPaveText.h"
#include "TROOT.h"
#include "TStopwatch.h"
#include "TString.h"
#include "TTask.h"

#include <iomanip>
#include <iostream>
#include <iterator>
#include <utility>

FairMonitor* FairMonitor::instance = NULL;

//_____________________________________________________________________________
FairMonitor::FairMonitor()
  : TNamed("FairMonitor","Monitor for FairRoot")
  , fRunMonitor(kFALSE)
  , fCurrentTask(0)
  , fNoTaskRequired(0)
  , fNoTaskCreated(0)
  , fRunTime(0.)
  , fTimeArray(new Double_t[1000])
  , fTimerHistList(new TList())
  , fTimerList(new TList())
{
}
//_____________________________________________________________________________

//_____________________________________________________________________________
FairMonitor::~FairMonitor()
{
}
//_____________________________________________________________________________

//_____________________________________________________________________________
FairMonitor* FairMonitor::GetMonitor()
{
  if (!instance) {
    instance = new FairMonitor();
  }
  return instance;
}
//_____________________________________________________________________________

//_____________________________________________________________________________
void FairMonitor::StartTimer(const TTask* tTask, const char* identStr) {
  if ( !fRunMonitor ) return;

  TString tempString = Form("hist_%p_%s_%s",tTask,tTask->GetName(),identStr);

  Int_t nofHists = fTimerHistList->GetEntries();
  Int_t ihist = 0;
  for ( ihist = 0 ; ihist < nofHists ; ihist++ ) {
    if ( !tempString.CompareTo(fTimerHistList->At(ihist)->GetName()) ) {
      break;
    }
  }
  if ( ihist == nofHists ) {
    fTimerList->Add(new TStopwatch()); 
    TString titleString = Form("Time evolution for %s, %s",tTask->GetName(),identStr);
    fTimerHistList->Add(new TH1F(tempString,titleString,1000,0,1000));
    fTimeArray[nofHists] = 0.;
  }
  ((TStopwatch*)(fTimerList->At(ihist)))->Start();
}
//_____________________________________________________________________________

//_____________________________________________________________________________
void FairMonitor::StopTimer(const TTask* tTask, const char* identStr) {
  if ( !fRunMonitor ) return;

  TString tempString = Form("hist_%p_%s_%s",tTask,tTask->GetName(),identStr);

  Int_t nofHists = fTimerHistList->GetEntries();
  Int_t ihist = 0;
  for ( ihist = 0 ; ihist < nofHists ; ihist++ ) {
    if ( !tempString.CompareTo(fTimerHistList->At(ihist)->GetName()) ) {
      break;
    }
  }
  if ( ihist == nofHists ) {
    LOG(INFO) << "FairMonitor::StopTimer() called without matching StartTimer()" << FairLogger::endl;
    return;
  }
  ((TStopwatch*)(fTimerList->At(ihist)))->Stop();
  Double_t time = ((TStopwatch*)(fTimerList->At(ihist)))->RealTime();
  TH1F* tempHist = ((TH1F*)(fTimerHistList->At(ihist)));
  Int_t nofEntries = tempHist->GetEntries();
  if ( nofEntries > tempHist->GetNbinsX() ) 
    tempHist->SetBins(tempHist->GetNbinsX()*10, 0, tempHist->GetXaxis()->GetXmax()*10);
  tempHist->SetBinContent(nofEntries+1,time);
  fTimeArray[ihist] += time;
  if ( tempString.EndsWith("_EXEC") )
    fRunTime += time;
}
//_____________________________________________________________________________

//_____________________________________________________________________________
void FairMonitor::RecordRegister(const char* name, const char* folderName, Bool_t toFile) {
  if ( !fRunMonitor ) return;

  LOG(DEBUG) << "*** FM::RecordRegister(" << name << ", " << folderName << (toFile?", kTRUE)":", kFALSE") << " for task >>" << fCurrentTask << "<< (" << (fCurrentTask?fCurrentTask->GetName():"") << ")" << FairLogger::endl;
  if ( fCurrentTask == 0 ) {
    fNoTaskCreated ++; 
    LOG(INFO) << "*** FM::RecordRegister(" << name << ", " << folderName << (toFile?", kTRUE)":", kFALSE") << " WITHOUT TASK" << FairLogger::endl;
    return;
  }
  TString tempString = Form("%p_%s",fCurrentTask,fCurrentTask->GetName());
  if ( toFile ) {
    fTaskCreated.insert(std::pair<TString, TString>(tempString,TString(name)));
  }
  else {
    fTaskCreatedTemp.insert(std::pair<TString, TString>(tempString,TString(name)));
  }
}
//_____________________________________________________________________________

//_____________________________________________________________________________
void FairMonitor::RecordGetting(const char* name) {
  if ( !fRunMonitor ) return;

  LOG(DEBUG) << "*** FM::RecordGetting(" << name << ") for task >>" << fCurrentTask << "<< (" << (fCurrentTask?fCurrentTask->GetName():"") << ")" << FairLogger::endl;

  if ( fCurrentTask == 0 ) {
    fNoTaskRequired ++;
    return;
  }
  TString tempString = Form("%p_%s",fCurrentTask,fCurrentTask->GetName());
  fTaskRequired.insert(std::pair<TString, TString>(tempString,TString(name)));
}
//_____________________________________________________________________________

//_____________________________________________________________________________
void FairMonitor::PrintTask(TTask* tempTask, Int_t taskLevel) {
  if ( !fRunMonitor ) return;

  TString tempString = Form("hist_%p_%s_%s",tempTask,tempTask->GetName(),"EXEC");
  Int_t nofHists = fTimerHistList->GetEntries();
  Int_t ihist = 0;
  Int_t longestName = 0;

  TString printString = " ";
  TString timeString = "";
  Double_t timePerc = 0.;
  Int_t timeFracLog = 0;

  for ( ihist = 0 ; ihist < nofHists ; ihist++ ) {
    if ( longestName < strlen(fTimerHistList->At(ihist)->GetName()) ) 
      longestName = strlen(fTimerHistList->At(ihist)->GetName());
  }
  for ( ihist = 0 ; ihist < nofHists ; ihist++ ) {
    if ( !tempString.CompareTo(fTimerHistList->At(ihist)->GetName()) ) {
      timeString = Form("%f", fTimeArray[ihist]/(((TH1F*)(fTimerHistList->At(ihist)))->GetEntries()));
      for ( Int_t itemp = timeString.Length() ; itemp < 10 ; itemp++ ) 
	printString += ' ';
      printString += timeString;
      printString.Remove(11,100);
      printString += " s/ev |";
      timePerc = 100.*fTimeArray[ihist]/fRunTime;
      if ( timePerc < 100. ) printString += ' ';
      if ( timePerc <  10. ) printString += ' ';
      timeString = Form("%f",timePerc);
      for ( Int_t itemp = timeString.Length() ; itemp < 8 ; itemp++ ) 
	printString += ' ';
      printString += timeString;
      printString.Remove(27,100);
      printString += " % ";
      tempString = printString.Data();
      printString = "] \"";
      for ( Int_t ilev = 0 ; ilev < taskLevel ; ilev++ )
	printString += "  ";
      printString += tempTask->GetName();
      if ( printString.Length() > 80 )
	printString.Remove(80,100);
      printString += "\"";
      timeFracLog = (Int_t)(TMath::Log2(timePerc/100.)/TMath::Log2(0.8));
      timeFracLog = 30-(Int_t)(timePerc/100.*30.);
      //      if ( timeFracLog > 30 ) timeFracLog = 30;

      if ( timePerc < 30 )
	LOG(INFO) << "[\033[42m" << FairLogger::flush;
      else if ( timePerc < 90 )
	LOG(INFO) << "[\033[43m" << FairLogger::flush;
      else
	LOG(INFO) << "[\033[41m" << FairLogger::flush;
      for ( Int_t ilen = 0 ; ilen < tempString.Length()-timeFracLog ; ilen++ ) {
	LOG(INFO) << tempString[ilen] << FairLogger::flush;
      }
      LOG(INFO) << "\033[0m" << FairLogger::flush;
      for ( Int_t ilen = tempString.Length()-timeFracLog ; ilen < tempString.Length() ; ilen++ ) {
	LOG(INFO) << tempString[ilen] << FairLogger::flush;
      }
      LOG(INFO) << printString.Data() << FairLogger::endl;
    }
  }

  TList* subTaskList = tempTask->GetListOfTasks();
  if ( !subTaskList ) return;
  for ( Int_t itask = 0 ; itask < subTaskList->GetEntries() ; itask++ ) {
    TTask* subTask = (TTask*)subTaskList->At(itask);
    if ( subTask ) 
      PrintTask(subTask,taskLevel+1);
  }
}
//_____________________________________________________________________________

//_____________________________________________________________________________
void FairMonitor::Print(TString specString) {
  if ( !fRunMonitor ) {
    LOG(WARNING) << "FairMonitor was disabled. Nothing to print!" << FairLogger::endl;
    return;
  }

  Int_t nofHists = fTimerHistList->GetEntries();
  for ( Int_t ihist = 0 ; ihist < nofHists ; ihist++ ) {
    TString histString = Form("%s",fTimerHistList->At(ihist)->GetName());
    if ( !histString.Contains("EXEC") && histString.Contains(specString) ) {
      LOG(INFO) << histString.Data() << " " << fTimeArray[ihist]/(((TH1F*)(fTimerHistList->At(ihist)))->GetEntries()) << " s/ev" << FairLogger::endl;
    }
  }
}
//_____________________________________________________________________________

//_____________________________________________________________________________
void FairMonitor::Print(Option_t* option) {
  if ( !fRunMonitor ) {
    LOG(WARNING) << "FairMonitor was disabled. Nothing to print!" << FairLogger::endl;
    return;
  }

  LOG(INFO) << "- Total Run Time: " << fRunTime << " s ---------------------------------------------------------" << FairLogger::endl;
  TTask* mainFairTask = (TTask*)(gROOT->GetListOfBrowsables()->FindObject("FairTaskList"));
  if ( mainFairTask ) 
    PrintTask(mainFairTask,0);
  LOG(INFO) << "-------------------------------------------------------------------------------------" << FairLogger::endl;
  
}

//_____________________________________________________________________________

//_____________________________________________________________________________
void FairMonitor::Draw(Option_t* option) {
  if ( !fRunMonitor ) {
    LOG(WARNING) << "FairMonitor was disabled. Nothing to print!" << FairLogger::endl;
    return;
  }

  typedef std::multimap<TString, TString>::iterator bnMapIter;

  typedef std::map<TString, Int_t>::iterator tiMapIter;
  tiMapIter iti;
  
  TTask* mainFairTask = (TTask*)(gROOT->GetListOfBrowsables()->FindObject("FairTaskList"));
  if ( !mainFairTask ) return;

  GetTaskMap(mainFairTask);

  for(bnMapIter itb = fTaskRequired.begin(); itb != fTaskRequired.end() ; itb++) {
    iti = fTaskMap.find(itb->first);
    if ( iti == fTaskMap.end() )
      fTaskMap.insert(std::pair<TString, Int_t> (itb->first,0));
    iti = fObjectMap.find(itb->second);
    if ( iti == fObjectMap.end() )
      fObjectMap.insert(std::pair<TString, Int_t> (itb->second,0));
  }
  for(bnMapIter itb = fTaskCreated.begin(); itb != fTaskCreated.end() ; itb++) {
    iti = fTaskMap.find(itb->first);
    if ( iti == fTaskMap.end() )
      fTaskMap.insert(std::pair<TString, Int_t> (itb->first,0));
    iti = fObjectMap.find(itb->second);
    if ( iti == fObjectMap.end() )
      fObjectMap.insert(std::pair<TString, Int_t> (itb->second,0));
  }
  for(bnMapIter itb = fTaskCreatedTemp.begin(); itb != fTaskCreatedTemp.end() ; itb++) {
    iti = fTaskMap.find(itb->first);
    if ( iti == fTaskMap.end() )
      fTaskMap.insert(std::pair<TString, Int_t> (itb->first,0));
    iti = fObjectMap.find(itb->second);
    if ( iti == fObjectMap.end() )
      fObjectMap.insert(std::pair<TString, Int_t> (itb->second,0));
  }
  
  AnalyzeObjectMap(mainFairTask);

  Int_t maxHierarchyNumber = 0;
  for ( iti = fObjectMap.begin() ; iti != fObjectMap.end() ; iti++ ) {
    if ( maxHierarchyNumber < iti->second ) 
      maxHierarchyNumber = iti->second; 
  }

  LOG(DEBUG) << "Max hierarchy number is " << maxHierarchyNumber << FairLogger::endl;

  TCanvas* tempCanv = new TCanvas("FairMonitor","Fair Monitor",10,10,960,600);
  tempCanv->cd();

  tempCanv->SetFillStyle(4000);
  tempCanv->Range(0,0,960,800);
  tempCanv->SetFillColor(0);
  tempCanv->SetBorderSize(0);
  tempCanv->SetBorderMode(0);
  tempCanv->SetFrameFillColor(0);

  for ( Int_t ihier = 0 ; ihier < maxHierarchyNumber+1 ; ihier++ ) {
    Int_t nofHier = 0;
    for ( iti = fTaskMap.begin() ; iti != fTaskMap.end() ; iti++ ) {
      if ( iti->second == ihier ) {
	nofHier ++;
      }
    }
    LOG(DEBUG) << "There are " << nofHier << " tasks on level " << ihier << "." << FairLogger::endl;

    if ( ihier == 0 ) {
      Double_t iObj = 0.;
      for ( iti = fTaskMap.begin() ; iti != fTaskMap.end() ; iti++ ) {
	if ( iti->second == ihier ) {
	  std::pair<Double_t, Double_t> tempPos(50,600-iObj*40);
	  fTaskPos.insert(std::pair<TString, std::pair<Double_t, Double_t> > (iti->first,tempPos));
	  iObj+=1.;
	}
      }
    }
    else {
      Int_t iObj = 0;
      Int_t secLine = 0;
      Int_t secLineEven = 0;
      if ( nofHier > 9 ) {
	secLine = 1;
	if ( nofHier%2 == 0 ) secLineEven = 1;
      }
      Int_t startingPosition = 575 - nofHier/(1+secLine)*45-secLine*(1-secLineEven)*45;

      Double_t topEdge = 800.-800.*(2.*(Double_t)ihier-0.5)/((Double_t)2*maxHierarchyNumber+1)+secLine*15;
      LOG(DEBUG) << "for level " << ihier << " will put top edge at " << topEdge 
		<< ". " << (secLineEven?"Two lines":"One line") << (secLineEven?" with offset":"") << FairLogger::endl;
      for ( iti = fTaskMap.begin() ; iti != fTaskMap.end() ; iti++ ) {
	if ( iti->second == ihier ) {
	  std::pair<Double_t, Double_t> tempPos(startingPosition+iObj*90/(1+secLine)-secLineEven*(iObj%2)*45,topEdge-15-secLine*(iObj%2)*35);
 	  fTaskPos.insert(std::pair<TString, std::pair<Double_t, Double_t> > (iti->first,tempPos));
	  iObj+=1;
	}
      }
    }

    nofHier = 0;
    for ( iti = fObjectMap.begin() ; iti != fObjectMap.end() ; iti++ ) {
      if ( TMath::Abs(iti->second) == ihier ) {
	nofHier ++;
      }
    }
    LOG(DEBUG) << "There are " << nofHier << " objects on level " << ihier << "." << FairLogger::endl;

    Int_t iObj = 0;
    Int_t secLine = 0;
    Int_t secLineEven = 0;
    if ( nofHier > 9 ) {
      secLine = 1;
      if ( nofHier%2 == 0 ) secLineEven = 1;
    }
    Int_t startingPosition = 575 - nofHier/(1+secLine)*45-secLine*(1-secLineEven)*45;

    Double_t topEdge = 800.-800.*(2.*(Double_t)ihier+0.5)/((Double_t)2*maxHierarchyNumber+1)+secLine*15;
    LOG(DEBUG) << "for level " << ihier << " will put top edge at " << topEdge 
	      << ". " << (secLineEven?"Two lines":"One line") << (secLineEven?" with offset":"") << FairLogger::endl;
    for ( iti = fObjectMap.begin() ; iti != fObjectMap.end() ; iti++ ) {
      if ( TMath::Abs(iti->second) == ihier ) {
	std::pair<Double_t, Double_t> tempPos(startingPosition+iObj*90/(1+secLine)-secLineEven*(iObj%2)*45,topEdge-15-secLine*(iObj%2)*35);
	fObjectPos.insert(std::pair<TString, std::pair<Double_t, Double_t> > (iti->first,tempPos));
	iObj+=1;
      }
    }
    
  }

  typedef std::map<TString, std::pair<Double_t, Double_t> >::iterator tddMapIter;
  tddMapIter itt;
  tddMapIter ito;
  
  for(bnMapIter itb = fTaskRequired.begin(); itb != fTaskRequired.end() ; itb++) {
    itt = fTaskPos.find(itb->first);
    ito = fObjectPos.find(itb->second);
    if ( itt != fTaskPos.end() && ito != fObjectPos.end() ) {
      std::pair<Double_t, Double_t> taskPos = itt->second;
      std::pair<Double_t, Double_t> objectPos = ito->second;
      TArrow* tempArrow = new TArrow(objectPos.first,objectPos.second-15,taskPos.first,taskPos.second+15,0.01,"|>");
      tempArrow->Draw();
    }
  }
  for(bnMapIter itb = fTaskCreated.begin(); itb != fTaskCreated.end() ; itb++) {
    itt = fTaskPos.find(itb->first);
    ito = fObjectPos.find(itb->second);
    if ( itt != fTaskPos.end() && ito != fObjectPos.end() ) {
      std::pair<Double_t, Double_t> taskPos = itt->second;
      std::pair<Double_t, Double_t> objectPos = ito->second;
      TArrow* tempArrow = new TArrow(taskPos.first,taskPos.second-15,objectPos.first,objectPos.second+15,0.01,"|>");
      tempArrow->Draw();
    }
  }
  for(bnMapIter itb = fTaskCreatedTemp.begin(); itb != fTaskCreatedTemp.end() ; itb++) {
    itt = fTaskPos.find(itb->first);
    ito = fObjectPos.find(itb->second);
    if ( itt != fTaskPos.end() && ito != fObjectPos.end() ) {
      std::pair<Double_t, Double_t> taskPos = itt->second;
      std::pair<Double_t, Double_t> objectPos = ito->second;
      TArrow* tempArrow = new TArrow(taskPos.first,taskPos.second-15,objectPos.first,objectPos.second+15,0.01,"|>");
      tempArrow->Draw();
    }
  }

  for ( itt = fTaskPos.begin() ; itt != fTaskPos.end() ; itt++ ) {
    std::pair<Double_t, Double_t> taskPos = itt->second;
    TBox* bkgBox = new TBox(taskPos.first-40,taskPos.second-15,taskPos.first+40,taskPos.second+15);
    bkgBox->SetFillColor(kGreen-9);
    bkgBox->Draw();

    TString tempString = Form("hist_%s_%s",itt->first.Data(),"EXEC");
    Int_t nofHists = fTimerHistList->GetEntries();
    for ( Int_t ihist = 0 ; ihist < nofHists ; ihist++ ) {
      if ( !tempString.CompareTo(fTimerHistList->At(ihist)->GetName()) ) {
	Double_t timeFrac = 80.*fTimeArray[ihist]/fRunTime;
	TBox* barBox = new TBox(taskPos.first-40,taskPos.second-15,taskPos.first-40+timeFrac,taskPos.second+15);
	barBox->SetFillColor(kRed);
	barBox->Draw();
      }
    }

    TPaveText* paveText = new TPaveText(taskPos.first-40,taskPos.second-15,taskPos.first+40,taskPos.second+15,"nb");
    paveText->SetFillColorAlpha(kWhite,0.);//4000);
    paveText->SetShadowColor(0);
    paveText->SetTextSizePixels(10);
    tempString = itt->first;
    tempString.Replace(0,tempString.First('_')+1,"");
    if ( tempString.Length() > 16 ) tempString.Replace(16,tempString.Length(),"");
    paveText->AddText(tempString);
    paveText->Draw();
  }

  for ( ito = fObjectPos.begin() ; ito != fObjectPos.end() ; ito++ ) {
    std::pair<Double_t, Double_t> objectPos = ito->second;
    iti = fObjectMap.find(ito->first);
    TPaveText* paveText = new TPaveText(objectPos.first-40,objectPos.second-15,objectPos.first+40,objectPos.second+15,"nb");
    paveText->SetFillColor(kMagenta-9);
    if ( iti != fObjectMap.end() )
      if ( iti->second < 0 ) 
	paveText->SetFillColor(kGray);
    paveText->SetShadowColor(0);
    paveText->SetTextSizePixels(10);
    TString tempString = ito->first;
    if ( tempString.Length() > 16 ) tempString.Replace(16,tempString.Length(),"");
    paveText->AddText(tempString);
    paveText->Draw();
  }


}
//_____________________________________________________________________________

//_____________________________________________________________________________
void FairMonitor::DrawHist(TString specString) {
  if ( !fRunMonitor ) {
    LOG(WARNING) << "FairMonitor was disabled. Nothing to draw!" << FairLogger::endl;
    return;
  }

  Int_t nofHists = fTimerHistList->GetEntries();
  Int_t nofDraws = 0;
  TString drawStr = "P";

  TLegend* tempLeg = new TLegend(0.0,0.5,0.5,0.9);

  Double_t histMax = 0.;
  for ( Int_t ihist = 0 ; ihist < nofHists ; ihist++ ) {
    TString histString = Form("%s",fTimerHistList->At(ihist)->GetName());
    if ( histString.Contains(specString) ) {
      TH1F* tempHist = ((TH1F*)(fTimerHistList->At(ihist)));
      if ( histMax < tempHist->GetMaximum() )
	histMax = tempHist->GetMaximum();
    }
  }

  for ( Int_t ihist = 0 ; ihist < nofHists ; ihist++ ) {
    TString histString = Form("%s",fTimerHistList->At(ihist)->GetName());
    if ( histString.Contains(specString) ) {
      TH1F* tempHist = ((TH1F*)(fTimerHistList->At(ihist)));
      if ( tempHist->GetXaxis()->GetXmax() > tempHist->GetEntries() )
	tempHist->SetBins(tempHist->GetEntries(),0,tempHist->GetEntries());
      tempHist->SetMarkerColor(nofDraws%6+2);
      tempHist->SetMarkerStyle(nofDraws%4+20);
      tempHist->SetAxisRange(0.,histMax*1.1,"Y");
      tempHist->Draw(drawStr);
      TString tempName = tempHist->GetName();
      tempName.Remove(0,tempName.First('_')+1);
      tempName.Remove(0,tempName.First('_')+1);
      tempLeg->AddEntry(tempHist,tempName,"p");
      nofDraws++;
      drawStr = "Psame";
    }
  }
  if ( nofDraws > 1 ) 
    tempLeg->Draw();
}
//_____________________________________________________________________________

//_Private function to fill the map of the tasks_______________________________
void FairMonitor::GetTaskMap(TTask* tempTask) {
  TString tempString = Form("%p_%s",tempTask,tempTask->GetName());
  fTaskMap.insert(std::pair<TString, Int_t> (tempString,0));

  TList* subTaskList = tempTask->GetListOfTasks();
  if ( !subTaskList ) return;
  for ( Int_t itask = 0 ; itask < subTaskList->GetEntries() ; itask++ ) {
    TTask* subTask = (TTask*)subTaskList->At(itask);
    if ( subTask ) 
      GetTaskMap(subTask);
  }
}
//_____________________________________________________________________________

//_Private function to analyze the object list:________________________________
// assign to each of the tasks and objects (TClonesArray, pressumably)
// one number that places them in a hierarchy
// objects number 0 are the ones in the input array
// tasks get number from the abs(highest number object they read) increased by one
// tasks number i create objects number i (if they go to output file) or
// tasks number i create objects number -i (if they are not persistent)
void FairMonitor::AnalyzeObjectMap(TTask* tempTask) {
  TString tempString = Form("%p_%s",tempTask,tempTask->GetName());

  Int_t hierarchyNumber = 0;
  
  typedef std::multimap<TString, TString>::iterator bnMapIter;
  typedef std::map<TString, Int_t>::iterator tiMapIter;
  tiMapIter iti;

  LOG(DEBUG) << "TASK \"" << tempTask->GetName() << "\" NEEDS:" << FairLogger::endl;
  for(bnMapIter itb = fTaskRequired.begin(); itb != fTaskRequired.end() ; itb++) {
    if ( itb->first != tempString ) continue;
    LOG(DEBUG) << "   \"" << itb->second.Data() << "\"" << FairLogger::endl;
    iti = fObjectMap.find(itb->second);
    if ( iti == fObjectMap.end() ) continue;
    if ( hierarchyNumber <= TMath::Abs(iti->second) )
      hierarchyNumber = TMath::Abs(iti->second)+1;
  }

  //  hierarchyNumber++;

  LOG(DEBUG) << "WILL GET hierarchyNumber = " << hierarchyNumber << FairLogger::endl;

  iti = fTaskMap.find(tempString);
  if ( iti != fTaskMap.end() ) 
    iti->second = hierarchyNumber;

  LOG(DEBUG) << "     \"" << tempTask->GetName() << "\" CREATES:" << FairLogger::endl;
  for(bnMapIter itb = fTaskCreated.begin(); itb != fTaskCreated.end() ; itb++) {
    if ( itb->first != tempString ) continue;
    LOG(DEBUG) << " + \"" << itb->second.Data() << "\"" << FairLogger::endl;
    iti = fObjectMap.find(itb->second);
    if ( iti == fObjectMap.end() ) continue;
    iti->second = hierarchyNumber;
  }

  for(bnMapIter itb = fTaskCreatedTemp.begin(); itb != fTaskCreatedTemp.end() ; itb++) {
    if ( itb->first != tempString ) continue;
    LOG(DEBUG) << " - \"" << itb->second.Data() << "\"" << FairLogger::endl;
    iti = fObjectMap.find(itb->second);
    if ( iti == fObjectMap.end() ) continue;
    iti->second = -hierarchyNumber;
  }


  TList* subTaskList = tempTask->GetListOfTasks();
  if ( !subTaskList ) return;
  for ( Int_t itask = 0 ; itask < subTaskList->GetEntries() ; itask++ ) {
    TTask* subTask = (TTask*)subTaskList->At(itask);
    if ( subTask ) 
      AnalyzeObjectMap(subTask);
  }

}
//_____________________________________________________________________________


ClassImp(FairMonitor)

