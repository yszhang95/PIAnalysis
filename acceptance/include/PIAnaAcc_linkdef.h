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
#pragma link C++ class PILocCluster + ;
#pragma link C++ class PITCluster + ;
#pragma link C++ class PIXYZCluster+;

#pragma link C++ class PIAnaEvtBase+;
#pragma link C++ class PIAnaAtarPho+;
#pragma link C++ class PIAnalyzer+;

#pragma link C++ class std::vector<PIAnaHit>+;
#pragma link C++ class std::vector<PIMCAtar>+;

#endif
