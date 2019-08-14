#version 120

void main(){
   vec4 eyeCoord  = gl_ModelViewMatrix * gl_Vertex;
   gl_Position    = gl_ProjectionMatrix * eyeCoord;	
   //gl_Position = ftransform();
}
