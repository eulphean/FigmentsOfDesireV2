#version 150

out vec4 outputColor;
uniform float time; 
uniform vec2 resolution; 
uniform vec2 mouse;

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
	vec2 p = -1.0 + 2.0 * q;
	vec2 m = -1.0 + 2.0 / resolution.xy;
	m.y = -m.y;
	p.y *= (resolution.x/resolution.y);

	// EXTERIOR.
	vec3 baseColor = vec3(0.77255, 0.78039, 0.78039);
	vec3 topColor;
	topColor.x = 0.062 + 0.7*fbm(2.0*p + vec2(time*0.4, time*0.5));
	topColor.y = 0.670 + 0.4*fbm(1.5*p + vec2(time*0.1, time*0.2));
	topColor.z = 0.796 + 0.2*fbm(1.0*p + vec2(time*0.3, time*0.3));

	float f = fbm( 10.0*p+time );
	baseColor = mix(baseColor, topColor, f);

	outputColor = vec4(baseColor,1.0);
}