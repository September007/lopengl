# HANDSOUT

## parmas

### vertex attribute

this is universal form of vertex input atttibute 

```cpp
 attribute vec4 position;
 attribute vec2 textureUV;
```

### fragment *uniform*

this is fragment's type-definition
```cpp
#define float4 vec4
#define float3 vec3
#define float2 vec2
#define Texture2D  sampler2D  
```

#### blend

```cpp
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
```

#### crop

```cpp
uniform Texture2D overlay;
uniform float roi_x;
uniform float roi_y;
uniform float    g_fTime;
uniform int overlay_Width;
uniform int overlay_Height;
uniform int dst_Width;
uniform int dst_Height;
```

#### nv12_to_rgb

```cpp
uniform sampler2D overlay;// Color texture for mesh
uniform int mode;
uniform int overlay_Width;
uniform int overlay_Height;
uniform int back_Width;
uniform int back_Height;
uniform Texture2D overlay;// Color texture for mesh
uniform int mode;
uniform int overlay_Width;
uniform int overlay_Height;
uniform int back_Width;
uniform int back_Height;
```

#### pixelRotate

```cpp
uniform int rotateType; //0:clockwise 90;1:clockwise 180;2:clockwise 270;3:flip X; 4:flip Y
uniform Texture2D overlay;
uniform float    g_fTime;
uniform int overlay_Width;
uniform int overlay_Height;
```

#### transformFilter

```cpp
uniform Texture2D overlay;
uniform float dScaleX;
uniform float dScaleY;
uniform float theta;
uniform float    g_fTime;
uniform int overlay_Width;
uniform int overlay_Height;
uniform int dst_Width;
uniform int dst_Height;
```

(peakLuminance|knee|ratio|maxCLL|gain|compressor|m1inv|m2inv|c1|c2|c3)