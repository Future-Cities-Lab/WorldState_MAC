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

int cosValue = 0;

const static int totalHeight = 24;
const static int totalWidth_72 = 72;

ofVideoPlayer myPlayer;

int mode = 4;

int pixelCraweler = 0;

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
    testImage.load("rainbow_texture2.jpg");
    ofPixels pix;
    pix.allocate(72, 24, 3);
    text.allocate(pix);


    myPlayer.loadMovie("movie.mp4");
    myPlayer.play();

}

void ofApp::update(){
    myPlayer.update(); // get all the new frames
    
    if ( needToLoad ){
        testImage.load(decodedImage);
        needToLoad = false;
        locked = false;
    }
    
    for (int i = 0; i < 864; i++) {
        int level = pixelsToSendTest[i];
        level--;
        level %= 255;
        pixelsToSendTest[i] = level;
    }

    cosValue++;

    if (mode == 0 || mode == 2) {
        cosValue%=24;
    } else if (mode == 3 || mode == 4) {
        cosValue%=72;
    }

    ofSleepMillis(200);
}

void ofApp::draw() {
    ofSetBackgroundColor(0.0);
    
  // SIN WAVE
    ofPixels pix;

    if (mode == 0) {
        for (int i = 0; i < 24; i++) {
            float co = ofMap(cosValue, 0, 24, 0.0, 2*PI);
            cosValue++;
            cosValue%=24;
            ofSetColor(ofMap(cos(co), -1.0, 1.0, 0, 255.0));
            ofFill();
            ofDrawLine(0, i, 74, i);
        }
        text.loadScreenData(0, 0, 72, 24);
        text.readToPixels(pix);
    } else if (mode == 1 || mode == 2 || mode == 3) {
        text.loadScreenData(0, 0, 72, 24);
        text.readToPixels(pix);
    } else if (mode == 4) {
        for (int i = 0; i < 72; i++) {
            float co = ofMap(cosValue, 0, 72, 0.0, 2*PI);
            cosValue++;
            cosValue%=72;
            ofSetColor(ofMap(cos(co), -1.0, 1.0, 0, 255.0));
            ofFill();
            ofDrawLine(i, 0, i, 24);
        }
        text.loadScreenData(0, 0, 72, 24);
        text.readToPixels(pix);
    }

    ofPixels pixels;
    
    if (mode == 0 || mode == 4) {
        pixels = pix;
    } else if (mode == 2) {
        pixels = pix;
        for (int j = 0; j < 72; j++) {
            pixels[3*(j + cosValue*72)+0] = 255;
            pixels[3*(j + cosValue*72)+1] = 255;
            pixels[3*(j + cosValue*72)+2] = 255;
        }
    } else if (mode == 1) {
        myPlayer.draw(0, 0);
        if (myPlayer.isFrameNew()){
            pixels = myPlayer.getPixels();
        }
    } else if (mode == 3) {
        pixels = pix;
        for (int j = 0; j < 24; j++) {
            pixels[3*(cosValue + j*72)+0] = 255;
            pixels[3*(cosValue + j*72)+1] = 255;
            pixels[3*(cosValue + j*72)+2] = 255;
        }
    }

    if (myPlayer.isFrameNew() || mode != 1) {
        for (int i = 0; i < 288; i++) {
            pixelsToSend[0][3*i] = pixels[panel1[i]+0];
            pixelsToSend[0][3*i+1] = pixels[panel1[i]+1];
            pixelsToSend[0][3*i+2] = pixels[panel1[i]+2];
        }
        for (int i = 0; i < 288; i++) {
            pixelsToSend[1][3*i] = pixels[panel2[i]+0];
            pixelsToSend[1][3*i+1] = pixels[panel2[i]+1];
            pixelsToSend[1][3*i+2] = pixels[panel2[i]+2];

        }
        for (int i = 0; i < 288; i++) {
            pixelsToSend[2][3*i] = pixels[panel3[i]+0];
            pixelsToSend[2][3*i+1] = pixels[panel3[i]+1];
            pixelsToSend[2][3*i+2] = pixels[panel3[i]+2];

        }
        for (int i = 0; i < 288; i++) {
            pixelsToSend[3][3*i] = pixels[panel4[i]+0];
            pixelsToSend[3][3*i+1] = pixels[panel4[i]+1];
            pixelsToSend[3][3*i+2] = pixels[panel4[i]+2];

        }
        for (int i = 0; i < 288; i++) {

            pixelsToSend[4][3*i] = pixels[panel5[i]+0];
            pixelsToSend[4][3*i+1] = pixels[panel5[i]+1];
            pixelsToSend[4][3*i+2] = pixels[panel5[i]+2];

        }
        for (int i = 0; i < 288; i++) {
            pixelsToSend[5][3*i] = pixels[panel6[i]+0];
            pixelsToSend[5][3*i+1] = pixels[panel6[i]+1];
            pixelsToSend[5][3*i+2] = pixels[panel6[i]+2];
        }
        for (int panel = 0; panel < 6; panel++) {
            moduleConnections[panel].Send(pixelsToSend[panel], 864);
            ofSleepMillis(1);
        }
        for (int panel = 0; panel < 6; panel++) {
            moduleConnections[panel].Send("*", 1);
        }
    }
}

void ofApp::onConnect( ofxLibwebsockets::Event& args ) {
    cout << "on connected" << endl;
}

void ofApp::onOpen( ofxLibwebsockets::Event& args ) {
    cout << "new connection open" << endl;
    messages.push_back("New connection from " + args.conn.getClientIP() );
}

void ofApp::onClose( ofxLibwebsockets::Event& args ) {
    cout << "on close" << endl;
    messages.push_back("Connection closed");
}

void ofApp::onIdle( ofxLibwebsockets::Event& args ) {
    cout << "on idle" << endl;
}

void ofApp::onMessage( ofxLibwebsockets::Event& args ) {
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

void ofApp::onBroadcast( ofxLibwebsockets::Event& args ) {
    cout << "got broadcast" << args.message << endl;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    if (key == 48) {
        mode = 0;
    } else if (key == 49) {
        mode = 1;
    } else if (key == 50) {
        mode = 2;
    } else if (key == 51) {
        mode = 3;
    } else if (key == 52) {
        mode = 4;
    }
}
void ofApp::keyReleased(int key){}
void ofApp::mouseMoved(int x, int y ){}
void ofApp::mouseDragged(int x, int y, int button){}
void ofApp::mousePressed(int x, int y, int button){}
void ofApp::mouseReleased(int x, int y, int button){}
void ofApp::windowResized(int w, int h){}
void ofApp::gotMessage(ofMessage msg){}
void ofApp::dragEvent(ofDragInfo dragInfo){}