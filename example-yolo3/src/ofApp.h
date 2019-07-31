#pragma once

#include "ofMain.h"

#include "ofxDarknet.h"

// #define USE_WEBCAM

class ofApp : public ofBaseApp
{
public:
	void setup();
	void update();
	void draw();

	ofxDarknet yolo;
#ifdef USE_WEBCAM
	ofVideoGrabber video;
#else
	ofVideoPlayer player;
#endif

	uint currentFrame = 0;
    float scale = .5;

};
