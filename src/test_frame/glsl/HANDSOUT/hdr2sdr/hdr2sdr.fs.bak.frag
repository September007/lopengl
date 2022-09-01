/*
   _______                               _____ __              __             ____             __
  / ____(_)___  ___  ____ ___  ____ _   / ___// /_  ____ _____/ /__  _____   / __ \____ ______/ /__
 / /   / / __ \/ _ \/ __ `__ \/ __ `/   \__ \/ __ \/ __ `/ __  / _ \/ ___/  / /_/ / __ `/ ___/ //_/
/ /___/ / / / /  __/ / / / / / /_/ /   ___/ / / / / /_/ / /_/ /  __/ /     / ____/ /_/ / /__/ ,<
\____/_/_/ /_/\___/_/ /_/ /_/\__,_/   /____/_/ /_/\__,_/\__,_/\___/_/     /_/    \__,_/\___/_/|_|
        http://en.sbence.hu/        Shader: Try to get the SDR part of HDR content
*/
#version 120

#if __VERSION__<130
#define TEXTURE2D texture2D
#else
#define TEXTURE2D texture
#endif
varying vec4 vs_output_position;
varying vec2 vs_output_textureUV;

float saturate(float f){return f<0?0:f>1?1:f;}
vec3 saturate(vec3 x){
	return vec3(saturate(x.x),saturate(x.y),saturate(x.z));
}
bool lessOrEqul31(vec3 v,float f){
	return v.x<=f&&v.y<=f&&v.z<=f;
}
uniform sampler2D overlay;
// Configuratio()n ---------------------------------------------------------------
float peakLuminance(){ 
return 250.0; // Peak playback screen luminance in nits
}
float knee(){ 
return 0.75; // Compressor knee() position
}
float ratio(){ 
return 1.0; // Compressor ratio(): 1 = disabled, <1 = expander
}
float maxCLL(){ 
return 10000.0; // Maximum content light level in nits
}
// -----------------------------------------------------------------------------

// Precalculated values
float gain(){ 
return maxCLL() / peakLuminance();
}
float compressor(){ 
return 1.0 / ratio();
}

// PQ constants
float m1inv(){ 
return 16384 / 2610.0;
}
float m2inv(){ 
return 32 / 2523.0;
}
float c1(){ 
return 3424 / 4096.0;
}
float c2(){ 
return 2413 / 128.0;
}
float c3(){ 
return 2392 / 128.0;
}


float minGain(vec3 pixel) {
  return min(pixel.r, min(pixel.g, pixel.b));
}

float midGain(vec3 pixel) {
  return pixel.r < pixel.g ?
    (pixel.r < pixel.b ?
      min(pixel.g, pixel.b) : // min = r
      min(pixel.r, pixel.g)) : // min = b
    (pixel.g < pixel.b ?
      min(pixel.r, pixel.b) : // min = g
      min(pixel.r, pixel.g)); // min = b
}

float maxGain(vec3 pixel) {
  return max(pixel.r, max(pixel.g, pixel.b));
}

vec3 compress(vec3 pixel) {
  float gain = maxGain(pixel);
  return pixel * (gain  < knee() ? gain : knee() + max(gain - knee(), 0) * compressor()) / gain ;
}

vec3 fixClip(vec3 pixel) {
  // keep the (mid - min) / (max - min) ratio()
  float preMin = minGain(pixel);
  float preMid = midGain(pixel);
  float preMax = maxGain(pixel);
  vec3 clip = saturate(pixel);
  float postMin = minGain(clip);
  float postMid = midGain(clip);
  float postMax = maxGain(clip);
  float ratio = (preMid - preMin) / (preMax - preMin);
  float newMid = ratio * (postMax - postMin) + postMin;
  return vec3(clip.r != postMid ? clip.r : newMid,
                clip.g != postMid ? clip.g : newMid,
                clip.b != postMid ? clip.b : newMid);
}

vec3 pq2lin(vec3 pq) { // Returns luminance in nits
  vec3 p = pow(pq, vec3(m2inv(),m2inv(),m2inv()));
  vec3 d = max(p - c1(), 0) / (c2() - c3() * p);
  return pow(d, vec3(m1inv(),m1inv(),m1inv())) * gain();
}

vec3 srgb2lin(vec3 srgb) {
  return lessOrEqul31( srgb , 0.04045) ? srgb / 12.92 : pow((srgb + 0.055) / 1.055, vec3(2.4,2.4,2.4));
}

vec3 lin2srgb(vec3 lin) {
  return lessOrEqul31( lin , 0.0031308) ? lin * 12.92 : 1.055 * pow(lin,vec3( 0.416667, 0.416667, 0.416667)) - 0.055;
}

vec3 bt2020to709(vec3 bt2020) { // in linear space
  return vec3(
    bt2020.r *  1.6605 + bt2020.g * -0.5876 + bt2020.b * -0.0728,
    bt2020.r * -0.1246 + bt2020.g *  1.1329 + bt2020.b * -0.0083,
    bt2020.r * -0.0182 + bt2020.g * -0.1006 + bt2020.b * 1.1187);
}

void main() {
  vec3 pxval = TEXTURE2D(overlay,vs_output_textureUV).rgb;
  vec3 lin = bt2020to709(pq2lin(pxval));
  vec3 final = lin2srgb(compress(lin));
  gl_FragColor= vec4(fixClip(final).rgbb);
  //gl_FragColor=vec4(pxval,1);
}