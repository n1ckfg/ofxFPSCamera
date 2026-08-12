# ofxFPSCamera Architecture

`ofxFPSCamera` is an openFrameworks addon that provides a first-person shooter style camera. It inherits directly from the core `ofCamera` class, extending it with keyboard (W, A, S, D or arrows) and mouse controls for intuitive 3D navigation.

## Design Philosophy

The main objective of `ofxFPSCamera` is to create a camera that always maintains a level horizon and allows free exploration of a 3D scene, similar to what you would experience in an FPS game. This is a fork and evolution of `ofxGameCamera`, modified to meet specific requirements:

- **Level Horizon:** It always maintains an UP direction relative to world coordinates. It completely removes the ability to roll on the Z-axis.
- **Fixed Height:** Boom up/down (translating along the local Y-axis) is disabled. The camera maintains a specific height, which can be programmatically controlled but not freely driven by the user using keys.
- **Constrained Pitch:** Looking up and down is limited by customizable thresholds (defaulting to +/- 30 degrees) to prevent camera flipping and disorientation.
- **Mouse Control Defaults:** Mouse look is enabled by default and features an "infinite mouse" implementation on supported platforms, allowing continuous rotation without the mouse cursor hitting screen edges.

## Class Reference: `ofxFPSCamera`

### Lifecycle & Event Methods
These methods hook the camera into the openFrameworks event loop.
* `ofxFPSCamera()` - Constructor. Sets up default variables.
* `~ofxFPSCamera()` - Destructor.
* `void setup()` - Initializes the camera, registers event listeners, and sets default properties.
* `void update(ofEventArgs& args)` - Updates the camera's translation and rotation based on current inputs and constraints.
* `void keyPressed(ofKeyEventArgs& args)` - Internal handler for keyboard input.
* `void keyReleased(ofKeyEventArgs& args)` - Internal handler for keyboard release.

### Manual Positioning & Target Control
* `void movedManually()` - Call this whenever you update the orientation or position of the camera manually outside of the built-in controls, so internal states can sync.
* `void reset()` - Sets the camera back to zero (origin).
* `void reset(float h)` - Resets the camera X/Z to zero and sets the camera height to `h`.
* `void reset(float x, float y, float z)` - Resets the camera position to the specified coordinates.
* `void reset(float x, float y, float z, float h)` - Resets the camera X/Z to the specified coordinates, and sets height to `h`.
* `void reset(ofVec3f v)` - Resets the camera position to the vector `v`.
* `void reset(ofVec3f v, float h)` - Resets the camera position to vector `v`, and sets height to `h`.
* `void target(ofVec3f v)` - Aims the camera at the specified point in world space.
* `ofVec3f getTarget()` - Returns the current look target of the camera.
* `void setCamHeight(float ch)` - Explicitly sets the height (Y position) of the camera.

### Movement & Control Modifiers
* `void enableMove()` - Enables translation controls (keyboard movement).
* `void disableMove()` - Disables translation controls.
* `void enableStrafe()` - Enables side-to-side (strafing) movement.
* `void disableStrafe()` - Disables strafing movement.
* `void setMinMaxY(float angleDown, float angleUp)` - Sets the vertical pitch constraints in degrees.

### Infinite Mouse Controls
* `void enableInfiniteMouse()` - Enables infinite mouse mode, allowing the user to look infinitely left or right without hitting the screen edge.
* `void disableInfiniteMouse()` - Disables infinite mouse mode.
* `bool isInfiniteMouseActive() const` - Returns true if the cursor is currently captured in infinite mouse mode.
* `bool isInfiniteMouseSupported() const` - Returns true if infinite mouse is supported on the current platform/OS.

### Saving & Loading
Relies on `ofxXmlSettings` to persist camera positioning.
* `void saveCameraPosition()` - Saves the current position and rotation to an XML file.
* `void loadCameraPosition()` - Loads the position and rotation from the saved XML file.

### Collision & Clipping
* `void clip(ofVec3f newPos)` - Handles clipping logic if the camera runs into bounds or collides.

---

## Public Variables

### Tweak Controls
* `float sensitivityX` - Mouse sensitivity for looking left/right.
* `float sensitivityY` - Mouse sensitivity for looking up/down.
* `float speed` - Base walking/movement speed.
* `float runSpeed` - Sprinting speed (usually activated with Shift).

### Enable/Disable Flags
* `bool usemouse` - Toggles mouse look controls.
* `bool useArrowKeys` - Toggles whether arrow keys can also control movement (in addition to WASD).
* `bool applyTranslation` - Toggles whether movement input actually translates the camera.

### Saving/Loading Flags
* `bool autosavePosition` - If true, automatically saves the camera position when the application exits.

### State & Constraint Variables
* `float camHeight` - The current forced height of the camera.
* `float totUpDown` - The total pitch applied.
* `float upAngle` - Maximum angle for looking up.
* `float downAngle` - Maximum angle for looking down.
* `bool keepTurning` - Internal flag or feature to continuously turn the camera.
* `float accel` - Acceleration modifier for smooth movement.
* `float speedMod` - Current speed modifier based on run state.
* `bool easeIn` - Toggles ease-in physics for movement.
* `bool infiniteMouse` - Turns the infinite mouse feature on or off (on by default).
* `bool rawMouseMotion` - Skips the OS pointer acceleration curve where supported, for raw input.
* `bool isClipped` - True if the camera is currently being clipped.
* `ofVec3f clipPos` - The position coordinates when clipped.
