#ifndef SH_TEXTURE_H
#define SH_TEXTURE_H
#pragma once

#include "../xrCore/xr_resource.h"

class ENGINE_API CAviPlayerCustom;
class ENGINE_API CTheoraSurface;

class ENGINE_API CTexture			: public xr_resource_named				{
public:
	//	Since DX10 allows up to 128 unique textures, 
	//	distance between enum values should be at leas 128
	enum ResourceShaderType	//	Don't change this since it's hardware-dependent
	{
		rstPixel = 0,	//	Default texture offset
		rstVertex = D3DVERTEXTEXTURESAMPLER0,
		rstGeometry = rstVertex+256,
		rstHull = rstGeometry+256,
		rstDomain = rstHull+256,
		rstCompute = rstDomain+256,
        rstInvalid = rstCompute+256
	};
	enum {
		TXLOAD_MANAGED = (1 << 0),  // загрузка в управляемый пул	
		TXLOAD_DEFAULT_POOL = (1 << 1),	 // форсированная загрузка в дефолтный пул  
		TXLOAD_APPLY_LOD = (1 << 2),  //	разрешение применять texture_lod
		TXLOAD_UNPACKED = (1 << 3),	 // загрузка в формате A8R8G8B8
		TXLOAD_DEFAULT = (1 << 31)	 // использовать предыдущие параметры, или заданные в конструкторе
	};
public:
	void	__stdcall					apply_load		(u32	stage);
	void	__stdcall					apply_theora	(u32	stage);
	void	__stdcall					apply_avi		(u32	stage);
	void	__stdcall					apply_seq		(u32	stage);
	void	__stdcall					apply_normal	(u32	stage);

	void								Preload			();
	void								Load			();
	void								LoadImpl		( Flags32 load_flags = { TXLOAD_DEFAULT } );
	void								PostLoad		();
	void								SetName			(LPCSTR _name);
	void								Unload			(void);
	
//	void								Apply			(u32 dwStage);

	void								surface_set		(ID3DBaseTexture* surf );
	ID3DBaseTexture*					surface_get 	();

	IC BOOL								isUser			()		{ return flags.bUser;					}
	IC u32								get_Width		()		{ desc_enshure(); return desc.Width;	}
	IC u32								get_Height		()		{ desc_enshure(); return desc.Height;	}

	void								video_Sync		(u32 _time){m_play_time=_time;}
	void								video_Play		(BOOL looped, u32 _time=0xFFFFFFFF);
	void								video_Pause		(BOOL state);
	void								video_Stop		();
	BOOL								video_IsPlaying	();

	CTexture							();
	virtual ~CTexture					();
	
#if defined(USE_DX10) || defined(USE_DX11)
	ID3DShaderResourceView*				get_SRView() {return m_pSRView;}
#endif	//	USE_DX10

private:
	IC BOOL								desc_valid		()		{ return pSurface==desc_cache; }
	IC void								desc_enshure	()		{ if (!desc_valid()) desc_update(); }
	void								desc_update		();
#if defined(USE_DX10) || defined(USE_DX11)
	void								Apply			(u32 dwStage);
	void								ProcessStaging();
	D3D_USAGE							GetUsage();
#endif	//	USE_DX10

	//	Class data
public:	//	Public class members (must be encapsulated furthur)
	struct 
	{
		u32					bLoaded		: 1;
		u32					bUser		: 1;
		u32					seqCycles	: 1;
		u32					MemoryUsage	: 28;
#if defined(USE_DX10) || defined(USE_DX11)
		u32					bLoadedAsStaging: 1;
#endif	//	USE_DX10
	}									flags;
	fastdelegate::FastDelegate1<u32>	bind;


	CAviPlayerCustom*					pAVI;
	CTheoraSurface*						pTheora;
	float								m_material;
	shared_str							m_bumpmap;
	float								m_time_apply;	    // alpet: время последнего использования текстуры в рендеринге		

	union{
		u32								m_play_time;		// sync theora time
		u32								seqMSPF;			// Sequence data milliseconds per frame
	};
	shared_str							reg_name;			// alpet: имя регистрации текстуры в списке (для заменяемых текстур)

private:
	ID3DBaseTexture*					pSurface;
	// Sequence data
	xr_vector<ID3DBaseTexture*>			seqDATA;

	// Description
	ID3DBaseTexture*					desc_cache;
	D3D_TEXTURE2D_DESC					desc;

#if defined(USE_DX10) || defined(USE_DX11)
	ID3DShaderResourceView*			m_pSRView;
	// Sequence view data
	xr_vector<ID3DShaderResourceView*>m_seqSRView;
#endif	//	USE_DX10
};
struct 	ENGINE_API	resptrcode_texture	: public resptr_base<CTexture>
{
	void				create			(LPCSTR	_name);
	void				destroy			()					{ _set(NULL);					}
	shared_str			bump_get		()					{ return _get()->m_bumpmap;		}
	bool				bump_exist		()					{ return 0!=bump_get().size();	}
};
typedef	resptr_core<CTexture,resptrcode_texture >	
	ref_texture;

#endif
