# Lab 3 : Galaga

# Install:

In shapeLib directory:

$ make all

Then

$ cd..

In video game directory:

$ make all

Enter shape-motion-demo:

$ make clean

$ make load

# Summary:

Welcome to the my game, Galaga. In this game you are the captain of a ship,
navigating through the vastness of space, avoidng aliens on the way. You can
shoot a missle, hitting the alien ships for points. Becareful, if you get hit
by an alien,you will be engulfed in the emptiness of space and be forever forgottem.

Game has 4 moving objects, the ship, the missle, and two invincible alien ships. The goal is
to avoid the alien ships while successfully hitting them for points.

# Buttons:

Button 1: Steer the ship left

Button 2: Steer the ship right

Button 3: Shoots missile

Button 4: "Reloads" the missle

# Objects:

Ship: White
Missile: Violet
Alien Ship One: Sea Green
Alien Ship Two: Red

# Bugs:
Missile comes out of the ship when hitting the sides, just press button 4 to
get missile back in the right position. Sometimes the missle will hit the ship
and not score the point, but it usually works, the boundaries are a little
wider for this reason. You need to reload after every shot or the missile
bounces up and down, could not figure out how to have the missle ignore the
frame. You have two lives however, the game over sign comes up when you are
hit once but the game continues. Also, the red ship moves to fast sometimes
which seems to skip over the ship and not signaling the collision. 

# Ending Thoughts:
Original implimentaion had the ships going through the bottom of the screen
and starting  from the top up of the screen just like the game Galaga. The
stars would also move down making it look like the ship is moving and the
missle would not bounce up and down as there was no frame to bounce off of, HOWEVER,
this implementation was extremely glicthy and the game was hardly
playable. Which is sad because it would have been exactly like Galaga, besides
the invincible ships.

Collaborated with:

Timothy McCrary
Richard Quinn
Andres Ramos
Lauren Eagan
Carlos Cabada







