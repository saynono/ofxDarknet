#include "ofApp.h"

void ofApp::setup() 
{
//	std::string cfgfile = ofToDataPath( "cfg/darknet.cfg" );
//	std::string weightfile = ofToDataPath( "darknet.weights" );
//	std::string nameslist = ofToDataPath( "cfg/imagenet.shortnames.list" );

    std::string cfgfile = ofToDataPath( "cfg/tiny-yolo.cfg" );
    std::string weightfile = ofToDataPath( "tiny-yolo.weights" );
    std::string nameslist = ofToDataPath( "cfg/9k.names" );

    darknet.init( cfgfile, weightfile, nameslist );

	video.setDeviceID( 0 );
	video.setDesiredFrameRate( 30 );
	video.initGrabber( 640, 480 );

	layer l;
	network_state state;
	network net;
	// backward_activation_layer_gpu(l, state);
	// forward_network_gpu(net, state);

}

void ofApp::update()
{
	ofLog() << ofGetFrameRate();
	video.update();
}

void ofApp::draw()
{
	video.draw( 0, 0 );

	if( video.isFrameNew() ) {
		classifications = darknet.classify( video.getPixels() );
	}
	
	int offset = 20;
	for( classification c : classifications )
	{
		std::stringstream ss;
		ss << c.label << " : " << ofToString( c.probability );
		ofDrawBitmapStringHighlight( ss.str(), 20, offset );
		offset += 20;
	}
}
