///////////////////////////////////////////////////////////////////////
//
//    FILE: DatasetManager.h
//   CLASS: DatasetManager
//  AUTHOR: I. Gonzalez Caballero
//    DATE: January, 2011
//
// CONTENT: This class helps getting the information of a MC dataset
//          and the MiniTree root files in our storages. The 
//          information is retrieved from the google doc page
//          maintained by Javier Fernandez
///////////////////////////////////////////////////////////////////////

#ifndef DatasetManager_h
#define DatasetManager_h 1

// ROOT includes
#include "TString.h"

// STL includes
#include <vector>


class DatasetManager 
{
	public:
		//! Constructor. It sets the basic parameters.
		//!  * localBasePath : Local path to data (defaults to /hadoop)
		//!  * skim: Optional. Fixes the skim used for the data.
		//!  * folder: Is the folder (i.e. tab) used to search for the samples 
		//!    (ex. MC_Fall10)
		//!  * forcereload: If set to true the information from the google doc page
		//!    is redownloaded 
		DatasetManager(const char* localBasePath,
				const char* skim,
				const char* folder,
				bool forcereload=false);
		
		//! Intelligent constructor (guesses basepath) for datasets with skim
		//!  * tab: Is the tab used to search for the samples (ex. MC_Fall10)
		//!  * skim: Optional. Fixes the skim used for the data.
		//!  * forcereload: If set to true the information from the google doc page
		//!    is redownloaded 
		DatasetManager(const char* tab, const char* skim, bool forcereload=false);
		
		//! Intelligent constructor (guesses basepath) for datasets with no skim
		//! (ex. TESCO Trees)
		//!  * tab: Is the tab used to search for the samples (ex. MC_Fall10)
		//!  * forcereload: If set to true the information from the google doc page
		//!    is redownloaded 
		DatasetManager(const char* tab, bool forcereload=false);
		
		~DatasetManager() {}
		
		//! Get methods
		//! * Returns the local base path to the datasets (ex. /hadoop)
		TString GetLocalBasePath() const {return fLocalBasePath;}
		//! * Returns the skim being used to search for the dataset files
		TString GetSkim() const {return fSkim;}
		//! * Returns the folder/tab being used to search for the dataset
		TString GetFolder() const {return fFolder;}
		//! * Return the name of the dataset currently loaded
                TString GetDatasetName() const {return fDataset;}
		
		//! Set methods
		//! * Sets the local base path to the datasets (ex. /hadoop)
		void SetLocalBasePath(const char* path) {fLocalBasePath=path;fFiles.clear();}
		//! * Sets the skim to use to search for the dataset files
		void SetSkim(const char* skim) { fSkim = skim; fFiles.clear();}
		//! * Sets the folder/tab to use to search for the dataset. Downloads the
		//!   information if needed. If redownload is set to false, the folder 
		//!   information will be redownloaded
		bool SetFolder(const char* folder, bool redownload = false);

		//! Load Dataset (datasetname) information. Optionally a folder and/or a skim
		//! can be provided overridding the current ones. 
		bool LoadDataset(const char* datasetname, 
				const char* skim = 0, const char* folder = 0);

		//! Returns information on the dataset just loaded
		//! * Events in the dataset
		int GetEventsInTheSample() const {return fEventsInTheSample;}
		//! * Dataset cross-section
		double GetCrossSection() const {return fCrossSection;}
		//! * Local folder path to tab/folder
		TString GetLocalFolder() const {return fLocalFolder;}
		//! * Format of the files corresponding to this dataset
		TString GetBaseFileName() const {return fBaseFileName;}
		//! * A vector of files (full path) in the dataset... It searches for them
		//!   the first time it is called and caches the data.
		const std::vector<TString>& GetFiles();

		//! Dumps all the information
	        void Dump() const;
  

                //! Download the information on the folder/tabs available from Google Docs
                static bool DownloadAvailableFolders();
                //! Download the information on a given folder/tab from Google Docs
                static bool DownloadFolder(const char* folder);
                //! This method writes the script that downloads and processes the information
                //! on the available datasets from Google Docs
                static TString WriteGetDatatabsScript();
                //! Redownload all the files
                static void RedownloadFiles();
                //! This method writes the script that downloads and processes the data from
                //! the Google Doc file
                static TString WriteGetDatasetsScript();
              
                //! Try to find the path to the local data
                //! Returns an empty string if the guess failed
                static TString GuessLocalBasePath();
              
                
                static std::vector<TString> GetRealDataFiles(const char* relativepath, 
              				       const char* filebasename);

		
		
	protected:
                //! This functions does the actual construction
                //!  * localBasePath : Local path to data (defaults to /hadoop)
                //!  * skim: Optional. Fixes the skim used for the data.
                //!  * folder: Is the folder (i.e. tab) used to search for the samples 
                //!   (ex. MC_Fall10)
                //!  * forcereload: If set to true the information from the google doc page
                //!   is redownloaded 
                void Init(const char* localBasePath, 
              	    const char* folder,
              	    const char* skim, 
              	    bool forcereload=false);
              
		
	protected:
                TString fLocalBasePath;
                TString fSkim;
                TString fFolder;
              
                TString fDataset;
                TString fLocalFolder;
                TString fBaseFileName;
                int fEventsInTheSample;
                double fCrossSection;
                std::vector<TString> fFiles;
};

#endif
