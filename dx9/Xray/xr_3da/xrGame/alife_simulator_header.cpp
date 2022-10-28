////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_simulator_header.cpp
//	Created 	: 05.01.2003
//  Modified 	: 12.05.2004
//	Author		: Dmitriy Iassenev
//	Description : ALife Simulator header
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "alife_simulator_header.h"
#include "../../build_config_defines.h"
XRCORE_API u32 build_id;

CALifeSimulatorHeader::~CALifeSimulatorHeader	()
{
}

void CALifeSimulatorHeader::save				(IWriter	&memory_stream)
{
	memory_stream.open_chunk	(ALIFE_CHUNK_DATA);
	memory_stream.w_u32			(ALIFE_VERSION);
#ifdef LUAICP_COMPAT
	memory_stream.w_u32			(build_id);
#ifdef NLC_EXTENSIONS
	memory_stream.w_stringZ		("NLC7");
#endif
#endif
	memory_stream.close_chunk	();
}

void CALifeSimulatorHeader::load				(IReader	&file_stream)
{
	R_ASSERT2					(file_stream.find_chunk(ALIFE_CHUNK_DATA),"Can't find chunk ALIFE_CHUNK_DATA");
	m_version					= file_stream.r_u32();
	R_ASSERT2					(m_version>=0x0002,"ALife version mismatch! (Delete saved game and try again)");
#ifdef LUAICP_COMPAT
	u32	ref_id					= file_stream.r_u32();
	R_ASSERT2					(ref_id >= 5700, "build_id version not compatible with engine! (Delete saved game and try again)");
#ifdef NLC_EXTENSIONS
	shared_str    mod_tag;
	file_stream.r_stringZ(mod_tag);
	R_ASSERT3					(mod_tag == "NLC7", "game modification tag wrong", *mod_tag);
#endif
#endif
	
};

bool CALifeSimulatorHeader::valid				(IReader	&file_stream) const
{
	if (!file_stream.find_chunk(ALIFE_CHUNK_DATA))
		return					(false);

	u32							version;
	file_stream.r				(&version,	sizeof(version));
	return						(version>=2);
}
