/** \file shapemotion.c
 *  \brief This is a simple shape motion demo.
 *  This demo creates two layers containing shapes.
 *  One layer contains a rectangle and the other a circle.
 *  While the CPU is running the green LED is on, and
 *  when the screen does not need to be redrawn the CPU
 *  is turned off along with the green LED.
 */

#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>
#include <stdio.h>

#define GREEN_LED BIT6

//Variable to keep score
int score = 0;
//Variable to keep lives
int lives = 1;
//Variable for assembly function
extern int checkLives(int lives);

const AbRect rect = {abRectGetBounds, abRectCheck, 1, 1};;

//Ships
AbCust customShape = {abCustGetBounds, abCustCheck, {10, 10}};
AbAlien alienShip = {abAlienGetBounds, abAlienCheck, {10, 10}};

AbRectOutline fieldOutline = {	/* playing field */
  abRectOutlineGetBounds, abRectOutlineCheck,   
  {screenWidth/2 - 3, screenHeight/2 - 10}
};

Layer star4 = {
  (AbShape *)&circle2,
  {10, screenHeight/4 + 35},
  {0,0}, {0,0},
  COLOR_YELLOW,
  0,
};

Layer star3 = {
  (AbShape *)&circle2,
  {35, 35},
  {0,0}, {0,0},
  COLOR_ORANGE,
  &star4,
};

Layer star2 = {
  (AbShape *)&circle2,
  {105, 15},
  {0,0}, {0,0},
  COLOR_GREEN,
  &star3,
};

Layer star1 = {
  (AbShape *)&circle2,
  {screenWidth/1.5, screenHeight/1.5},
  {0,0}, {0,0},
  COLOR_BLUE,
  &star2,
};

Layer layer6 = {                 /**< Layer with alien ship 2 */
  (AbShape *)&alienShip,
  {screenWidth/2 + 15, screenHeight/2 + 15},
  {0,0}, {0,0},
  COLOR_RED,
  &star1,
};

Layer layer3 = {		/**< Layer with missile */ 
  (AbShape *)&rect,
  {screenWidth/4, screenHeight/1.3}, 
  {0,0}, {0,0},				    
  COLOR_VIOLET,
  &layer6,
};

Layer fieldLayer = {		/* playing field as a layer */
  (AbShape *) &fieldOutline,
  {screenWidth/2, screenHeight/2},/**< center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_BLACK,
  &layer3
};

Layer layer1 = {		/**< Layer with  alienShip */
  (AbShape *)&alienShip,
  {screenWidth/3, screenHeight/3}, /**< center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_SEA_GREEN,
  &fieldLayer,
};

Layer layer0 = {		/**< Layer with player ship */
  (AbShape *)&customShape,
  {(screenWidth/4), (screenHeight/1.3)}, /**< bit below & right of center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_WHITE,
  &layer1,
};

/** Moving Layer
 *  Linked list of layer references
 *  Velocity represents one iteration of change (direction & magnitude)
 */
typedef struct MovLayer_s {
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

/* initial value of {0,0} will be overwritten */
MovLayer ml6 = { &layer6, {2,3}, 0 };    //Alien Ship 2
MovLayer ml3 = { &layer3, {0,0}, &ml6 }; //Missile
MovLayer ml1 = { &layer1, {1,2}, &ml3 }; //Alien Ship 1
MovLayer ml0 = { &layer0, {0,0}, &ml1 }; //Player Ship 

void movLayerDraw(MovLayer *movLayers, Layer *layers)
{
  int row, col;
  MovLayer *movLayer;

  and_sr(~8);			/**< disable interrupts (GIE off) */
  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Layer *l = movLayer->layer;
    l->posLast = l->pos;
    l->pos = l->posNext;
  }
  or_sr(8);			/**< disable interrupts (GIE on) */

  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Region bounds;
    layerGetBounds(movLayer->layer, &bounds);
    lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1], 
		bounds.botRight.axes[0], bounds.botRight.axes[1]);
    for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++) {
      for (col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++) {
	Vec2 pixelPos = {col, row};
	u_int color = bgColor;
	Layer *probeLayer;
	for (probeLayer = layers; probeLayer; 
	     probeLayer = probeLayer->next) { /* probe all layers, in order */
	  if (abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos)) {
	    color = probeLayer->color;
	    break; 
	  } /* if probe check */
	} // for checking all layers at col, row
	lcd_writeColor(color); 
      } // for col
    } // for row
  } // for moving layer being updated
}	  

/** Advances a moving shape within a fence
 *  
 *  \param ml The moving shape to be advanced
 *  \param fence The region which will serve as a boundary for ml
 */
void mlAdvance(MovLayer *ml, Region *fence) {
  Vec2 newPos;
  u_char axis;
  
  Region shapeBoundary;
 
  for (; ml; ml = ml->next) {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    for (axis = 0; axis < 2; axis ++) {
      if ((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) ||
	  (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis])) {
	int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);
      }	//if outside of fence
    } // for axis 
    ml->layer->posNext = newPos;
  } /**< for ml */
}

void astroidCollisionOne(MovLayer *ml) {
  Vec2 newPos;
  u_char axis;

  Region shapeBounds;
  
  for (; ml; ml = ml->next) {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBounds);
    for (axis = 0; axis < 2; axis++) {
      //If ship collides with alien ship, game ends
      if (layer6.pos.axes[1] >= layer0.pos.axes[1] - 5 && layer6.pos.axes[1] <= layer0.pos.axes[1] + 5 && layer6.pos.axes[0] >= layer0.pos.axes[0] - 5 && layer6.pos.axes[0] <= layer0.pos.axes[0] + 5){
	lives -= 1;
	if (checkLives(lives) == 0) {
	  endGame();
	}
      }
    }
  }
}

void astroidCollisionTwo(MovLayer *ml) {

  Vec2 newPos;
  u_char axis;

  Region shapeBounds;
  
  for (; ml; ml = ml->next) {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBounds);
    for (axis = 0; axis < 2; axis++) {
       //If ship collides with alien ship, game ends
      if (layer1.pos.axes[1] >= layer0.pos.axes[1] - 5 && layer1.pos.axes[1] <= layer0.pos.axes[1] + 5 && layer1.pos.axes[0] >= layer0.pos.axes[0] - 5 && layer1.pos.axes[0] <= layer0.pos.axes[0] + 5){
	lives -= 1;
	if (checkLives(lives) == 0) {
	  endGame();
	}
      }
    }
  }
}

void missileCollision(MovLayer *ml) {

  Vec2 newPos;
  u_char axis;
  char scoreArr[10] = "SCORE:";
  
  Region shapeBounds;

  drawString5x7(1,1, "SCORE:", COLOR_GREEN, COLOR_RED);
  
  for (; ml; ml = ml->next) {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBounds);
    for (axis = 0; axis < 2; axis++) {
      //Layer3 = Missile, if missle collides with any of the astroids the score counter goes up
      if (layer1.pos.axes[1] >= layer3.pos.axes[1] - 8 && layer1.pos.axes[1] <= layer3.pos.axes[1] + 5 && layer1.pos.axes[0] >= layer3.pos.axes[0] - 8 && layer1.pos.axes[0] <= layer3.pos.axes[0] + 8|| layer6.pos.axes[1] >= layer3.pos.axes[1] - 8 && layer6.pos.axes[1] <= layer3.pos.axes[1] + 8 && layer6.pos.axes[0] >= layer3.pos.axes[0] - 8 && layer6.pos.axes[0] <= layer3.pos.axes[0] + 8){
	//Score iterates in multiples of 6 due to looping
	score = score + 1;
	itoa(score/6,scoreArr,10);
	drawString5x7(45,1, scoreArr, COLOR_GREEN, COLOR_RED);
      }
    }
  }
}

void gameOverNoise() {
  drawString5x7(30,30, "GAME OVER", COLOR_GREEN, COLOR_RED);
  for(int i = 0; i < 10000; i++) {
    buzzer_set_period(1000);
  }
  for(int i = 0; i < 10000; i++) {
    buzzer_set_period(1200);
  }
  for(int i = 0; i < 10000; i++) {
    buzzer_set_period(1500);
  }
  buzzer_set_period(100);
}

void endGame() {
    gameOverNoise();
    or_sr(0x10);
    and_sr(~8);
}
     
u_int bgColor = COLOR_BLACK;     /**< The background color */
int redrawScreen = 1;           /**< Boolean for whether screen needs to be redrawn */

Region fieldFence;		/**< fence around playing field  */
Region ship;

/** Initializes everything, enables interrupts and green LED, 
 *  and handles the rendering for the screen
 */
void main()
{
  P1DIR |= GREEN_LED;		/**< Green led on when CPU on */		
  P1OUT |= GREEN_LED;

  configureClocks();
  lcd_init();
  shapeInit();
  p2sw_init(15);
  buzzer_init();
  
  shapeInit();

  layerInit(&layer0);
  layerDraw(&layer0);
  
  layerGetBounds(&fieldLayer, &fieldFence);

  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */

  for(;;) {
    u_int switches = p2sw_read(), i;
    
    char str[5];

    //State machine to handle buttons
    for (i = 0; i < 4; i++) {
      str[i] = (switches & (1<<i)) ? 0 : 1;
      str[4] = 0;

      //Button 1 moves ship and missle left
      if(str[0]) {
	ml0.velocity.axes[0] = -5;
	ml3.velocity.axes[0] = -5;
      }

      //Button 2 moves ship and missle right
      else if (str[1]) {
	ml0.velocity.axes[0] = 5;
	ml3.velocity.axes[0] = 5;
      }
      
      //Button 3 shoots Missile
      else if (str[2]) {
	//Make shooting noise	
       	for(int i = 0; i < 10000; i++) {
	  buzzer_set_period(5000);
	}
	for(int i = 0; i < 10000; i++) {
	  buzzer_set_period(9200);
	}
	buzzer_set_period(100);
	//Velocity for Missile
	ml3.velocity.axes[1] = -10;
      }
      
      //Button 4 reloads and resets missile back to position (within the ship)
      else if (str[3]) {
	ml3.velocity.axes[0] = 0;
	ml3.velocity.axes[1] = 0;
	layer3.posNext.axes[0] = 100;
	//Moves missle to the ships position 
	layer3.posNext.axes[0] = layer0.posNext.axes[0];
	layer3.posNext.axes[1] = layer0.posNext.axes[1];
      }
      
      //Halts ship when buttons not using it
      else {
	ml0.velocity.axes[0] = 0;
	ml0.velocity.axes[1] = 0;
	ml3.velocity.axes[0] = 0;
      }
    }
    
    while (!redrawScreen) { /**< Pause CPU if screen doesn't need updating */
      P1OUT &= ~GREEN_LED;    /**< Green led off witHo CPU */
      or_sr(0x8);	      /**< CPU OFF */
    }
    P1OUT |= GREEN_LED;       /**< Green led on when CPU on */
    redrawScreen = 0;
    //Following Methods handle all collisions
    astroidCollisionOne(&ml6);
    astroidCollisionTwo(&ml6);
    missileCollision(&ml1);
    
    movLayerDraw(&ml0, &layer0);
  }
}

/** Watchdog timer interrupt handler. 15 interrupts/sec */
void wdt_c_handler()
{
  static short count = 0;
  P1OUT |= GREEN_LED;		      /**< Green LED on when cpu on */
  
  count ++;
  if (count == 15) {
    layerGetBounds(&layer0, &ship);

    mlAdvance(&ml0, &fieldFence);
   
    if (p2sw_read())
      redrawScreen = 1;
    count = 0;
  }
  P1OUT &= ~GREEN_LED;		    /**< Green LED off when cpu off */
}
