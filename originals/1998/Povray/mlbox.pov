
// Persistence of Vision Ray Tracer Scene Description File
// File: .pov
// Vers: 3
// Desc: Rotierendes Leasy-SOFT und easy-Word auf Stein
// Date: 24.09.1997
// Auth: M. Lercher


// ==== Standard POV-Ray Includes ====
#include "colors.inc"	// Standard Color definitions
#include "textures.inc"	// Standard Texture definitions
#include "stones.inc"

// Set a color of the background (sky)
//background { color red 0.07 green 0.25 blue 0.9 }
background { color red 1 green 1 blue 1 }

camera
{
  location  <0.0 , 0.0 ,-4.0>
  look_at   <0.0 , 0.0 , 0.0>
  up        y
  right     8/3*x
  rotate    x * clock
  //rotate    45 * x
}

// create a regular point light source
light_source
{
  0*x // light's position (translated below)
  color red 0.8  green 0.8  blue 0.8  // light's color
  translate <-30, 20, -40>
}

light_source
{
  0*x // light's position (translated below)
  color red 0.9  green 0.8  blue 0.8  // light's color
  translate <-25, 20, -40>
  rotate 90*x
}

light_source
{
  0*x // light's position (translated below)
  color red 0.8  green 0.9  blue 0.8  // light's color
  translate <-20, 20, -40>
  rotate 180*x
}

light_source
{
  0*x // light's position (translated below)
  color red 0.7  green 0.9  blue 0.9  // light's color
  translate <-25, 20, -40>
  rotate 270*x
}

//difference {
  box
  {
    <-3.7, -1, -1>  // one corner position <X1 Y1 Z1>
    < 3.7,  1, 1>  // other corner position <X2 Y2 Z2>
    texture { T_Stone17 }
    //texture { pigment { color red 0.8 green 0.8 blue 0.8 } }
    //finish { Shiny }
    scale 1
  }

  text
  {
    ttf "Verdana.ttf" "Leasy-SOFT" 0.3, 0
    scale 1.15
    pigment { Gold }
    finish { reflection .25 specular 1 }
    translate <-3.4, -0.25, -1.2>
  }

  text
  {
    ttf "Verdana.ttf" "easy-Word" 0.3, 0
    scale 1.15
    pigment { Gold }
    finish { reflection .25 specular 1 }
    translate <-2.9, -0.25, -1.2>
    rotate <90, 0, 0>
  }

  text
  {
    ttf "Verdana.ttf" "Leasy-SOFT" 0.3, 0
    scale 1.15
    pigment { Gold }
    finish { reflection .25 specular 1 }
    translate <-3.4, -0.25, -1.2>
    rotate <180, 0, 0>
  }

  text
  {
    ttf "Verdana.ttf" "easy-Word" 0.3, 0
    scale 1.15
    pigment { Gold }
    finish { reflection .25 specular 1 }
    translate <-2.9, -0.25, -1.2>
    rotate <270, 0, 0>
  }
//}
