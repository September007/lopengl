#version 120

#if __VERSION__<130
#define TEXTURE2D texture2D
#else
#define TEXTURE2D texture
#endif
#define Texture2D  sampler2D  

#define SWITCH(var)int v_var=var;{do{
    #define CASE(val)}if(v_var==val){
    #define DEFAULT()};
#define ENDSWITCH()}while(false);

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
uniform int rotateType; //0:clockwise 90;1:clockwise 180;2:clockwise 270;3:flip X; 4:flip Y
uniform Texture2D overlay;        
uniform float    g_fTime;                   
uniform int overlay_Width;
uniform int overlay_Height;


struct VS_INPUT
{
    vec4 Position; // vertex position 
    vec2 TextureUV;   // vertex texture coords 
};

struct VS_OUTPUT
{
    vec4 Position; // vertex position 
    vec2 TextureUV;  // vertex texture coords 
}vs_output;

varying vec4 vs_output_position;
varying vec2 vs_output_textureUV;
//--------------------------------------------------------------------------------------
// Pixel shader output structure
//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
    vec4 RGBColor;  // Pixel color
};


//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
//       color with diffuse material color
//--------------------------------------------------------------------------------------


PS_OUTPUT PS_2D( VS_OUTPUT In)
{ 
    PS_OUTPUT Output;
    float ow = float(overlay_Width);
    float oh = float(overlay_Height);
   
    vec2 tc = In.TextureUV;
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
	
	Output.RGBColor = color;
    return Output;
}

// 
VS_OUTPUT VS_2D(VS_INPUT vin)
{
	VS_OUTPUT vOut;
	vOut.Position = vin.Position;
	vOut.TextureUV = vin.TextureUV;
	return vOut;
}

void main(){
    vs_output.Position=vs_output_position;
    vs_output.TextureUV=vs_output_textureUV;
    
    PS_OUTPUT ps_output=PS_2D(vs_output);

    gl_FragColor=ps_output.RGBColor;
}
