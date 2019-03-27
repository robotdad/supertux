#pragma once
#include <string>

class Roadblock {
private:
  std::string type;
  int level;
public:
  Roadblock(std::string roadblockType, int roadblockCount);
  std::string getType();
  int getLevel();
};
Roadblock::Roadblock(std::string roadblockType, int roadblockLevel)
{
  type = roadblockType;
  level = roadblockLevel;
}
std::string Roadblock::getType() {
  return type;
}
int Roadblock::getLevel() {
  return level;
}
