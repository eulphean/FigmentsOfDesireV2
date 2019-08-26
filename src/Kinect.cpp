#include "Kinect.h"

void Kinect::setup() {
    //see how many devices we have.
    ofxKinectV2 tmp;
    std::vector <ofxKinectV2::KinectDeviceInfo> deviceList = tmp.getDeviceList();
    if (deviceList.size() > 0) {
      // Start Kinect.
      kinectOpen = kinect.open(deviceList[0].serial);
      // Setup GUI.
      initialize();
    } else {
      cout << "ERROR: Kinect not fuond.";
    }
  
}

void Kinect::update() {
   // Is there a valid Kinect connection?
    if (kinectOpen) {
      // Update Kinect to process next frame.
      kinect.update();
      
      // Process Kinect Data only when it's a new valid frame.
      if (kinect.isFrameNew()) {
          texRGB.loadData(kinect.getPixels());
          texRGBRegistered.loadData(kinect.getRegisteredPixels());
          texIR.loadData(kinect.getIRPixels());
        
          // Get depth pixels.
          auto depthPixels = kinect.getDepthPixels();
        
          // [NOTE] Loading these pixels after ofxCv operations, I take a
          // performance hit.
          texDepth.loadData(depthPixels);
        
          // Add preprocessor effects.
          ofxCv::blur(depthPixels, depthPixels, blurVal);
          ofxCv::erode(depthPixels, depthPixels, erosion);
          ofxCv::dilate(depthPixels, depthPixels, dilation);
          ofxCv::threshold(depthPixels, depthPixels, imageThreshold);
        
          // Updated depth texture.
          // If I reload texture here, I take a performance hit.

          // Set contour finder's properties.
          contourFinder.setMinAreaRadius(minArea);
          contourFinder.setMaxAreaRadius(maxArea);
          contourFinder.setThreshold(contourThreshold);
          
          // Find contours.
          auto depthImgMat = ofxCv::toCv(depthPixels);
          contourFinder.findContours(depthImgMat);
        
          // To fit the contours on top of the entire screen, we calculate scale.
          auto xScale = ofGetWidth()/texDepth.getWidth();
          auto yScale = ofGetHeight()/texDepth.getHeight();
          scaleVariables = glm::vec2(xScale, yScale);
      }
    }
}

void Kinect::draw(bool isDebug, bool hideKinectGui) {
    if (kinectOpen) {
      // Kinect debug view.
      if (isDebug) {
          drawContent();
          
          // Use the texture width, height as the baseline to draw all the 4 debug screens.
          auto w = texDepth.getWidth(); auto h = texDepth.getHeight();
        
          drawTextureAtRowAndColumn("Depth Pixels, Mapped", texDepth, 0, 0, w, h);
          drawTextureAtRowAndColumn("IR Pixels, Mapped", texIR, 0, 1, w, h);
          drawTextureAtRowAndColumn("RGB Pixels, Registered", texRGBRegistered, 1, 0, w, h);
          drawTextureAtRowAndColumn("RGB Pixels", texRGB, 1, 1, w, h);
        
          // Draw contours on top of the depth pixels with Age, Label.
          ofPushMatrix();
            ofPushStyle();
              ofSetColor(ofColor::green);
              ofSetLineWidth(3);
              for (int i = 0; i < contourFinder.size(); i++) {
                contourFinder.getPolyline(i).draw();
                auto label = contourFinder.getLabel(i);
                auto age = contourFinder.getTracker().getAge(label);
                ofDrawBitmapStringHighlight(ofToString(label) + " : " + ofToString(age), ofxCv::toOf(contourFinder.getCenter(i)));
              }
            ofPopStyle();
          ofPopMatrix();
        
          // Draw the X line and Y line at the center of the depth pixels
          ofPushStyle();
            ofSetColor(ofColor::red);
            // 1st Panel (Depth Pixels)
            ofDrawLine(0, h/2, w, h/2); // Horizontal center
            ofDrawLine(w/2, 0, w/2, h); // Vertical center
        
            // 2nd Panel (IR Pixels)
            ofDrawLine(0, 3*h/2, w, 3*h/2);
            ofDrawLine(w/2, h, w/2, 2*h);
          ofPopStyle();
      }
    }
  
    if (hideKinectGui) {
      gui.draw();
    }
}

void Kinect::initialize() {
   // Setup GUI.
  settings.setName("Kinect Params");
  
  // Preprocess depth pixels params.
  imageParams.setName("Image Params");
  imageParams.add(blurVal.set("Blur", 0, 0, 20));
  imageParams.add(imageThreshold.set("Threshold", 128, 0, 255));
  imageParams.add(erosion.set("Erode", 0, 0, 30));
  imageParams.add(dilation.set("Dilate", 0, 0, 30));

  // Contour finder GUI params.
  contourParams.setName("Contour Finder");
  contourParams.add(minArea.set("Min Area", 10, 1, 100));
  contourParams.add(maxArea.set("Max Area", 200, 1, 500));
  contourParams.add(contourThreshold.set("Threshold", 128, 0, 255));
  
  // Open Kinect
  kinect.params.setName("Distance Thresholds");
  settings.add(kinect.params);
  settings.add(imageParams);
  settings.add(contourParams);
  gui.setup(settings);
  
  gui.setPosition(250, 20);
  
  gui.loadFromFile("Kinect.xml");
}

std::vector<glm::vec2> Kinect::getBodyCentroids() {
  std::vector<glm::vec2> centroids;
  
  // Master matrix
  ofMatrix4x4 scaleMatrix = ofMatrix4x4::newScaleMatrix(ofVec3f(scaleVariables.x, scaleVariables.y, 0));
  for (int i = 0; i < contourFinder.size(); i++) {
    auto center = ofxCv::toOf(contourFinder.getCenter(i));
    auto newPoint = ofVec3f(center.x, center.y, 0) * scaleMatrix;
    centroids.push_back(glm::vec2(newPoint.x, newPoint.y));
  }
  
  return centroids;
}

void Kinect::drawContent() {
  ofPushMatrix();
    ofScale(scaleVariables.x, scaleVariables.y);
      // Draw circle where the center of the contour is (to track position)
      for (int i = 0; i < contourFinder.size(); i++) {
        ofPushMatrix();
          auto center = ofxCv::toOf(contourFinder.getCenter(i));
          ofTranslate(center);
          ofPushStyle();
            ofSetColor(ofColor::green);
            ofDrawCircle(0, 0, 5);
          ofPopStyle();
        ofPopMatrix();
      }
  ofPopMatrix();
}

void Kinect::drawTextureAtRowAndColumn(const std::string& title,
                                       const ofTexture& tex,
                                       int row, int column,
                                       float width, float height) {
    float displayWidth = width;
    float displayHeight = height;
    float xPos = row * displayWidth; float yPos = column * displayHeight;
  
    ofPushMatrix();
      ofTranslate(xPos, yPos);
        ofPushStyle();
        ofSetColor(255); // Standard color of the texture.
        if (tex.isAllocated()) {
            tex.draw(0, 0, width, height);
            ofDrawBitmapStringHighlight(title, glm::vec2(xPos, yPos) + glm::vec3(14, 20, 0));
        }
        ofPopStyle();
    ofPopMatrix();
}
