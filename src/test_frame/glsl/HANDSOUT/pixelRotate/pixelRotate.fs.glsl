#version 120

#if __VERSION__<130
#define TEXTURE2D texture2D
#else
#define TEXTURE2D texture
#endif

#define SWITCH(var)int v_var=var;{do{
    #define CASE(val)}if(v_var==val){
    #define DEFAULT()};
#define ENDSWITCH()}while(false);

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
uniform int rotateType; //0:clockwise 90;1:clockwise 180;2:clockwise 270;3:flip X; 4:flip Y
uniform sampler2D overlay;                     
// uniform int overlay_Width;
// uniform int overlay_Height;


varying vec4 vs_output_position;
varying vec2 vs_output_textureUV;

vec4 PS_2D( vec2  TextureUV)
{
    vec2 tc =  TextureUV;
    vec2 temCoord = tc;
    vec4 color = vec4(0.0,0.0,0.0,0.0); 
        //PROTATE_90_CLOCKWISE  = 0,
        //PROTATE_180 = 1,
        //PROTATE_270_CLOCKWISE = 2, 
        //PMIRROR_VERT = 3,
        //PMIRROR_HORZ = 4
    SWITCH(rotateType )
    {
        CASE(0)
                temCoord = vec2(temCoord.y,1.0 - temCoord.x);
                break;
        CASE(1)
                temCoord = 1.0 - temCoord;
                break;
        CASE(2)
                temCoord =  vec2(1.0 - temCoord.y,temCoord.x);
                break;
        CASE(3)
                temCoord =  vec2(1.0 - temCoord.x,temCoord.y); 
                break;
        CASE(4)
                temCoord =  vec2(temCoord.x,1.0 - temCoord.y); 
                break;
        DEFAULT()
                temCoord = vec2(temCoord.y,1.0 - temCoord.x);
				break;
        ENDSWITCH()
    }
 
	color = TEXTURE2D(overlay,temCoord);
	return color;
}


void main(){
    gl_FragColor= PS_2D(vs_output_textureUV);
}
