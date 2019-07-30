#pragma once

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define GPU

#include "ofMain.h"

#include "activations.h"
#include "avgpool_layer.h"
#include "activation_layer.h"
#include "convolutional_layer.h"
#include "option_list.h"
#include "image.h"
#include "parser.h"
#include "list.h"
#include "box.h"
#include "tree.h"
#include "layer.h"
#include "matrix.h"
#include "connected_layer.h"
#include "gru_layer.h"
#include "rnn_layer.h"
#include "crnn_layer.h"
#include "detection_layer.h"
#include "region_layer.h"
#include "normalization_layer.h"
#include "reorg_layer.h"
#include "cost_layer.h"
#include "softmax_layer.h"
#include "route_layer.h"
#include "shortcut_layer.h"
#include "cuda.h"
#include "utils.h"
#include "rnn.h"

#include "yolo_v2_class.hpp"    // imported functions from DLL


#include "ofxOpenCv.h"

#ifdef OPENCV
#include "opencv2/highgui/highgui_c.h"
#endif


struct detected_object {
	ofRectangle rect;
	std::string label;
	float probability;
    vector<float> features;
	ofColor color;
};

struct classification {
	std::string label;
	float probability;
};

struct activations {
    std::vector<float> acts;
    int rows;
    int cols;
    float min;
    float max;
    void getImage(ofImage & img);
};


// #define C_SHARP_MAX_OBJECTS 1000

// struct bbox_t {
//     unsigned int x, y, w, h;       // (x,y) - top-left corner, (w, h) - width & height of bounded box
//     float prob;                    // confidence - probability that the object was found correctly
//     unsigned int obj_id;           // class of object - from range [0, classes-1]
//     unsigned int track_id;         // tracking id for video (0 - untracked, 1 - inf - tracked object)
//     unsigned int frames_counter;   // counter of frames on which the object was detected
//     float x_3d, y_3d, z_3d;        // center of object (in Meters) if ZED 3D Camera is used
// };

// struct image_t {
//     int h;                        // height
//     int w;                        // width
//     int c;                        // number of chanels (3 - for RGB)
//     float *data;                  // pointer to the image data
// };

// struct bbox_t_container {
//     bbox_t candidates[C_SHARP_MAX_OBJECTS];
// };


class ofxDarknet: public ofThread
{
public:
	ofxDarknet();
	~ofxDarknet();

	void init( std::string cfgfile, std::string weightfile, std::string nameslist = "");
    bool isLoaded() {return loaded;}    
    void yolo_nono( ofPixels & pix, float threshold = 0.24f, float maxOverlap = 0.5f );
    
    void threadedFunction();

protected:
    void update(ofEventArgs & a);
    image convert( ofPixels & pix );
    ofPixels convert( image & image );
    
	// list1 *options1;
	char **names;
    // vector<string> layerNames;
	network net;
    bool loaded;
    bool labelsAvailable;

    std::shared_ptr<Detector> detector;
    std::vector<std::string> obj_names;

    std::vector<std::string> objectsNamesFromFile(std::string const filename);
    ofThreadChannel<ofPixels> toAnalyze;
    ofThreadChannel<detected_object> analyzed;

};


#include "ofxDarknetGo.h"
