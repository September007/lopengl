#version 120

#pragma optimize(off)
#if __VERSION__<130
#define TEXTURE2D texture2D
#else
#define TEXTURE2D texture
#endif
//#define Texture2D sampler2D

uniform sampler2D overlay;// Color texture for mesh
uniform int mode;
uniform int overlay_Width;
uniform int overlay_Height;
uniform int back_Width;
uniform int back_Height;
// clang-format off
#define SWITCH(var)int v_var=var;{do{
    #define CASE(val)}if(v_var==val){
    #define DEFAULT()};
#define ENDSWITCH()}while(false);
//clang-format on

#define EQN_EPS 1e-9f

vec4 funcNv12ToRGBABT709CSC(vec3 yuv)
{
    vec3 rgb;
    rgb.x=1.1644*(yuv.x-16.)+1.7927*(yuv.z-128.);
    rgb.y=1.1644*(yuv.x-16.)-.5329*(yuv.z-128.)-.2132*(yuv.y-128.);
    rgb.z=1.1644*(yuv.x-16.)+2.1123*(yuv.y-128.);
    return vec4(rgb/255.,1.);
}

//BT. 709, color Range: Full.  (HDTV)
vec4 funcNv12ToRGBABT709HDTV(vec3 yuv)
{
    vec3 rgb;
    rgb.x=yuv.x+1.5748*(yuv.z-128.);
    rgb.y=yuv.x-.4681*(yuv.z-128.)-.1873*(yuv.y-128.);
    rgb.z=yuv.x+1.8556*(yuv.y-128.);
    return vec4(rgb/255.,1.);
}

//BT.601 PAL/ BT.601 NTSC /SMPTE 240M, color Range:Limited (CSC)
vec4 funcNv12ToRGBABT601CSC(vec3 yuv)
{
    vec3 rgb;
    rgb.x=1.1644*(yuv.x-16.)+1.5960*(yuv.z-128.);
    rgb.y=1.1644*(yuv.x-16.)-.8129*(yuv.z-128.)-.3917*(yuv.y-128.);
    rgb.z=1.1644*(yuv.x-16.)+2.0172*(yuv.y-128.);
    return vec4(rgb/255.,1.);
}

//BT.601 PAL/ BT.601 NTSC /SMPTE 240M,, color Range:Full
vec4 funcNv12ToRGBABT601HDTV(vec3 yuv)
{
    vec3 rgb;
    rgb.x=yuv.x+1.4746*(yuv.z-128.);
    rgb.y=yuv.x-.5713*(yuv.z-128.)-.1645*(yuv.y-128.);
    rgb.z=yuv.x+1.8814*(yuv.y-128.);
    return vec4(rgb/255.,1.);
}

vec4 choosMode(vec3 yuv,int _mode)
{
    
    yuv=yuv*255.;
    vec4 outColor;
    SWITCH(_mode)
    {
        CASE(0)//BT. 709, color Range:Limited.   (CSC) , default mode
        outColor=funcNv12ToRGBABT709CSC(yuv);
        //outColor=vec4(1,0,0,1);
        break;
        CASE(1)//BT. 709, color Range: Full.
        outColor=funcNv12ToRGBABT709HDTV(yuv);
        //outColor=vec4(0.0902, 0.6196, 0.4431, 1.0);
        break;
        CASE(2)//BT.601 PAL/ BT.601 NTSC /SMPTE 240M, color Range:Limited
        outColor=funcNv12ToRGBABT601CSC(yuv);
        break;
        CASE(3)//BT.601 PAL/ BT.601 NTSC /SMPTE 240M,, color Range:Full
        outColor=funcNv12ToRGBABT601HDTV(yuv);
        break;
        DEFAULT()
        outColor=funcNv12ToRGBABT601CSC(yuv);
        break;
        ENDSWITCH()
    }
    return outColor;
}
bool isZero(float x){
    return(x>-EQN_EPS&&x<EQN_EPS);
}

int mod(in int i,int m){
    return i-i/m*m;
}
vec4 PS_2D(vec2 TextureUV)
{
    float ow=float(overlay_Width);
    float oh=float(overlay_Height);
    float bw=float(back_Width);
    float bh=float(back_Height);
    vec2 tc=TextureUV;
    float w=(float(tc.x)+.5)*float(overlay_Width)/float(back_Width);
    float h=(float(tc.y)+.5)*float(overlay_Height)/float(back_Height);
    int nH=int(overlay_Height+1)/2;
    int nw=int(w);
    int nh=int(h);
    int x=nw/2+(overlay_Width+1)/2*(mod(nh,2));
    int ux=nw/2+(overlay_Width+1)/2*(mod(nh,4)/2);
    
    vec4 InYColor=TEXTURE2D(overlay,vec2(x/ow,nh/2/oh));
    vec4 InUVColor=TEXTURE2D(overlay,vec2(ux/ow,(nh/4+nH)/oh));
    
    vec4 retColor=vec4(0.,0.,0.,0.);
    // if(mod(nw,2)==0)
    // retColor=choosMode(vec3(InYColor.x,InUVColor.x,InUVColor.y),mode);
    // else
    // retColor=choosMode(vec3(InYColor.y,InUVColor.x,InUVColor.y),mode);
    retColor=choosMode(vec3(InYColor.x,InUVColor.x,InUVColor.x),mode);
    
    return retColor;
    
}

// tc: coord of texture pixel
vec4 PS3_2D(vec2 tc){
    //  tc.y = 1.0 - tc.y;
    float bw=float(back_Width);
    float bh=float(back_Height);
    float mw=float(overlay_Width);
    float mh=float(overlay_Height)/1.5;
    
    float w=tc.x*mw;
    float h=tc.y*mh;
    int nH=int(bh+1)/2;
    int uvw=int(w)/2 * 2;


    // if(mod(int(h/2),2)==0)
    //     uvw+=overlay_Width/2;
    // Y-data coord & NV-data coord
    vec2 Y_coord=vec2(w/mw,h/mh*2/3);
    // vec2 U_coord=vec2(uvw/mw,h/3/mh+2./3);
    vec2 U_coord=vec2(uvw/mw,(h / 2 + mh) / overlay_Height);
    vec2 V_coord=vec2((uvw + 1)/mw,(h / 2 + mh) / overlay_Height);
    //vec2 V_coord=vec2(uvw/mw,h/6/mh+5./6);
    
    vec4 Ycolor=TEXTURE2D(overlay,Y_coord);
    vec4 Ucolor=TEXTURE2D(overlay,U_coord);
    vec4 Vcolor=TEXTURE2D(overlay,V_coord);
    vec4 color=vec4(Ycolor.x,Ucolor.x,Vcolor.x,1.0f);
    //Ucolor.x=0.5*mod(int(w/10),2);
    //if(mod(int(w),100)==0)Ucolor.x=0.4;
    // return color;
    vec3 yuv=vec3(
        Ycolor.x,
        Ucolor.x,
        Vcolor.x
        ); 
    // yuv=vec3(Ycolor.x,0,0);
    
    color=choosMode(yuv,mode);
    //color=vec4(color.z,color.y,color.x,color.a);
    //if(Ycolor.x<0||Ycolor.x>1||Ucolor.x<0||Ucolor.x>1||Vcolor.x<0||Vcolor.x>1)discard;
    // return vec4(yuv,1);
    return color;
   // return vec4(Ycolor.x, 1.0,1.0,1.0);
    
}
varying vec4 vs_output_position;// vertex position
varying vec2 vs_output_TextureUV;// vertex texture coords

float uc(float c){
    return int(c*overlay_Width)/2*2/float(overlay_Width);
}
float vc(float c){
    return int(c*overlay_Width+1)/2*2/float(overlay_Width);
}
void main(){
    vec4 xy=vs_output_position;
    vec2
    //tc=(vs_output_TextureUV.x*overlay_Width,vs_output_TextureUV*overlay_Height);
    //tc=vec2((xy.x+1)/2*overlay_Width,(xy.y+1)/2*overlay_Height);
    tc=vs_output_TextureUV;
    gl_FragColor=PS3_2D(tc);
    // gl_FragColor=PS_2D(vec2(tc.x*1440,tc.y*1350));
    gl_FragColor= TEXTURE2D(overlay,
    vec2(uc(tc.x),
    tc.y
    ///3+2./3
    *2/3
    ));
    //gl_FragColor=vs_output_position;vec4(0,1,0,1);
    
}
