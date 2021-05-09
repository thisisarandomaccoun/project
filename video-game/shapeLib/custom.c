#include "shape.h"

//Method for creating ship
int
abCustCheck(const AbCust *cust, const Vec2 *centerPos, const Vec2 *pixel)
{
  Region bounds;
  abCustGetBounds(cust, centerPos, &bounds);
  int within = 1, axis;

  Vec2 relPos; 
  vec2Sub(&relPos, pixel, centerPos);

  //Makes a triangle, then cuts off the sides
  for (axis = 0; axis < 2; axis ++) {
    int p = pixel->axes[axis];
    if (p > bounds.botRight.axes[axis] || p < bounds.topLeft.axes[axis]){
      within = 0;
    } 
    else if (relPos.axes[0] >= 0 && relPos.axes[0] * 2 > relPos.axes[1] + ((bounds.botRight.axes[1] - bounds.topLeft.axes[1]) / 2)){
      within = 0;
    }
    if (relPos.axes[0] <= 0 && relPos.axes[0] * -2 > relPos.axes[1] + ((bounds.botRight.axes[1] - bounds.topLeft.axes[1]) /2)){
      within = 0;
    }
    if (p > bounds.botRight.axes[axis] || p > bounds.botRight.axes[axis] - 3 || p < bounds.topLeft.axes[axis] || p < bounds.topLeft.axes[axis] + 3) {
      within = 0;
    }
  }
  
  return within;
}

void abCustGetBounds(const AbCust *cust, const Vec2 *centerPos, Region *bounds)
{
  vec2Sub(&bounds->topLeft, centerPos, &cust->halfSize);
  vec2Add(&bounds->botRight, centerPos, &cust->halfSize);
}
