#version 120

uniform float time; 
uniform vec2 resolution; 
uniform bool isOccupied; 

// ------------------------------------------------------- //
// Noise Helpers
// ------------------------------------------------------- //
float hash( float n )
{
	//return fract(sin(n)*43758.5453);
	return fract(sin(n)); // By not multiplying with such a large factor, it fixes the clipping mac os mojave.  
}

float noise( in vec2 x )
{
	vec2 p = floor(x);
	vec2 f = fract(x);
	f = f*(3-2.0*f);
	float n = p.x + p.y*57.0;
	float a = mix(hash(n + 0.0), hash(n +  1.0),f.x); 
	float b = mix(hash(n + 57.0), hash(n + 58.0),f.x); 
	float res = mix(a, b, f.y);
	return res;
}

float fbm( vec2 p )
{
	float f = 0.0;
	mat2 m = mat2( 0.8,  0.6, -0.6,  0.6 );
	f += 0.50000*noise( p ); p = m*p*2.02;
	f += 0.25000*noise( p ); p = m*p*2.03;
	f += 0.12500*noise( p ); p = m*p*2.01;
	f += 0.06250*noise( p ); p = m*p*2.04;
	f += 0.03125*noise( p );
	return f/0.984375;
}

// ------------------------------------------------------- //
// BEGIN.
// ------------------------------------------------------- //
void main(void)
{    	
	float newTime = time/1000; 
	vec2 q = gl_FragCoord.xy / resolution.xy;
	vec2 p = -1.0 + 1.5 * q;
	vec2 m = -1.0 + 2.0 / resolution.xy;
	m.y = -m.y;
	p.y *= (resolution.x/resolution.y);

	// Base and Top colors are mixed to create a background.
	vec3 baseColor = vec3(0.662, 0.847, 0.917);
	vec3 occupiedColor; vec3 emptyColor; 
	
	if (isOccupied) {
	   occupiedColor.x = 0.91 + 0.09*fbm(2.0*p + vec2(newTime*0.4, newTime*0.1));
	   occupiedColor.y = 0.18 + 0.82*fbm(1.5*p + vec2(newTime*0.1, newTime*0.2));
	   occupiedColor.z = 0.12 + 0.88*fbm(1.0*p + vec2(newTime*0.3, newTime*0.1));
	   gl_FragColor = vec4(occupiedColor,1.0);
	} else {
               emptyColor.x = 0.10 + 0.90*fbm(2.0*p + vec2(newTime*0.4, newTime*0.1));
	   emptyColor.y = 0.43 + 0.57*fbm(1.5*p + vec2(newTime*0.1, newTime*0.2));
	   emptyColor.z = 0.75 + 0.25*fbm(1.0*p + vec2(newTime*0.3, newTime*0.1));
	   gl_FragColor = vec4(emptyColor,1.0);
	}
}

// baseColor = mix(baseColor, topColor, f);
// Pure blue
// emptyColor.x = 0.27 + 0.73*fbm(2.0*p + vec2(newTime*0.4, newTime*0.1));
// emptyColor.y = 0.72 + 0.28*fbm(1.5*p + vec2(newTime*0.1, newTime*0.2));
// emptyColor.z = 0.94 + 0.06*fbm(1.0*p + vec2(newTime*0.3, newTime*0.1));