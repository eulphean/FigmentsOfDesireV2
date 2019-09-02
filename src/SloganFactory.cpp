#pragma once
#include "SloganFactory.h"

Slogan SloganFactory::getSlogan() {
    // Returns a slogan
  if (allocatedSlogans.size() < slogans.size()) {
    // Look for a random slogan.
    int randIdx = floor(ofRandom(slogans.size()));
    Slogan slogan = slogans[randIdx];
    
    while(ofContains(allocatedSlogans, slogan.msg)) {
      randIdx = floor(ofRandom(slogans.size()));
      slogan = slogans[randIdx];
    }
    
    // Push the obtained slogan back into the allocated strings.
    allocatedSlogans.push_back(slogan.msg);
    
    return slogan;
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
