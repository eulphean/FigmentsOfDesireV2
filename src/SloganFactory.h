#pragma once
#include "ofMain.h"

class SloganFactory {
  public:
    SloganFactory() {
    
    };
  
    string getSlogan();
    void clearAllocatedSlogans();
    
    static SloganFactory &instance();
  
  private:
    static SloganFactory m;
    
    std::vector<string> slogans = {
      "Amazon Burns",
      "Zero F*** Given",
      "Water Crisis",
      "Ban Artists",
      "Digital Brainwash",
      "F*** Solidarity",
      "Starving Artist",
      "Digital Revolution",
      "Bulshit, Bulshit",
      "Guns, Guns, Guns",
      "Global Warming",
      "Ghost Labor",
      "Blue Whale Challenge",
      "Libra != Bitcoin",
      "Crypto Revolution",
      "Digital Injustice"
      "Digital Power",
      "Blame Cellphones",
      "Bots Agents Ghosts",
      "Decntralization",
      "Synthetic Intelligence",
      "Bitcoin Farms",
      "Click Farms",
      "Black Money",
      "Corruption",
      "Inequality",
      "Injustice",
      "Child Abuse",
      "Child Labour",
      "Digital Criminals",
      "Privacy Breach",
      "Death Wish",
      "Legal Guns",
      "Selfie Culture",
      "Millenial Shame"
    };
  
    // These are populated when an agents gets a slogan.
    std::vector<string> allocatedSlogans = {};
};

//SloganFactory &SloganFactory::instance() {
//  return m; 
//}
//
//// Initialize static variable
//SloganFactory SloganFactory::m;
