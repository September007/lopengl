#version 330

#define PRIMITIVE_NONE  0
#define PRIMITIVE_CIRCLE_POINT 1
#define PRIMITIVE_LINE 2

//in vec4 color;
in vec2 texCoord;
in vec3 Pos;
in vec4 color;
out vec4 FragColor;
uniform vec4 offset;
uniform sampler2D ourTexture1;
uniform sampler2D ourTexture2;
uniform int primitive_id;
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
    // draw O,X,Y point
    if(primitive_id==PRIMITIVE_CIRCLE_POINT){
        vec2 relative_point=gl_PointCoord-vec2(0.5,0.5);
        if(length(relative_point)>0.5)
            {
                discard;
                FragColor=vec4(0.0118, 0.0118, 0.0118, 0.0);
            }
        else
            FragColor=vec4(0.0627, 0.851, 0.9059, 1.0);
            return ;
    }
    
    // draw Line O-X , O-Y
    vec4 coord=vec4(Pos,1);
    int ax=AxisMap(coord.x),ay=AxisMap(coord.y);
    if(ax==0&&ay==0)
        // O
        {
            //discard;
            FragColor=vec4(0,0,0,1);
        }
    else if (ay==0)
        // X  axis
        FragColor=vec4(1,0,0,1);
    else if(ax==0)
        // Y  axis 
        FragColor=vec4(0,1,0,1);        
}