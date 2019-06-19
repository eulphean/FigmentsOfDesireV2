#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  // Open files
  amay.open("amay.txt", ofFile::WriteOnly);
  azra.open("azra.txt", ofFile::WriteOnly);
  
  ofBuffer buffer = ofBufferFromFile("Azra2.txt");
  auto text = buffer.getText();
  
  // All the lines in the file. Start writing the sorting algorithm
  auto lines = ofSplitString(text, "\n");
  
  for (auto &l : lines) {
    auto i = l.find("[");
    if (i == 0) {
      // New message from someone.
      auto startName = l.find("]");
      
      // Sender name
      string senderName = l.substr(startName + 2, 4);
      
      if (senderName == "Amay") {
        amay << endl;
        lastSender = "Amay";
        auto startMessage = startName + 8;
        auto message = l.substr(startMessage);
        amay << "Amay:" << message;
      } else if (senderName == "Azra") {
        azra << endl;
        lastSender = "Azra";
        auto startMessage = startName + 15;
        auto message = l.substr(startMessage);
        azra << "Azra:" << message;
      }
    } else {
      // Its a part of previous message, take this entire line and append it to the file
      // of its last sender.
      if (lastSender == "Amay") {
        amay << l;
      } else if (lastSender == "Azra") {
        azra << l;
      }
    }
  }
  
  amay.close();
  azra.close();
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}
