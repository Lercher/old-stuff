// Martin Lercher 18:23 02.02.1996 

#include "colors.inc"
#include "textures.inc"
#include "shapes.inc"

camera {
	location <0, 2, -6>
	direction z*2
	look_at <0, 1, 2>
}

fog {color Gray60 distance 30}

plane {
	y,0
	pigment {
		checker
			color Red
			color Blue
	}
}

difference {
	sphere { -x, 1.1 }
	sphere { +x, 1.5}
	pigment { color Gray60 }
	finish {reflection 1 ambient 0 diffuse 0.2}
	scale 1.5
	rotate 70*y
	translate y
}

//cone {
//	y,0.3
//	<1,2,3>,1.0
//	open
//	pigment { DMFWood4 scale 4 }
//	finish {Shiny}
//}

//box {
//	<-1,0,0>,
//	<1,0.5,3>
//	pigment { DMFWood4 scale 4 }
//	rotate y*20
//}


//sphere {
//	<0, 1, 2>, 1
//	texture {
//		PinkAlabaster
//	}
//}

// light_source {	<-12, 14, -1> color Gray }
light_source {	<2, 4, -6> color White }