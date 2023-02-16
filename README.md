# EgammaAnalysis-TnPTreeProducer

Package of the EGamma group to produce Tag-and-Probe trees

## Overview of branches

| Branch                                     | release            | tnpEleIDs          | tnpPhoIDs          | tnpEleTrig         | tnpEleReco         | purpose                                |
| ------------------------------------------ | ------------------ |:------------------:|:------------------:|:------------------:|:------------------:|:--------------------------------------:|
|                                            |                    | *miniAOD*          | *miniAOD*          | *miniAOD*          | *AOD*              |                                        |
| [RunIIfinal](../../tree/RunIIfinal)        | CMSSW\_10\_2       | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | Run II analysis                        |
| [RunIIfinal](../../tree/RunIIfinal)        | CMSSW\_10\_6       | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | Run II analysis using ultra-legacy     |
| [CMSSW\_11\_X\_Y](../../tree/CMSSW_11_X_Y) | CMSSW\_11          | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :white_check_mark: | Development for Run III (experimental) |

Note: because of a dataformat CMSSW\_10\_6 can only be used for ultra-legacy samples, and CMSSW\_10\_2 should be used for the rereco samples.

## Available tuples

### ReReco 2016, 2017 and 2018
If you do not need changes to the default code, you can simply use existing flat tag and probe trees, avalaible for both 2016, 2017 and 2018 (RunIIfinal branch):

```bash
ls /eos/cms/store/group/phys_egamma/tnpTuples/tomc/2020-06-09/*/merged/
```

These inlcude the tnpEleTrig, tnpEleIDs and tnpPhoIDs trees produced with the RunIIfinal branch.
*Main change with respect to the 2020-02-28 production is the inclusion of some additional branches, e.g. the leptonMva's*

### ReReco 2016, 2017 and 2018 - L1 matched
In case you need L1 matching for the measurement of doubleEle HLT triggers, you can use the tnpEleTrig trees found in:

```bash
ls /eos/cms/store/group/phys_egamma/tnpTuples/tomc/2020-03-03/*/merged/*L1matched.root
```

### UL2017 and UL2018
For ultra-legacy  we have tnpEleTrig, tnpEleIDs and tnpPhoIDs trees available at:
```
ls /eos/cms/store/group/phys_egamma/tnpTuples/tomc/2020-05-20/UL2018/merged
ls /eos/cms/store/group/phys_egamma/tnpTuples/tomc/2020-05-20/UL2017/merged
ls /eos/cms/store/group/phys_egamma/tnpTuples/rasharma/2021-02-10/UL2016postVFP/merged
ls /eos/cms/store/group/phys_egamma/tnpTuples/rasharma/2021-02-10/UL2016preVFP/merged
```


## To produce new tuples
### 1. Install for ultra-legacy (CMSSW\_10\_6\_X, works for UL2017 and UL2018 data/MC)

```bash
cmsrel CMSSW_10_6_13
cd CMSSW_10_6_13/src
cmsenv
git clone -b photonHLT git@github.com:Allen319/EgammaAnalysis-TnPTreeProducer.git EgammaAnalysis/TnPTreeProducer
scram b -j8
```

### 2. Try-out
You can find the cmsRun executable in EgammaAnalysis/TnPTreeProducer/python:
```bash
cd EgammaAnalysis/TnPTreeProducer/python/
cmsRun TnPTreeProducer_cfg.py isMC=True doTrigger=True era=UL2018
```
Check [TnPTreeProducer\_cfg.py](python/TnPTreeProducer_cfg.py) for all available options. Update the code if you need to implement custom-made recipes.

Test files can be defined in [python/etc/tnpInputTestFiles\_cff.py](python/etc/tnpInputTestFiles_cff.py)
If you update the code, you can use the ./runTests.py script in the test directory to check for new differences in the 2016, 2017 and 2018 test files.
