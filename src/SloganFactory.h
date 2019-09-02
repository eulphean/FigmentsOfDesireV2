#pragma once
#include "ofMain.h"

struct Slogan {
  public:
    Slogan(string m, int s) {
      msg = m;
      size = s;
    }
  
    string msg;
    int size;
};

class SloganFactory {
  public:
    SloganFactory() {
    
    };
  
    Slogan getSlogan();
    void clearAllocatedSlogans();
    
    static SloganFactory &instance();
  
  private:
    static SloganFactory m;
    
    std::vector<Slogan> slogans = {
      Slogan("Amazon Burns", 20),
      Slogan("Cambridge Analytica", 15),
      Slogan("Privacy Breaach", 18),
      Slogan("Global Warming", 18),
      Slogan("Digital Injustice", 17),
      Slogan("Bitcoin Farms", 17),
      Slogan("Selfie Deaths", 17),
      Slogan("Blue Whale", 17),
      Slogan("Decentralization", 17),
      Slogan("Social Inequality", 16),
      Slogan("Black Money", 18),
      Slogan("Starving Artist", 17),
      Slogan("Human 2.0", 18),
      Slogan("Lack of Trust", 17),
      Slogan("Firewalls", 20),
      Slogan("Digital Power", 18),
      Slogan("Libra Token", 18),
      Slogan("Racial Bias", 17),
      Slogan("Crypto Revolution", 16),
      Slogan("Server Farms", 17),
      Slogan("Cyber Warfare", 17),
      Slogan("Smart Weapons", 17),
      Slogan("CRISPR", 20),
      Slogan("Spam Agents", 18),
      Slogan("Self Driving Cars", 15),
      Slogan("Bitcoin", 20)
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
