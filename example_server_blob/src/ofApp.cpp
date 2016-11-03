#include "ofApp.h"
#include "ofxCrypto.h"
#include "ofxUDPManager.h"
#include <typeinfo>

#ifdef TARGET_OSX
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#endif

/*
 TODO: Test 
 See if we can reasonably open up to sockets.
 
 */

ofImage testImage1;
ofImage testImage2;

ofBuffer decodedImage1;
ofBuffer decodedImage2;

ofxUDPManager udpConnectionBroadcast;

ofxUDPManager moduleConnectionsZone1[6];
ofxUDPManager moduleConnectionsZone2[7];


char pixelsToSendZone1[6][864];
char pixelsToSendZone2[7][864];

int cosValue = 0;

const static int totalHeight_24 = 24;

const static int totalWidth_68 = 68;
const static int totalWidth_80 = 80;

ofVideoPlayer myPlayer;
ofVideoPlayer myPlayerZone2;


int mode = 4;

int pixelCraweler = 0;

void ofApp::setup(){
    ofSetFrameRate(120);
    ofxLibwebsockets::ServerOptions options = ofxLibwebsockets::defaultServerOptions();
    options.port = 9093;
    
    bool connected = server.setup( options );
    
    server.addListener(this);
    font.load("myriad.ttf", 20);
    messages.push_back("WebSocket server setup at "+ofToString( server.getPort() ) + ( server.usingSSL() ? " with SSL" : " without SSL") );
    locked = needToLoad = false;
    
    
    /* Module Communication */
    udpConnectionBroadcast.Create();
    udpConnectionBroadcast.SetNonBlocking(true);
    udpConnectionBroadcast.Bind(6000);
    udpConnectionBroadcast.Connect("192.168.2.255", 8888);
    udpConnectionBroadcast.SetEnableBroadcast(true);
    std::string messageSent = "**";
    udpConnectionBroadcast.Send(messageSent.c_str(), messageSent.size());
    
    ofSleepMillis(100);
    
    char udpMessage[1000];
    auto ret = udpConnectionBroadcast.Receive(udpMessage, 1000);
    while (udpMessage[0] != '\0') {
        int num = ((int)udpMessage[0]) - 101;
        if (num < 7) {
            ofxUDPManager udpConnectionRx;
            moduleConnectionsZone2[((int)udpMessage[0]) - 101] = udpConnectionRx;
            moduleConnectionsZone2[((int)udpMessage[0]) - 101].Create();
            moduleConnectionsZone2[((int)udpMessage[0]) - 101].SetNonBlocking(true);
            moduleConnectionsZone2[((int)udpMessage[0]) - 101].Bind(6000);
            string ip = "192.168.2." + ofToString((int)udpMessage[0]);
            moduleConnectionsZone2[((int)udpMessage[0]) - 101].Connect(ip.c_str(), 8888);
            moduleConnectionsZone2[((int)udpMessage[0]) - 101].SetEnableBroadcast(false);
            std::fill_n(udpMessage, 1000, 0);
            auto ret = udpConnectionBroadcast.Receive(udpMessage, 1000);
        } else {
            ofxUDPManager udpConnectionRx;
            moduleConnectionsZone1[((int)udpMessage[0]) - 108] = udpConnectionRx;
            moduleConnectionsZone1[((int)udpMessage[0]) - 108].Create();
            moduleConnectionsZone1[((int)udpMessage[0]) - 108].SetNonBlocking(true);
            moduleConnectionsZone1[((int)udpMessage[0]) - 108].Bind(6000);
            string ip = "192.168.2." + ofToString((int)udpMessage[0]);
            cout << ip << endl;
            moduleConnectionsZone1[((int)udpMessage[0]) - 108].Connect(ip.c_str(), 8888);
            moduleConnectionsZone1[((int)udpMessage[0]) - 108].SetEnableBroadcast(false);
            std::fill_n(udpMessage, 1000, 0);
            auto ret = udpConnectionBroadcast.Receive(udpMessage, 1000);
        }
        cout << "" << endl;
    }
    ofSleepMillis(1000);
    
    testImage1.load("texture.png");
    testImage2.load("texture2.png");

}

void ofApp::update(){
    if ( needToLoad ) {
        testImage1.load(decodedImage1);
        testImage2.load(decodedImage2);
        needToLoad = false;
        locked = false;
    }
}

void ofApp::draw() {
    ofSetBackgroundColor(0.0);
    
    ofPixels pix1;

    if ( testImage1.isAllocated() ) {
        ofPixels pix;
        testImage1.draw(0, 0);
        pix = testImage1.getPixels();
        for (int i = 0; i < 288; i++) {
            pixelsToSendZone1[0][3*i] = pix[panel1Z1[i]+0];
            pixelsToSendZone1[0][3*i+1] = pix[panel1Z1[i]+1];
            pixelsToSendZone1[0][3*i+2] = pix[panel1Z1[i]+2];
        }
        for (int i = 0; i < 288; i++) {
            pixelsToSendZone1[1][3*i] = pix[panel2Z1[i]+0];
            pixelsToSendZone1[1][3*i+1] = pix[panel2Z1[i]+1];
            pixelsToSendZone1[1][3*i+2] = pix[panel2Z1[i]+2];

        }
        for (int i = 0; i < 288; i++) {
            pixelsToSendZone1[2][3*i] = pix[panel3Z1[i]+0];
            pixelsToSendZone1[2][3*i+1] = pix[panel3Z1[i]+1];
            pixelsToSendZone1[2][3*i+2] = pix[panel3Z1[i]+2];

        }
        for (int i = 0; i < 288; i++) {
            pixelsToSendZone1[3][3*i] = pix[panel4Z1[i]+0];
            pixelsToSendZone1[3][3*i+1] = pix[panel4Z1[i]+1];
            pixelsToSendZone1[3][3*i+2] = pix[panel4Z1[i]+2];
        }
        for (int i = 0; i < 288; i++) {
            pixelsToSendZone1[4][3*i] = pix[panel5Z1[i]+0];
            pixelsToSendZone1[4][3*i+1] = pix[panel5Z1[i]+1];
            pixelsToSendZone1[4][3*i+2] = pix[panel5Z1[i]+2];

        }
        for (int i = 0; i < 288; i++) {
            pixelsToSendZone1[5][3*i] = pix[panel6Z1[i]+0];
            pixelsToSendZone1[5][3*i+1] = pix[panel6Z1[i]+1];
            pixelsToSendZone1[5][3*i+2] = pix[panel6Z1[i]+2];
        }
        for (int panel = 0; panel < 6; panel++) {
            moduleConnectionsZone1[panel].Send(pixelsToSendZone1[panel], 864);
            ofSleepMillis(1);
        }
        for (int panel = 0; panel < 6; panel++) {
            moduleConnectionsZone1[panel].Send("*", 1);
        }
    }
    if ( testImage2.isAllocated()) {
        ofPixels pix;
        testImage2.draw(0, 24);
        pix = testImage2.getPixels();
        for (int i = 0; i < 288; i++) {
            pixelsToSendZone2[0][3*i] = pix[panel1Z2[i]+0];
            pixelsToSendZone2[0][3*i+1] = pix[panel1Z2[i]+1];
            pixelsToSendZone2[0][3*i+2] = pix[panel1Z2[i]+2];
        }
        for (int i = 0; i < 288; i++) {
            pixelsToSendZone2[1][3*i] = pix[panel2Z2[i]+0];
            pixelsToSendZone2[1][3*i+1] = pix[panel2Z2[i]+1];
            pixelsToSendZone2[1][3*i+2] = pix[panel2Z2[i]+2];

        }
        for (int i = 0; i < 288; i++) {
            pixelsToSendZone2[2][3*i] = pix[panel3Z2[i]+0];
            pixelsToSendZone2[2][3*i+1] = pix[panel3Z2[i]+1];
            pixelsToSendZone2[2][3*i+2] = pix[panel3Z2[i]+2];

        }
        for (int i = 0; i < 288; i++) {
            pixelsToSendZone2[3][3*i] = pix[panel4Z2[i]+0];
            pixelsToSendZone2[3][3*i+1] = pix[panel4Z2[i]+1];
            pixelsToSendZone2[3][3*i+2] = pix[panel4Z2[i]+2];

        }
        for (int i = 0; i < 288; i++) {

            pixelsToSendZone2[4][3*i] = pix[panel5Z2[i]+0];
            pixelsToSendZone2[4][3*i+1] = pix[panel5Z2[i]+1];
            pixelsToSendZone2[4][3*i+2] = pix[panel5Z2[i]+2];

        }
        for (int i = 0; i < 288; i++) {
            pixelsToSendZone2[5][3*i] = pix[panel6Z2[i]+0];
            pixelsToSendZone2[5][3*i+1] = pix[panel6Z2[i]+1];
            pixelsToSendZone2[5][3*i+2] = pix[panel6Z2[i]+2];
        }
        for (int i = 0; i < 288; i++) {
            pixelsToSendZone2[6][3*i] = pix[panel7Z2[i]+0];
            pixelsToSendZone2[6][3*i+1] = pix[panel7Z2[i]+1];
            pixelsToSendZone2[6][3*i+2] = pix[panel7Z2[i]+2];
        }
        for (int panel = 0; panel < 7; panel++) {
            moduleConnectionsZone2[panel].Send(pixelsToSendZone2[panel], 864);
            ofSleepMillis(1);
        }
        for (int panel = 0; panel < 7; panel++) {
            moduleConnectionsZone2[panel].Send("*", 1);
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
        decodedImage1.clear();
        decodedImage2.clear();
        decodedImage1 = ofxCrypto::base64_decode_to_buffer(ofSplitString(args.message, "STARTNEWIMAGE")[0]);
        decodedImage2 = ofxCrypto::base64_decode_to_buffer(ofSplitString(args.message, "STARTNEWIMAGE")[1]);
        needToLoad = true;
        locked = true;
    } else if (args.isBinary && !locked ) {
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
    } else if (key == 53) {
        mode = 5;
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
