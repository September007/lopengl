#version 120

#pragma optimize(off)
#if __VERSION__<130
#define TEXTURE2D texture2D
#else
#define TEXTURE2D texture
#endif

#define Texture2D sampler2D
// clang-format off
#define SWITCH(var)int v_var=var;{do{
	#define CASE(val)}if(v_var==val){
	#define DEFAULT()};
#define ENDSWITCH()}while(false);
// clang-format on

uniform int PqMode;
uniform sampler2D overlay;// Color texture for mesh
uniform float MaxCLL=1000;
uniform float y_hdr_ref=203.f;
uniform float de_saturation=0.f;
uniform float sdr_peak=100.f;
uniform float hlg_peak=1000.f;//hlg is dependent on the peak white of the  master display so it is a relative encoding!! //according to BT.2390-3 Lw = 392
uniform float pq_peak=10000.f;//pq is absolute encoding
#define EQN_EPS 1e-5f
#define MP_REF_WHITE 203.f
#define MP_REF_WHITE_HLG 3.17955f
#define sdr_avg .25f

vec3 mpow(vec3 v,float p){
	return pow(v,vec3(p,p,p));
}
vec3 vsSub(vec3 v,float f){
	return vec3(v.x-f,v.y-f,v.z-f);
}
vec3 svSub(float f,vec3 v){
	return vec3(f-v.x,f-v.y,f-v.z);
}
vec3 svMax(float f,vec3 v){
	return vec3(max(f,v.x),max(f,v.y),max(f,v.z));
}

bool iszero(float value) 
{
	return value >= -EQN_EPS && value <= EQN_EPS;
}
bool lessOrEqul31(vec3 v,float f){
	return v.x<=f&&v.y<=f&&v.z<=f;
}
float log10(float f){return log(f)/log(10);}
int XX(){
	return 1;
}
float lerp(float l,float r,float deg){
	return l+(r-l)*deg;
}
vec3 lerp(vec3 l,vec3 r,float deg){
	return vec3(l.x+(r.x-l.x)*deg,
	l.y+(r.y-l.y)*deg,
	l.z+(r.z-l.z)*deg);
}


vec3  PQ_EOTF_12(vec3 pq_sinal) 
{

    float m1 = (2610.0f) / (4096.0f * 4.0f);
    float m2 = (2523.0f * 128.0f) / 4096.0f;
    float c1 = (3424.0f) / 4096.0f;
    float c2 = (2413.0f *  32.0f) / 4096.0f;
    float c3 = (2392.0f *  32.0f) / 4096.0f;
	
    vec3 sinal = clamp(pq_sinal, 0.0f, 1.0f);
    vec3 tempValue = mpow(sinal, (1.0f / m2));
    tempValue = (mpow(svMax(0.0f, vsSub(tempValue , c1)) / svSub(c2 , c3 * tempValue), (1.0f / m1)));
    return tempValue * 10000.0f / MP_REF_WHITE;
}
vec3 HLG_INV_OETF_12(vec3 hlg_signal)
{
	float m_tfScale=12.f;//19.6829249; // transfer function scaling - assuming super whites
	float m_invTfScale=1.f/m_tfScale;
	float m_normalFactor=1.f;
	float m_a=.17883277f;
	float m_b=.28466892f;
	float m_c=.55991073f;
	
	vec3 tmp3=lessOrEqul31( hlg_signal,.5f)?hlg_signal*hlg_signal*4.f:exp((hlg_signal-m_c)/m_a)+m_b;
	tmp3*=1.f/MP_REF_WHITE_HLG;
	return tmp3;
}
vec3 HLG_OOTF_12(vec3 RGBs,float Lw,float Lb)
{
	float peak=1000.f/MP_REF_WHITE;
	vec3 RGBd;
	float Ys=.2627f*RGBs.x+.6780f*RGBs.y+.0593f*RGBs.z;
	float gamma=1.2f+.42f*log10(Lw/1000.f);
	gamma=max(1.f,gamma);
	float factor=peak/pow(12.f/MP_REF_WHITE_HLG,gamma)*pow(Ys,gamma-1.f);
	RGBd.x=RGBs.x*factor;
	RGBd.y=RGBs.y*factor;
	RGBd.z=RGBs.z*factor;
	return RGBd;
}
vec3 hable3(vec3 x)
{
	float A=.15f,B=.50f,C=.10f,D=.20f,E=.02f,F=.30f;
	return(x*(x*A+B*C)+D*E)/(x*(x*A+B)+D*F)-E/F;
}
float BT1886_INV_EOTF(float Rd)
{
	float m_gamma=2.4f;
	float m_inverseGamma=1.f/m_gamma;
	float c=clamp(Rd,0.f,1.f);
	return pow(c,m_inverseGamma);
}
vec4 PS_2D(vec2 tc)
{
	
	vec3 bt2020_bt709_0=vec3(1.66049695f,-.587656736f,-.0728399456f);
	vec3 bt2020_bt709_1=vec3(-.124547064f,1.13289523f,-.00834798440f);
	vec3 bt2020_bt709_2=vec3(-.0181536824f,-.100597292f,1.11875105f);
	float hdr_peak_luminance=hlg_peak;
	vec4 color=TEXTURE2D(overlay,tc);
	if(PqMode==1)hdr_peak_luminance=pq_peak;
	float sig_peak=1000.f/MP_REF_WHITE;
	color.xyz=clamp(color.xyz,vec3(0.f,0.f,0.f),vec3(1.f,1.f,1.f));
	if(PqMode==1)
	{
		color.xyz=PQ_EOTF_12(color.xyz);
		sig_peak=MaxCLL/MP_REF_WHITE;
	}
	else
	{
		color.xyz=HLG_INV_OETF_12(color.xyz);
		color.xyz=HLG_OOTF_12(color.xyz,1000.f,0.f);
	}
	int sig_idx=-1;
	float sig_max=max(max(color.x,color.y),color.z);
	if(iszero(sig_max-color.x))sig_idx=0;
	if(iszero(sig_max-color.y))sig_idx=1;
	if(iszero(sig_max-color.z))sig_idx=2;
	float sig_avg=.250000f;
	vec3 sig=min(color.xyz,sig_peak);
	float dst_scale=200.f/203.f;
	if(dst_scale>1.f)
	{
		sig*=1.f/dst_scale;
		sig_peak*=1.f/dst_scale;
	}
	float sig_orig=0.f;
	if(sig_idx==0)
	{
		sig_orig=sig.x;
	}
	if(sig_idx==1)
	{
		sig_orig=sig.y;
	}
	if(sig_idx==2)
	{
		sig_orig=sig.z;
	}
	float max_boost=1.f;
	float slope=min(max_boost,sdr_avg/sig_avg);
	sig*=slope;
	sig_peak*=slope;
	sig=hable3(max(vec3(0.f,0.f,0.f),sig))/hable3(vec3(sig_peak,sig_peak,sig_peak)).x;
	//sig = DX11DSK(sig);
	sig_max=0.f;
	if(sig_idx==0)
	{
		sig_max=sig.x;
	}
	if(sig_idx==1)
	{
		sig_max=sig.y;
	}
	if(sig_idx==2)
	{
		sig_max=sig.z;
	}
	vec3 sig_lin=color.xyz*(sig_max/sig_orig);
	int desat=1;
	float desat_exp=1.5f;
	if(desat>0)
	{
		float base=.18f*dst_scale;
		float coeff=max(sig_max-base,1e-6f)/max(sig_max,1.f);
		coeff=float(desat)*pow(coeff,desat_exp);
		color.xyz=lerp(sig_lin,dst_scale*sig,coeff);
	}
	else
	{
		color.xyz=sig_lin;
	}
	float R_SDR_709=dot(bt2020_bt709_0,color.xyz);
	float G_SDR_709=dot(bt2020_bt709_1,color.xyz);
	float B_SDR_709=dot(bt2020_bt709_2,color.xyz);
	color.xyz=vec3(R_SDR_709,G_SDR_709,B_SDR_709);
	
	float cmin=min(min(color.x,color.y),color.z);
	if(cmin<0.f)
	{
		float luma=.2627f*color.x+.6780f*color.y+.0593f*color.z;
		float coeff=cmin/(cmin-luma);
		color.xyz=lerp(color.xyz,vec3(luma,luma,luma),coeff);
	}
	float cmax=1.f/dst_scale*max(max(color.x,color.y),color.z);
	if(cmax>1.f)color.xyz/=cmax;
	float dst_range=dst_scale;
	color.xyz*=1.f/dst_range;
	// delinearize
	color.xyz=clamp(color.xyz,0.f,1.f);
	color.x=BT1886_INV_EOTF(color.x);
	color.y=BT1886_INV_EOTF(color.y);
	color.z=BT1886_INV_EOTF(color.z);
	return color;
}

varying vec4 vs_output_position;
varying vec2 vs_output_textureUV;
void main(){
	gl_FragColor=PS_2D(vs_output_textureUV);
}