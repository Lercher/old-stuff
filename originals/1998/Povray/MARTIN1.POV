// M. Lercher 3.2.96
#include "colors.inc"
#include "textures.inc"

#declare R = 0.33
#declare M1 = <-1, -1, 0>
#declare M2 = <-1, 1, 0>
#declare M3 = <0, -0.3, 0>
#declare M4 = <1, 1, 0>
#declare M5 = <1, -1, 0>
#declare L1 = <0, 1, 0>
#declare L2 = <0, -1, 0>
#declare L3 = <1.5, -1, 0>

camera{
  location <-1, 2, -10>
  direction z*2
  look_at <0, 2.3, 0>
}

fog { color SkyBlue distance 50 }

light_source { <4, 7, -5> color White }

plane {
	y, -1
	pigment { Blue }
}

plane {
  y, 0
  texture { Water }
}//end plane

union {
	sphere { M1, R }
	sphere { M2, R }
	sphere { M3, R }
	sphere { M4, R }
	sphere { M5, R }
	cylinder { M1, M2, R }
	cylinder { M2, M3, R }
	cylinder { M3, M4, R }
	cylinder { M4, M5, R }
	texture {
		Polished_Chrome
		normal {bumps 0.1 scale 0.1}
	}
	scale <1, 1.5, 1>
	translate y*2
	translate -x
}

union {
	sphere { L1, R }
	sphere { L2, R }
	sphere { L3, R }
	cylinder { L1, L2, R }
	cylinder { L2, L3, R }
	texture {
		Gold_Metal
		normal {bumps 0.1 scale 0.1}
	}
	scale <1, 1.5, 1>
	translate y*2
	translate x*0.8
}
