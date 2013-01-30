///////////////////////////////////////////////////////////////////////
//
//    FILE: DatasetManager.c
//   CLASS: DatasetManager
//  AUTHOR: I. Gonzalez Caballero
//    DATE: January, 2011
//
///////////////////////////////////////////////////////////////////////

#include "DatasetManager.h"

#include "TSystem.h"
#include "TObjArray.h"

#include <iostream>
#include <fstream>

#include <time.h> //For time types and functions

using namespace std;

///////////////////////////////////////////////
//
// A couple of constants
//
// + Time between redownloads in seconds
const time_t MAXREDOWNLOADTIME = 86400;
// + N. of times DatasetManager tries to find files
const unsigned int MAXTRIALS = 5; 


// Uncomment the line below to get debug information.
// Set DEBUGDS to to for more debug information
//#define DEBUGDS 1

#ifdef DEBUG
#define DEBUGDS DEBUG
#endif

//////////////
// Constructors
//
// Main constructor. It sets the basic parameters.
// * localBasePath : Local path to data (defaults to /hadoop)
// * skim: Optional. Fixes the skim used for the data.
// * folder: Is the folder (i.e. tab) used to search for the samples 
//   (ex. MC_Fall10)
// * forcereload: If set to true the information from the google doc page
//   is redownloaded 
DatasetManager::DatasetManager(const char* localBasePath, 
			       const char* skim,
			       const char* tab,
			       bool forcereload) 
{
	Init(localBasePath, tab, skim, forcereload);
}


//Intelligent constructor (guesses basepath) for datasets with skim
// * tab: Is the tab used to search for the samples (ex. MC_Fall10)
// * skim: Optional. Fixes the skim used for the data.
// * forcereload: If set to true the information from the google doc page
//   is redownloaded 
DatasetManager::DatasetManager(const char* tab, const char* skim, 
			       bool forcereload) 
{
	Init(0, tab, skim, forcereload);
}
 
//Intelligent constructor (guesses basepath) for datasets with no skim
//(ex. TESCO Trees)
// * tab: Is the tab used to search for the samples (ex. MC_Fall10)
// * forcereload: If set to true the information from the google doc page
//   is redownloaded 
DatasetManager::DatasetManager(const char* tab, bool forcereload) 
{
	// Use . as the skim so the search is not done in a new folder
	Init(0, tab, ".", forcereload);
}

//////////////
// Sets the folder/tab to use to search for the dataset. Downloads the
// information if needed.
//
bool DatasetManager::SetFolder(const char* folder, bool redownload) 
{ 
	//Replace spaces by _
	fFolder = folder;
	fFolder.ReplaceAll(" ", "_");
	
	//Clear files
       fFiles.clear();
       
       //Check if the tabs.dat is already there and new enough
       Long_t id = 0;
       Long_t size = 0;
       Long_t flag = 0;
       Long_t modtime = 0; //Mod time in seconds
       if (gSystem->GetPathInfo("Datasets/tabs.dat", 
			   &id, &size, &flag, &modtime) == 1) 
       {
	       cerr << "NOTE: Could not find Datasets/tabs.dat" <<  endl
		       << "      We will retrieve it." << endl;
	       if (!DownloadAvailableFolders())
	       {
		       return false;
	       }
       }
       else if ((time(NULL) - modtime) > MAXREDOWNLOADTIME) 
       {
	       cerr << "NOTE: Datasets/tabs.dat is older than 24 hours" 
		       <<  endl
		       << "      We will redownload it." << endl;
	       if (!DownloadAvailableFolders())
	       {
		       return false;
	       }
       }


       //Check if folder.dat exists and is new enough
       TString file("Datasets/");
       file+=fFolder;
       file += ".dat";
       Long_t fmodtime = 0; //Mod time in seconds
       if (!redownload) 
       {
	       if (gSystem->GetPathInfo(file, 
				       &id, &size, &flag, &fmodtime) == 1) 
	       {
		       cerr << "NOTE: Could not find " << file <<  endl
			       << "      We will retrieve it." << endl;
	       }
	       else if ((time(NULL) - fmodtime) > MAXREDOWNLOADTIME) 
	       {
		       cerr << "NOTE: " << file << " is older than 24 hours" 
			       <<  endl
			       << "      We will redownload it." << endl;
	       }
	       else if (modtime > fmodtime) 
	       {
		       cerr << "NOTE: " << file << " is newer than Datasets/tabs.dat" 
			       <<  endl
			       << "      We will redownload the first one." << endl;
	       }
	       else 
	       {
		       return true;
	       }
       }
       
       if (!DownloadFolder(fFolder)) 
       {
	       cerr << "ERROR: Could not download folder " << fFolder << endl;
	       fFolder = "";
	       return false;
       }
       
       return true;
}



//////////////
// Load Dataset (datasetname) information. Optionally a folder and/or a skim
// can be provided overridding the current ones. 
bool DatasetManager::LoadDataset(const char* datasetname, 
				 const char* skim,
				 const char* folder) {
#ifdef DEBUGDS
  cout << "DEBUG: Loading dataset " << datasetname;
  if (folder)
    cout << " in new folder \"" << folder << "\"";
  else if (fFolder)
    cout << " in old folder \"" << fFolder << "\"";
  else
    cout << " in NO folder!!!";
  
  cout << " ..." << endl;
#endif
#if (DEBUGDS >= 2)
  cout << "DEBUG: + fLocalBasePath = \"" << fLocalBasePath << "\"" << endl;
  cout << "DEBUG: + fSkim = \"" << fSkim << "\"" << endl;
  cout << "DEBUG: + fFolder = \"" << fFolder << "\"" << endl;
#endif

  //Remove information from previous dataset
  fFiles.clear();

  //Check if folder and skim are passed as arguments
  if (skim) {
    fSkim = skim;
#ifdef DEBUGDS
    cout << "DEBUG: New skim \"" << fSkim << "\"" << endl;
#endif
  }
  if (folder)
    SetFolder(folder);


  if (fSkim == "") {
    cerr << "ERROR: Skim is not set in DatasetManager!" << endl;
    return false;
  }
  
  if (fFolder == "") {
    cerr << "ERROR: Folder is not set in DatasetManager!" << endl;
    return false;
  }
  
  //Build file names
  TString file("Datasets/");
  file+=fFolder;
  file += ".dat";
  TString tmpfile = file + "tmp";
  
  //Grep to remove commentaries
  TString command("grep -v \\# ");
  command+= file;
  command+= " > ";
  command+= tmpfile;
  
#ifdef DEBUGDS
  cout << "DEBUG: Executing " << command << endl;
#endif
  gSystem->Exec(command);
  
  //Read file
#ifdef DEBUGDS
  cout << "DEBUG: Opening file " << tmpfile << endl;
#endif
  ifstream is(tmpfile);

  TString tmpdataset;
  char tmpline[256];
  while (is) {
    is >> tmpdataset;
#if (DEBUGDS >= 2)
    cout << "DEBUG: + Found dataset = \"" << tmpdataset << "\"" << endl;
#endif
    if (tmpdataset != datasetname)
      is.getline(tmpline, 255);
    else {
#if (DEBUGDS >= 2)
    cout << "DEBUG:   - Reading..." << endl;
#endif
      fDataset = datasetname;
      is >> fEventsInTheSample;
      is >> fCrossSection;
      is >> fLocalFolder;
      is >> fBaseFileName;
      //Delete temporary file
      gSystem->Unlink(tmpfile);
#if (DEBUGDS >= 2)
    cout << "DEBUG:   - Deleting temporary file..." << endl;
#endif
      return true;
    }
  }

  return false;
}

///////////
// Returns a vector with the full path to the files in the dataset
const vector<TString>& DatasetManager::GetFiles() {
  //Check if the list of files has been built. Otherwise construct it
  unsigned int trials = 0;
  while (fFiles.size() == 0 && trials < MAXTRIALS) {
    //Check if file exists
    TString fullpath = fLocalBasePath + "/" + fLocalFolder + "/" + fSkim;
    TString basefile = fBaseFileName;
#ifdef DEBUGDS
    cout << "DEBUG: basefile = " << basefile << endl;
    cout << "DEBUG: fBaseFileName = " << fBaseFileName << endl;
    cout << "DEBUG: fullpath = " << fullpath.Data() << endl;
#endif

    //Find files if the sample was split in several pieces
    // NOTE: The command gSystem->FindFile screws the variables passed.
    //       I need to set them again.
    basefile = fBaseFileName;
    TString command("ls ");
    command += 
      fullpath + "/" + basefile + " " +
      fullpath + "/" + basefile(0, basefile.Sizeof() - 6) + "_[0-9].root " +
      fullpath + "/" + basefile(0, basefile.Sizeof() - 6) + "_[0-9][0-9].root";
#ifdef DEBUGDS
    cout << "DEBUG: Executing command " << command << endl;
#else
    command += " 2> /dev/null";
#endif
    //We cannot use GetFromPipe because it is too verbose, so we implement
    //the full code
    //    TString result=gSystem->GetFromPipe(command);
    TString result;
    FILE *pipe = gSystem->OpenPipe(command, "r");
    if (!pipe) {
      cerr << "ERROR: in DatasetManager::GetFiles. Cannot run command \""
	   << command << "\"" << endl;
    }
    else {
      TString line;
      while (line.Gets(pipe)) {
	if (result != "")
	  result += "\n";
	result += line;
      }

      gSystem->ClosePipe(pipe);
    }
   
#ifdef DEBUGDS
    cout << "DEBUG: Result is " << result << endl;
#endif
    if (result != "" ) {
      TObjArray* filesfound = result.Tokenize(TString('\n'));
      if (!filesfound)
	cerr << "ERROR: Could not parse output while finding files" << endl;
      else {
	for (int i = 0; i < filesfound->GetEntries(); i++) {
	  fFiles.push_back(filesfound->At(i)->GetName());
	}
	filesfound->Clear();
	delete filesfound;
      }
    }

    // Increment the counter on the number of trials
    trials++;
    if (fFiles.size() == 0 && trials == 5)
      cerr << "ERROR: Could not find files in /" << fLocalBasePath
	   << "/" << fLocalFolder << "/" << fSkim << endl
	   << "       of the form " << fBaseFileName << endl;
  }


  return fFiles;
}





///////////
// Dumps all the information
void DatasetManager::Dump() const {
  cout << "Dataset Name: " << fDataset << endl;
  cout << "N. Events: " << fEventsInTheSample << endl;
  cout << "Cross Section: " << fCrossSection << endl;
  cout << "Base File Name: " << fBaseFileName << endl;
  cout << "Files:" << endl;
  for (unsigned int i = 0; i < fFiles.size(); i++)
    cout << " + " << fFiles[i] << endl;
}





///////////
//Download the information on the folder/tabs available from Google Docs
bool DatasetManager::DownloadAvailableFolders() {
  //Write script
  TString scriptname = WriteGetDatatabsScript();

  //Execute script
  int ret = gSystem->Exec(scriptname);

  //Delete script
  gSystem->Unlink(scriptname);
  
  TString tabsdat("tabs.dat");
  if (!gSystem->FindFile("Datasets", tabsdat)) {
    cerr << "ERROR: Could not find the available folders from the Google Docs spreadsheet!" << endl;
    ret = -1;
  }

  return (!ret);
}


///////////
// Redownload all the files
bool DatasetManager::DownloadFolder(const char* folder) {
  //Write script
  //cout << "DEB: Writting..." << endl;
  TString scriptname = WriteGetDatasetsScript();

  //Execute script
  //cout << "DEB: Executing " << scriptname << " " << folder << "..." << endl;
  int ret = gSystem->Exec(scriptname + " " + folder);

  //Delete script
  gSystem->Unlink(scriptname);

  TString dsdat(folder);
  dsdat+=".dat";
  if (!gSystem->FindFile("Datasets", dsdat)) {
    cerr << "ERROR: Could not create Datasets/" << folder << ".dat!" << endl;
    ret = -1;
  }

  //cout << "DEB: ret=" << ret << endl;
  return (!ret);
}





///////////
// This method writes the script that downloads and processes the information
// on the available datasets from Google Docs
TString DatasetManager::WriteGetDatatabsScript() {
  //Build file name with User ID in the /tmp directory
  TString scriptname="/tmp/";
  scriptname+=gSystem->GetUid();
  scriptname+="-gettabs.sh";

  //Open file
  ofstream os(scriptname);



  os << "#!/bin/bash" << endl;
  os << "#Some configuration values" << endl;
  os << "tabsurl=\"http://spreadsheets.google.com/feeds/worksheets/t2jDBQxd2cmplDeuQReRGlQ/public/values\"" << endl;
  os << "tabsxml=\"/tmp/values.xml\"" << endl;
  os << "tmpfiles=\"$tabsxml\"" << endl;
  os << "" << endl;
  os << "#Remove temp files if the script is stoped by some reason" << endl;
  os << "trap \"rm -f $tmpfiles; exit\" INT TERM EXIT" << endl;
  os << "" << endl;

  os << "#Download the information on the available tabs" << endl;
  os << "curl \"$tabsurl\" 2> /dev/null | xmllint --format - > $tabsxml" << endl;
  os << "" << endl;
  os << "#Build directories if they don't exit" << endl;
  os << "pathtodatfiles=\"./Datasets\"" << endl;
  os << "if [ ! -d \"$pathtodatfiles\" ]; then" << endl;
  os << "    echo \">> Creating $pathtodatfiles to store the information of the datasets\"" << endl;
  os << "    mkdir $pathtodatfiles" << endl;
  os << "fi" << endl;
  os << "" << endl;
  os << "#Output file" << endl;
  os << "outputfile=\"$pathtodatfiles/tabs.dat\"" << endl;
  os << "" << endl;
  os << "#Compute the number of tabs" << endl;
  os << "#echo DEB:tabsxml=$tabsxml" << endl;  
  os << "#ls -l $tabsxml" << endl;
  os << "ntabs=`grep \"<entry>\" $tabsxml  | wc -l`" << endl;
  os << "#echo \"DEB: NTABS = $ntabs\"" << endl;
  os << "" << endl;
  os << "#Get the titles and the links in some array" << endl;
  os << "tabtitles=( `grep \"<entry>\" -A 4 $tabsxml| grep title | cut -d \\> -f 2| cut -d \\< -f 1 | sed 's\\ \\_\\g'` )" << endl;
  os << "tablinks=( `grep \"<entry>\" -A 7 $tabsxml| grep \"<link \" | grep cells | cut -d \\\" -f 6` )" << endl;
  os << "" << endl;
  os << "#Store output in $outputfile" << endl;
  os << "rm -f $outputfile" << endl;
  os << "touch $outputfile" << endl;
  os << "for ((i=0;i<$ntabs;i++)); do" << endl;
  os << "   #echo -e \"DEB: [$i] ${tabtitles[${i}]}\\t${tablinks[${i}]}\"" << endl;
  os << "   echo -e \" ${tabtitles[${i}]} ${tablinks[${i}]}\" >> $outputfile" << endl;
  os << "done" << endl;


  //Close file
  os.close();

  //Set apropiate permisions
  gSystem->Chmod(scriptname, 448/*700 in decimal = rwx------*/);

  //Return file name
  return scriptname;

}



///////////
// Writes the scripts that retrieves the dataset information for a given 
// folder/tab
TString DatasetManager::WriteGetDatasetsScript() {
  //Build file name with User ID in the /tmp directory
  TString scriptname="/tmp/";
  scriptname+=gSystem->GetUid();
  scriptname+="-getdatasets.sh";

  //Open file
  ofstream os(scriptname);
  
  //Write script

  os << "#!/bin/bash" << endl;
  os << "#Some configuration values" << endl;
  os << "xmlfile=/tmp/$UID-datasets.xml" << endl;
  os << "formatedfile=/tmp/$UID-datasets.html" << endl;
  os << "tmpfiles=\"$xmlfile $formatedfile\"" << endl;
  os << "" << endl;
  os << "#Remove temp files if the script is stoped by some reason" << endl;
  os << "trap \"rm -f $tmpfiles; exit\" INT TERM EXIT" << endl;
  os << " " << endl;
  os << "#Check arguments" << endl;
  os << "if [ -z $1 ]; then" << endl;
  os << "    echo \"ERROR: $0 needs a parameter specifiying the dataset\"" << endl;
  os << "    exit 100" << endl;
  os << "fi" << endl;
  os << "dataset=`echo $1 | sed 's\\ \\_\\g'`" << endl;
  os << "#echo DEB:$dataset" << endl;
  os << " " << endl;
  os << "#Build directories if they don't exit" << endl;
  os << "pathtodatfiles=\"./Datasets\"" << endl;
  os << "if [ ! -d \"$pathtodatfiles\" ]; then" << endl;
  os << "    echo \"ERROR: Could not find directory $pathtodatfiles. Exiting!\"" << endl;
  os << "    exit 1" << endl;
  os << "fi" << endl;
  os << "" << endl;
  os << "echo \">> Reading tab for dataset $dataset...\"" << endl;
  os << "datasetsdb=\"$pathtodatfiles/tabs.dat\"" << endl;
  os << "if [ ! -f \"$datasetsdb\" ]; then" << endl;
  os << "    echo \"ERROR: Could not find file \\\"$datasetsdb\\\". Exiting!\"" << endl;
  os << "    exit 2" << endl;
  os << "fi" << endl;
  os << "googlexmlfeed=`grep \" $dataset \" $datasetsdb | cut -d\\  -f 3`" << endl;
  os << "if [ \"$googlexmlfeed\" == \"\" ]; then" << endl;
  os << "    echo \"ERROR: Could not find an entry for $dataset in $datasetsdb\"" << endl;
  os << "    exit 99" << endl;
  os << "fi" << endl;
  os << "#echo \"   + Getting feed from $googlexmlfeed\"" << endl;
  os << "curl \"$googlexmlfeed\" > $xmlfile 2> /dev/null" << endl;
  os << "xmllint --format $xmlfile > $formatedfile" << endl;
  os << "" << endl;
  os << "#Find last cell" << endl;
  os << "lastcell=`grep -e \"</title>\"  $formatedfile | tail -1 | cut -d \\> -f 2 | cut -d \\< -f 1`" << endl;
  os << "#Get letter of last cell and ASCII code for the letter" << endl;
  os << "lastcelllet=`echo ${lastcell:0:1}`" << endl;
  os << "lastcellnum=`echo ${lastcell:1}`" << endl;
  os << "lastcellascii=`printf \"%d\\n\" \"'$lastcelllet\"`" << endl;
  os << "#echo \"   DEB: Last Cell = $lastcelllet ($lastcellascii)\"" << endl;
  os << "" << endl;
  os << "#Get the letters from A to lastcell" << endl;
  os << "letters=`awk 'BEGIN{for(i=65;i<=85;i++) printf(\"%c \",i);}'`" << endl;
  os << "" << endl;
  os << "" << endl;
  os << "#Main folder" << endl;
  os << "mainfolder=`grep -A 1 \"O1</title>\" $formatedfile | grep -v title| cut -d \\> -f 2 | cut -d \\< -f 1|cut -d \\  -f 1`" << endl;
  os << "skim1folder=`grep -A 1 \"R1</title>\" $formatedfile | grep -v title | cut -d \\> -f 2| cut -d \\< -f 1| sed 's/[F,f]older//g' | sed 's/ //g'`" << endl;
  os << "skim2folder=`grep -A 1 \"T1</title>\" $formatedfile | grep -v title | cut -d \\> -f 2| cut -d \\< -f 1| sed 's/[F,f]older//g' | sed 's/ //g'`" << endl;
  os << "" << endl;
  os << "#echo \"   DEB: Main folder = $mainfolder\"" << endl;
  os << "#echo \"   DEB: Skim 1 folder = $skim1folder\"" << endl;
  os << "#echo \"   DEB: Skim 2 folder = $skim2folder\"" << endl;
  os << "" << endl;
  os << "outputfile=$pathtodatfiles/$dataset.dat" << endl;
  os << "echo \"   + Writting info to $outputfile...\"" << endl;
  os << "if [ -f $outputfile ]; then" << endl;
  os << "    echo \"     - $outputfile already exists. Moving it to $outputfile.bak\"" << endl;
  os << "    mv $outputfile $outputfile.bak" << endl;
  os << "fi" << endl;
  os << "touch $outputfile" << endl;
  os << "echo -e \"#Dataset\\tN_Events\\tCross Section\\tMain Folder\\tBase File\" >> $outputfile" << endl;
  os << "" << endl;
  os << "#Iterate over rows (first 2 rows are titles)" << endl;
  os << "echo \"   + Iterating over rows...\"" << endl;
  os << "for i in `seq 3 $lastcellnum`; do" << endl;
  os << "    #echo -n \"$i \"" << endl;
  os << "    Ai=`grep -A 1 \"A$i</title>\" $formatedfile | grep -v title| cut -d \\> -f 2 | cut -d \\< -f 1`" << endl;
  os << "    " << endl;
  os << "    #echo \"Dataset name = $Ai\"" << endl;
  os << "    " << endl;
  os << "    # Get general information of the sample" << endl;
  os << "    rootfilebase=`grep -A 1 \"O$i</title>\" $formatedfile | grep -v title| cut -d \\> -f 2 | cut -d \\< -f 1`" << endl;
  os << "    nevents=`grep -A 1 \"D$i</title>\" $formatedfile | grep -v title| cut -d \\> -f 2 | cut -d \\< -f 1 | sed 's/,/./g'`" << endl;
  os << "    xsection=`grep -A 1 \"Q$i</title>\" $formatedfile | grep -v title| cut -d \\> -f 2 | cut -d \\< -f 1 | sed 's/,/./g'`" << endl;
  os << "    #echo \"     - DEB: File = $rootfilebase\" " << endl;
  os << "    #echo \"     - DEB: NEvt = $nevents\" " << endl;
  os << "    #echo \"     - DEB: Xsec = $xsection\" " << endl;
  os << "    " << endl;
  os << "    #Only printout if number of events and cross section are numbers" << endl;
  os << "    [[ $nevents =~ \"^[0-9]+([.][0-9]+)?$\" ]] && [[ $xsection =~ \"^[0-9]+([.][0-9]+)?$\" ]] && echo -e \"$Ai\\t$nevents\\t$xsection\\t$mainfolder\\t$rootfilebase\" >> $outputfile" << endl;
  os << "done #rows" << endl;
  os << "#echo \"\"" << endl;
  os << "exit 0" << endl;




  //Close file
  os.close();

  //Set apropiate permisions
  gSystem->Chmod(scriptname, 448/*700 in decimal = rwx------*/);

  //Return file name
  return scriptname;

}

void DatasetManager::RedownloadFiles() {
  //Just download the available folders.
  //This will trigger the redownload of old folder.dat files when requested
  DownloadAvailableFolders();
};


TString DatasetManager::GuessLocalBasePath() {
    TString host = gSystem->HostName();
    if (host.Contains("geol.uniovi.es"))
      return TString("/hadoop");
    else if (host.Contains("ifca.es"))
      return TString("/gpfs/csic_projects/tier3data");
    else if (host.Contains("gcsic"))
      return TString("/gpfs/csic_projects/tier3data");
    else if (host.Contains("wn")) // including workernodes
      return TString("/gpfs/csic_projects/tier3data");
    else if (host.Contains("astro")) // including astro workernodes
      return TString("/gpfs/csic_projects/tier3data");
    else if (host.Contains("cern.ch"))
      return TString("/pool/data1/MiniTrees/");
    else {
      cerr << "ERROR: Could not guess base path from host name " << host << endl;
      return TString("");
    }
}


//This functions does the actual construction
// * localBasePath : Local path to data (defaults to /hadoop)
// * skim: Optional. Fixes the skim used for the data.
// * folder: Is the folder (i.e. tab) used to search for the samples 
//   (ex. MC_Fall10)
// * forcereload: If set to true the information from the google doc page
//   is redownloaded 
void DatasetManager::Init(const char* localBasePath, 
			  const char* tab,
			  const char* skim,
			  bool forcereload) {
  //Initialize data members
  fLocalBasePath="";
  fFolder="";
  fDataset="";
  fBaseFileName="";
  fEventsInTheSample=0;
  fCrossSection=0;
  
  //If no local base path is provided try to guess it
  if (!localBasePath) {
    fLocalBasePath=DatasetManager::GuessLocalBasePath();
#ifdef DEBUGDS
    cout << "DEBUG: Using fLocalBasePath " << fLocalBasePath << endl;
#endif
    if (fLocalBasePath=="") {
      cerr << "       Please use another DatasetManager constructor" << endl;
      return;
    }
  }

 
  if (skim)
    fSkim = skim;
  else
    fSkim = "";

  //Check file lastreload
  Long_t id = 0;
  Long_t size = 0;
  Long_t flag = 0;
  Long_t modtime = 0; //Mod time in seconds

  if (forcereload)
    DownloadAvailableFolders();
  else if (gSystem->GetPathInfo("Datasets/tabs.dat", 
				&id, &size, &flag, &modtime) == 1) {
    cerr << "NOTE: Could not find Datasets/datasets" <<  endl
	 << "      Downloading it..." 
	 << endl;
    DownloadAvailableFolders();
  }
  // time(NULL) returns current time
  else if ((time(NULL) - modtime) > MAXREDOWNLOADTIME) {
    cerr << "NOTE: Datasets/datasets is older than 24 hours" <<  endl
	 << "      Downloading it again..." 
	 << endl;
    DownloadAvailableFolders();
  }

  if (tab)
    SetFolder(tab);

#ifdef DEBUGDS
  cout << "DEBUG: + fLocalBasePath = \"" << fLocalBasePath << "\"" << endl;
  cout << "DEBUG: + fSkim = \"" << fSkim << "\"" << endl;
  cout << "DEBUG: + fFolder = \"" << fFolder << "\"" << endl;
#endif

}


vector<TString> DatasetManager::GetRealDataFiles(const char* relativepath, 
					     const char* filebasename) {
  TString fullpath;
  vector<TString> theFiles;
  TString basefile(filebasename);
  if (basefile.Contains("TESCO"))
  {
	  fullpath = DatasetManager::GuessLocalBasePath() + "/TESCOtrees/Data/" + relativepath + "/";
  }
  else
  {
	  fullpath = DatasetManager::GuessLocalBasePath() + "/Data/" + relativepath + "/";
  } 

  TString command("ls ");
  command += 
    fullpath + "/" + basefile + "*.root ";  //Simpler +
    //fullpath + "/" + basefile + "_[0-9].root " +
    //fullpath + "/" + basefile + "_[0-9][0-9].root " +
    //fullpath + "/" + basefile + "_[0-9]*.root";  // Just when incorporates the luminosity in the filename
#ifdef DEBUGDS
  cout << "DEBUG: Executing command " << command << endl;
#else
  command += " 2> /dev/null";
#endif
  //We cannot use GetFromPipe because it is too verbose, so we implement
  //the full code
  //    TString result=gSystem->GetFromPipe(command);
  TString result;
  FILE *pipe = gSystem->OpenPipe(command, "r");
  if (!pipe) {
    cerr << "ERROR: in DatasetManager::GetFiles. Cannot run command \""
	 << command << "\"" << endl;
  }
  else {
    TString line;
    while (line.Gets(pipe)) {
      if (result != "")
	result += "\n";
      result += line;
    }
  
    gSystem->ClosePipe(pipe);
  }
  
 #ifdef DEBUGDS
    cout << "DEBUG: Result is " << result << endl;
#endif
    if (result != "" ) {
      TObjArray* filesfound = result.Tokenize(TString('\n'));
      if (!filesfound)
	cerr << "ERROR: Could not parse output while finding files" << endl;
      else {
	for (int i = 0; i < filesfound->GetEntries(); i++) {
 
	  theFiles.push_back(filesfound->At(i)->GetName());
	}
	filesfound->Clear();
	delete filesfound;
      }
    }

    if (theFiles.size() == 0)
      cerr << "ERROR: Could not find data!" << endl;

    return theFiles;
}

