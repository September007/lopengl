#version 120

#pragma optimize(off)
#if __VERSION__<130
#define TEXTURE2D texture2D
#else
#define TEXTURE2D texture
#endif

#define Texture2D  sampler2D  
// clang-format off
#define SWITCH(var)int v_var=var;{do{
    #define CASE(val)}if(v_var==val){
    #define DEFAULT()};
#define ENDSWITCH()}while(false);
//clang-format on

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
uniform int blendMode;
uniform float kRender_Alpha;
uniform int ovlAlphaPreMul;
uniform Texture2D overlay;// Color texture for mesh
uniform Texture2D background;

uniform int overlay_Width;
uniform int overlay_Height;
uniform int back_Width;
uniform int back_Height;
uniform float blend_x;
uniform float blend_y;

varying vec4 vs_output_position;
varying vec2 vs_output_textureUV;
struct VS_INPUT
{
    vec4 Position;// vertex position
    vec2 TextureUV;// vertex texture coords
};

struct VS_OUTPUT
{
    vec4 Position;// vertex position
    vec2 TextureUV;// vertex texture coords
}vs_output;
//--------------------------------------------------------------------------------------
// Pixel shader output structure
//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
    vec4 RGBColor;// Pixel color
}ps_output;

//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
//       color with diffuse material color
//--------------------------------------------------------------------------------------
vec4 colorDodge(vec4 bgCol,vec4 overlay)
{
    vec4 outputColor=bgCol/(1.-overlay);
    
    if(overlay.x>.99999)
    outputColor.x=1.;
    if(overlay.y>.99999)
    outputColor.y=1.;
    if(overlay.z>.99999)
    outputColor.z=1.;
    
    return outputColor;
}

vec4 colorBurn(vec4 bgCol,vec4 overlay)
{
    
    vec4 outputColor=1.-(1.-bgCol)/overlay;
    
    if(overlay.x<.000001)
    outputColor.x=0.;
    if(overlay.y<.000001)
    outputColor.y=0.;
    if(overlay.z<.000001)
    outputColor.z=0.;
    return outputColor;
}

vec4 colorDodgeForHardMix(vec4 bgCol,vec4 overlay)
{
    vec4 outputColor=bgCol/(1.-overlay);
    
    if(bgCol.x<.000001)
    outputColor.x=0.;
    if(bgCol.y<.000001)
    outputColor.y=0.;
    if(bgCol.z<.000001)
    outputColor.z=0.;
    
    return outputColor;
}

vec4 colorBurnForHardMix(vec4 bgCol,vec4 overlay)
{
    
    vec4 outputColor=1.-(1.-bgCol)/overlay;
    
    if(bgCol.x>.9999999)
    outputColor.x=1.;
    if(bgCol.y>.9999999)
    outputColor.y=1.;
    if(bgCol.z>.9999999)
    outputColor.z=1.;
    return outputColor;
}

//tempMatt: matt without alpha
//matt: matt with altph.

vec4 blending(vec4 backGround,vec4 ovl,float matt,float tempMatt,float exeMatt,int blendingMode,float opacity,int ovlAlphaPreMul)
{
    vec3 a=vec3(0.,0.,0.);
    vec3 b=vec3(0.,0.,0.);
    vec4 outputColor=vec4(0.,0.,0.,0.);
    vec4 overlay=ovl*tempMatt;
    vec4 bgCol=backGround;
    float tempOpacity=opacity*matt*exeMatt;
    float invTemOpacity=1.-tempOpacity;
   SWITCH( blendingMode)
    {
        CASE(0)// normal,
        //bgCol = vec4(bgCol.xyz*bgCol.w, bgCol.w);
        
        outputColor=overlay;
        if(ovlAlphaPreMul==0)
        {
            outputColor.w=tempOpacity+invTemOpacity*bgCol.w;
            outputColor.xyz=outputColor.xyz*tempOpacity+invTemOpacity*bgCol.xyz;
            return outputColor;
        }
        else{
            outputColor.w=tempOpacity+invTemOpacity*bgCol.w;
            float fOpacity=opacity*tempMatt;
            outputColor.xyz=outputColor.xyz*fOpacity+invTemOpacity*bgCol.xyz;
            return outputColor;
        } 
        CASE(1)// Darken
        outputColor=min(overlay,bgCol);
        break;
        CASE(2)//multiply
        outputColor=bgCol*overlay;
        break;
        CASE(3)//  color burn // 1 - (1-Target) / Blend
        {
            vec4 temp=(1.-bgCol)/overlay;
            if(bgCol.x>.99999)
            temp.x=0.;
            if(bgCol.y>.99999)
            temp.y=0.;
            if(bgCol.z>.99999)
            temp.z=0.;
            outputColor=1.-temp;
        }
        break;
        CASE(4)// Linear burn
        outputColor=overlay+bgCol-1.;
        break;
        CASE(5)//screen
        outputColor=1.-(1.-bgCol)*(1.-overlay);
        break;
        CASE(6)//color dodge
        {
            outputColor=bgCol/(1.-overlay);
            if(bgCol.x<.00001)
            outputColor.x=0.;
            if(bgCol.y<.00001)
            outputColor.y=0.;
            if(bgCol.z<.00001)
            outputColor.z=0.;
        }
        break;
        CASE(7)//Linear Dodge
        outputColor=overlay+bgCol;
        break;
        CASE(8)//overlay // (Target > 0.5) * (1 - (1-2*(Target-0.5)) * (1-Blend)) + (Target <= 0.5) * ((2*Target) * Blend)
        {
            a=vec3((bgCol.x>.5?1.:0.),(bgCol.y>.5?1.:0.),(bgCol.z>.5?1.:0.));
            b=vec3((bgCol.x<=.5?1.:0.),(bgCol.y<=.5?1.:0.),(bgCol.z<=.5?1.:0.));
            outputColor.xyz=a*(1.-(1.-2.*(bgCol.xyz-.5))*(1.-overlay.xyz))+b*((2.*bgCol.xyz)*overlay.xyz);
        }
        break;
        CASE(9)//Soft Light //
        {
            a=vec3((overlay.x>.5?1.:0.),(overlay.y>.5?1.:0.),(overlay.z>.5?1.:0.));
            b=vec3((overlay.x<=.5?1.:0.),(overlay.y<=.5?1.:0.),(overlay.z<=.5?1.:0.));
            outputColor.xyz=a*(2.*bgCol.xyz*(1.-overlay.xyz)+sqrt(bgCol.xyz)*(2.*overlay.xyz-1.))+b*(2.*bgCol.xyz*overlay.xyz+bgCol.xyz*bgCol.xyz*(1.-2.*overlay.xyz));
        }
        break;
        CASE(10)//Hard Light //(Blend > 0.5) * (1 - (1-Target) * (1-2*(Blend-0.5))) + (Blend <= 0.5) * (Target * (2*Blend))
        {
            a=vec3(float(overlay.x>.5?1.:0.),float(overlay.y>.5?1.:0.),float(overlay.z>.5?1.:0.));
            b=vec3(float(overlay.x<=.5?1.:0.),float(overlay.y<=.5?1.:0.),float(overlay.z<=.5?1.:0.));
            outputColor.xyz=a*(1.-(1.-bgCol.xyz)*(1.-2.*(overlay.xyz-.5)))+b*(bgCol.xyz*(2.*overlay.xyz));
        }
        break;
        CASE(11)//vivid light //// (Blend > 0.5) * (1 - (1-Target) / (2*(Blend-0.5))) + (Blend <= 0.5) * (Target / (1-2*Blend))
        {
            a=vec3(float(overlay.x>.5?1.:0.),float(overlay.y>.5?1.:0.),float(overlay.z>.5?1.:0.));
            b=vec3(float(overlay.x<=.5?1.:0.),float(overlay.y<=.5?1.:0.),float(overlay.z<=.5?1.:0.));
            outputColor.xyz=b*colorBurn(bgCol,(2.*overlay)).xyz+a*colorDodge(bgCol,(2.*(overlay-.5))).xyz;
        }
        break;
        CASE(12)// Linear Light//  (Blend > 0.5) * (Target + 2*(Blend-0.5)) + (Blend <= 0.5) * (Target + 2*Blend - 1)
        {
            a=vec3(float(overlay.x>.5?1.:0.),float(overlay.y>.5?1.:0.),float(overlay.z>.5?1.:0.));
            b=vec3(float(overlay.x<=.5?1.:0.),float(overlay.y<=.5?1.:0.),float(overlay.z<=.5?1.:0.));
            outputColor.xyz=a*(bgCol.xyz+2.*(overlay.xyz-.5))+b*(bgCol.xyz+2.*overlay.xyz-1.);
        }
        break;
        CASE(13)//PIN Light// (Blend > 0.5) * (max(Target,2*(Blend-0.5))) + (Blend <= 0.5) * (min(Target,2*Blend)))
        {
            a=vec3(float(overlay.x>.5?1.:0.),float(overlay.y>.5?1.:0.),float(overlay.z>.5?1.:0.));
            b=vec3(float(overlay.x<=.5?1.:0.),float(overlay.y<=.5?1.:0.),float(overlay.z<=.5?1.:0.));
            outputColor.xyz=a*(max(bgCol.xyz,2.*(overlay.xyz-.5)))+b*(min(bgCol.xyz,2.*overlay.xyz));
        }
        break;
        CASE(14)// hardmix  (VividLight(A,B) < 128) ? 0 : 255
        {
            a=vec3(float(overlay.x>.5?1.:0.),float(overlay.y>.5?1.:0.),float(overlay.z>.5?1.:0.));
            b=vec3(float(overlay.x<=.5?1.:0.),float(overlay.y<=.5?1.:0.),float(overlay.z<=.5?1.:0.));
            outputColor.xyz=b*colorBurnForHardMix(bgCol,(2.*overlay)).xyz+a*colorDodgeForHardMix(bgCol,(2.*(overlay-.5))).xyz;
            outputColor.xyz=vec3(float(outputColor.x>=.5?1.:0.),float(outputColor.y>=.5?1.:0.),float(outputColor.z>=.5?1.:0.));
            //outputColor.xyz = vec3( float(overlay.x + bgCol.x >= 1.0?1.0:0.0), float(overlay.y + bgCol.y >= 1.0?1.0:0.0),float(overlay.z + bgCol.z >= 1.0?1.0:0.0));
        }
        break;
        CASE(15)//Difference
        outputColor=abs(overlay-bgCol);
        break;
        CASE(16)//exclusion // 0.5 - 2*(Target-0.5)*(Blend-0.5)
        outputColor=.5-2.*(overlay-.5)*(bgCol-.5);
        break;
        CASE(17)//Lighten // max(Target,Blend)
        outputColor=max(overlay,bgCol);
        break;
        CASE(19)// hollow in
        outputColor=bgCol*overlay.w;
        outputColor=clamp(outputColor,vec4(0.,0.,0.,0.),vec4(1.,1.,1.,1.));
        break;
        CASE(20)// hollow out
        outputColor=bgCol;
        if(bgCol.w<.000001)
        outputColor=overlay;
        else
        outputColor=bgCol*(1.-overlay.w);
        outputColor=clamp(outputColor,vec4(0.,0.,0.,0.),vec4(1.,1.,1.,1.));
        break;
        CASE(21)// backGround hollow in
        outputColor=overlay*bgCol.w;
        outputColor=clamp(outputColor,vec4(0.,0.,0.,0.),vec4(1.,1.,1.,1.));
        break;
        CASE(22)// replace
        if(tempMatt*exeMatt>.0001)
        return vec4(overlay.xyz,overlay.w);
        else
        return bgCol;
        CASE(23)//add
        outputColor=bgCol+overlay;
        break;
        DEFAULT()
        bgCol=vec4(bgCol.xyz*bgCol.w,bgCol.w);
        outputColor=overlay;
        
        if(ovlAlphaPreMul==0)
        {
            tempOpacity=opacity*matt*exeMatt;
        }
        else{
            tempOpacity=opacity*tempMatt*exeMatt;
        }
        outputColor=clamp(outputColor,vec4(0.,0.,0.,0.),vec4(1.,1.,1.,1.));
        outputColor.w=overlay.w+(1.-overlay.w)*bgCol.w;
        outputColor.xyz=outputColor.xyz*tempOpacity+invTemOpacity*bgCol.xyz;
        //outputColor.xyz = clamp( outputColor.xyz / outputColor.w, vec3(0.0), vec3(1.0) );
        return outputColor;
        ENDSWITCH()
    }
    
    outputColor=clamp(outputColor,vec4(0.,0.,0.,0.),vec4(1.,1.,1.,1.));
    outputColor.w=overlay.w+(1.-overlay.w)*bgCol.w;
    outputColor.xyz=outputColor.xyz*tempOpacity+invTemOpacity*bgCol.xyz;
    outputColor.xyz=clamp(outputColor.xyz,vec3(0.,0.,0.),vec3(1.,1.,1.));
    
    return outputColor;
    
}

#define EQN_EPS 1e-9f

bool isZero(float x){
    return(x>-EQN_EPS&&x<EQN_EPS);
}

PS_OUTPUT PS_2D(VS_OUTPUT In)
{
    PS_OUTPUT Output;
    float ow=float(overlay_Width);
    float oh=float(overlay_Height);
    float bw=float(back_Width);
    float bh=float(back_Height);
    float roi_x0=blend_x*bw;
    float roi_y0=blend_y*bh;
    float roi_x1=roi_x0+ow;
    float roi_y1=roi_y0+oh;
    roi_x1=clamp(roi_x1,0.,bw);
    roi_y1=clamp(roi_y1,0.,bh);
    float roi_width=roi_x1-roi_x0;
    float roi_height=roi_y1-roi_y0;
    roi_x0=roi_x0/bw;
    roi_y0=roi_y0/bh;
    roi_x1=roi_x1/bw;
    roi_y1=roi_y1/bh;
    
    float over_x0=0.;
    float over_y0=0.;
    float over_x1=roi_width/ow;
    float over_y1=roi_height/oh;
    vec2 tc=In.TextureUV;
    float matt_b=step(roi_x0,tc.x)*step(tc.x,roi_x1)*step(roi_y0,tc.y)*step(tc.y,roi_y1);
    
    float resizeCoord_x=(tc.x-roi_x0)*roi_width/bw+over_x0;
    float resizeCoord_y=(tc.y-roi_y0)*roi_height/bh+over_y0;
    vec2 resizeCoord=vec2(resizeCoord_x,resizeCoord_y);
    
    vec4 bgCol=TEXTURE2D(background,tc);
    if(isZero(matt_b)){
        Output.RGBColor=bgCol;
        return Output;
    }
    
    vec4 ovlCol=TEXTURE2D(overlay,resizeCoord);
    
    float grid=ovlCol.w;
    float roiMat=matt_b;
    
    vec4 FragColor=blending(bgCol,ovlCol,grid,roiMat,1.,blendMode,kRender_Alpha,ovlAlphaPreMul);
    
    Output.RGBColor=FragColor;
    return Output;
}

void main(){
    vec2 tc=vs_output_textureUV;
    vs_output.Position=vs_output_position;
    vs_output.TextureUV=vs_output_textureUV;
    
    ps_output=PS_2D(vs_output);

    gl_FragColor=ps_output.RGBColor;
    // gl_FragColor=texture2D(
    //     background//overlay
    //     ,tc);
}