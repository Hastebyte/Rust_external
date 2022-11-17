#ifndef OFFSETS_H
#define OFFSETS_H

#include <windows.h>

constexpr uint64_t OFFSET_GAME_OBJECT_MANAGER		= 0x15FF218; // 0x15FC218;
constexpr uint64_t OFFSET_CORRESPONDING_OBJECT		= 0x30;

constexpr uint64_t OFFSET_OBJECT_LAYER				= 0x50;
constexpr uint64_t OFFSET_OBJECT_TAG				= 0x54;
constexpr uint64_t OFFSET_OBJECT_NAME				= 0x60;

constexpr uint64_t OFFSET_ENTITY_TRANSFORM			= 0x08;
constexpr uint64_t OFFSET_TRANSFORM_VISUALSTATE		= 0x38;
constexpr uint64_t OFFSET_VISUALSTATE_POSITION		= 0xB0;

constexpr uint64_t OFFSET_ENTITY_REF				= 0x18;
constexpr uint64_t OFFSET_BASE_ENTITY				= 0x28;
constexpr uint64_t OFFSET_BASE_PLAYER_STEAMID		= 0x538;
constexpr uint64_t OFFSET_BASE_PLAYER_NAME			= 0x4B0;
constexpr uint64_t OFFSET_BASE_PLAYER_IS_SLEEPING	= 0x571;
constexpr uint64_t OFFSET_BASE_PLAYER_PLAYER_MODEL	= 0x4C8;
constexpr uint64_t OFFSET_BASE_PLAYER_PLAYER_FLAGS	= 0x530;
constexpr uint64_t OFFSET_BASE_PLAYER_TEAM_NUMBER	= 0x5A8;
constexpr uint64_t OFFSET_BASE_COMBAT_ENTITY_HEALTH	= 0x1D0;

constexpr uint64_t OFFSET_BASE_PLAYER_INPUT			= 0x478;
constexpr uint64_t OFFSET_BASE_PLAYER_MOVEMENT		= 0x480;
constexpr uint64_t OFFSET_ACTIVE_ITEM_ID			= 0x5B0;

constexpr uint64_t OFFSET_PLAYER_MODEL_VELOCITY		= 0x1B4;
constexpr uint64_t OFFSET_PLAYER_MODEL_VISIBLE		= 0x218;
constexpr uint64_t OFFSET_PLAYER_MODEL_SKIN			= 0xE0;
constexpr uint64_t OFFSET_BONE_DICTIONARY			= 0x20;
constexpr uint64_t OFFSET_BONE_DICTIONARY_ENTRY		= 0x18;
constexpr uint64_t OFFSET_ENTRY_BONE_VALUE			= 0x30;
constexpr uint64_t OFFSET_BONE_VALUE_TRANS			= 0x10;

constexpr uint64_t OFFSET_TOD_CYCLE_PARAMETERS		= 0x18;
constexpr uint64_t OFFSET_TOD_HOUR					= 0x10;

constexpr uint64_t OFFSET_CAMERA_OBJECT				= 0x30;
constexpr uint64_t OFFSET_MAIN_CAMERA				= 0x18;
constexpr uint64_t OFFSET_CAMERA_VIEW_MATRIX		= 0x2E4;
constexpr uint64_t OFFSET_CAMERA_POSITION			= 0x42C;

enum E_OBJECT_TAG : uint16_t
{
	ID_TAG_INVALID	= 0,	 //
	ID_TAG_CAMERA	= 5,	 // 
	ID_TAG_PLAYER	= 6,	 // 
	ID_TAG_SKY		= 20011, //
};

enum bones : uint32_t
{
	l_hip = 1,
	l_knee,
	l_foot,
	l_toe,
	l_ankle_scale,
	pelvis,

	penis,
	GenitalCensor,
	GenitalCensor_LOD0,
	Inner_LOD0,
	GenitalCensor_LOD1,
	GenitalCensor_LOD2,
	r_hip,
	r_knee,
	r_foot,
	r_toe,
	r_ankle_scale,
	spine1,
	spine1_scale,
	spine2,
	spine3,
	spine4,
	l_clavicle,
	l_upperarm,
	l_forearm,
	l_hand,
	l_index1,
	l_index2,
	l_index3,
	l_little1,
	l_little2,
	l_little3,
	l_middle1,
	l_middle2,
	l_middle3,
	l_prop,
	l_ring1,
	l_ring2,
	l_ring3,
	l_thumb1,
	l_thumb2,
	l_thumb3,
	IKtarget_righthand_min,
	IKtarget_righthand_max,
	l_ulna,
	neck,
	head,
	jaw,
	eyeTranform,
	l_eye,
	l_Eyelid,
	r_eye,
	r_Eyelid,
	r_clavicle,
	r_upperarm,
	r_forearm,
	r_hand,
	r_index1,
	r_index2,
	r_index3,
	r_little1,
	r_little2,
	r_little3,
	r_middle1,
	r_middle2,
	r_middle3,
	r_prop,
	r_ring1,
	r_ring2,
	r_ring3,
	r_thumb1,
	r_thumb2,
	r_thumb3,
	IKtarget_lefthand_min,
	IKtarget_lefthand_max,
	r_ulna,
	l_breast,
	r_breast,
	BoobCensor,
	BreastCensor_LOD0,
	BreastCensor_LOD1,
	BreastCensor_LOD2,
	collision,
	displacement,
};


#ifdef _AMD64_
	#define _PTR_MAX_VALUE ( 0x000F000000000000 )
#else 
	#define _PTR_MAX_VALUE ( 0xFFE00000 )
#endif

inline bool _VALID( UINT_PTR Ptr )
{
	return ( Ptr >= 0x10000 ) && ( Ptr < _PTR_MAX_VALUE );
}

#endif