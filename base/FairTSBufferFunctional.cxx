#include "FairTSBufferFunctional.h"

#include "FairRootManager.h"
#include "FairTimeStamp.h"

ClassImp(FairTSBufferFunctional);


FairTSBufferFunctional::FairTSBufferFunctional(TString branchName, TTree* sourceTree, BinaryFunctor* function)
  :TObject(),
   fOutputArray(NULL),
   fInputArray(NULL),
   fBufferArray(NULL),
   fBranch(NULL),
   fBranchIndex(-1),
   fFunction (function),
   fVerbose(2)
{
  fBranch = sourceTree->GetBranch(branchName.Data());
  if (fBranch == 0) {
    std::cout << "-E- FairTSBufferFunctional::FairTSBufferFunctional Branch " << branchName << " does not exist!" << std::endl;
  }
  FairRootManager* ioman = FairRootManager::Instance();
  fInputArray = (TClonesArray*)ioman->GetObject(branchName.Data());
  fBufferArray = new TClonesArray(fInputArray->GetClass()->GetName());
  fOutputArray = new TClonesArray(fInputArray->GetClass()->GetName());
}

TClonesArray* FairTSBufferFunctional::GetData(Double_t parameter)
{
#if ROOT_VERSION_CODE >= ROOT_VERSION(5,29,1)

  Double_t actualTime = 0.;
  int posBuffer = 0;

  if (fFunction == 0) {                 //no function is given ==> read in data in traditional way event by event
    ReadInNextEntry();
    fOutputArray->AbsorbObjects((TClonesArray*)fInputArray, 0, fInputArray->GetEntriesFast() - 1);
    return fOutputArray;
  }

  std::cout << "-I- FairTSBufferFunctional::GetData for parameter: " << parameter << std::endl;

  //if the BufferArray is empty fill it
  if (fBufferArray->GetEntriesFast() == 0) {
    std::cout << "-I- FairTSBufferFunctional::GetData fBufferArray is empty: Read in Data" << std::endl;
    ReadInNextFilledEntry();
  }

  //if the BufferArray is still empty you have reached the end of your data set
  std::cout << "fBufferArray->GetEntriesFast(): " << fBufferArray->GetEntriesFast() << std::endl;
  FairTimeStamp* dataPoint = (FairTimeStamp*)fBufferArray->Last();
  if (dataPoint == 0) {
    std::cout << "-I- FairTSBufferFunctional::GetData dataPoint is empty ==> All Data read in" << std::endl;
    return fOutputArray;
  }

  dataPoint = (FairTimeStamp*)fBufferArray->First();

  while (!(*fFunction)(dataPoint, parameter)) {     //check if you have reached end of requested data
    posBuffer++;
    //if you have reached the end of the BufferArray fill it with new data from tree
    if (posBuffer == fBufferArray->GetEntriesFast()) {
      ReadInNextFilledEntry();
    }
    //if you are still at the end of the BufferArray than break (no new data in tree)
    if (posBuffer == fBufferArray->GetEntriesFast()) {
      break;
    }
    dataPoint = (FairTimeStamp*)fBufferArray->At(posBuffer);
    std::cout << posBuffer << " TimeStampData: " << dataPoint->GetTimeStamp() << std::endl;
  }

  std::cout << "-I- FairTSBufferFunctional::GetData Index for Absorb: " << posBuffer << " BufferArray size: " << fBufferArray->GetEntriesFast() << std::endl;
  if (posBuffer < fBufferArray->GetEntriesFast() && posBuffer > 0) {
    std::cout << "-I- FairTSBufferFunctional::GetData absorb BufferArray up to posBuffer " << posBuffer << " into fOutputArray" << std::endl;
    fOutputArray->AbsorbObjects(fBufferArray, 0, posBuffer - 1);
    posBuffer = 0;
    return fOutputArray;
  }

  std::cout << "Index: " << posBuffer << " BranchIndex: " << fBranchIndex << " NBranch " << fBranch->GetEntries() << std::endl;

  if (posBuffer >= fBufferArray->GetEntriesFast() && posBuffer != 0 && fBranchIndex + 1 >= fBranch->GetEntries()) {
    std::cout << "-I- FairTSBufferFunctional::GetData end of data reached. Send the rest to the OutputArray!" << std::endl;
    fOutputArray->AbsorbObjects(fBufferArray, 0, fBufferArray->GetEntries() - 1);
  }

  if (fVerbose > 1) {
    std::cout << "-I- FairTSBufferFunctional::GetData: Read in up to entry: " << fBranchIndex << " with actualTime " << actualTime << " and requested parameter " << parameter << std::endl;
  }
#endif
  return fOutputArray;
}


void FairTSBufferFunctional::ReadInNextFilledEntry()
{
#if ROOT_VERSION_CODE >= ROOT_VERSION(5,29,1)
  fInputArray->Delete();

  std::cout << "-I- FairTSBufferFunctional::ReadInNextFilledEntry: Entries in InputArray " << fInputArray->GetEntriesFast() << " Branch Entries: " << fBranch->GetEntries() << std::endl;
  while (fInputArray->GetEntriesFast() == 0 && fBranchIndex + 1 < fBranch->GetEntries()) {
    fBranchIndex++;
    fBranch->GetEntry(fBranchIndex);
    std::cout << "-I- FairTSBufferFunctional::ReadInNextFilledEntry BranchIndex: " << fBranchIndex << " Entries: " << fInputArray->GetEntriesFast() << std::endl;

  }
  if (fInputArray->GetEntriesFast() > 0) {
    std::cout << "-I- FairTSBufferFunctional::ReadInNextFilledEntry: Absorb InputArray into Buffer" << std::endl;
    fBufferArray->AbsorbObjects(fInputArray, 0, fInputArray->GetEntries() - 1);
  }
#endif
}

void FairTSBufferFunctional::ReadInNextEntry()
{
  fInputArray->Delete();
  if (fBranchIndex + 1 < fBranch->GetEntries()) {
    fBranchIndex++;
    fBranch->GetEntry(fBranchIndex);
    std::cout << "-I- FairTSBufferFunctional::ReadInNexEntry BranchIndex: " << fBranchIndex << " Entries: " << fInputArray->GetEntriesFast() << std::endl;

  }
}

Bool_t FairTSBufferFunctional::AllDataProcessed()
{
  if (fBranchIndex + 1 >= fBranch->GetEntries()) {
    if(fBufferArray->GetEntriesFast() == 0) {
      if (fOutputArray->GetEntriesFast() == 0) {
        return kTRUE;
      } else { return kFALSE; }
    } else { return kFALSE; }
  } else { return kFALSE; }
}