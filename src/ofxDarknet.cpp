#include "ofxDarknet.h"

ofxDarknet::ofxDarknet()
{
    loaded = false;
    labelsAvailable = false;
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
    startThread();
    ofLogVerbose("ofxDarknet::init") << "All done.";
}


void ofxDarknet::update(ofEventArgs & a){
	if(analyzed.tryReceive(detectedObjects)){
		ofLogVerbose("ofxDarknet::update") << "Received new detectionObject: " << detectedObjects.objects.size() << std::endl;
	}
}

// std::vector< detected_object > ofxDarknet::yolo_nono( ofPixels & pix, float threshold /*= 0.24f */, float maxOverlap /*= 0.5f */ )
void ofxDarknet::yolo_nono( ofPixels & pix, float threshold /*= 0.24f */, float maxOverlap /*= 0.5f */ )
{
    if(pix.getWidth() * pix.getHeight() > 0){
        if(!bDetectionIsBusy){
            toAnalyze.send(pix);
        }else{
            ofLogVerbose("ofxDarknet::yolo_nono") << " >>>>>>>>>>>  Yolo busy.<<<<<<<<<<<";
        }
    }
}



// void activations::getImage(ofImage & img) {
//     ofPixels pix;
//     pix.allocate(rows, cols, OF_PIXELS_GRAY);
//     for (int i=0; i<rows*cols; i++) {
//         pix[i] = ofMap(acts[i], min, max, 0, 255);
//     }
//     img.setFromPixels(pix);
// }


//
//std::shared_ptr<image_t> ofxDarknet::convert( ofPixels & pix, std::shared_ptr<image_t> image_ptr )
//{
//	unsigned char *data = ( unsigned char * ) pix.getData();
//	int h = pix.getHeight();
//	int w = pix.getWidth();
//	int c = pix.getNumChannels();
//	int step = w * c;
//
////    std::shared_ptr<image_t> image_ptr(new image_t, [](image_t *img) { if (img->data) free(img->data); delete img; });
//	// image_ptr->data = convert( pix2 ).data;
//	if(w*h*c != image_ptr->w*image_ptr->h*image_ptr->c){
//		image_ptr->w = w;
//		image_ptr->h = h;
//		image_ptr->c = c;
//		image_ptr->data = (float *)calloc(w*h*c, sizeof(float));
//	}
//
//	// image im = make_image( w, h, c );
//	int i, j, k, count = 0;;
//
//	for( k = 0; k < c; ++k ) {
//		for( i = 0; i < h; ++i ) {
//			for( j = 0; j < w; ++j ) {
//				image_ptr->data[ count++ ] = data[ i*step + j*c + k ] / 255.;
//			}
//		}
//	}
//
//	return image_ptr;
//}


void ofxDarknet::convert( ofPixels & pix, image_t* image_ptr )
{
    unsigned char *data = ( unsigned char * ) pix.getData();
    int h = pix.getHeight();
    int w = pix.getWidth();
    int c = pix.getNumChannels();
    int step = w * c;

//    std::shared_ptr<image_t> image_ptr(new image_t, [](image_t *img) { if (img->data) free(img->data); delete img; });
    // image_ptr->data = convert( pix2 ).data;
    if(w*h*c != image_ptr->w*image_ptr->h*image_ptr->c){
        image_ptr->w = w;
        image_ptr->h = h;
        image_ptr->c = c;
        image_ptr->data = (float *)calloc(w*h*c, sizeof(float));
    }

    // image im = make_image( w, h, c );
    int i, j, k, count = 0;;

    for( k = 0; k < c; ++k ) {
        for( i = 0; i < h; ++i ) {
            for( j = 0; j < w; ++j ) {
                image_ptr->data[ count++ ] = data[ i*step + j*c + k ] / 255.;
            }
        }
    }
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
//    image_ptr = std::make_shared<image_t>();
    image_t imageToAnalyse;
    imageToAnalyse.w = 0;
    imageToAnalyse.h = 0;
    imageToAnalyse.c = 0;
    imageToAnalyse.data = 0;
    while(toAnalyze.receive(pix)){
        bDetectionIsBusy = true;
		float threshold = 0.24;
		int originalWidth = pix.getWidth();
		int originalHeight = pix.getHeight();
		ofPixels  pix2( pix );
	    if (pix2.getImageType() != OF_IMAGE_COLOR) {
	        pix2.setImageType(OF_IMAGE_COLOR);
	    }
	    if( pix2.getWidth() != detector->get_net_width() && pix2.getHeight() != detector->get_net_height() ) {
	        pix2.resize( detector->get_net_width(), detector->get_net_height() );
	    }
		// image im = convert( pix2 );
//		ofLog() << "net.n : " << net.n;
//		layer l = net.layers[ net.n - 1 ];

		// std::shared_ptr<image_t> imgToDetect = std::make_shared<image_t>();
		// image_t imgToDetect;
		// imgToDetect.data = im.data;
		// imgToDetect.w = pix2.getWidth();
		// imgToDetect.h = pix2.getHeight();
		// imgToDetect.c = 3;

	 //    std::shared_ptr<image_t> image_ptr(new image_t, [](image_t *img) { if (img->data) free(img->data); delete img; });
		// image_ptr->data = convert( pix2 ).data;
		// image_ptr->w = pix2.getWidth();
		// image_ptr->h = pix2.getHeight();
		// image_ptr->c = 3;
//		std::shared_ptr<image_t> image_ptr = convert( pix2 );
        convert( pix2, &imageToAnalyse );

	    std::vector<bbox_t> result_vec;
	    // detect_resized(std::shared_ptr<image_t>&, int&, int&, float&, bool)’
	    // detect_resized(image_t img, int init_w, int init_h, float thresh = 0.2, bool use_mean = false)
	    result_vec = detector->detect_resized(imageToAnalyse, originalWidth, originalHeight, threshold, true);  // true
//        detector->detect_resized(imageToAnalyse, originalWidth, originalHeight, threshold, true);  // true
		ofLogVerbose("ofxDarknet::threadedFunction") << " result_vec: " << result_vec.size();

        detected_objects objects;
        objects.pix = pix;
		for(auto res: result_vec){
            detected_object dectObj;
            dectObj.label = obj_names[res.obj_id];
            dectObj.rect = ofRectangle(res.x,res.y,res.w,res.h);
            dectObj.probability = res.prob;
            dectObj.id = res.track_id;
            objects.objects.push_back(dectObj);
			ofLogVerbose("ofxDarknet::threadedFunction") << "ID : " << res.track_id << "	RECT: " << res.x << " " << res.y << "  w: " << res.w << "  h: " << res.h << "		LabelID: " << obj_names[res.obj_id];
		}
        bDetectionIsBusy = false;



    	result_vec.clear();

#if __cplusplus>=201103
        analyzed.send(std::move(objects));
#else
        analyzed.send(objects);
#endif
	}
}


detected_objects ofxDarknet::getDetectedObjects(){
    return detectedObjects;
}