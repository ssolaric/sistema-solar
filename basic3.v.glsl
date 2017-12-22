attribute vec3 coord3d;
attribute vec3 color;
uniform mat4 mvp;
//uniform mat4 model;

varying vec3 f_color;

void main(void){
    gl_Position = mvp * vec4(coord3d, 1);
    f_color = color;
}
