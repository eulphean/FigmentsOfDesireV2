#version 120

uniform float time; 
uniform vec2 resolution; 
uniform bool isOccupied; 

// ------------------------------------------------------- //
// Noise Helpers
// ------------------------------------------------------- //
// float hash( float n )
// {
// 	return fract(sin(n)*43758.5453);
// }

// float noise( in vec2 x )
// {
// 	vec2 p = floor(x);
// 	vec2 f = fract(x);
// 	f = f*f*(3.0-2.0*f);
// 	float n = p.x + p.y*57.0;
// 	float a = mix(hash(n + 0.0), hash(n +  1.0),f.x); 
// 	float b = mix(hash(n + 57.0), hash(n + 58.0),f.x); 
// 	float res = mix(a, b, f.y);
// 	return res;
// }

// 2D Random
float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))
                 * 43758.5453123);
}

// 2D Noise based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    // Smooth Interpolation

    // Cubic Hermine Curve.  Same as SmoothStep()
    vec2 u = f*f*(3.0-2.0*f);
    // u = smoothstep(0.,1.,f);

    // Mix 4 coorners percentages
    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
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

vec4 mixFixed( in vec4 v1, in vec4 v2, in float a )
{
    vec4 result;
    result.x = v1.x * v1.x * (1 - a) + v2.x * v2.x * a;
    result.y = v1.y * v1.y  * (1 - a) + v2.y * v2.y * a;
    result.z = v1.z * v1.z  * (1 - a) + v2.z * v2.z * a;
    result.w = v1.w * v1.w  * (1 - a) + v2.w * v2.w * a;

    result.x = sqrt(result.x);
    result.y = sqrt(result.y);
    result.z = sqrt(result.z);
    result.w = sqrt(result.w);
   
    return result;
}

float fbmNew( vec2 p )
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
	float f = fbmNew( 5.0*p+newTime*0.002 );

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