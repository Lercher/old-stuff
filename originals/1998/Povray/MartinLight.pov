
// Persistence of Vision Ray Tracer Scene Description File
// File: .pov
// Vers: 3
// Desc:
// Date:
// Auth:


// ==== Standard POV-Ray Includes ====
#include "colors.inc"	// Standard Color definitions
#include "textures.inc"	// Standard Texture definitions
#include "STONES.INC"

camera
{
  location  <0.0 , 0.0 ,-11>
  look_at   <0.0 , 0.0 , 0.0>
  up        0.10*y
  right     0.8*x
}

light_source
{
    <0, 0, -400>
    color red 0.4  green 0.4  blue 0.4  // light's color
}

// create a regular point light source
light_source
{
  0*x // light's position (translated below)
  color red 1  green 1  blue 1  // light's color
  translate <-11+clock, 0.8, -3>
}

// create a regular point light source
light_source
{
  0*x // light's position (translated below)
  color red 1  green 1  blue 1  // light's color
  translate <-33+clock, 0.8, -3>
}

// create a regular point light source
light_source
{
  0*x // light's position (translated below)
  color red 1  green 1  blue 1  // light's color
  translate <+11+clock, 0.8, -3>
}

// An infinite planar surface
// plane {<A, B, C>, D } where: A*x + B*y + C*z = D
plane
{
  z, // <X Y Z> unit surface normal, vector points "away from surface"
  1.0 // distance from the origin in the direction of the surface normal

  pigment { red 0 green 0 blue 1 }
  finish {
    phong 0.9
    ambient 0.4
  }
}

text
{
    ttf "Verdana.ttf" "Martin Lercher" 0.25, 0
    pigment { Gold }
    finish { reflection .25 specular 1 }
    translate <-3.7, -0.3, 0>
}

