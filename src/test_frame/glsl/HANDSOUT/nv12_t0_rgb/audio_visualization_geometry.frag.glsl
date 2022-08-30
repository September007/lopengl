#version 120

#ifdef GL_ES
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#endif

#if __VERSION__==120||__VERSION__==100
#extension GL_EXT_gpu_shader4:enable
#define texture texture2D
#define fragColor gl_FragColor
uniform sampler2D background_tex;
uniform sampler2D gradient_color_tex;
uniform sampler2D audio_tex;
#else
layout(location=0)out vec4 fragColor;
layout(binding=0)uniform sampler2D background_tex;
layout(binding=1)uniform sampler2D gradient_color;
layout(binding=2)uniform sampler2D audio_tex;
#endif

uniform vec2 iResolution;
uniform float time_second;

uniform float scale_x;
uniform float scale_y;

uniform int rotate_x;
uniform int rotate_y;
uniform int rotate_z;

uniform float intensity;

uniform float alpha;

uniform int single_color;
uniform int color;
uniform float color_r;
uniform float color_g;
uniform float color_b;

uniform bool use_gradient_color;
uniform int gradient_color_count;
uniform int gradient_color_direction;

uniform float position_x;
uniform float position_y;

uniform bool flip_y;
uniform bool flip_x;

uniform int glow;
uniform float glow_intensity;

uniform int geometry_type;

uniform int direction;
uniform float offset_x;

uniform int spectrum_count;
uniform float spectrum_width;
uniform float spectrum_minimum_height;
uniform float spectrum_round;

uniform float point_size;
uniform int point_count;

uniform float line_width;
uniform int line_count;

uniform int pulse_count;
uniform float pulse_width;
uniform bool pulse_fill;
uniform float pulse_line_width;

uniform int circle_column_count;
uniform float circle_size;
uniform float clrcle_gap_size;

uniform int rectangle_column_count;
uniform float rectangle_width;
uniform float rectangle_height;
uniform float rectangle_gap_size;

uniform float audio_left_db;
uniform float audio_right_db;

uniform float audio_db_min;
uniform float audio_db_max;

uniform float audio_frequency_min;
uniform float audio_frequency_max;

uniform bool audio_mirror;
uniform bool audio_averaging;

float end=200.;
const float EPSILON=.002;
const float M_PI=3.1415926535897932384626433832795;

vec3 quad_point[4];
float quad_width;
float quad_height;

vec3 right_vector;
vec3 up_vector;

float transparent;

vec3 hsv2rgb(vec3 c){
    vec4 K=vec4(1.,2./3.,1./3.,3.);
    vec3 p=abs(fract(c.xxx+K.xyz)*6.-K.www);
    return c.z*mix(K.xxx,clamp(p-K.xxx,0.,1.),c.y);
}

vec3 rgb2hsv(vec3 c)
{
    vec4 K=vec4(0.,-1./3.,2./3.,-1.);
    vec4 p=mix(vec4(c.bg,K.wz),vec4(c.gb,K.xy),step(c.b,c.g));
    vec4 q=mix(vec4(p.xyw,c.r),vec4(c.r,p.yzx),step(p.x,c.r));
    
    float d=q.x-min(q.w,q.y);
    float e=1.e-10;
    return vec3(abs(q.z+(q.w-q.y)/(6.*d+e)),d/(q.x+e),q.x);
}

vec3 int2rgb(int c){
    return vec3(color_r,color_g,color_b);
}

vec4 getColor(sampler2D tex_,vec2 tex_coord){
    return texture(tex_,tex_coord);
}

vec3 getGradientColor(float pos){
    
    vec3 col=vec3(0.);
    for(float i=0.;i<float(gradient_color_count)-1.;++i){
        vec4 first=texture(gradient_color_tex,vec2((i+.5)/float(gradient_color_count),0.));
        vec4 second=texture(gradient_color_tex,vec2((i+1.5)/float(gradient_color_count),0.));
        if(pos>=first.a&&pos<=second.a){
            pos=(pos-first.a)/(second.a-first.a);
            col=mix(first.rgb,second.rgb,pos);
            break;
        }else if(0.==i&&pos<first.a){
            col=first.rgb;
            break;
        }else if(i==float(gradient_color_count)-2.&&pos>second.a){
            col=second.rgb;
            break;
        }
    }
    #ifdef GOOGLE_ANGLE
    col.rgb=col.bgr;
    #endif
    return col;
}

float cubicPulse(float c,float w,float h,float x)
{
    x=abs(x-c);
    if(x>w)return 0.;
    x/=w;
    return(1.-x*x*(3.-2.*x))*h;
}

float getAudioIntensity(float u){
    
    if(audio_mirror){
        u=abs(u-.5)*2.;
        u=1.-u;
    }
    
    u=(audio_frequency_max-audio_frequency_min)*u+audio_frequency_min;
    
    float val;
    if(audio_averaging){
        const float averaging_step=5.;
        const float averaging_offset=1./averaging_step;
        const float averaging_width=averaging_offset*.9;
        
        float left_u=u-mod(u,averaging_offset);
        float right_u=left_u+averaging_offset;
        
        float left_val=texture(audio_tex,vec2(left_u,.0)).r;
        float right_val=texture(audio_tex,vec2(right_u,.0)).r;
        
        val=cubicPulse(left_u,averaging_width,left_val,u)+cubicPulse(right_u,averaging_width,right_val,u);
    }else{
        val=texture(audio_tex,vec2(u,.0)).r;
    }
    
    if(val<.05)val=0.;
    
    val*=smoothstep(audio_db_min,audio_db_max,audio_left_db)*smoothstep(0.,1.,intensity)*(1.+smoothstep(1.,3.,intensity)*.5);
    return smoothstep(0.,1.,val);
}

float getAudioValue(float u){
    float val=getAudioIntensity(u);
    val=(val-.5);
    if(abs(val)<.05)val=0.;
    return clamp(val,-.5,.5);
}

float getGlow(float dist,float radius,float intensity){
    return pow(radius/dist,intensity);
}

float opSmoothSubtraction(float d1,float d2,float k){
    float h=clamp(.5-.5*(d2+d1)/k,0.,1.);
return mix(d2,-d1,h)+k*h*(1.-h);}

float opSubtraction(float d1,float d2){return max(-d1,d2);}

float sdCappedCylinder(vec3 p,float h,float r)
{
    vec2 d=abs(vec2(length(p.xz),p.y))-vec2(r,h);
    return min(max(d.x,d.y),0.)+length(max(d,0.));
}

float sdPoint(vec2 pos,vec2 point,float point_radius){
    return length(pos-point)-point_radius;
}

float sdLine(in vec2 p,in vec2 a,in vec2 b,float line_width){
    vec2 ba=b-a;
    vec2 pa=p-a;
    float h=clamp(dot(pa,ba)/dot(ba,ba),0.,1.);
    return length(pa-h*ba)-line_width*.5;
}

float sdPlane(vec3 p,vec3 n)
{
    return dot(p,normalize(n));
}

float sdBox(in vec2 p,in vec2 b)
{
    vec2 d=abs(p)-b;
    return length(max(d,0.))+min(max(d.x,d.y),0.);
}

// b.x = width
// b.y = height
// r.x = roundness top-right
// r.y = roundness boottom-right
// r.z = roundness top-left
// r.w = roundness bottom-left
float sdRoundBox(in vec2 p,in vec2 b,in vec4 r)
{
    r.xy=(p.x>0.)?r.xy:r.zw;
    r.x=(p.y>0.)?r.x:r.y;
    vec2 q=abs(p)-b+r.x;
    return min(max(q.x,q.y),0.)+length(max(q,0.))-r.x;
}

float dot2(in vec3 v){return dot(v,v);}
float udQuad(in vec3 p,in vec3 v1,in vec3 v2,in vec3 v3,in vec3 v4)
{
    #if 1
    // handle ill formed quads
    if(dot(cross(v2-v1,v4-v1),cross(v4-v3,v2-v3))<0.)
    {
        vec3 tmp=v3;
        v3=v4;
        v4=tmp;
    }
    #endif
    
    vec3 v21=v2-v1;vec3 p1=p-v1;
    vec3 v32=v3-v2;vec3 p2=p-v2;
    vec3 v43=v4-v3;vec3 p3=p-v3;
    vec3 v14=v1-v4;vec3 p4=p-v4;
    vec3 nor=cross(v21,v14);
    
    return sqrt((sign(dot(cross(v21,nor),p1))+
    sign(dot(cross(v32,nor),p2))+
    sign(dot(cross(v43,nor),p3))+
    sign(dot(cross(v14,nor),p4))<3.)
    ?
    min(min(dot2(v21*clamp(dot(v21,p1)/dot2(v21),0.,1.)-p1),
    dot2(v32*clamp(dot(v32,p2)/dot2(v32),0.,1.)-p2)),
    min(dot2(v43*clamp(dot(v43,p3)/dot2(v43),0.,1.)-p3),
    dot2(v14*clamp(dot(v14,p4)/dot2(v14),0.,1.)-p4)))
    :
    dot(nor,p1)*dot(nor,p1)/dot2(nor));
}

vec2 usqdLineSegment(vec3 ro,vec3 rd,vec3 a,vec3 b)
{
    // http://www.sousakuba.com/Programming/gs_two_lines_intersect.html
    vec3 ab=normalize(b-a),ao=a-ro;
    float d0=dot(rd,ab),d1=dot(rd,ao),d2=dot(ab,ao);
    float len=(d0*d1-d2)/(1.-d0*d0);
    len=clamp(len,0.,length(b-a));
    vec3 p=a+ab*len;
    float dis=length(cross(p-ro,rd));
    float t=dot(p-ro,rd);
    return vec2(dis,t);
}

vec2 normalizeOnQuad(vec3 pos){
    vec3 vector=pos-quad_point[0];
    float x=dot(vector,right_vector);
    float y=dot(vector,up_vector);
    return vec2(x,y);
}

float CubicPulseSDF(float c,float w,float h,vec2 pos){
    
    float y=cubicPulse(c,w,h,pos.x);
    if(0.==y)return abs(pos.y);
    
    if((sign(y)>0.&&pos.y>y)||
    (sign(y)<0.&&pos.y<y)){
        
        float x=abs(pos.x-c);
        x/=w;
        
        float slope=-6.*x*x*h-6.*x*h;
        
        vec2 p0=vec2(pos.x,y);
        vec2 p1=p0+vec2(1.,slope);
        vec2 dir=normalize(p1-p0);
        
        float h=abs(cross(vec3(dir,0.),vec3(normalize(pos-p0),0.)).z);
        return h;
        
        // float dist = sdLine(pos, p0, p1, EPSILON);
        // return;
    }else if((sign(y)>0.&&pos.y<0.)||
    (sign(y)<0.&&pos.y>0.)){
        return abs(pos.y);
    }else{
        return 0.;
    }
    
}

float sdBezier(vec2 pos,vec2 A,vec2 B,vec2 C,float line_width){
    vec2 a=B-A;
    vec2 b=A-2.*B+C;
    vec2 c=a*2.;
    vec2 d=A-pos;
    
    float kk=1./dot(b,b);
    float kx=kk*dot(a,b);
    float ky=kk*(2.*dot(a,a)+dot(d,b))/3.;
    float kz=kk*dot(d,a);
    
    float res=0.;
    
    float p=ky-kx*kx;
    float p3=p*p*p;
    float q=kx*(2.*kx*kx-3.*ky)+kz;
    float h=q*q+4.*p3;
    
    if(h>=0.){
        h=sqrt(h);
        vec2 x=(vec2(h,-h)-q)/2.;
        vec2 uv=sign(x)*pow(abs(x),vec2(1./3.));
        float t=uv.x+uv.y-kx;
        t=clamp(t,0.,1.);
        
        // 1 root
        vec2 qos=d+(c+b*t)*t;
        res=length(qos);
    }else{
        float z=sqrt(-p);
        float v=acos(q/(p*z*2.))/3.;
        float m=cos(v);
        float n=sin(v)*1.732050808;
        vec3 t=vec3(m+m,-n-m,n-m)*z-kx;
        t=clamp(t,0.,1.);
        
        // 3 roots
        vec2 qos=d+(c+b*t.x)*t.x;
        float dis=dot(qos,qos);
        
        res=dis;
        
        qos=d+(c+b*t.y)*t.y;
        dis=dot(qos,qos);
        res=min(res,dis);
        
        qos=d+(c+b*t.z)*t.z;
        dis=dot(qos,qos);
        res=min(res,dis);
        
        res=sqrt(res);
    }
    
    return res-line_width*.5;
}

float sdBox(in vec2 pos,in vec2 center,in vec2 size)
{
    vec2 d=abs(pos-center)-size;
    return length(max(d,0.))+min(max(d.x,d.y),0.);
}

float spectrum_sdf(vec2 pos){
    
    float dist=end;
    
    float spectrum_num=float(spectrum_count);
    float step_width=quad_width/spectrum_num;
    float tex_offset_x=1./spectrum_num;
    float spectrum_w=spectrum_width*quad_width*.02;
    
    float center_index=floor(pos.x/step_width);
    float begin_index=clamp(center_index-2.,0.,spectrum_num-1.);
    float end_index=clamp(center_index+2.,0.,spectrum_num-1.);
    for(float i=begin_index;i<=end_index;++i){
        float index=floor(mod(i+(offset_x*spectrum_num),spectrum_num));
        float val=getAudioIntensity(tex_offset_x*index);
        float spectrum_h=quad_height*(spectrum_minimum_height*.01*(1.-val)+val);
        vec2 spectrum_size=vec2(spectrum_w*.5,spectrum_h*.5);
        
        float spectrum_x=(i*step_width)+step_width*.5;
        float spectrum_y;
        if(0==direction){
            spectrum_y=spectrum_h*.5;
        }else if(1==direction){
            spectrum_y=quad_height*.5;
        }
        
        vec4 round_size=vec4(spectrum_size.x*spectrum_round);
        dist=min(dist,max(0.,sdRoundBox(pos-vec2(spectrum_x,spectrum_y),spectrum_size,round_size)));
        if(dist<=EPSILON)break;
    }
    
    return dist;
}

float points_sdf(vec2 pos){
    
    float dist=end;
    
    float point_num=float(point_count);
    float step_width=quad_width/point_num;
    float tex_offset_x=1./point_num;
    float point_radius=point_size*quad_width*.005;
    
    float center_index=floor(pos.x/step_width);
    float begin_index=clamp(center_index-2.,0.,point_num-1.);
    float end_index=clamp(center_index+2.,0.,point_num-1.);
    for(float i=begin_index;i<=end_index;++i){
        float index=floor(mod(i+(offset_x*point_num),point_num));
        float val=(0==direction)?getAudioIntensity(tex_offset_x*index):getAudioValue(tex_offset_x*index);
        
        float point_x=(i*step_width)+step_width*.5;
        float point_y=quad_height*val;
        if(1==direction)point_y+=quad_height*.5;
        
        dist=min(dist,max(0.,sdPoint(pos,vec2(point_x,point_y),point_radius)));
        if(dist<=EPSILON)break;
    }
    
    return dist;
}

float lines_sdf(vec2 pos){
    float dist=end;
    float line_num=float(line_count);
    float step_width=quad_width/line_num;
    float tex_offset_x=1./line_num;
    float line_w=line_width*quad_width*.003;
    
    float center_index=floor(pos.x/step_width);
    float begin_index=clamp(center_index-2.,0.,line_num-2.);
    float end_index=clamp(center_index+2.,0.,line_num-2.);
    
    for(float i=begin_index;i<=end_index;++i){
        
        float index=floor(mod(i+(offset_x*line_num),line_num));
        float val=(0==direction)?getAudioIntensity(tex_offset_x*index):getAudioValue(tex_offset_x*index);
        
        float line_x=(i*step_width)+step_width*.5;
        float line_y=quad_height*val;
        if(1==direction)line_y+=quad_height*.5;
        
        vec2 from=vec2(line_x,line_y);
        
        index=floor(mod((i+1.)+(offset_x*line_num),line_num));
        val=(0==direction)?getAudioIntensity(tex_offset_x*index):getAudioValue(tex_offset_x*index);
        
        line_x=((i+1.)*step_width)+step_width*.5;
        line_y=quad_height*val;
        if(1==direction)line_y+=quad_height*.5;
        
        vec2 to=vec2(line_x,line_y);
        
        dist=min(dist,max(0.,sdLine(pos,from,to,line_w)));
        if(dist<=EPSILON)break;
    }
    
    return dist;
}

float pulse_sdf(vec2 pos){
    float dist=end;
    float plus_num=float(pulse_count);
    float plus_width=quad_width*2.*pulse_width*.05;
    float tex_offset_x=1./plus_num;
    
    float step_width=quad_width/plus_num;
    float center_offset_x=plus_width-offset_x*quad_width;
    
    float samples=pulse_fill?.01:pulse_line_width*.01;
    for(float x1=pos.x-samples*2.;x1<=pos.x+samples*2.;x1+=samples){
        float begin_index=floor(((x1-plus_width*2.)/quad_width+offset_x)*plus_num);
        float end_index=ceil((x1/quad_width+offset_x)*plus_num);
        for(float y1=pos.y-samples*2.;y1<=pos.y+samples*2.;y1+=samples){
            
            float val=0.;
            
            for(float i=begin_index;i<=end_index;++i){
                float audio_val=(0==direction)?getAudioIntensity(tex_offset_x*i):getAudioValue(tex_offset_x*i);
                val+=cubicPulse(step_width*i+center_offset_x,plus_width,audio_val*quad_height,x1);
            }
            
            float base_y=(1==direction)?quad_height*.5:0.;
            val+=base_y;
            if(x1>=0.&&x1<=quad_width){
                if(!pulse_fill){
                    float line_w=pulse_line_width*quad_width*.003;
                    vec2 _pos=vec2(x1,val);
                    dist=min(dist,max(0.,length(pos-_pos)-line_w));
                }else{
                    if(val>=base_y&&y1>base_y){
                        dist=min(dist,max(0.,y1-val));
                    }else if(val<base_y&&y1<base_y){
                        dist=min(dist,max(0.,-(y1-val)));
                    }else{
                        dist=min(dist,abs(y1-base_y));
                    }
                }
            }
            
            if(dist<=EPSILON)break;
        }
        if(dist<=EPSILON)break;
    }
    
    return dist;
}

float circle_specturm_sdf(vec2 pos){
    
    float dist=end;
    
    float cols=float(circle_column_count);
    float step_width=quad_width/cols;
    float tex_offset_x=1./cols;
    float circle_radius=circle_size*quad_width*.015;
    float circle_gap=clrcle_gap_size*quad_width*.03;
    
    float center_index=floor(pos.x/step_width);
    float begin_index=clamp(center_index-2.,0.,cols-1.);
    float end_index=clamp(center_index+2.,0.,cols-1.);
    
    for(float i=begin_index;i<=end_index;++i){
        float index=floor(mod(i+(offset_x*cols),cols));
        float val=getAudioIntensity(tex_offset_x*index);
        
        float circle_x=(i*step_width)+step_width*.5;
        
        float height=quad_height*val;
        float circle_count=1.;
        
        height-=circle_radius*2.;
        if(sign(height)>.0){
            circle_count+=floor(height/(circle_radius*2.+circle_gap));
        }
        
        float circle_y0;
        if(0==direction){
            circle_y0=circle_radius;
        }else{
            circle_count=1.+floor((circle_count-1.)*.5)*2.;
            circle_y0=(quad_height*.5)-(circle_radius+((circle_count-1.)*.5*(circle_radius*2.+circle_gap)));
        }
        
        for(float j=0.;j<circle_count;++j){
            float circle_y=circle_y0+j*(circle_radius*2.+circle_gap);
            dist=min(dist,max(0.,sdPoint(pos,vec2(circle_x,circle_y),circle_radius)));
            if(dist<=EPSILON)return dist;
        }
    }
    
    return dist;
    
}

float rect_specturm_sdf(vec2 pos){
    
    float dist=end;
    
    float cols=float(rectangle_column_count);
    float step_width=quad_width/cols;
    float tex_offset_x=1./cols;
    float rect_height=rectangle_height*quad_width*.015;
    float rect_width=rectangle_width*quad_width*.015;
    float rect_gap=rectangle_gap_size*quad_width*.03;
    
    float center_index=floor(pos.x/step_width);
    float begin_index=clamp(center_index-2.,0.,cols-1.);
    float end_index=clamp(center_index+2.,0.,cols-1.);
    
    for(float i=begin_index;i<=end_index;++i){
        float index=floor(mod(i+(offset_x*cols),cols));
        float val=getAudioIntensity(tex_offset_x*index);
        
        float rect_x=(i*step_width)+step_width*.5;
        
        float height=quad_height*val;
        float rect_count=1.;
        
        height-=rect_height*2.;
        if(sign(height)>.0){
            rect_count+=floor(height/(rect_height*2.+rect_gap));
        }
        
        float rect_y0;
        if(0==direction){
            rect_y0=rect_height;
        }else{
            rect_count=1.+floor((rect_count-1.)*.5)*2.;
            rect_y0=(quad_height*.5)-(rect_height+((rect_count-1.)*.5*(rect_height*2.+rect_gap)));
        }
        for(float j=0.;j<rect_count;++j){
            float rect_y=rect_y0+j*(rect_height*2.+rect_gap);
            dist=min(dist,max(0.,sdBox(pos,vec2(rect_x,rect_y),vec2(rect_width,rect_height))));
            if(dist<=EPSILON)return dist;
        }
    }
    
    return dist;
    
}

vec2 SDF(vec3 p){
    vec2 pos_2d=normalizeOnQuad(p);
    
    float distToGeometry=end;
    
    if(0==geometry_type){
        distToGeometry=min(distToGeometry,spectrum_sdf(pos_2d));
    }else if(1==geometry_type){
        distToGeometry=min(distToGeometry,points_sdf(pos_2d));
    }else if(2==geometry_type){
        distToGeometry=min(distToGeometry,lines_sdf(pos_2d));
    }else if(3==geometry_type){
        distToGeometry=min(distToGeometry,pulse_sdf(pos_2d));
    }else if(4==geometry_type){
        distToGeometry=min(distToGeometry,circle_specturm_sdf(pos_2d));
    }else if(5==geometry_type){
        distToGeometry=min(distToGeometry,rect_specturm_sdf(pos_2d));
    }
    
    return vec2(distToGeometry,0.);
}

vec2 raymarch(vec3 eye,vec3 dir){
    
    float min_dist=end;
    float min_t=end;
    
    vec3 plane_point=quad_point[0];
    vec3 plane_normal=normalize(cross(right_vector,up_vector));
    if(dot(plane_normal,dir)!=0.){
        float len=(dot(plane_normal,plane_point)-dot(plane_normal,eye))/dot(plane_normal,dir);
        min_t=abs(len);
        if(len>=0.){
            vec3 p=eye+dir*len;
            min_dist=SDF(p).x;
        }
    }
    
    return vec2(min_t,max(EPSILON,min_dist));
}

void getCameraVector(vec3 camera,vec3 target,out vec3 forward,out vec3 right,out vec3 up){
    forward=normalize(target-camera);
    
    up=vec3(0.,1.,.0);
    
    if(abs(dot(up,forward))>.99){
        up=vec3(0.,0.,1.);
    }
    
    right=normalize(cross(forward,up));
    up=normalize(cross(right,forward));
}

vec3 rayDirection(vec3 camera,vec3 target,float fieldOfView,vec2 size,vec2 fragCoord){
    vec3 forward,right,up;
    getCameraVector(camera,target,forward,right,up);
    
    vec2 xy=fragCoord-size/2.;
    float z=(size.y/2.)/tan(radians(fieldOfView)/2.);
    
    vec3 near_plane_center=camera+forward*z;
    
    vec3 xyz=near_plane_center+right*xy.x+up*xy.y;
    
    return normalize(xyz-camera);
}

void main()
{
    if(scale_x==0.||scale_y==0.){
        fragColor=vec4(vec3(0.),1.);
        return;
    }
    
    transparent=alpha/100.;
    
    float len=20.;
    vec3 camera_pos=vec3(.0,.0,1.*len);
    vec3 target=vec3(.0,.0,.0);
    
    float fov=30.;
    
    vec3 left_lower_dir=rayDirection(camera_pos,target,fov,iResolution.xy,vec2(0.,iResolution.y));
    float left_lower_dist=-camera_pos.z/left_lower_dir.z;
    vec3 left_upper_dir=rayDirection(camera_pos,target,fov,iResolution.xy,vec2(0.));
    float left_upper_dist=-camera_pos.z/left_upper_dir.z;
    vec3 right_lower_dir=rayDirection(camera_pos,target,fov,iResolution.xy,iResolution.xy);
    float right_lower_dist=-camera_pos.z/right_lower_dir.z;
    vec3 right_upper_dir=rayDirection(camera_pos,target,fov,iResolution.xy,vec2(iResolution.x,0.));
    float right_upper_dist=-camera_pos.z/right_upper_dir.z;
    
    quad_point[0]=camera_pos+left_lower_dir*left_lower_dist;
    quad_point[1]=camera_pos+right_lower_dir*right_lower_dist;
    quad_point[2]=camera_pos+right_upper_dir*right_upper_dist;
    quad_point[3]=camera_pos+left_upper_dir*left_upper_dist;
    
    float rad=radians(float(rotate_x));
    mat3 rotate_x_mat=mat3(1.,.0,.0,
        .0,cos(rad),sin(rad),
        .0,-sin(rad),cos(rad));
        rad=radians(float(rotate_y));
        mat3 rotate_y_mat=mat3(cos(rad),.0,-sin(rad),
        .0,1.,.0,
        sin(rad),.0,cos(rad));
        rad=radians(float(rotate_z));
        mat3 rotate_z_mat=mat3(cos(rad),sin(rad),.0,
        -sin(rad),cos(rad),.0,
    .0,.0,1.);
    
    mat3 scale_mat=mat3(scale_x,0.,0.,
        0.,scale_y,0.,
    0.,0.,1.);
    
    right_vector=quad_point[1]-quad_point[0];
    up_vector=quad_point[3]-quad_point[0];
    
    float normalize_position_x=(position_x/100.);
    float normalize_position_y=(position_y/100.);
    
    vec3 translate_x=right_vector*(normalize_position_x-.5)*2.;
    vec3 translate_y=up_vector*(normalize_position_y-.5)*2.;
    vec3 translate_val=translate_x+translate_y;
    
    mat4 translate_mat=mat4(1.,0.,0.,0.,
        0.,1.,0.,0.,
        0.,0.,1.,0.,
    translate_val.x,translate_val.y,translate_val.z,1.);
    
    mat3 flip_y_mat=mat3(1.,.0,.0,
        .0,cos(M_PI),sin(M_PI),
        .0,-sin(M_PI),cos(M_PI));
        
        mat3 flip_x_mat=mat3(cos(M_PI),.0,-sin(M_PI),
        .0,1.,.0,
        sin(M_PI),.0,cos(M_PI));
        
        for(int i=0;i<4;++i){
            if(flip_x)quad_point[i]=flip_x_mat*quad_point[i];
            if(flip_y)quad_point[i]=flip_y_mat*quad_point[i];
            quad_point[i]=scale_mat*quad_point[i];
        }
        
        vec3 rotate_offset_y=(quad_point[3]-quad_point[0])*.5;
        mat4 rotate_offset_mat=mat4(1.,0.,0.,0.,
            0.,1.,0.,0.,
            0.,0.,1.,0.,
        rotate_offset_y.x,rotate_offset_y.y,rotate_offset_y.z,1.);
        
        mat4 rotate_offset_revert_mat=mat4(1.,0.,0.,0.,
            0.,1.,0.,0.,
            0.,0.,1.,0.,
        -rotate_offset_y.x,-rotate_offset_y.y,-rotate_offset_y.z,1.);
        
        for(int i=0;i<4;++i){
            if(0==direction){
                quad_point[i]=(rotate_offset_mat*vec4(quad_point[i],1.)).xyz;
                quad_point[i]=rotate_z_mat*rotate_y_mat*rotate_x_mat*quad_point[i];
                quad_point[i]=(rotate_offset_revert_mat*vec4(quad_point[i],1.)).xyz;
            }else{
                quad_point[i]=rotate_z_mat*rotate_y_mat*rotate_x_mat*quad_point[i];
            }
            
            quad_point[i]=(translate_mat*vec4(quad_point[i],1.)).xyz;
        }
        
        right_vector=normalize(quad_point[1]-quad_point[0]);
        up_vector=normalize(quad_point[3]-quad_point[0]);
        
        quad_width=normalizeOnQuad(quad_point[2]).x;
        quad_height=normalizeOnQuad(quad_point[2]).y;
        
        vec3 dir=rayDirection(camera_pos,target,fov,iResolution.xy,gl_FragCoord.xy);
        
        vec2 dist=raymarch(camera_pos,dir);
        
        vec2 tex_coord=gl_FragCoord.xy/iResolution.xy;
        
        // fragColor = texture(background_tex, tex_coord);
        
        vec3 col;
        if(1==single_color){
            col=int2rgb(color);
        }else{
            vec3 pos=camera_pos+dir*dist.x;
            vec2 pos_2d=normalizeOnQuad(pos);
            if(use_gradient_color){
                float ratio;
                if(0==gradient_color_direction){
                    ratio=clamp(pos_2d.y/quad_height,0.,1.);
                }else{
                    ratio=clamp(pos_2d.x/quad_width,0.,1.);
                }
                col=getGradientColor(ratio);
            }else{
                col=vec3(hsv2rgb(vec3(pos_2d.x/quad_width,1.,1.)));
            }
            
        }
        
        if(0==glow){
            bool need_aa_effect=false;
            if(geometry_type==2)need_aa_effect=true;
            if(geometry_type==3)need_aa_effect=true;
            
            float alpha_;
            if(need_aa_effect){
                alpha_=min(1.,pow(smoothstep(.0,1.,EPSILON/dist.y),.8)*20.)+min(1.,pow(smoothstep(.0,1.,EPSILON/dist.y),1.5)*10.);
            }else{
                alpha_=min(1.,pow(EPSILON*3.5/dist.y,1.5));
            }
            // fragColor = mix(fragColor, vec4(col, 1.), transparent * alpha_);
            fragColor=vec4(col,clamp(transparent*alpha_,.0,.99));
            #ifdef GOOGLE_ANGLE
            fragColor.rgb=fragColor.bgr;
            #endif
        }else{
            float alpha_=pow(EPSILON/dist.y,.3+smoothstep(1.,.1,glow_intensity)*.4);
            fragColor=vec4(col,clamp(transparent*alpha_,.0,.99));
            #ifdef GOOGLE_ANGLE
            fragColor.rgb=fragColor.bgr;
            #endif
        }
        
    }