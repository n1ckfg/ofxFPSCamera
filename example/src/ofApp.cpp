#include "ofApp.h"

/*****************************************************
// Based on ofxFPSCamera example by Ivaylo Getov, 2014
*****************************************************/

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetVerticalSync(true);
	ofSetFrameRate(60);
	
    
    
	camera.setup();
    
    camera.target(ofVec3f(1,0,0));

    
    for (int i = 0; i < 50; i++) {
        cylinders[i].set(20,100);
        cylinders[i].setPosition(ofRandom(0,ofGetWidth()), 0, ofRandom(-600,600));
    }
  
    
    ofHideCursor();
    ofSetWindowPosition(ofGetScreenWidth()/2 - ofGetWidth()/2, ofGetScreenHeight()/2 - ofGetHeight()/2);
}

//--------------------------------------------------------------
void ofApp::update(){
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackground(0);
	
	camera.begin();
    

    ofSetColor(100);
    int step = 20;
    for (int i = 0; i < step; i++) {
        ofDrawLine(i*ofGetWidth()/step, -50, -ofGetWidth(), i*ofGetWidth()/step, -50, ofGetWidth());
        ofDrawLine(0, -50, i*ofGetWidth()/step, ofGetWidth(), -50, i*ofGetWidth()/step);
        ofDrawLine(0, -50, -i*ofGetWidth()/step, ofGetWidth(), -50, -i*ofGetWidth()/step);
    }
    
    ofSetColor(255);
    for (int i = 0; i < 50; i++) {
        cylinders[i].drawWireframe();
    }

    
	camera.end();
	
    
    
	ofSetColor(255);
	ofDrawBitmapString("use mouse to look around\nw: forward\ns: backwards\na: strafe left\nd: strafe right\nq/e: move down/up\nshift: run\n\nspace bar: reset camera to (0,0,0)\nt: toggle movement ease-in\nz: toggle fly mode\nf: toggle full-screen\nsee ofApp.cpp for available methods and vars", ofPoint(30, 30));
    
    if (camera.easeIn) {
        ofDrawBitmapString("easing is ON", ofPoint(30, ofGetHeight()-30));
    } else {
        ofDrawBitmapString("easing is OFF", ofPoint(30, ofGetHeight()-30));
    }
    if (camera.flyMode) {
        ofDrawBitmapString("fly mode is ON", ofPoint(30, ofGetHeight()-15));
    } else {
        ofDrawBitmapString("fly mode is OFF", ofPoint(30, ofGetHeight()-15));
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch (key) {
        case ' ':
            camera.setPosition(0, 0, 0);
            camera.setCamHeight(0.0);
            camera.target(ofVec3f(1,0,0));
            break;
            
        case 'f':
            ofToggleFullscreen();
            break;
            
        case 't':
            camera.easeIn = !camera.easeIn;
            break;
            
        case 'z':
            camera.flyMode = !camera.flyMode;
            break;
            
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}
