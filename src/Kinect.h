// This is a class for memory object that gets created for each joint.
#pragma once
#include "ofMain.h"
#include "ofxKinectV2.h"
#include "ofxCv.h"
#include "ofxGui.h"

class Kinect {
  public:
    void setup();
    void update();
    void draw();
    std::vector<glm::vec2> getBodyCentroids(); // TODO: This should be a vector
  
    // Flags
    bool kinectOpen;
  
    // Kinect Gui.
    ofxPanel gui;
  
  private:
    ofxKinectV2 kinect;
  
    // Helper methods.
    void drawContent();
    void initialize();
    void drawTextureAtRowAndColumn(const std::string& title,
                                   const ofTexture& tex,
                                   int row, int column,
                                   float width, float height);

    ofParameterGroup settings;
  
    // Preprocess depth pixels params.
    ofParameterGroup imageParams;
    ofParameter<float> blurVal;
    ofParameter<int> imageThreshold;
    ofParameter<int> erosion;
    ofParameter<int> dilation;
  
    // Contour limitation params.
    ofParameterGroup contourParams;
    ofParameter<float> minArea;
    ofParameter<float> maxArea;
    ofParameter<int> contourThreshold;

    // Kinect working textures.
    ofTexture texRGB;
    ofTexture texRGBRegistered;
    ofTexture texIR;
    ofTexture texDepth;
  
    // Contour Finding.
    ofxCv::ContourFinder contourFinder;
    glm::vec2 scaleVariables;
};
