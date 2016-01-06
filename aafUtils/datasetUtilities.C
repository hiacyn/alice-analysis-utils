#if !defined(__CINT__) || defined(__MAKECINT__)

#include <Riostream.h>

// ROOT includes
#include "TString.h"
#include "TSystem.h"
#include "TArrayI.h"

#include "TProof.h" // FIXME: see later
#endif

//______________________________________________________________________________
TString GetRunNumber ( TString queryString )
{
  TString found = "";
  for ( Int_t ndigits=9; ndigits>=6; ndigits-- ) {
    TString sre = "";
    for ( Int_t idigit=0;idigit<ndigits; idigit++ ) sre += "[0-9]";
    found = queryString(TRegexp(sre.Data()));
    if ( ! found.IsNull() ) break;
  }
  return found;
}

//______________________________________________________________________________
void CheckOutputFile ( TString filename, Int_t nRuns )
{
  ifstream inFile(filename.Data());
  if ( ! inFile.is_open() ) {
    printf("Error: cannot open %s\n",filename.Data());
    return;
  }
  Int_t nFull=0, nEmpty=0, nPartial=0;
  TString currLine;
  while ( ! inFile.eof() ) {
    currLine.ReadLine(inFile);
    if ( currLine.Contains("Staged") ) {
      if ( currLine.Contains("100.0 %") ) nFull++;
      else if ( currLine.Contains(" 0.0 %" ) ) nEmpty++;
      else nPartial++;
    }
  }
  inFile.close();
  printf("\nTotal runs %i (expected %i). Full %i  Empty %i  Partial %i\n",nFull+nEmpty+nPartial,nRuns,nFull,nEmpty,nPartial);
}


//______________________________________________________________________________
void checkAafStaging ( TString inFilename, TString searchString = "%s", TString outFilename = "/tmp/aafStagingOut.txt", TString aaf = "dstocco@nansafmaster2.in2p3.fr", Bool_t forceUpdate = kTRUE )
{
  // If inFilename is a list of run, a search string must be provided so that the dataset is built on the fly
  // e.g.: Find;BasePath=/alice/data/2015/LHC15o/000%s/muon_calo_pass1/AOD/;FileName=AliAOD.Muons.root;"
  gSystem->ExpandPathName(inFilename);

  Bool_t isTmp = kFALSE;
  if ( gSystem->AccessPathName(inFilename) ) {
    isTmp = kTRUE;
    inFilename.ReplaceAll(","," ");
    TObjArray* arr = inFilename.Tokenize(" ");
    inFilename = "tmp_list.txt";
    ofstream outFile(inFilename);
    TIter next(arr);
    TObjString* str = 0x0;
    while ( (str = static_cast<TObjString*>(next())) ) {
      outFile << str->GetName() << endl;
    }
    outFile.close();
    delete arr;
  }

  TProof::Open(aaf.Data(),"masteronly");
  if ( forceUpdate && ! searchString.Contains("ForceUpdate") ) {
    searchString.Append(";ForceUpdate");
    searchString.ReplaceAll(";;",";");
  }

  TList inputList;
  inputList.SetOwner();
  ifstream inFile(inFilename.Data());
  TString currLine = "";
  Int_t nRuns = 0;
  while ( ! inFile.eof() ) {
    currLine.ReadLine(inFile);
    if ( currLine.IsNull() ) continue;
    inputList.Add(new TObjString(currLine));
    nRuns++;
  }
  inFile.close();

  if ( isTmp ) gSystem->Exec(Form("rm %s",inFilename.Data()));

//  freopen (outFilename.Data(),"w",stdout);
//  int backup, newstream;
  fflush(stdout);
  int backup = dup(1);
  int newstream = open(outFilename.Data(), O_WRONLY|O_TRUNC);
  dup2(newstream, 1);
  close(newstream);

//  std::ofstream out(outFilename.Data());
//  std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
//  std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

  TIter next(&inputList);
  TObjString* str = 0x0;
  while ( (str = static_cast<TObjString*>(next())) ) {
//    cout << Form(searchString.Data(),str->GetName()) << endl; continue; // REMEMBER TO CUT
    gProof->ShowDataSet(Form(searchString.Data(),str->GetName()));
  }
  

//  fclose(stdout);
  fflush(stdout);
  dup2(backup, 1);
  close(backup);

//  std::cout.rdbuf(coutbuf); //reset to standard output again

  CheckOutputFile(outFilename,nRuns);
  
}


//______________________________________________________________________________
void runNumberToDataset ( TString runListFilename, TString searchString, TString outputDatasetName = "dataset.txt" )
{
  gSystem->ExpandPathName(runListFilename);
  if ( gSystem->AccessPathName(runListFilename) ) {
    printf("Error: cannot find %s\n",runListFilename.Data());
  }

  ofstream outFile(outputDatasetName);
  ifstream inFile(runListFilename.Data());
  TString currLine = "";
  while ( ! inFile.eof() ) {
    currLine.ReadLine(inFile);
    TString runNum = GetRunNumber(currLine);
    if ( ! runNum.IsNull() ) outFile << Form(searchString,runNum.Data()) << endl;
  }
  inFile.close();
  outFile.close();
}

//______________________________________________________________________________
void datasetToRunNumber ( TString datasetFilename, TString outputRunListName = "runList.txt" )
{
  gSystem->ExpandPathName(datasetFilename);
  if ( gSystem->AccessPathName(datasetFilename) ) {
    printf("Error: cannot find %s\n",datasetFilename.Data());
  }

  ofstream outFile(outputRunListName);
  ifstream inFile(datasetFilename.Data());
  TString currLine = "";
  while ( ! inFile.eof() ) {
    currLine.ReadLine(inFile);
    TString runNum = GetRunNumber(currLine);
    if ( ! runNum.IsNull() ) outFile << runNum.Atoi() << endl;
  }
  inFile.close();
  outFile.close();
}





