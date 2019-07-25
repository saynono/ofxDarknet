#include "ofxDarknetGo.h"


ofxDarknetGo::ofxDarknetGo() : ofxDarknet()
{
    active = -1;
    inverted = 1;
    noi = 1;
    nind = 5;
}

void ofxDarknetGo::setMouseActive(bool active) {
    if (active) {
        ofAddListener(ofEvents().mouseMoved, this, &ofxDarknetGo::mouseMoved);
        ofAddListener(ofEvents().mousePressed, this, &ofxDarknetGo::mousePressed);
    }else {
        ofRemoveListener(ofEvents().mouseMoved, this, &ofxDarknetGo::mouseMoved);
        ofRemoveListener(ofEvents().mousePressed, this, &ofxDarknetGo::mousePressed);
    }
}

void ofxDarknetGo::setNumRecommendations(int n) {
    nind = n;
}

image ofxDarknetGo::make_empty_image(int w, int h, int c)
{
    image out;
    out.data = 0;
    out.h = h;
    out.w = w;
    out.c = c;
    return out;
}

void ofxDarknetGo::rotate_image_cw(image im, int times)
{
    assert(im.w == im.h);
    times = (times + 400) % 4;
    int i, x, y, c;
    int n = im.w;
    for(i = 0; i < times; ++i){
        for(c = 0; c < im.c; ++c){
            for(x = 0; x < n/2; ++x){
                for(y = 0; y < (n-1)/2 + 1; ++y){
                    float temp = im.data[y + im.w*(x + im.h*c)];
                    im.data[y + im.w*(x + im.h*c)] = im.data[n-1-x + im.w*(y + im.h*c)];
                    im.data[n-1-x + im.w*(y + im.h*c)] = im.data[n-1-y + im.w*(n-1-x + im.h*c)];
                    im.data[n-1-y + im.w*(n-1-x + im.h*c)] = im.data[x + im.w*(n-1-y + im.h*c)];
                    im.data[x + im.w*(n-1-y + im.h*c)] = temp;
                }
            }
        }
    }
}

image ofxDarknetGo::float_to_image(int w, int h, int c, float *data)
{
    image out = make_empty_image(w,h,c);
    out.data = data;
    return out;
}

void ofxDarknetGo::flip_image(image a)
{
    int i,j,k;
    for(k = 0; k < a.c; ++k){
        for(i = 0; i < a.h; ++i){
            for(j = 0; j < a.w/2; ++j){
                int index = j + a.w*(i + a.h*(k));
                int flip = (a.w - j - 1) + a.w*(i + a.h*(k));
                float swap = a.data[flip];
                a.data[flip] = a.data[index];
                a.data[index] = swap;
            }
        }
    }
}

void ofxDarknetGo::string_to_board(char *s, float *board)
{
    int i, j;
    //memset(board, 0, 1*19*19*sizeof(float));
    int count = 0;
    for(i = 0; i < 91; ++i){
        char c = s[i];
        for(j = 0; j < 4; ++j){
            int me = (c >> (2*j)) & 1;
            int you = (c >> (2*j + 1)) & 1;
            if (me) board[count] = 1;
            else if (you) board[count] = -1;
            else board[count] = 0;
            ++count;
            if(count >= 19*19) break;
        }
    }
}

void ofxDarknetGo::board_to_string(char *s, float *board)
{
    int i, j;
    memset(s, 0, (19*19/4+1)*sizeof(char));
    int count = 0;
    for(i = 0; i < 91; ++i){
        for(j = 0; j < 4; ++j){
            int me = (board[count] == 1);
            int you = (board[count] == -1);
            if (me) s[i] = s[i] | (1<<(2*j));
            if (you) s[i] = s[i] | (1<<(2*j + 1));
            ++count;
            if(count >= 19*19) break;
        }
    }
}

void ofxDarknetGo::print_board(float *board, int swap, int *indexes)
{
    //FILE *stream = stdout;
    FILE *stream = stderr;
    int i,j,n;
    fprintf(stream, "\n\n");
    fprintf(stream, "   ");
    for(i = 0; i < 19; ++i){
        fprintf(stream, "%c ", 'A' + i + 1*(i > 7 && noi));
    }
    fprintf(stream, "\n");
    for(j = 0; j < 19; ++j){
        fprintf(stream, "%2d", (inverted) ? 19-j : j+1);
        for(i = 0; i < 19; ++i){
            int index = j*19 + i;
            if(indexes){
                int found = 0;
                for(n = 0; n < nind; ++n){
                    if(index == indexes[n]){
                        found = 1;
                        if(n == 0) fprintf(stream, " 1");
                        else if(n == 1) fprintf(stream, " 2");
                        else if(n == 2) fprintf(stream, " 3");
                        else if(n == 3) fprintf(stream, " 4");
                        else if(n == 4) fprintf(stream, " 5");
                    }
                }
                if(found) continue;
            }
            //if(board[index]*-swap > 0) fprintf(stream, "\u25C9 ");
            //else if(board[index]*-swap < 0) fprintf(stream, "\u25EF ");
            if(board[index]*-swap > 0) fprintf(stream, " O");
            else if(board[index]*-swap < 0) fprintf(stream, " X");
            else fprintf(stream, "  ");
        }
        fprintf(stream, "\n");
    }
}

void ofxDarknetGo::flip_board(float *board)
{
    int i;
    for(i = 0; i < 19*19; ++i){
        board[i] = -board[i];
    }
}

void ofxDarknetGo::getRecommendations(){
    int multi = 1;
    float *output = network_predict(net, board);
    copy_cpu(19*19, output, 1, move, 1);
    int i;
    if(multi){
        
        image bim = float_to_image(19, 19, 1, board);
        for(i = 1; i < 8; ++i){
            rotate_image_cw(bim, i);
            if(i >= 4) flip_image(bim);
            
            float *output = network_predict(net, board);
            image oim = float_to_image(19, 19, 1, output);
            
            if(i >= 4) flip_image(oim);
            rotate_image_cw(oim, -i);
            
            axpy_cpu(19*19, 1, output, 1, move, 1);
            
            if(i >= 4) flip_image(bim);
            rotate_image_cw(bim, -i);
        }
        scal_cpu(19*19, 1./8., move, 1);
        
    }
    for(i = 0; i < 19*19; ++i){
        if(board[i]) move[i] = 0;
    }
    
    int * indexes = new int[ nind ];
    int row, col;
    top_k(move, 19*19, nind, indexes);
    //print_board(board, color, indexes);
    recommendations.clear();
    probabilities.clear();
    for(i = 0; i < nind; ++i){
        int index = indexes[i];
        recommendations.push_back(index);
        probabilities.push_back(move[index]);
        row = index / 19;
        col = index % 19;
        //printf("%d: %c %d, %.2f%%\n", i+1, col + 'A' + 1*(col > 7 && noi), (inverted)?19 - row : row+1, move[index]*100);
    }
	delete indexes;
}

void ofxDarknetGo::makeMove(int row, int col) {
    if (abs(board[row*19 + col])==1) {
        ofLog(OF_LOG_ERROR, "Can't move there, there is already a piece there");
        return;
    }
    board[row*19 + col] = 1;
    flip_board(board);
    color = -color;
    getRecommendations();
}

void ofxDarknetGo::setDrawPosition(int x, int y, int width, int height) {
    box.set(x, y, width, height);
}

void ofxDarknetGo::drawBoard() {
    int swap = color;
    ofPushStyle();
    
    float margin = min(box.width, box.height) / 18.0;
    float rad = 0.48 * margin;
    
    ofColor clr = ofColor::orange;
    ofSetColor(clr);
    ofDrawRectangle(box.x, box.y, box.width, box.height);
    
    ofSetColor(0);
    for(int j = 0; j < 19; ++j){
        float x_ = ofMap(j, 0, 18, box.x, box.x+box.width);
        ofDrawLine(x_, box.y, x_, box.y+box.height);
        ofDrawBitmapString(ofToString(alphanum[j]), x_-3, box.y-rad-6);
    }
    for(int i = 0; i < 19; ++i){
        float y_ = ofMap(i, 0, 18, box.y, box.y+box.height);
        ofDrawLine(box.x, y_, box.x+box.width, y_);
        ofDrawBitmapString(ofToString(19-i), box.x-rad-20, y_+5 );
    }
    
    for(int j = 0; j < 19; ++j){
        for(int i = 0; i < 19; ++i){
            int index = j*19 + i;
            float x_ = ofMap(i, 0, 18, box.x, box.x+box.width);
            float y_ = ofMap(j, 0, 18, box.y, box.y+box.height);
            if(board[index]*-swap > 0) {
                ofFill();
                ofSetColor(255);
                ofDrawCircle(x_, y_, rad);
                ofNoFill();
                ofSetColor(40);
                ofDrawCircle(x_, y_, rad);
            }
            else if(board[index]*-swap < 0) {
                ofSetColor(0);
                ofFill();
                ofDrawCircle(x_, y_, rad);
            }
            else if(index == active) {
                ofSetColor(color == 1 ? ofColor(0,127) : ofColor(255,127));
                ofFill();
                ofDrawCircle(x_, y_, rad);
                ofNoFill();
                ofSetColor(255, 0, 0, 127);
                ofDrawCircle(x_, y_, rad);
            }
        }
    }
    ofPopStyle();
}

void ofxDarknetGo::drawRecommendations(int x, int y){
    ofPushMatrix();
    ofTranslate(x, y);
    ofDrawBitmapStringHighlight("Darknet recommends for "+ofToString((color==1?"black":"white"))+":", 0, 20);
    for (int i=0; i<recommendations.size(); i++) {
        int index = recommendations[i];
        int row = index / 19;
        int col = index % 19;
        ofDrawBitmapStringHighlight(" #"+ofToString(i+1)+": "+alphanum[col]+""+ofToString((inverted)?19 - row : row+1)+", "+ofToString(probabilities[i]*100)+"%", 0, 40 + i*18);
    }
    ofPopMatrix();
}

void ofxDarknetGo::nextAuto() {
    int index = recommendations[0];
    int row = index / 19;
    int col = index % 19;
    makeMove(row, col);
}

void ofxDarknetGo::setup(string cfgfile, string weightfile) {
    init( cfgfile, weightfile );    
    int multi = 1;
    srand(time(0));
    set_batch_network(&net, 1);
    board = (float*)calloc(19*19, sizeof(float));
    move = (float*)calloc(19*19, sizeof(float));
    color = 1;
    getRecommendations();
}

void ofxDarknetGo::mouseMoved(ofMouseEventArgs &evt){
    float mx = box.width / 18.0;
    float my = box.height / 18.0;
    if (!ofRectangle(box.x-0.5*mx, box.y-0.5*my, box.width+mx, box.height+my).inside(evt.x, evt.y)) {
        active = -1;
        return;
    }
    int x = ofClamp(ofMap(evt.x, box.x-0.5*mx, box.x+box.width-0.5*mx, 0, 18), 0, 18);
    int y = ofClamp(ofMap(evt.y, box.y-0.5*my, box.y+box.height-0.5*my, 0, 18), 0, 18);
    active = y*19 + x;
}

void ofxDarknetGo::mousePressed(ofMouseEventArgs &evt){
    if (active != -1) {
        int row = active / 19;
        int col = active % 19;
        makeMove(row, col);
    }
}
