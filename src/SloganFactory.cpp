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
  } else {
    cout << "All Slogans Utilized. Returning the first Slogan in the collection" << endl;
    return slogans[0];
  }
}

SloganFactory &SloganFactory::instance() {
  return m; 
}

// Initialize static variable
SloganFactory SloganFactory::m;
