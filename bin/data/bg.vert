#version 150

// Passed in from OF programmable renderer. 
uniform mat4 modelViewProjectionMatrix;
// Passed from the mesh by default. 
in vec4 position;

void main(){
    gl_Position = modelViewProjectionMatrix * position;
}
