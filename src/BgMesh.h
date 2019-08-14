#pragma once
#include "ofMain.h"

class BgMesh {
  public:
    BgMesh() {}
  
    void setParams(ofParameterGroup params);
  
    // Core methods. 
    void setup();
    void update(std::vector<ofMesh> meshes);
    void draw(bool debug);
    bool isAllocated();
    void destroy();
  
  private:
    ofFbo bgFbo;
    ofFbo mainFbo; 
    ofShader shader;
    ofTexture bgTex;
  
    ofParameterGroup bgParams;
};
