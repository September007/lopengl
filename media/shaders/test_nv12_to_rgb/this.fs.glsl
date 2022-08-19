#version 330
//in vec4 color;
in vec2 texCoord;
in vec4 color;
out vec4 FragColor;

uniform vec4 offset;
uniform sampler2D ourTexture1;
uniform sampler2D ourTexture2;
// use offset to change linear param
bool floatEqual(in float f,const in float limit){
    return abs(f-limit)<.01;
}

// map[0]=0;map[1]=1;map[other]=2
int AxisMap(float f){
    if(floatEqual(f,0))
    return 0;
    if(floatEqual(f,1))
    return 1;
    return 2;
}
void main(){
    // if(length(texCoord)!=0){
    //     FragColor=vec4(1,0,0,1);
    //     FragColor=texture(ourTexture2,texCoord);
    //     return;
    // }
    vec4 coord=gl_FragCoord;
    int ax=AxisMap(coord.x),ay=AxisMap(coord.y);
    int sw=ax*3+ay;
    if(ax==0||ay==0){
        FragColor=vec4(1,0,0,1);
        return;
    }
    switch(sw){
        case 0:
        FragColor=vec4(0,0,0,1);
        return;
        case 3:
        FragColor=vec4(1,0,0,1);
        return;
        case 1:
        FragColor=vec4(0,1,0,1);
        return;
        default:
        if(texCoord.x>0)
        FragColor=texture(ourTexture2,texCoord);
        else
        FragColor=texture(ourTexture1,texCoord);
    }
}