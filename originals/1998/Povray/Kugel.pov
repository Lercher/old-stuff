
// Persistence of Vision Ray Tracer Scene Description File
// File: .pov
// Vers: 3
// Desc:
// Date:
// Auth:


// ==== Standard POV-Ray Includes ====
#include "colors.inc"	// Standard Color definitions
#include "textures.inc"	// Standard Texture definitions


// Set a color of the background (sky)
background { color SkyBlue }

camera
{
  location  <0.8 , 2.0 ,-5.0>
  look_at   <0.5 , 0.0 , 0.0>
}

// create a sphere shape
sphere
{
  <0, 1, 0> // center of sphere <X Y Z>
  2       // radius of sphere
  pigment { White filter 1 }
  finish {
    ambient 0
    diffuse 0
    reflection 0.25
    phong 1
    phong_size 100
    refraction 1
    ior 1.45
    specular 0.5
  }
}

// An infinite planar surface
// plane {<A, B, C>, D } where: A*x + B*y + C*z = D
plane
{
  y, // <X Y Z> unit surface normal, vector points "away from surface"
  -4.0 // distance from the origin in the direction of the surface normal
  pigment { checker Blue, Green }
  translate <0.33, 0.6, 0>
}

// create a regular point light source
light_source
{
  <10, 2, -5>
  color red 1.0  green 1.0  blue 1.0  // light's color
}
light_source
{
  <-5, 8, 6>
  color red 1.0  green 0.5  blue 0.5  // light's color
}
 
