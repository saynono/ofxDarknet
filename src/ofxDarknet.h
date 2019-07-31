#pragma once

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define GPU
#define OPENCV
//#define TRACK_OPTFLOW

#include "ofMain.h"

#include "ofxCv.h"
#include "ofxOpenCv.h"

#include "yolo_v2_class.hpp"    // imported functions from DLL

#ifdef OPENCV
#include "opencv2/highgui/highgui_c.h"
#endif


struct DetectedObject {
    unsigned int id;
	ofRectangle rect;
    ofRectangle rectPredicted;
	std::string label;
	float probability;
	uint64_t lastDetected;
};

struct DetectedObjects {
//    ofPixels pix;
    std::map<unsigned int, DetectedObject> objects;
};


class ofxDarknet: public ofThread
{
public:
	ofxDarknet();
	~ofxDarknet();

	void init( std::string cfgfile, std::string weightfile, std::string nameslist = "");
    bool isLoaded() {return loaded;}    
    void yolo_nono( ofPixels & pix, float threshold = 0.24f, float maxOverlap = 0.5f );

    DetectedObjects getDetectedObjects();

    
    void threadedFunction();

protected:
    void update(ofEventArgs & a);
    // std::shared_ptr<image_t> convert( ofPixels & pix, std::shared_ptr<image_t> image_ptr );
    void convert( ofPixels & pix, image_t* image );
    ofPixels convert( image_t & image );
    
	// list1 *options1;
	char **names;
    // vector<string> layerNames;
	// network net;
    string cfgfile;
    string weightfile;
    string nameslist;

    bool loaded;
    bool labelsAvailable;
    bool bDetectionIsBusy = false;

    std::shared_ptr<Detector> detector;
    std::vector<std::string> obj_names;
    track_kalman_t trackKalman;


    struct AnalyseObject {
        ofPixels pixels;
        float threshold;
        float maxOverlap;
        std::vector<bbox_t> result_vec;
    };


    std::vector<std::string> objectsNamesFromFile(std::string const filename);
    ofThreadChannel<AnalyseObject> toAnalyze;
    ofThreadChannel<AnalyseObject> analyzed;
    DetectedObjects detectedObjects;

    int trackingMaxDist = 200;
    int trackingFrameHistory = 10;
    bool trackingChangeHistory = true;

    uint64_t milliSecsUntilRemove = 500;

};
