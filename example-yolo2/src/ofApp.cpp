#include "ofApp.h"

void ofApp::setup() 
{
	// std::string cfgfile = ofToDataPath( "cfg/yolo9000.cfg" );
	// std::string weightfile = ofToDataPath( "yolo9000.weights" );
	// std::string nameslist = ofToDataPath( "cfg/9k.names" );

	// std::string cfgfile = ofToDataPath( "cfg/yolo.cfg" );
	// std::string weightfile = ofToDataPath( "yolo.weights" );
	// std::string nameslist = ofToDataPath( "cfg/names.list" );

	std::string cfgfile = ofToDataPath( "cfg/yolo-voc.cfg" );
	std::string weightfile = ofToDataPath( "yolo-voc.weights" );
	std::string nameslist = ofToDataPath( "cfg/voc.names" );	

	darknet.init( cfgfile, weightfile, nameslist );

#ifdef USE_WEBCAM
	video.setDeviceID( 0 );
	video.setDesiredFrameRate( 30 );
	video.initGrabber( 640, 480 );
#else
	player.load("/home/nono/Downloads/scrutton_st.mp4");
	player.setLoopState(OF_LOOP_NORMAL);
	player.play();
#endif

}

void ofApp::update()
{
#ifdef USE_WEBCAM
	video.update();
#else
	player.update();
#endif
}

void ofApp::draw()
{
    // detected objects with confidence < threshold are omitted
	float thresh = ofMap( ofGetMouseX(), 0, ofGetWidth(), 0, 1 );

    // if a detected object overlaps >maxOverlap with another detected
    // object with a higher confidence, it gets omitted
    float maxOverlap = 0.25;
    
	ofSetColor( 255 );
	bool isNewFrame = false;
	ofPixels pix;
#ifdef USE_WEBCAM
	video.draw( 0, 0 );
	if(video.isFrameNew()){
		isNewFrame = true;
		pix = video.getPixels();
	}
#else
	player.draw( 0, 0 );
	// if(player.isFrameNew()){
		isNewFrame = true;
		pix = player.getPixels();
	// }
#endif

	if( isNewFrame ) {
		std::vector< detected_object > detections = darknet.yolo( pix, thresh, maxOverlap );

		ofNoFill();	
		for( detected_object d : detections )
		{
			ofSetColor( d.color );
			glLineWidth( ofMap( d.probability, 0, 1, 0, 8 ) );
			ofNoFill();
			ofDrawRectangle( d.rect );
			ofDrawBitmapStringHighlight( d.label + ": " + ofToString(d.probability), d.rect.x, d.rect.y + 20 );
            
            // optionally, you can grab the 1024-length feature vector associated
            // with each detected object
            vector<float> & features = d.features;
		}
	}
}
