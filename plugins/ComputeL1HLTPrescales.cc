//#if 0
// system include files
#include <memory>
#include <cmath>
#include <boost/foreach.hpp>
// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/Candidate/interface/Candidate.h"

#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include <DataFormats/PatCandidates/interface/Photon.h>
#include <DataFormats/PatCandidates/interface/PackedTriggerPrescales.h>
#include "FWCore/Utilities/interface/RegexMatch.h"

#include "FWCore/Common/interface/TriggerNames.h"
#include "FWCore/Common/interface/TriggerResultsByName.h"
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"
#include "HLTrigger/HLTcore/interface/HLTPrescaleProvider.h"
#include "EgammaAnalysis/TnPTreeProducer/plugins/WriteValueMap.h"

//
// class declaration
//

class ComputeL1HLTPrescales : public edm::EDProducer {
public:
  explicit ComputeL1HLTPrescales(const edm::ParameterSet&);
  ~ComputeL1HLTPrescales();

private:
  virtual void produce(edm::Event&, const edm::EventSetup&);
  virtual void beginRun(edm::Run&, edm::EventSetup const&);

  // ----------member data ---------------------------
  //const edm::EDGetTokenT<edm::View<reco::Candidate>> probesLabel_;
  edm::EDGetTokenT<std::vector<pat::Photon> > probesToken_;
  //edm::Handle<edm::TriggerResults> triggerResults;
  edm::EDGetTokenT<edm::TriggerResults           > triggerToken_;
  edm::EDGetTokenT<pat::PackedTriggerPrescales> triggerPrescaleToken_;
  HLTPrescaleProvider hltPrescaleProvider_;
  edm::ParameterSetID triggerNamesID_;
  const std::string hltConfigLabel_;
  std::vector<std::string> hltPaths_;
  std::vector<std::string> hltPathsWithVer_;
  //HLTConfigProvider hltPrescaleProvider_.hltConfigProvider();

  virtual void initPattern(const edm::TriggerResults & result,
                        const edm::EventSetup& iSetup,
                        const edm::TriggerNames & triggerNames);
  void writeGlobalFloat(edm::Event &iEvent, const edm::Handle<edm::View<reco::Candidate> > &probes, const float value, const std::string &label) ;
};

ComputeL1HLTPrescales::ComputeL1HLTPrescales(const edm::ParameterSet& iConfig):
  probesToken_(consumes<std::vector<pat::Photon> >(iConfig.getParameter<edm::InputTag>("probes"))),
  //probesLabel_(consumes<edm::View<reco::Candidate>>(iConfig.getParameter<edm::InputTag>("probes"))),
  triggerToken_    (consumes<edm::TriggerResults           >(iConfig.getParameter<edm::InputTag>("triggerResults"))),
  triggerPrescaleToken_(consumes<pat::PackedTriggerPrescales>(iConfig.getUntrackedParameter<std::string>("triggerPrescaleInputTag"))),
  hltPrescaleProvider_(iConfig, consumesCollector(), *this),
  triggerNamesID_(),
  hltConfigLabel_(iConfig.getParameter<std::string>("hltConfig")),
  hltPaths_(iConfig.getParameter<std::vector<std::string> >("hltPaths"))
{
  std::cout<<hltPaths_.size()<<std::endl;
  for(unsigned int j=0; j<hltPaths_.size(); ++j) {
    // Trigger name should include "_v" at the end
    std::string tmpStr = hltPaths_[j].substr(0, hltPaths_[j].find("_v")); 
    while(tmpStr.find("_")!=std::string::npos) {tmpStr.replace(tmpStr.find("_"),1,"");}
    produces<edm::ValueMap<float> >( (tmpStr+"TotalPrescale").c_str() );
    produces<edm::ValueMap<float> >( (tmpStr+"PassTrigger").c_str() );
    //std::cout<<tmpStr+"TotalPrescale"<<std::endl;
  }
}


ComputeL1HLTPrescales::~ComputeL1HLTPrescales() {}


void ComputeL1HLTPrescales::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;

  // Read input
  edm::Handle<std::vector<pat::Photon>> probes;
  iEvent.getByToken(probesToken_, probes);
  edm::Handle<edm::TriggerResults> triggerResults;
  iEvent.getByToken(triggerToken_, triggerResults);
  edm::Handle<pat::PackedTriggerPrescales> triggerPrescalesH;
  iEvent.getByToken(triggerPrescaleToken_, triggerPrescalesH);
  //if (!triggerResults.isValid()) {
    //cout << "Error in getting TriggerResults product from Event!" << endl;
    //return;
  //}
     //        
  
  std::vector<float> tmp_ps;
   typename std::vector<pat::Photon>::const_iterator probe, endprobes = probes->end();

  for (probe = probes->begin(); probe != endprobes; ++probe) tmp_ps.push_back(-2);
  if(!triggerResults.isValid()) {
    std::cout << "Error in getting TriggerResults product from Event!" << std::endl;
    for(unsigned int j=0; j<hltPaths_.size(); ++j) {
      std::string tmpStr = hltPaths_[j].substr(0, hltPaths_[j].find("_v")); 
      while(tmpStr.find("_")!=std::string::npos) {tmpStr.replace(tmpStr.find("_"),1,"");}
      writeValueMap(iEvent, probes, tmp_ps, tmpStr+"TotalPrescale");
      //writeGlobalFloat(iEvent, probes, -2., tmpStr+"TotalPrescale");
    }
    return;
  }
  edm::TriggerNames const& triggerNames_ = iEvent.triggerNames(*triggerResults);
  bool config_changed = false;
  if (triggerNamesID_ != triggerNames_.parameterSetID()) {
    triggerNamesID_ = triggerNames_.parameterSetID();
    config_changed = true;
  }
  // (re)run the initialization of the container with the trigger patterns 
  // - this is the first event 
  // - or the HLT table has changed 
  if (config_changed) {
      initPattern(*triggerResults, iSetup, triggerNames_);  
  }
  size_t nTriggers = triggerResults->size();
  if (!triggerPrescalesH.isValid()) throw cms::Exception("ComputeL1HLTprescale::produce: error getting PackedTriggerPrescales product from Event!"); 
  for(unsigned int j=0; j<hltPathsWithVer_.size(); ++j) {
    /*
    std::string trigName = "";

    for(unsigned int iHltPath=0; iHltPath<hltPrescaleProvider_.hltConfigProvider().size(); ++iHltPath) {
      if( hltPrescaleProvider_.hltConfigProvider().triggerName(iHltPath).find(hltPaths_[j]) != std::string::npos ) {
	trigName = hltPrescaleProvider_.hltConfigProvider().triggerName(iHltPath);
	break;
      }
    } // end for(iHltPath)
    */
    std::string trigName = hltPathsWithVer_[j];
    bool passTrigger = false;
    for (unsigned int k=0; k < nTriggers; k++){
      if (trigName != triggerNames_.triggerName(k)) continue;
      else {
        passTrigger = triggerResults->accept(k);
        break;
      }
    }
    std::vector<float> totPs;
    std::vector<float> passTriggerVec;
    if(trigName.length()>0) {
      std::pair<int,int> pss = hltPrescaleProvider_.prescaleValues(iEvent, iSetup, trigName);
      for (probe = probes->begin(); probe != endprobes; ++probe) {
        totPs.push_back(pss.first * pss.second);
        if (passTrigger) passTriggerVec.push_back(1);
        else passTriggerVec.push_back(0);
      }
    }
    // Works if trigger name does not contain "_v" in the middle... 
    // Otherwise use HLTConfigProvider::removeVersion() or regexp "_v[0-9]+$"
    trigName = trigName.substr(0, trigName.find("_v")); 
    while(trigName.find("_")!=std::string::npos) {trigName.replace(trigName.find("_"),1,"");}
    //writeGlobalFloat(iEvent, probes, totPs, trigName+"TotalPrescale");
    writeValueMap(iEvent, probes, totPs, trigName+"TotalPrescale");
    writeValueMap(iEvent, probes, passTriggerVec, trigName+"PassTrigger");
    //std::cout << totPs << " for " << trigName+"TotalPrescale" <<std::endl;
  } // end for(j)
}

void ComputeL1HLTPrescales::beginRun(edm::Run& iRun, edm::EventSetup const& iSetup) {
  bool changed = true;
  if(!hltPrescaleProvider_.init(iRun,iSetup,hltConfigLabel_,changed))
  //if(!hltPrescaleProvider_.hltConfigProvider().init(iRun, iSetup, hltConfigLabel_, changed)) 
    std::cout << "Warning, didn't find HLTConfigProvider with label " 
	      << hltConfigLabel_.c_str() << " in run " << iRun.run() << std::endl;
}

void ComputeL1HLTPrescales::initPattern(const edm::TriggerResults & result,
                        const edm::EventSetup& iSetup,
                        const edm::TriggerNames & triggerNames)
//--------------------------------------------------------------------------
{
    unsigned int n;
    
    // clean up old data
    hltPathsWithVer_.clear();
    //prunedhltPathsByName_.clear();

    if (hltPaths_.empty()) {
        // for empty input vector, default to all HLT trigger paths
        n = result.size();
        hltPathsWithVer_.resize(n);
        for (unsigned int i = 0; i < n; ++i) {
            hltPathsWithVer_[i] = triggerNames.triggerName(i);
        }
    } else {
        // otherwise, expand wildcards in trigger names...
        BOOST_FOREACH(const std::string & pattern, hltPaths_) {
            if (edm::is_glob(pattern)) {
                // found a glob pattern, expand it
                std::vector< std::vector<std::string>::const_iterator > matches = edm::regexMatch(triggerNames.triggerNames(), pattern);
                if (matches.empty()) {
                    // pattern does not match any trigger paths
                    //std::cout<<"No patterns found.  Please check quality file... "<<std::endl;
                    exit(0);
                } else {
                    // store the matching patterns
                    BOOST_FOREACH(std::vector<std::string>::const_iterator match, matches)
                        hltPathsWithVer_.push_back(*match);
                }
            } else {
                // found a trigger name, just copy it
                hltPathsWithVer_.push_back(pattern);
            }
        }
        
    }
}
void ComputeL1HLTPrescales::writeGlobalFloat(edm::Event &iEvent, const edm::Handle<edm::View<reco::Candidate> > &probes, const float value, const std::string &label) { 
  //std::unique_ptr<edm::ValueMap<float> > out(new edm::ValueMap<float>());
  auto out = std::make_unique<edm::ValueMap<float>>();
  edm::ValueMap<float>::Filler filler(*out);
  std::vector<float> values(probes->size(), value);
  filler.insert(probes, values.begin(), values.end());
  filler.fill();
  iEvent.put(std::move(out), label);
}

// Define this module as plugin
DEFINE_FWK_MODULE(ComputeL1HLTPrescales);
//#endif
