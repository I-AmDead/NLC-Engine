#ifndef        COMMON_H
#define        COMMON_H

// #define USE_SUPER_SPECULAR

#include "shared\common.h"
#include "ogse_functions.h"
#include "ogse_config.h"
//////////////////////////////////////////////////////////////////////////////////////////
// *** options

// #define DBG_TEST_NMAP
// #define DBG_TEST_NMAP_SPEC
// #define DBG_TEST_SPEC
// #define DBG_TEST_LIGHT
// #define DBG_TEST_LIGHT_SPEC

// #define USE_GAMMA_22
// #define USE_SJITTER
// #define USE_SUNFILTER
// #define USE_FETCH4
// #define USE_MBLUR                	//- HW-options defined
// #define USE_HWSMAP                	//- HW-options defined

// #define USE_HWSMAP_PCF				//- nVidia GF3+, R600+

// #define USE_BRANCHING        		//- HW-options defined
// #define USE_VTF                		//- HW-options defined, VertexTextureFetch
// #define FP16_FILTER                	//- HW-options defined
// #define FP16_BLEND                	//- HW-options defined
//
// #define USE_PARALLAX                	//- shader defined
// #define USE_TDETAIL                	//- shader defined
// #define USE_LM_HEMI                	//- shader defined
// #define USE_DISTORT                	//- shader defined
// #define USE_SUNMASK                		//- shader defined
// #define DBG_TMAPPING
//////////////////////////////////////////////////////////////////////////////////////////
#ifndef SMAP_size
#define SMAP_size        1024
#endif
#define PARALLAX_H 0.02
#define parallax float2(PARALLAX_H, -PARALLAX_H/2)

#ifdef        USE_R2_STATIC_SUN
#  define xmaterial half(1.0h/4.h)
#else
#  define xmaterial half(L_material.w)
#endif
//////////////////////////////////////////////////////////////////////////////////////////
uniform half4                hemi_cube_pos_faces;
uniform half4                hemi_cube_neg_faces;
uniform half4                L_material;                            // 0,0,0,mid
uniform half4                Ldynamic_color;                      // dynamic light color (rgb1)        - spot/point
uniform half4                Ldynamic_pos;                       // dynamic light pos+1/range(w) - spot/point
uniform half4                Ldynamic_dir;                        // dynamic light direction         - sun

uniform half4                J_direct        [6];
uniform half4                J_spot                [6];

half          calc_fogging               (half4 w_pos)      { return dot(w_pos,fog_plane);         }
half2         calc_detail                (half3 w_pos)      {
        float                 dtl        = distance                (w_pos,eye_position)*dt_params.w;
                              dtl        = min              (dtl*dtl, 1);
        half                  dt_mul     = 1  - dtl;        // dt*  [1 ..  0 ]
        half                  dt_add     = .5 * dtl;        // dt+  [0 .. 0.5]
        return                half2      (dt_mul,dt_add);
}
float3         calc_reflection     (float3 pos_w, float3 norm_w)
{
    return reflect(normalize(pos_w-eye_position), norm_w);
}

float3        calc_sun_r1                (float3 norm_w)    { return L_sun_color*saturate(dot((norm_w),-L_sun_dir_w));                 }
float3        calc_model_hemi_r1         (float3 norm_w)    { return max(0,norm_w.y)*L_hemi_color;                                         }
float3        calc_model_lq_lighting     (float3 norm_w)    { return L_material.x*calc_model_hemi_r1(norm_w) + L_ambient + L_material.y*calc_sun_r1(norm_w);         }

//////////////////////////////////////////////////////////////////////////////////////////
struct         v_static                {
        float4      P                	: POSITION;                        // (float,float,float,1)
        float4      Nh                	: NORMAL;                // (nx,ny,nz,hemi occlusion)
        float4      T                 	: TANGENT;                // tangent
        float4      B                 	: BINORMAL;                // binormal
        float2      tc                	: TEXCOORD0;        // (u,v)
        float2      lmh                	: TEXCOORD1;        // (lmu,lmv)
        float4      color               : COLOR0;                        // (r,g,b,dir-occlusion)
};

struct         v_tree               	{
        float4      P                	: POSITION;                // (float,float,float,1)
        float4      Nh                	: NORMAL;                // (nx,ny,nz)
        float3      T                 	: TANGENT;                // tangent
        float3      B                 	: BINORMAL;                // binormal
        float4      tc                	: TEXCOORD0;        // (u,v,frac,???)
};

struct         v_model                 	{
        float4      P                   : POSITION;                // (float,float,float,1)
        float3      N                	: NORMAL;                // (nx,ny,nz)
        float3      T                	: TANGENT;                // (nx,ny,nz)
        float3      B                	: BINORMAL;                // (nx,ny,nz)
        float2      tc                	: TEXCOORD0;        // (u,v)
};

struct        v_detail                    {
        float4      pos                : POSITION;                // (float,float,float,1)
        int4        misc        : TEXCOORD0;        // (u(Q),v(Q),frac,matrix-id)
};

#ifdef  USE_HWSMAP
struct         v_shadow_direct_aref
{
        float4      hpos:        POSITION;       // Clip-space position         (for rasterization)
        float2      tc0:        TEXCOORD1;       // Diffuse map for aref
};
struct         v_shadow_direct
{
        float4      hpos:        POSITION;       // Clip-space position         (for rasterization)
};
#else
struct         v_shadow_direct_aref
{
        float4      hpos:        POSITION;       // Clip-space position         (for rasterization)
        float       depth:         TEXCOORD0;     // Depth
        float2      tc0:        TEXCOORD1;       // Diffuse map for aref
};
struct         v_shadow_direct
{
        float4      hpos:        POSITION;       // Clip-space position         (for rasterization)
        float       depth:         TEXCOORD0;     // Depth
};


#endif

//////////////////////////////////////////////////////////////////////////////////////////
struct         p_bumped        {
        float4      hpos        : POSITION;
#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
        float4            tcdh        : TEXCOORD0;        // Texture coordinates,         w=sun_occlusion
#else
        float2            tcdh        : TEXCOORD0;        // Texture coordinates
#endif
        float4      position        : TEXCOORD1;        // position + hemi
        half3       M1                : TEXCOORD2;        // nmap 2 eye - 1
        half3       M2                : TEXCOORD3;        // nmap 2 eye - 2
        half3       M3                : TEXCOORD4;        // nmap 2 eye - 3
#ifdef USE_TDETAIL
        float2      tcdbump     	: TEXCOORD5;        // d-bump
    #ifdef USE_LM_HEMI
        float2      lmh             : TEXCOORD6;        // lm-hemi
    #endif
#else
    #ifdef USE_LM_HEMI
        float2      lmh             : TEXCOORD5;        // lm-hemi
    #endif
#endif
};
//////////////////////////////////////////////////////////////////////////////////////////
struct         p_flat                  {
        float4                 hpos        : POSITION;
#if defined(USE_R2_STATIC_SUN) && !defined(USE_LM_HEMI)
    float4                    tcdh        : TEXCOORD0;        // Texture coordinates,         w=sun_occlusion
#else
    float2                    tcdh        : TEXCOORD0;        // Texture coordinates
#endif
        float4                position        : TEXCOORD1;        // position + hemi
        half3                N                : TEXCOORD2;        // Eye-space normal        (for lighting)
  #ifdef USE_TDETAIL
        float2                tcdbump                : TEXCOORD3;        // d-bump
    #ifdef USE_LM_HEMI
        float2         lmh      : TEXCOORD4;        // lm-hemi
    #endif
  #else
    #ifdef USE_LM_HEMI
        float2         lmh      : TEXCOORD3;        // lm-hemi
    #endif
  #endif
};

//////////////////////////////////////////////////////////////////////////////////////////
struct                  f_deffer        		{
        float4          position        		: COLOR0;        // px,py,pz, m-id
        float4       	C                		: COLOR1;        // r, g, b,  gloss
};
//////////////////////////////////////////////////////////////////////////////////////////
struct  				p_screen                {
        float4          hpos               		: POSITION;
        float2          tc0                		: TEXCOORD0;        // Texture coordinates         (for sampling maps)
};
//////////////////////////////////////////////////////////////////////////////////////////
// Geometry phase / deferring               	//
uniform sampler2D       s_base;             	//
uniform sampler2D       s_bump;             	//
uniform sampler2D       s_bumpX;                //
uniform sampler2D       s_detail;               //
uniform sampler2D       s_detailBump;           //
uniform sampler2D       s_detailBumpX;          //	Error for bump detail
uniform sampler2D       s_bumpD;                //
uniform sampler2D       s_hemi;             	//

uniform sampler2D       s_mask;                	//

uniform sampler2D       s_dt_r;                	//
uniform sampler2D       s_dt_g;                	//
uniform sampler2D       s_dt_b;                	//
uniform sampler2D       s_dt_a;                	//

uniform sampler2D       s_dn_r;                	//
uniform sampler2D       s_dn_g;                	//
uniform sampler2D       s_dn_b;                	//
uniform sampler2D       s_dn_a;                	//

//////////////////////////////////////////////////////////////////////////////////////////
// Lighting/shadowing phase                     //
uniform sampler2D       s_depth;                //
uniform sampler2D       s_position;             //
uniform sampler2D       s_normal;               //
uniform sampler         s_lmap;             	// 2D/cube projector lightmap
uniform sampler3D       s_material;             //
uniform sampler1D       s_attenuate;        	//
//////////////////////////////////////////////////////////////////////////////////////////
// Combine phase                                //
uniform sampler2D       s_diffuse;              // rgb.a = diffuse.gloss
uniform sampler2D       s_accumulator;      	// rgb.a = diffuse.specular
uniform sampler2D       s_generic;              //
uniform sampler2D       s_bloom;                //
uniform sampler         s_image;                // used in various post-processing
uniform sampler2D       s_tonemap;              // actually MidleGray / exp(Lw + eps)
//////////////////////////////////////////////////////////////////////////////////////////
// Defines                                		//
#define def_gloss       half(2.f /255.f)
#define def_aref        half(200.f/255.f)
#define def_dbumph      half(0.333f)
#define def_virtualh    half(0.05f)              // 5cm
#define def_distort     half(0.05f)             // we get -0.5 .. 0.5 range, this is -512 .. 512 for 1024, so scale it
#define def_hdr         half(9.h)         		// hight luminance range half(3.h)
#define def_hdr_clip	half(0.75h)        		//



/*
#define USABLE_BIT_1                uint(0x00002000)
#define USABLE_BIT_2                uint(0x00004000)
#define USABLE_BIT_3                uint(0x00008000)
#define USABLE_BIT_4                uint(0x00010000)
#define USABLE_BIT_5                uint(0x00020000)
#define USABLE_BIT_6                uint(0x00040000)
#define USABLE_BIT_7                uint(0x00080000)
#define USABLE_BIT_8                uint(0x00100000)
#define USABLE_BIT_9                uint(0x00200000)
#define USABLE_BIT_10               uint(0x00400000)
#define USABLE_BIT_11               uint(0x00800000)   // At least two of those four bit flags must be mutually exclusive (i.e. all 4 bits must not be set together)
#define USABLE_BIT_12               uint(0x01000000)   // This is because setting 0x47800000 sets all 5 FP16 exponent bits to 1 which means infinity
#define USABLE_BIT_13               uint(0x02000000)   // This will be translated to a +/-MAX_FLOAT in the FP16 render target (0xFBFF/0x7BFF), overwriting the 
#define USABLE_BIT_14               uint(0x04000000)   // mantissa bits where other bit flags are stored.
#define USABLE_BIT_15               uint(0x80000000)
#define MUST_BE_SET                 uint(0x40000000)   // This flag *must* be stored in the floating-point representation of the bit flag to store
*/

// Holger Gruen AMD - I change normal packing and unpacking to make sure N.z is accessible without ALU cost
// this help the HDAO compute shader to run more efficiently
float2 gbuf_pack_normal( float3 norm )
{
   float2 res;

   res.x  = norm.z;
   res.y  = 0.5f * ( norm.x + 1.0f ) ;
   res.y *= ( norm.y < 0.0f ? -1.0f : 1.0f );

   return res;
}

float3 gbuf_unpack_normal( float2 norm )
{
   float3 res;

   res.z  = norm.x;
   res.x  = ( 2.0f * abs( norm.y ) ) - 1.0f;
   res.y = ( norm.y < 0 ? -1.0 : 1.0 ) * sqrt( abs( 1 - res.x * res.x - res.z * res.z ) );

   return res;
}
/*
float gbuf_pack_hemi_mtl( float hemi, float mtl )
{
   uint packed_mtl = uint( ( mtl / 1.333333333 ) * 31.0 );
//   uint packed = ( MUST_BE_SET + ( uint( hemi * 255.0 ) << 13 ) + ( ( packed_mtl & uint( 31 ) ) << 21 ) );
	//	Clamp hemi max value
	uint packed = ( MUST_BE_SET + ( uint( saturate(hemi) * 255.9 ) << 13 ) + ( ( packed_mtl & uint( 31 ) ) << 21 ) );

   if( ( packed & USABLE_BIT_13 ) == 0 )
      packed |= USABLE_BIT_14;

   if( packed_mtl & uint( 16 ) )
      packed |= USABLE_BIT_15;

   return asfloat( packed );
}

float gbuf_unpack_hemi( float mtl_hemi )
{
//   return float( ( asuint( mtl_hemi ) >> 13 ) & uint(255) ) * (1.0/255.0);
	return float( ( asuint( mtl_hemi ) >> 13 ) & uint(255) ) * (1.0/254.8);
}

float gbuf_unpack_mtl( float mtl_hemi )
{
   uint packed       = asuint( mtl_hemi );
   uint packed_hemi  = ( ( packed >> 21 ) & uint(15) ) + ( ( packed & USABLE_BIT_15 ) == 0 ? 0 : 16 );
   return float( packed_hemi ) * (1.0/31.0) * 1.333333333;
}
*/
float3 GetPosition(float2 tc)
{
	float linearDepth = tex2D(s_position, tc).z;
	return float3(linearDepth * ( (tc * pos_decompression_params2.xy) * pos_decompression_params.zw - pos_decompression_params.xy ), linearDepth);
}

float3 GetNormal(float2 tc)
{
	return gbuf_unpack_normal(tex2D(s_position, tc).xy);
}

void GetGBuffer(out float4 P, out float4 N, float2 tc)
{
	float4 texBaseP = tex2D(s_position, tc);

	N.xyz = gbuf_unpack_normal(texBaseP.xy);
	N.w = texBaseP.w;//gbuf_unpack_hemi(texBase.w);
	
	P.xyz = float3(texBaseP.z * ( (tc * pos_decompression_params2.xy) * pos_decompression_params.zw - pos_decompression_params.xy ), texBaseP.z);
	P.xyz = P.xyz + N.xyz * 0.5 * def_virtualh;
	P.w = 0.25;//gbuf_unpack_mtl(texBase.w);
}

f_deffer pack_gbuffer( float4 norm, float4 pos, float4 col )
{
	f_deffer res;

	res.position	= float4( gbuf_pack_normal( norm.xyz ), pos.z, norm.w /*gbuf_pack_hemi_mtl( norm.w, pos.w )*/ );
	res.C			   = col;

	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////
#define	LUMINANCE_VECTOR                 half3(0.3f, 0.38f, 0.22f)
void        tonemap              (out half4 low, out half4 high, half3 rgb, half scale)
{
        rgb     =      	rgb*scale       ;

		const float fWhiteIntensity = 1.7;

		const float fWhiteIntensitySQR = fWhiteIntensity*fWhiteIntensity;
#ifdef	USE_BRANCHING		// ps_3_0
        //low		=       rgb.xyzz		;

		low		=	( (rgb*(1+rgb/fWhiteIntensitySQR)) / (rgb+1) ).xyzz;

        high	=		low/def_hdr		;        // 8x dynamic range
#else
        low		=       half4           ( ( (rgb*(1+rgb/fWhiteIntensitySQR)) / (rgb+1) ),           0 )	;
        high	=       half4       	(rgb/def_hdr,   0 )	;		// 8x dynamic range
#endif

/*
	rgb		=	rgb*scale;

	low		=	rgb.xyzz;
	high	=	low/def_hdr;	// 8x dynamic range
*/

//		low		= 	half4	(rgb, 0);
//		rgb		/=	def_hdr	;
//		high	= 	half4	(rgb, dot(rgb,0.333f)-def_hdr_clip)		;
}
half4		combine_bloom        (half3  low, half4 high)	{
        return        half4(low + high*high.a, 1.h);
}

float3	v_hemi        	(float3 n)                        	{        return L_hemi_color*(.5f + .5f*n.y);                   }
float3	v_hemi_wrap     (float3 n, float w)                	{        return L_hemi_color*(w + (1-w)*n.y);                   }
float3	v_sun           (float3 n)                        	{        return L_sun_color*dot(n,-L_sun_dir_w);                }
float3	v_sun_wrap      (float3 n, float w)                	{        return L_sun_color*(w+(1-w)*dot(n,-L_sun_dir_w));      }
half3   p_hemi          (float2 tc)                         {
//        half3        	t_lmh         = tex2D             	(s_hemi, tc);
//        return  dot     (t_lmh,1.h/4.h);
        half4        	t_lmh         = tex2D             	(s_hemi, tc);
        return t_lmh.a;
}

half   get_hemi( half4 lmh)
{
	//return lmh.g;
	return dot(lmh.rgb,1.h/3.h);
}

half   get_sun( half4 lmh)
{
	return lmh.w;
}

//	contrast function
half Contrast(half Input, half ContrastPower)
{
     //piecewise contrast function
     bool IsAboveHalf = Input > 0.5 ;
     half ToRaise = saturate(2*(IsAboveHalf ? 1-Input : Input));
     half Output = 0.5*pow(ToRaise, ContrastPower);
     Output = IsAboveHalf ? 1-Output : Output;
     return Output;
}

#define FXPS technique _render{pass _code{PixelShader=compile ps_3_0 main();}}
#define FXVS technique _render{pass _code{VertexShader=compile vs_3_0 main();}}

#endif
