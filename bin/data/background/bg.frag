#version 120

uniform float time; 
uniform vec2 resolution; 

// ------------------------------------------------------- //
// Noise Helpers
// ------------------------------------------------------- //
float hash( float n )
{
	return fract(sin(n)*43758.5453);
}

float noise( in vec2 x )
{
	vec2 p = floor(x);
	vec2 f = fract(x);
	f = f*f*(3.0-2.0*f);
	float n = p.x + p.y*57.0;
	float res = mix(mix( hash(n+  0.0), hash(n+  1.0),f.x), mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y);
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
	vec2 q = gl_FragCoord.xy / resolution.xy;
	vec2 p = -1.0 + 1.5 * q;
	vec2 m = -1.0 + 2.0 / resolution.xy;
	m.y = -m.y;
	p.y *= (resolution.x/resolution.y);

	// Base and Top colors are mixed to create a background.
	// vec3 baseColor = vec3(0.77255, 0.78039, 0.78039);
	vec3 baseColor = vec3(0.662, 0.847, 0.917);
	vec3 topColor = vec3(0.278, 0.729, 0.941); 

	// topColor.x = clamp(0.278 + 0.941*fbm(2.0*p + vec2(time*0.4, time*0.1)), 0, 1.0);
	// topColor.y = clamp(0.729 + 0.470*fbm(1.5*p + vec2(time*0.1, time*0.2)), 0, 1.0);
	// topColor.z = clamp(0.941 + 0.278*fbm(1.0*p + vec2(time*0.3, time*0.1)), 0, 1.0);

	// topColor.x = clamp(0.278 + 0.941*fbm(2.0*p + vec2(time*0.4, time*0.1)), 0, 1.0);
	// topColor.y = clamp(0.729 + 0.470*fbm(1.5*p + vec2(time*0.1, time*0.2)), 0, 1.0);
	// topColor.z = clamp(0.941 + 0.278*fbm(1.0*p + vec2(time*0.3, time*0.1)), 0, 1.0);

	// float f = fbm( 5.0*p+time*0.002 );
	float f = fbm(5.0*q + time*0.002);
	baseColor = mix(baseColor, topColor, sin(time));

	gl_FragColor = vec4(baseColor,1.0);
}
