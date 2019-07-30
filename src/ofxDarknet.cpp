#include "ofxDarknet.h"

ofxDarknet::ofxDarknet()
{
    loaded = false;
    labelsAvailable = false;
    startThread();
} 

ofxDarknet::~ofxDarknet()
{
	toAnalyze.close();
	analyzed.close();
	waitForThread(true);
    ofRemoveListener(ofEvents().update, this, &ofxDarknet::update);
}

void ofxDarknet::init( std::string cfgfile, std::string weightfile, std::string nameslist )
{
	ofAddListener(ofEvents().update, this, &ofxDarknet::update);
    detector = std::make_shared<Detector>(cfgfile, weightfile);
    obj_names = objectsNamesFromFile(nameslist);

    ofLogVerbose("ofxDarknet::init") << "All done.";
}


void ofxDarknet::update(ofEventArgs & a){
	detected_object detectionObject;
	if(analyzed.tryReceive(detectionObject)){
		ofLogVerbose("ofxDarknet::update") << " Received new detectionObject" << std::endl;
	}
}

// std::vector< detected_object > ofxDarknet::yolo_nono( ofPixels & pix, float threshold /*= 0.24f */, float maxOverlap /*= 0.5f */ )
void ofxDarknet::yolo_nono( ofPixels & pix, float threshold /*= 0.24f */, float maxOverlap /*= 0.5f */ )
{
	toAnalyze.send(pix);
}



// void activations::getImage(ofImage & img) {
//     ofPixels pix;
//     pix.allocate(rows, cols, OF_PIXELS_GRAY);
//     for (int i=0; i<rows*cols; i++) {
//         pix[i] = ofMap(acts[i], min, max, 0, 255);
//     }
//     img.setFromPixels(pix);
// }



image ofxDarknet::convert( ofPixels & pix )
{
	unsigned char *data = ( unsigned char * ) pix.getData();
	int h = pix.getHeight();
	int w = pix.getWidth();
	int c = pix.getNumChannels();
	int step = w * c;
	image im = make_image( w, h, c );
	int i, j, k, count = 0;;

	for( k = 0; k < c; ++k ) {
		for( i = 0; i < h; ++i ) {
			for( j = 0; j < w; ++j ) {
				im.data[ count++ ] = data[ i*step + j*c + k ] / 255.;
			}
		}
	}

	return im;
}

ofPixels ofxDarknet::convert( image & im )
{
	unsigned char *data = ( unsigned char* ) calloc( im.w*im.h*im.c, sizeof( char ) );
	int i, k;
	for( k = 0; k < im.c; ++k ) {
		for( i = 0; i < im.w*im.h; ++i ) {
			data[ i*im.c + k ] = ( unsigned char ) ( 255 * im.data[ i + k*im.w*im.h ] );
		}
	}

	ofPixels pix;
	pix.setFromPixels( data, im.w, im.h, im.c );
	return pix;
}



std::vector<std::string> ofxDarknet::objectsNamesFromFile(std::string const filename) {
    std::ifstream file(filename);
    std::vector<std::string> file_lines;
    if (!file.is_open()) return file_lines;
    for(std::string line; getline(file, line);) file_lines.push_back(line);
    return file_lines;
}



void ofxDarknet::threadedFunction(){
    ofPixels pix;
    while(toAnalyze.receive(pix)){
		float threshold = 0.24;
		int originalWidth = pix.getWidth();
		int originalHeight = pix.getHeight();
		ofPixels  pix2( pix );
	    if (pix2.getImageType() != OF_IMAGE_COLOR) {
	        pix2.setImageType(OF_IMAGE_COLOR);
	    }
	    if( pix2.getWidth() != net.w && pix2.getHeight() != net.h ) {
	        pix2.resize( net.w, net.h );
	    }
		image im = convert( pix2 );
		layer l = net.layers[ net.n - 1 ];

		// std::shared_ptr<image_t> imgToDetect = std::make_shared<image_t>();
		image_t imgToDetect;
		imgToDetect.data = im.data;
		imgToDetect.w = pix2.getWidth();
		imgToDetect.h = pix2.getHeight();
		imgToDetect.c = 3;

	    std::vector<bbox_t> result_vec;
	    // detect_resized(std::shared_ptr<image_t>&, int&, int&, float&, bool)’
	    // detect_resized(image_t img, int init_w, int init_h, float thresh = 0.2, bool use_mean = false)
	    result_vec = detector->detect_resized(imgToDetect, originalWidth, originalHeight, threshold, true);  // true
		ofLogVerbose("ofxDarknet::threadedFunction") << " result_vec: " << result_vec.size();
		for(auto res: result_vec){
			ofLogVerbose("ofxDarknet::threadedFunction") << "ID : " << res.track_id << "	RECT: " << res.x << " " << res.y << "  w: " << res.w << "  h: " << res.h << "		LabelID: " << res.obj_id;
		}
		detected_object dectObj;

#if __cplusplus>=201103
        analyzed.send(std::move(dectObj));
#else
        analyzed.send(dectObj);
#endif
	}
}
