#pragma once
#include "SloganFactory.h"

string SloganFactory::getSlogan() {
    // Returns a slogan
  if (allocatedSlogans.size() < slogans.size()) {
    // Look for a random slogan.
    int randIdx = floor(ofRandom(slogans.size()));
    string s = slogans[randIdx];
    
    while(ofContains(allocatedSlogans, s)) {
      randIdx = floor(ofRandom(slogans.size()));
      s = slogans[randIdx];
    }
    
    // Push the obtained slogan back into the allocated strings.
    allocatedSlogans.push_back(s);
    
    return s; 
  } else {
    cout << "All slogan allocated. Resetting allocated slogans array." << endl;
    allocatedSlogans.clear();
    return slogans[0];
  }
}

void SloganFactory::clearAllocatedSlogans() {
  allocatedSlogans.clear(); 
}

SloganFactory &SloganFactory::instance() {
  return m; 
}

// Initialize static variable
SloganFactory SloganFactory::m;
