#include "ofApp.h"

void ofApp::setup() 
{

	ofSetLogLevel(OF_LOG_VERBOSE);
	// std::string cfgfile = ofToDataPath( "cfg/yolo9000.cfg" );
	// std::string weightfile = ofToDataPath( "yolo9000.weights" );
	// std::string nameslist = ofToDataPath( "cfg/9k.names" );

	// std::string cfgfile = ofToDataPath( "cfg/yolo.cfg" );
	// std::string weightfile = ofToDataPath( "yolo.weights" );
	// std::string nameslist = ofToDataPath( "cfg/names.list" );

	// std::string cfgfile = ofToDataPath( "cfg/yolo-voc.cfg" );
	// std::string weightfile = ofToDataPath( "yolo-voc.weights" );
	// std::string nameslist = ofToDataPath( "cfg/voc.names" );	

    std::string  nameslist = "data/cfg/coco.names";
    std::string  cfgfile = "data/cfg/yolov3.cfg";
    std::string  weightfile = "data/yolov3.weights";


	darknet.init( cfgfile, weightfile, nameslist );

#ifdef USE_WEBCAM
	video.setDeviceID( 0 );
	video.setDesiredFrameRate( 30 );
	video.initGrabber( 640, 480 );
	ofSetWindowShape(video.getWidth(),video.getHeight());
#else
	player.load("/home/nono/Downloads/scrutton_st.mp4");
	player.setLoopState(OF_LOOP_NORMAL);
	player.play();
	ofSetWindowShape(player.getWidth(),player.getHeight());
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
		currentFrame++;
	}
#else
	player.draw( 0, 0 );
	if(currentFrame!=player.getCurrentFrame()){
        currentFrame=player.getCurrentFrame();
        isNewFrame = true;
        pix = player.getPixels();
    }

	#endif

	if( isNewFrame ) {
        darknet.yolo_nono(pix, thresh, maxOverlap);
    }
    {
		// auto pixRes = darknet.yolo_nono( pix, thresh, maxOverlap );
		// ofImage img(pixRes);
		// img.draw(0,0,pixRes.getWidth(),pixRes.getHeight());
		// ofLog() << pixRes.getWidth() << "   " << pixRes.getHeight();

		std::vector< detected_object > detections = darknet.getDetectedObjects().objects;// = darknet.yolo_nono( pix, thresh, maxOverlap );
		ofNoFill();	
		for( detected_object d : detections )
		{
			ofSetColor( ofColor::azure );
			ofNoFill();
			ofDrawRectangle( d.rect );
			ofDrawBitmapStringHighlight( ofToString(d.id) + "    " + d.label + ": " + ofToString(d.probability,3), d.rect.x+3, d.rect.y + 18 );
            glLineWidth( 6 );
			ofDrawLine(d.rect.getBottomLeft(), glm::vec2(d.rect.x+d.rect.getWidth()*d.probability,d.rect.getBottom()));
            glLineWidth( 1 );

            // optionally, you can grab the 1024-length feature vector associated
            // with each detected object
//            vector<float> & features = d.features;
		}

		ofDrawBitmapStringHighlight("Detections: " + ofToString(detections.size()), 20, 30 );
		ofDrawBitmapStringHighlight("Threshold: " + ofToString(thresh,2), 20, 50 );

	}
}
