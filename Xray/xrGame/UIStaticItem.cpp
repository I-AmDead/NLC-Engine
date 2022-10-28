#include "stdafx.h"
#include "uistaticitem.h"
#include "hudmanager.h"

ref_geom		hGeom_fan = NULL;	

#pragma optimize("gyts", off)

void CreateUIGeom()
{
	hGeom_fan.create(FVF::F_TL, RCache.Vertex.Buffer(), 0);
}

void DestroyUIGeom()
{
	hGeom_fan = NULL;
}

ref_geom	GetUIGeom()
{
	return hGeom_fan;
}

CUIStaticItem::CUIStaticItem()
{    
	dwColor			= 0xffffffff;
	iTile.set		(1, 1);
	iRem.set		(0.0f,0.0f);
	alpha_ref		= -1;
	hShader			= NULL;
#ifdef DEBUG
	dbg_tex_name = NULL;
#endif
}

CUIStaticItem::~CUIStaticItem()
{
}

void CUIStaticItem::CreateShader(LPCSTR tex, LPCSTR sh)
{	
	hShader.create(sh,tex);
	
#ifdef DEBUG
	dbg_tex_name = tex;
#endif
	uFlags.set(flValidRect, FALSE);
}

void CUIStaticItem::SetShader(const ref_shader& sh)
{
	hShader = sh;
}

void CUIStaticItem::Init(LPCSTR tex, LPCSTR sh, float left, float top, u32 align)
{
	uFlags.set(flValidRect, FALSE);

	CreateShader	(tex,sh);
	SetPos			(left,top);
	SetAlign		(align);
}

void CUIStaticItem::Render()
{
	VERIFY(g_bRendering);
	// установить обязательно перед вызовом CustomItem::Render() !!!
	R_ASSERT(hShader);
//	__try
//	{
		RCache.set_Shader(hShader);
//	}
//	__except (SIMPLE_FILTER)
//	{
//		Msg("!#EXCEPTION: in CUIStatic %p, shader assigned = %s ", (void*)this, hShader ? "yep" : "no" );
//	}
	if(alpha_ref!=-1)
		RCache.set_AlphaRef(alpha_ref);

	// convert&set pos
	Fvector2		bp;
	UI()->ClientToScreenScaled	(bp,float(iPos.x),float(iPos.y));
	UI()->AlignPixel(bp.x);
	UI()->AlignPixel(bp.y);

	// actual rendering
	Fvector2					pos;
	Fvector2					f_len;
	UI()->ClientToScreenScaled	(f_len, iVisRect.x2, iVisRect.y2 );

	int tile_x					= fis_zero(iRem.x)?iTile.x:iTile.x+1;
	int tile_y					= fis_zero(iRem.y)?iTile.y:iTile.y+1;
	if (! (tile_x>0 && tile_y>0) )		return;
	
	// render

	//UIRender->StartPrimitive(8*tile_x*tile_y, IUIRender::ptTriList, UI()->m_currentPointType);
	u32							vOffset;
	FVF::TL* start_pv = (FVF::TL*)RCache.Vertex.Lock(8*tile_x*tile_y, hGeom_fan.stride(), vOffset);
	FVF::TL* pv = start_pv;
	for (int x=0; x<tile_x; ++x)
	{
		for (int y=0; y<tile_y; ++y)
		{
			pos.set				(bp.x+f_len.x*x,bp.y+f_len.y*y);
			inherited::Render	(pv,pos,dwColor);
		}
	}
	Frect clip_rect				= {iPos.x,iPos.y,iPos.x+iVisRect.x2*iTile.x+iRem.x,iPos.y+iVisRect.y2*iTile.y+iRem.y};
	// set scissor
	UI()->PushScissor			(clip_rect);
	//UIRender->FlushPrimitive	();
	std::ptrdiff_t p_cnt = pv - start_pv;
	VERIFY(u32(p_cnt) <= 8);

	RCache.Vertex.Unlock(u32(p_cnt), hGeom_fan.stride());
	RCache.set_Geometry(hGeom_fan);

	u32 primCount = (u32)(p_cnt / 3);
	if (primCount > 0)
		RCache.Render(D3DPT_TRIANGLELIST, vOffset, primCount);


	UI()->PopScissor			();

	if(alpha_ref!=-1)
		RCache.set_AlphaRef(0);
}

void CUIStaticItem::Render(float angle)
{
	VERIFY						(g_bRendering);
	// установить обязательно перед вызовом CustomItem::Render() !!!
	VERIFY						(hShader);
	RCache.set_Shader(hShader);
	if(alpha_ref!=-1)
		RCache.set_AlphaRef(alpha_ref);

	// convert&set pos
	Fvector2		bp_ns;
	bp_ns.set		(iPos);


	// actual rendering
//.	UIRender->StartTriFan(32);
	//UIRender->StartPrimitive	(32, IUIRender::ptTriList, UI()->m_currentPointType);
	u32							vOffset;
	FVF::TL* start_pv = (FVF::TL*)RCache.Vertex.Lock(32, hGeom_fan.stride(), vOffset);
	FVF::TL* pv = start_pv;
	inherited::Render			(pv,bp_ns,dwColor,angle);

//.	UIRender->FlushTriFan();
	//UIRender->FlushPrimitive();
	std::ptrdiff_t p_cnt = pv - start_pv;
	VERIFY(u32(p_cnt) <= 32);

	RCache.Vertex.Unlock(u32(p_cnt), hGeom_fan.stride());
	RCache.set_Geometry(hGeom_fan);

	u32 primCount = (u32)(p_cnt / 3);
	if (primCount > 0)
		RCache.Render(D3DPT_TRIANGLELIST, vOffset, primCount);

	if(alpha_ref!=-1)
		RCache.set_AlphaRef(alpha_ref);
}
