#if defined(__ROOTCLING__)

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ nestedclasses;

#pragma link C++ class PIAnaHit+;
#pragma link C++ class PIAnaHitMerger+;
#pragma link C++ class PIAnaG4StepDivider+;

#pragma link C++ class PIAnaGraph+ ;
#pragma link C++ class PIAnaPointCloud+;
#pragma link C++ class PIAnaPointCloud1D+;
#pragma link C++ class PIAnaPointCloud3D+;
#pragma link C++ class PIAnaPointCloudXYZ+;
#pragma link C++ class PIAnaPointCloudT+;
#pragma link C++ class PIPointCloud < double, PIAnaHit> + ;
#pragma link C++ class PILocCluster+;
#pragma link C++ class PITCluster+;
#pragma link C++ class PIXYZCluster+;

#pragma link C++ class PIAnaEvtBase+;
#pragma link C++ class PIPiDARFilter+;
#pragma link C++ class PIPiDecayInSi+;
#pragma link C++ class PIFilterBase+;
#pragma link C++ class PIAnaAtarPho+;
#pragma link C++ class PIAnalyzer+;

#pragma link C++ namespace PIAna+;
#pragma link C++ class PIAna::PITkFinder+;
#pragma link C++ class PIAna::PITkPCA+;
#pragma link C++ class PIAna::PIEventData+;
#pragma link C++ class PIAna::PIEventAction+;
#pragma link C++ class PIAna::PIEventAnalyzer+;
#pragma link C++ class PIAna::PIEventFilter+;
#pragma link C++ class PIAna::PIEventProducer+;
#pragma link C++ class PIAna::PIJobManager+;
#pragma link C++ class PIAna::PIPiDARFilter+;
#pragma link C++ class PIAna::PIHitProducer+;
#pragma link C++ nestedclass;
// #pragma link C++ nestedfunction;
#pragma link C++ nestedtypedef;

#pragma link C++ class std::vector<PIAnaHit>+;
#pragma link C++ class std::vector<PIMCAtar>+;

#endif
