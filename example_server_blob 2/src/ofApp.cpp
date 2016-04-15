#include "ofApp.h"
#include "ofxCrypto.h"
#include "ofxUDPManager.h"

#ifdef TARGET_OSX
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#endif

ofImage testImage;
ofBuffer decodedImage;

ofxUDPManager udpConnectionBroadcast;
ofxUDPManager moduleConnections[6];

char pixelsToSend[6][864];
char pixelsToSendTest[864];


//const int numLedsPerModule = 388;
const static int totalHeight = 24;
const static int totalWidth_72 = 72;
//const static int totalWidth_84 = 84;
//const int totalLeds_24x84 = 2016;
//const int totalLeds_24x72 = 1728;


void ofApp::setup(){
    ofSetFrameRate(60);
    ofxLibwebsockets::ServerOptions options = ofxLibwebsockets::defaultServerOptions();
    options.port = 9093;
    
    bool connected = server.setup( options );
    
    server.addListener(this);
    font.load("myriad.ttf", 20);
    messages.push_back("WebSocket server setup at "+ofToString( server.getPort() ) + ( server.usingSSL() ? " with SSL" : " without SSL") );
    locked = needToLoad = false;
    
    for (int i = 0; i < 864; i++) {
        pixelsToSendTest[i] = '\0';
    }
    
    /* Module Communication */
    udpConnectionBroadcast.Create();
    udpConnectionBroadcast.SetNonBlocking(true);
    udpConnectionBroadcast.Bind(6000);
    udpConnectionBroadcast.Connect("192.168.2.255", 8888);
    udpConnectionBroadcast.SetEnableBroadcast(true);
    std::string messageSent = "**";
    udpConnectionBroadcast.Send(messageSent.c_str(), messageSent.size());

    // WAIT
    ofSleepMillis(100);
    
    char udpMessage[1000];
    auto ret = udpConnectionBroadcast.Receive(udpMessage, 1000);
    
    while (udpMessage[0] != '\0') {
        
        ofxUDPManager udpConnectionRx;
        moduleConnections[((int)udpMessage[0]) - 100] = udpConnectionRx;
        moduleConnections[((int)udpMessage[0]) - 100].Create();
        moduleConnections[((int)udpMessage[0]) - 100].SetNonBlocking(true);
        moduleConnections[((int)udpMessage[0]) - 100].Bind(6000);
        string ip = "192.168.2." + ofToString((int)udpMessage[0]);
        cout << ip << endl;
        moduleConnections[((int)udpMessage[0]) - 100].Connect(ip.c_str(), 8888);
        moduleConnections[((int)udpMessage[0]) - 100].SetEnableBroadcast(false);
        std::fill_n(udpMessage, 1000, 0);
        auto ret = udpConnectionBroadcast.Receive(udpMessage, 1000);
        
    }
    
    ofSleepMillis(1000);
    //testImage.load("rainbow_texture2.jpg");
}

void ofApp::update(){
    if ( needToLoad ){
        testImage.load(decodedImage);
        needToLoad = false;
        locked = false;
    }
//    if ( needToLoad ){
//        const char * pixelData = decodedImage.getData();
//        testImage.setFromPixels((unsigned char*)decodedImage.getData(), 1212, 676, OF_IMAGE_COLOR_ALPHA);
//        needToLoad = false;
//        locked = false;
//    }
    for (int i = 0; i < 864; i++) {
        int level = pixelsToSendTest[i];
        level--;
        level %= 255;
        pixelsToSendTest[i] = level;
    }
}

void ofApp::draw() {
    ofBackground(0);


    if ( testImage.isAllocated() ) {
        testImage.draw(0,0);
        ofPixels pixels = testImage.getPixels();
        cout << pixels.size() << endl;
        for (int i = 0; i <= 5; i++) {
            int positionInBufferToSend = 0;
            for (int column = i*12; column < (i*12)+12; column+=2) {
                for (int row = 0; row < 8; row++) {
                    for (int x = column; x < column + 2; x++) {
                        for (int y = row*3; y < row*3 + 3; y++) {


                            pixelsToSend[i][3*positionInBufferToSend + 0] = pixels[3*(y*totalWidth_72 + x)+0];
                            pixelsToSend[i][3*positionInBufferToSend + 1] = pixels[3*(y*totalWidth_72 + x)+1];
                            pixelsToSend[i][3*positionInBufferToSend + 2] = pixels[3*(y*totalWidth_72 + x)+2];
                            
                            //cout << "X = " << x << ", Y = " << y << ", BufferPos Start = " << positionInBufferToSend << ", BitmapPos = " << y*totalWidth_72 + x << endl;
                            positionInBufferToSend++;
                        }
                    }
                    //cout << "" << endl;
                }
            }
        }
        //cout << "" << endl;
        for (int i = 0; i < 6; i++) {
            moduleConnections[i].Send(pixelsToSend[i], 864);
            ofSleepMillis(1);
        }
        for (int i = 0; i < 6; i++) {
            moduleConnections[i].Send("*", 1);
        }
    }
    ofDrawBitmapString("framerate is " + ofToString(ofGetFrameRate()) + " fps", 0, 100);
}

void ofApp::onConnect( ofxLibwebsockets::Event& args ){
    cout << "on connected" << endl;
}

void ofApp::onOpen( ofxLibwebsockets::Event& args ){
    cout << "new connection open" << endl;
    messages.push_back("New connection from " + args.conn.getClientIP() );
}

void ofApp::onClose( ofxLibwebsockets::Event& args ){
    cout << "on close" << endl;
    messages.push_back("Connection closed");
}

void ofApp::onIdle( ofxLibwebsockets::Event& args ){
    cout << "on idle" << endl;
}
`
void ofApp::onMessage( ofxLibwebsockets::Event& args ){
    if ( !args.isBinary && !locked ) {
        decodedImage.clear();
        decodedImage = ofxCrypto::base64_decode_to_buffer(args.message);
        needToLoad = true;
        locked = true;
    } else if (args.isBinary && !locked ) {
        decodedImage.clear();
        decodedImage.set(args.data.getData(), args.data.size());
        locked = true;
        needToLoad = true;
    }
}

void ofApp::onBroadcast( ofxLibwebsockets::Event& args ){
    cout << "got broadcast" << args.message << endl;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){}

void ofApp::keyReleased(int key){}

void ofApp::mouseMoved(int x, int y ){}

void ofApp::mouseDragged(int x, int y, int button){}

void ofApp::mousePressed(int x, int y, int button){}

void ofApp::mouseReleased(int x, int y, int button){}

void ofApp::windowResized(int w, int h){}

void ofApp::gotMessage(ofMessage msg){}

void ofApp::dragEvent(ofDragInfo dragInfo){}