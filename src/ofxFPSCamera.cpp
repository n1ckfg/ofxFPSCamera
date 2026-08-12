/*
 *
 *  ofxFPSCamera.cpp
 *
 *  by Ivaylo Getov, 2014
 *
 *
 *
 *  Derived from ofxGameCamera, created by James George and FlightPhase
 *
 */

#include "ofxFPSCamera.h"

// The infinite mouse is handled by GLFW, which every desktop openFrameworks window
// is built on. GLFW ships with oF (libs/glfw) on macOS and Windows and comes from
// pkg-config on Linux, so the header is on the include path of a normal oF project.
#if defined(TARGET_GLFW_WINDOW) && defined(__has_include)
    #if __has_include(<GLFW/glfw3.h>)
        #include <GLFW/glfw3.h>
        #define OFX_FPSCAMERA_HAS_GLFW 1
    #endif
#endif

#ifdef OFX_FPSCAMERA_HAS_GLFW
namespace {
    // oF hands back the underlying GLFWwindow* through the generic window context
    // pointer. Returns null for windows that aren't GLFW-backed (ofAppEGLWindow on
    // the Raspberry Pi, ofAppNoWindow), which is the "unsupported" case.
    GLFWwindow * getGLFWWindowPtr(){
        ofAppBaseWindow * window = ofGetWindowPtr();
        return window == nullptr ? nullptr : static_cast<GLFWwindow *>(window->getWindowContext());
    }
}
#endif


ofxFPSCamera::ofxFPSCamera() {
	unsavedChanges = false;
	sensitivityX = 0.10f;
	sensitivityY = 0.10f;

	speed = 8.0f;
	
	lastMouse = ofVec2f(0,0);
    
	usemouse = true;
	autosavePosition = false;
	useArrowKeys = false;
	applyTranslation = true;
    
	justResetAngles = false;

	
	cameraPositionFile =  "_gameCameraPosition.xml";
    eventsRegistered = false;
    
    camHeight = 0.0;
    totUpDown = 0.0;
    
    upAngle = -30.0;  // must be negative
    downAngle = 30.0; // must be positive
    
    keepTurning = false;
    
    easeIn = true;
    speedMod = 0.1;
    accel = 0.3;
    
    movKey = false;
    forw = false;
    back = false;
    left = false;
    rigt = false;
    strafe = true;
    
    clipPos = ofVec3f(0,0,0);
    isClipped = false;

    infiniteMouse = true;
    rawMouseMotion = false;
    infiniteMouseActive = false;
}
ofxFPSCamera::~ofxFPSCamera(){
    if(infiniteMouseActive){                                                // never leave the app with a captured, invisible cursor
        setCursorCaptured(false);
        infiniteMouseActive = false;
    }

    if(eventsRegistered){
        ofRemoveListener(ofEvents().update, this, &ofxFPSCamera::update);
        ofRemoveListener(ofEvents().keyPressed, this, &ofxFPSCamera::keyPressed);
        ofRemoveListener(ofEvents().keyReleased , this, &ofxFPSCamera::keyReleased);
        eventsRegistered = false;
    }
}

void ofxFPSCamera::setup(){
    if(!eventsRegistered){
        ofAddListener(ofEvents().update, this, &ofxFPSCamera::update);
        ofAddListener(ofEvents().keyPressed, this, &ofxFPSCamera::keyPressed);
        ofAddListener(ofEvents().keyReleased , this, &ofxFPSCamera::keyReleased);
        eventsRegistered = true;
    }
    
    /***********************************************************/
    // Had to add this for some reason or it wouldn't let me start fresh without loading an xml
    // -ivaylo
    if (!autosavePosition) {
        reset();
    }
    /***********************************************************/

    if(infiniteMouse && !isInfiniteMouseSupported()){
        ofLogWarning("ofxFPSCamera") << "infinite mouse needs a GLFW-backed window; "
                                     << "the mouse will stop at the edge of the screen";
    }
}

void ofxFPSCamera::update(ofEventArgs& args){
	bool rotationChanged = false;
	bool positionChanged = false;
	
	//lastRot == lastRot << this is nananers check
	if( (lastRot == lastRot && lastRot != getOrientationQuat()) ||
	    (lastPos == lastPos && lastPos != getPosition()))
	{
		movedManually();
	}

	if(applyTranslation){
		if(forw){
			if (easeIn) {
                dolly(-speedMod);
                speedMod+=accel;
                if (speedMod>speed) {speedMod=speed;}
            } else {
                dolly(-speed);
            }
            ofVec3f curPos(getPosition());                                      //Constant camera height
            curPos.y = camHeight;                                               //
            setPosition(curPos);                                                //
			positionChanged = true;
		}
		
		if(back){
            if (easeIn) {
                dolly(speedMod);
                speedMod+=accel;
                if (speedMod>speed) {speedMod=speed;}
            } else {
                dolly(speed);
            }
            ofVec3f curPos(getPosition());                                      //Constant camera height
            curPos.y = camHeight;                                               //
            setPosition(curPos);                                                //
			positionChanged = true;
		}
		
		if(left && strafe ){
            if (easeIn) {
                truck(-speedMod);
                speedMod+=accel;
                if (speedMod>speed) {speedMod=speed;}
            } else {
                truck(-speed);
            }
			positionChanged = true;
		}
		
		if(rigt && strafe ){
            if (easeIn) {
                truck(speedMod);
                speedMod+=accel;
                if (speedMod>speed) {speedMod=speed;}
            } else {
                truck(speed);
            }
			positionChanged = true;
		}
        
        
        ///////////////////////////////////////////////////
        //
        // ToDo: Implement method to determine clipping
        /*
         
        ofVec3f curPos(getPosition());
         
         if (curPos.z < something) {
            curPos.z = something;
            clip(curPos);        
         }
         
        */
        ///////////////////////////////////////////////////
        
        if (isClipped) {
            setPosition(clipPos);
            positionChanged = true;
            isClipped = false;
            clipPos = ofVec3f(0,0,0);
        }
        
        
	}
	
	if(positionChanged){
		currLookTarget = getPosition() + getLookAtDir();
	}
    
    ofVec2f mouse;
    if (usemouse) {
        mouse = ofVec2f(ofGetMouseX(), ofGetMouseY());
    } else {
        mouse = ofVec2f(0, 0);
    }
    
    //ofVec2f mouse( ofGetMouseX(), ofGetMouseY() );
	if(!justResetAngles){
		if(usemouse){
            float dx = (mouse.x - lastMouse.x) * sensitivityX;
			float dy = (mouse.y - lastMouse.y) * sensitivityY;
            
            if (keepTurning && ofGetElapsedTimeMillis()>2000) {                 // when mouse hits edge of screen, keep turning in that direction
                if (dx==0 && mouse.x <= -1 * (ofGetWindowPositionX() - 5) ) {
                    dx = -speed/2 * sensitivityX;
                }
                if (dx==0 && mouse.x >= (ofGetScreenWidth() - ofGetWindowPositionX() - 5)) {
                    dx = speed/2 * sensitivityX;
                }
            }
            
            if (totUpDown+dy > downAngle) {                                     // Limit up/down look. Defaults to +/- 30deg
                //dy = 0;
                dy = downAngle - totUpDown;
                totUpDown+=dy;
            } else if (totUpDown+dy < upAngle) {
                //dy = 0;
                dy = upAngle - totUpDown;
                totUpDown+=dy;
            } else {
                totUpDown+=dy;
            }

			currLookTarget.rotate(-dx, getPosition(), currentUp);
			ofVec3f sideVec = (currentUp).getCrossed(currLookTarget - getPosition());
			lookAt(currLookTarget, currentUp);

			currLookTarget.rotate(dy, getPosition(), sideVec);
            currentUp = ofVec3f(0,1,0);                                         // Maintain UP direction relative to world coordinates
            lookAt(currLookTarget, currentUp);

			rotationChanged = true;
		}
	}
	lastMouse = mouse;
    justResetAngles = false;
	lastRot = getOrientationQuat();
	lastPos = getPosition();
    
    /***********************************************************/
    // Infinite mouse AKA don't limit mouse at ends of screen.
    //
    // The old per-OS code warped the cursor back to the middle of the window every
    // time it reached a screen edge (CGWarpMouseCursorPosition on macOS, SetCursorPos
    // on Windows, nothing at all on Linux). GLFW does the same job natively on all
    // three platforms, so there is now one code path instead of three. See
    // updateInfiniteMouse() below.
    updateInfiniteMouse();
    // End Infinite Mouse
    /***********************************************************/


	//did we make a change?
	unsavedChanges |= positionChanged || rotationChanged;
	
	//if we didnt make a change this frame, save. this helps not saving every frame when moving around
	if( !rotationChanged && !positionChanged && unsavedChanges && autosavePosition){
		saveCameraPosition();
	}
	/*
	 cout << "STATS:" << endl
	 << "	position" << getPosition() << endl
	 << "	current up vec " << currentUp << endl
	 << "	current look " << currLookTarget << endl
	 << "	vec to look " << (currLookTarget-getPosition()) << endl;
	 */
	
}

void ofxFPSCamera::keyPressed(ofKeyEventArgs& args){
    bool isW = (args.key == 'w' || args.key == 'W');
    bool isA = (args.key == 'a' || args.key == 'A');
    bool isS = (args.key == 's' || args.key == 'S');
    bool isD = (args.key == 'd' || args.key == 'D');

    if ((isW && !useArrowKeys) || (args.key == OF_KEY_UP && useArrowKeys)) {
        forw = true;
    }
    if ((isS && !useArrowKeys) || (args.key == OF_KEY_DOWN && useArrowKeys)) {
        back = true;
    }
    if ((isA && !useArrowKeys) || (args.key == OF_KEY_LEFT && useArrowKeys)) {
        left = true;
    }
    if ((isD && !useArrowKeys) || (args.key == OF_KEY_RIGHT && useArrowKeys)) {
        rigt = true;
    }
    
    if (forw || back || left || rigt) {
        movKey = true;
    }
}

void ofxFPSCamera::keyReleased(ofKeyEventArgs& args){
    bool isW = (args.key == 'w' || args.key == 'W');
    bool isA = (args.key == 'a' || args.key == 'A');
    bool isS = (args.key == 's' || args.key == 'S');
    bool isD = (args.key == 'd' || args.key == 'D');

    if ((isW && !useArrowKeys) || (args.key == OF_KEY_UP && useArrowKeys)) {
        forw = false;
    }
    if ((isS && !useArrowKeys) || (args.key == OF_KEY_DOWN && useArrowKeys)) {
        back = false;
    }
    if ((isA && !useArrowKeys) || (args.key == OF_KEY_LEFT && useArrowKeys)) {
        left = false;
    }
    if ((isD && !useArrowKeys) || (args.key == OF_KEY_RIGHT && useArrowKeys)) {
        rigt = false;
    }
    
    if (forw || back || left || rigt) {
        movKey = true;
    } else {
        movKey = false;
        speedMod = 0.1;
    }
}

/***********************************************************/
// Infinite mouse
//
// GLFW_CURSOR_DISABLED hides the cursor, locks it to the window and then reports a
// *virtual* cursor position that is never clamped to the screen. The look deltas in
// update() therefore keep arriving no matter how far the user turns, which is exactly
// what the old warp-the-cursor-back-to-centre code was faking. GLFW implements the
// mode natively on macOS (Quartz), Windows (raw input + clipped cursor) and Linux
// (XInput2 pointer grab / Wayland pointer constraints), so no #ifdef per OS.
//
// Because we only ever use (mouse - lastMouse), the unbounded coordinates are fine;
// nothing here depends on the cursor staying inside the window.

void ofxFPSCamera::enableInfiniteMouse(){
    infiniteMouse = true;
}

void ofxFPSCamera::disableInfiniteMouse(){
    infiniteMouse = false;
}

bool ofxFPSCamera::isInfiniteMouseActive() const {
    return infiniteMouseActive;
}

bool ofxFPSCamera::isInfiniteMouseSupported() const {
#ifdef OFX_FPSCAMERA_HAS_GLFW
    return getGLFWWindowPtr() != nullptr;
#else
    return false;
#endif
}

void ofxFPSCamera::setCursorCaptured(bool captured){
#ifdef OFX_FPSCAMERA_HAS_GLFW
    GLFWwindow * window = getGLFWWindowPtr();
    if(window == nullptr){
        return;
    }

    glfwSetInputMode(window, GLFW_CURSOR, captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

    #ifdef GLFW_RAW_MOUSE_MOTION                                            // GLFW 3.3+
    if(glfwRawMouseMotionSupported()){
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION,
                         (captured && rawMouseMotion) ? GLFW_TRUE : GLFW_FALSE);
    }
    #endif
#endif
}

void ofxFPSCamera::updateInfiniteMouse(){
#ifdef OFX_FPSCAMERA_HAS_GLFW
    GLFWwindow * window = getGLFWWindowPtr();
    if(window == nullptr){
        infiniteMouseActive = false;
        return;
    }

    // keepTurning needs a real screen edge to push against, so the two are exclusive -
    // same as the original code, which only warped when keepTurning was false.
    // Letting go of the cursor when the window loses focus keeps cmd-tab/alt-tab working.
    bool wantCapture = infiniteMouse
                       && usemouse
                       && !keepTurning
                       && glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_TRUE;

    // Read the live mode rather than trusting a cached flag: ofHideCursor()/ofShowCursor()
    // drive GLFW_CURSOR too, so the sketch can change it behind our back.
    infiniteMouseActive = glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;

    if(wantCapture == infiniteMouseActive){
        return;
    }

    setCursorCaptured(wantCapture);
    infiniteMouseActive = wantCapture;

    // Entering and leaving the mode both move the reported cursor position, so drop a
    // single frame of delta instead of letting the view snap.
    lastMouse = ofVec2f(ofGetMouseX(), ofGetMouseY());
    justResetAngles = true;
#else
    infiniteMouseActive = false;
#endif
}
/***********************************************************/


void ofxFPSCamera::clip(ofVec3f newPos){
    clipPos = newPos;
    isClipped = true;
}

void ofxFPSCamera::enableStrafe(){
    strafe = true;
}

void ofxFPSCamera::disableStrafe(){
    strafe = false;
}


void ofxFPSCamera::movedManually(){
	currLookTarget = getPosition() + getLookAtDir();
    currentUp = ofVec3f(0,1,0);                             // Maintain UP direction relative to world coordinates
	justResetAngles = true;
}

void ofxFPSCamera::setCamHeight(float ch){
    camHeight = ch;
}

void ofxFPSCamera::setMinMaxY(float angleDown, float angleUp){
    downAngle = abs(angleDown);
    upAngle = -1 * abs(angleUp);
}

void ofxFPSCamera::reset(){
    camHeight = 0.0;
    setPosition(ofVec3f(0,0,0));
	currLookTarget = ofVec3f(0,0,1);
    currentUp = ofVec3f(0,1,0);
    lookAt(currLookTarget, currentUp);
    setOrientation(ofQuaternion());
	movedManually();
}

void ofxFPSCamera::reset(float h){
    camHeight = h;
	setPosition(ofVec3f(0,0,0));
	currLookTarget = ofVec3f(0,0,1);
    currentUp = ofVec3f(0,1,0);
    lookAt(currLookTarget, currentUp);
    //setOrientation(ofQuaternion());
	movedManually();
}

void ofxFPSCamera::reset(float x, float y, float z){
    camHeight = 0.0;
	setPosition(ofVec3f(x,y,z));
	currLookTarget = getPosition() + ofVec3f(0,0,1);
    currentUp = ofVec3f(0,1,0);
    lookAt(currLookTarget, currentUp);
    //setOrientation(ofQuaternion());
	movedManually();
}

void ofxFPSCamera::reset(float x, float y, float z, float h){
    camHeight = h;
	setPosition(ofVec3f(x,y,z));
	currLookTarget = getPosition() + ofVec3f(0,0,1);
    currentUp = ofVec3f(0,1,0);
    lookAt(currLookTarget, currentUp);
    //setOrientation(ofQuaternion());
	movedManually();
}

void ofxFPSCamera::reset(ofVec3f v){
    camHeight = 0.0;
	setPosition(v);
	currLookTarget = getPosition() + ofVec3f(0,0,1);
    currentUp = ofVec3f(0,1,0);
    lookAt(currLookTarget, currentUp);
    //setOrientation(ofQuaternion());
	movedManually();
}

void ofxFPSCamera::reset(ofVec3f v, float h){
    camHeight = h;
	setPosition(v);
	currLookTarget = getPosition() + ofVec3f(0,0,1);
    currentUp = ofVec3f(0,1,0);
    lookAt(currLookTarget, currentUp);
    //setOrientation(ofQuaternion());
	movedManually();
}

void ofxFPSCamera::target(ofVec3f v){
    currLookTarget = ofVec3f(v.x,v.y,v.z);
    lookAt(currLookTarget, currentUp);
    movedManually();
}

ofVec3f ofxFPSCamera::getTarget(){
    return currLookTarget;
}

void ofxFPSCamera::enableMove () {
    usemouse = true;
    applyTranslation = true;
}

void ofxFPSCamera::disableMove () {
    usemouse = false;
    applyTranslation = false;
}


// SAVE and LOAD functions below are unchanged from ofxGameCamera:

void ofxFPSCamera::saveCameraPosition()
{
	ofxXmlSettings savePosition;
	savePosition.addTag("camera");
	savePosition.pushTag("camera");

	savePosition.addTag("position");
	savePosition.pushTag("position");
	savePosition.addValue("X", getPosition().x);
	savePosition.addValue("Y", getPosition().y);
	savePosition.addValue("Z", getPosition().z);
	savePosition.popTag(); //pop position

	savePosition.addTag("up");
	savePosition.pushTag("up");
	savePosition.addValue("X", currentUp.x);
	savePosition.addValue("Y", currentUp.y);
	savePosition.addValue("Z", currentUp.z);
	savePosition.popTag(); //pop up

	savePosition.addTag("look");
	savePosition.pushTag("look");
	savePosition.addValue("X", currLookTarget.x);
	savePosition.addValue("Y", currLookTarget.y);
	savePosition.addValue("Z", currLookTarget.z);
	savePosition.popTag(); //pop look
	
	savePosition.addValue("FOV", getFov());

	savePosition.popTag(); //camera;

	savePosition.saveFile(cameraPositionFile);
	
	unsavedChanges = false;
}

void ofxFPSCamera::loadCameraPosition()
{
	ofxXmlSettings loadPosition;
	cout << "Camera pos " << cameraPositionFile << endl;
	if(loadPosition.loadFile(cameraPositionFile)){
//		reset();
//		loadPosition.pushTag("camera");
		cout << "loaded camera pos" << endl;
		// tig: defaulting with floats so as to get floats back.
		setPosition(ofVec3f(loadPosition.getValue("camera:position:X", 0.),
							loadPosition.getValue("camera:position:Y", 0.),
							loadPosition.getValue("camera:position:Z", 0.)));
		
		currentUp = ofVec3f(loadPosition.getValue("camera:up:X", 0.),
							loadPosition.getValue("camera:up:Y", 1.),
							loadPosition.getValue("camera:up:Z", 0.));

		currLookTarget = ofVec3f(loadPosition.getValue("camera:look:X", 0.),
								 loadPosition.getValue("camera:look:Y", 1.),
								 loadPosition.getValue("camera:look:Z", 0.));

		float fov = loadPosition.getValue("camera:FOV", -1.0);
		if(fov != -1.0){
			setFov(fov);
		}

		lookAt(currLookTarget, currentUp);
		movedManually();
		
	}
	else{
		ofLog(OF_LOG_ERROR, "ofxFPSCamera: couldn't load position!");
	}

}
