#version 120

uniform float time; 
uniform vec2 resolution; 
uniform float bgState; 

// ------------------------------------------------------- //
// Noise Helpers
// ------------------------------------------------------- //
float hash( float n )
{
	//return fract(sin(n)*43758.5453);
	return fract(sin(n+0.5) * 0.5); // By not multiplying with such a large factor, it fixes the clipping mac os mojave.  
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
	vec3 color;

	  // oneColor.x = 0.10 + 0.90*fbm(2.0*p + vec2(newTime*0.4, newTime*0.1));
	  // oneColor.y = 0.43 + 0.57*fbm(1.5*p + vec2(newTime*0.1, newTime*0.2));
	  // oneColor.z = 0.75 + 0.25*fbm(1.0*p + vec2(newTime*0.3, newTime*0.1));
	  // gl_FragColor = vec4(oneColor,1.0);
	
	if (bgState == 1) {
	   color.x = 0.10 + 0.90*fbm(2.0*p + vec2(newTime*0.4, newTime*0.1));
	   color.y = 0.43 + 0.57*fbm(1.5*p + vec2(newTime*0.1, newTime*0.2));
	   color.z = 0.75 + 0.25*fbm(1.0*p + vec2(newTime*0.3, newTime*0.1));
	   gl_FragColor = vec4(color,1.0);
	} else if (bgState == 2) {
	   color.x = 0.08 + 0.964*fbm(2.0*p + vec2(newTime*0.4, newTime*0.1));
	   color.y = 0.39 + 0.917*fbm(1.5*p + vec2(newTime*0.1, newTime*0.2));
	   color.z = 0.70 + 0.552*fbm(1.0*p + vec2(newTime*0.3, newTime*0.1));
	   gl_FragColor = vec4(color,1.0);
	} else if (bgState == 3) {
	   color.x = 0.06 + 0.964*fbm(2.0*p + vec2(newTime*0.4, newTime*0.1));
	   color.y = 0.30 + 0.85*fbm(1.5*p + vec2(newTime*0.1, newTime*0.2));
	   color.z = 0.68 + 0.45*fbm(1.0*p + vec2(newTime*0.3, newTime*0.1));
	   gl_FragColor = vec4(color,1.0);
	} else if (bgState == 4) {
	  color.x = 0.91 + 0.09*fbm(2.0*p + vec2(newTime*0.4, newTime*0.1));
	  color.y = 0.18 + 0.82*fbm(1.5*p + vec2(newTime*0.1, newTime*0.2));
	  color.z = 0.12 + 0.88*fbm(1.0*p + vec2(newTime*0.3, newTime*0.1));
	  gl_FragColor = vec4(color,1.0);
	} else if (bgState == 5) {
	  color.x = 0.87 + 0.09*fbm(2.0*p + vec2(newTime*0.4, newTime*0.1));
	  color.y = 0.16 + 0.78*fbm(1.5*p + vec2(newTime*0.1, newTime*0.2));
	  color.z = 0.08 + 0.75*fbm(1.0*p + vec2(newTime*0.3, newTime*0.1));
	  gl_FragColor = vec4(color,1.0);
	}
}



	