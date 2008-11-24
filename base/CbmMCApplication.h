
#ifndef CBM_MC_APPLICATION_H
#define CBM_MC_APPLICATION_H

#include "CbmTask.h"
#include "TVirtualMCApplication.h"
#include <map>
#include "TLorentzVector.h"

class CbmRootManager;
class TRefArray;
class TObjArray;
class CbmGenericStack;
class CbmModule;
class CbmDetector;
class CbmField;
class TTask;
class CbmPrimaryGenerator;
class CbmTrajFilter;
class TChain;
class CbmVolume;
class CbmRadLenManager;


/**
 * The Main Application ( Interface to MonteCarlo application )
 * @author M. Al-Turany, D. Bertini
 * @version 0.1
 * @since 12.01.04
 */

class CbmMCApplication : public TVirtualMCApplication
{

  public:
    /** Standard constructor 
   *@param name      name
   *@param title     title
   *@param ModList  a TObjArray containing all detectors and modules used in this simulation
   *@param MatName  material file name 
   */
    CbmMCApplication(const char * name,   const char * title, TObjArray *ModList, const char *MatName);
   /** default constructor 
   */
    CbmMCApplication();
   /** Special constructor, used for initializing G3 for Geane track propagation 
   *@param Geane    true to initialize Geane
   */
    CbmMCApplication(Bool_t Geane);
    /** default destructor 
   */
    virtual ~CbmMCApplication();
    /** Singelton instance 
    */
    static CbmMCApplication* Instance(); 
    virtual void          AddDecayModes();  
	 /**  Add user defined particles (optional) */
    virtual void          AddParticles();                                   // MC Application
	 /** Add user defined ions (optional) */
    virtual void          AddIons();                                        // MC Application
     /** Add user defined Tasks to be executed after each event (optional) */
	void                  AddTask(TTask *fTask);
    /** Define actions at the beginning of the event */
	virtual void          BeginEvent();                                     // MC Application
	/** Define actions at the beginning of primary track */
    virtual void          BeginPrimary();                                   // MC Application
    /** Construct user geometry */
	virtual void          ConstructGeometry();                              // MC Application
    /** Define parameters for optical processes (optional) */
	virtual void          ConstructOpGeometry();                            // MC Application
	/** Calculate user field  b at point x */
    virtual void          Field(const Double_t* x, Double_t* b) const;      // MC Application
	/** Define actions at the end of event */
    virtual void          FinishEvent();                                    // MC Application
	/** Define actions at the end of primary track */
    virtual void          FinishPrimary();                                  // MC Application
	/** Define actions at the end of run */
    void                  FinishRun();
	/** Generate primary particles */
    virtual void          GeneratePrimaries();                              // MC Application
	/** Return detector by name  */
    CbmDetector*          GetDetector(const char *DetName);
	/** Return Field used in simulation*/
    CbmField*             GetField(){return fxField;}
	/**Return primary generator*/
    CbmPrimaryGenerator*  GetGenerator();
	/**Return list of tasks*/
    TTask*                GetListOfTasks();    
    CbmGenericStack*      GetStack();
    TChain*               GetChain();
	/** Initialize geometry */
    virtual void          InitGeometry();                                   // MC Application
	/** Initialize MC engine */
    void                  InitMC(const char *setup,  const char *cuts);
    void                  InitTasks();
    Bool_t 		          IsGeane(){return fGeane;}
	/**Define actions at the end of each track */
    virtual void          PostTrack();                                      // MC Application
	/** Define actions at the beginning of each track*/
    virtual void          PreTrack();                                       // MC Application
    void                  RunMC(Int_t nofEvents);
    void                  SetField(CbmField *field);
    void                  SetGenerator(CbmPrimaryGenerator *fxGenerator);
    void                  SetPythiaDecayer(Bool_t decayer){fPythiaDecayer=decayer;}
    void                  SetRadiationLengthReg(Bool_t RadLen);
    void                  SetTrackingDebugMode( Bool_t set ) {fDebug = set;}
    void                  SetUserDecay(Bool_t decay){fUserDecay= decay;}
	/** Define action at each step */
    virtual void          Stepping();                                       // MC Application
    virtual void          StopRun();                                        
	/**Define maximum radius for tracking (optional) */
    virtual Double_t      TrackingRmax() const;                             // MC Application
    /** Define maximum z for tracking (optional) */
	virtual Double_t      TrackingZmax() const;                             // MC Application
    void                  GeanePreTrack(Float_t *x,Float_t *p ,Int_t PDG);                     // PreTrack for Geane
private:
    // methods
    void RegisterStack();
    // data members
    /**Iterator for active detector list*/
    TIterator           *fActDetIter;//!
    /**List of active detector */	
    TRefArray           *fActiveDetectors;
    /**List of CbmTask*/
    CbmTask             *fCbmTaskList;//!
    /**Iterator for detector list (Passive and Active)*/
    TIterator           *fDetIter; //!
    /**detector list (Passive and Active)*/
    TRefArray           *fDetectors;
    /**Map used for dispatcher*/
    TRefArray           *fDetMap;
    /**Flag for Geane*/
    Bool_t	         fGeane;
    /**Iterator for Module list*/
    TIterator           *fModIter; //!
    /**Module list in simulation*/
    TObjArray           *fModules; 
    /**Number of sensetive volumes in simulation session*/
    Int_t                fNoSenVolumes; //!
    /**flag for using Pythia as external decayer */
    Bool_t               fPythiaDecayer;
    /** Simulation Stack  */
    CbmGenericStack     *fStack; //!
    /**Pointer to thr I/O Manager */
    CbmRootManager      *fRootManager; //!
    /**List of sensetive volumes in all detectors*/
    TRefArray           *fSenVolumes; //!
    /**Magnetic Field Pointer*/
    CbmField            *fxField; //
    /**Primary generator*/
    CbmPrimaryGenerator *fEvGen; //
    /**MC Engine 1= Geant3, 2 = Geant4*/
    Int_t                fMcVersion;     // mc Version
    /** Track visualization manager */
    CbmTrajFilter       *fTrajFilter; //!
    /**Flag for accepted tracks for visualization*/
    Bool_t               fTrajAccepted; //!
    /**Flag for using user decay*/
    Bool_t               fUserDecay;
    /** Debug flag*/
    Bool_t               fDebug;//!
    /**dispatcher internal use */
    CbmVolume 	        *fDisVol;
    /**dispatcher internal use */ 
    CbmDetector         *fDisDet;
    /**dispatcher internal use */  
    std::multimap <Int_t, CbmVolume* > fVolMap;//!
    /**dispatcher internal use */   
    std::multimap <Int_t, CbmVolume* >::iterator fVolIter; //!
    /** Track position*/
    TLorentzVector fTrkPos; //!
    /** Flag for Radiation length register mode  */
    Bool_t   fRadLength;  //!
    
    /**Radiation length Manager*/
    CbmRadLenManager  *fRadLenMan; //!

    ClassDef(CbmMCApplication,1)  //Interface to MonteCarlo application
private:
   CbmMCApplication(const CbmMCApplication &M);
   CbmMCApplication& operator= (const CbmMCApplication&) {return *this;}
};

// inline functions

inline CbmMCApplication* CbmMCApplication::Instance()
{ return (CbmMCApplication*)(TVirtualMCApplication::Instance());}

#endif
