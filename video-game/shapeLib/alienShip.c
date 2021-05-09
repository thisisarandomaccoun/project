#include "shape.h"

//Method for creating alien ship
int
abAlienCheck(const AbAlien *alien, const Vec2 *centerPos, const Vec2 *pixel)
{
  Region bounds;
  abAlienGetBounds(alien, centerPos, &bounds);
  int within = 1, axis;

  Vec2 relPos; 
  vec2Sub(&relPos, pixel, centerPos);
    
  //Makes a rectangle, then cuts off the sides
  for (axis = 0; axis < 2; axis++) {
    int p = pixel->axes[axis];
    if (p > bounds.botRight.axes[axis] || p < bounds.topLeft.axes[axis]) {
      within = 0;
    }
    
    else if (relPos.axes[0] * 2 < relPos.axes[1]) {
      within = 0;
    }
   
    else if (relPos.axes[0] * -2 < relPos.axes[1]) { 
      within = 0;
    }
  }
  return within;
}
void abAlienGetBounds(const AbAlien *alien, const Vec2 *centerPos, Region *bounds)
{
  vec2Sub(&bounds->topLeft, centerPos, &alien->halfSize);
  vec2Add(&bounds->botRight, centerPos, &alien->halfSize);
}
