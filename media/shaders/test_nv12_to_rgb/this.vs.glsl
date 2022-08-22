#version 120

#if __VERSION__==120
#define layout_location(p) attribute
#define out_varing  varying
#else
#define layout_location(p) layout(location=p)
#define out_varing  out
#endif
layout_location(0) vec3 aPos;
layout_location(3) vec2 aTexCoord;
//layout(location=1)in vec3 aCol;
layout_location(5)float aChooseTex;


// layout_location(0) vec2 aTexCoord;
// layout_location(2) vec3 aPos;
// //layout(location=1)in vec3 aCol;
// layout_location(5)float aChooseTex;

out_varing vec4 color;
out_varing vec2 texCoord;
out_varing float ChooseTex;
out_varing vec3 Pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int primitive_id;
void main(){
    // gl_Position=vec4(aPos,1)+vec4(offset[0],offset[1],offset[2],0);
    // gl_Position=projection*view*model*vec4(aPos,1);
    gl_Position=
    projection*
    view*
    model*
    vec4(aPos,1);
    texCoord=aTexCoord;
    Pos=aPos;
    //gl_Position =normalize(gl_Position);
   // color=vec4(aCol,1.f);
}