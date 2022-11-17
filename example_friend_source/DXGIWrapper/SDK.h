#ifndef SDK_H
#define SDK_H

#pragma once

#pragma warning (disable:4267)

typedef size_t offset_t;

constexpr uint64_t GlobalObjectManager = 0x15FC218;

constexpr uint64_t CorrespondingObject = 0x30;
constexpr uint64_t EntityRef_Value = 0x18;
constexpr uint64_t Base_Entitym = 0x28;
constexpr uint64_t BasePlayer_SteamID = 0x538;
constexpr uint64_t BasePlayer_Name = 0x4B0;
constexpr uint64_t BasePlayer_isSleeping = 0x571;
constexpr uint64_t BasePlayer_PlayerModel = 0x4C8;
constexpr uint64_t BasePlayer_PlayerFlags = 0x530;
constexpr uint64_t BasePlayer_TeamNumber = 0x5A8;
constexpr uint64_t BaseCombatEntity_Health = 0x1D0;

constexpr uint64_t BasePlayerInput = 0x478;
constexpr uint64_t BasePlayerMovement = 0x480;
constexpr uint64_t ActiveItemId = 0x5B0;

constexpr uint64_t Object_Name = 0x60;
constexpr uint64_t Object_Layer = 0x50;
constexpr uint64_t Object_Tag = 0x54;

constexpr uint64_t CameraObject = 0x30;
constexpr uint64_t LocalEntity_MainCamera = 0x18;
constexpr uint64_t CameraViewMatrix = 0x2E4;
constexpr uint64_t CameraPos = 0x42C;

constexpr uint64_t LocalEntity_Transform = 0x8;
constexpr uint64_t Transform_VisualState = 0x38;
constexpr uint64_t VisualState_Position = 0xB0;

constexpr uint64_t PlayerModel_velocity = 0x1B4;
constexpr uint64_t PlayerModel_visible = 0x218; //byte
constexpr uint64_t PlayerModel_Skin = 0xE0;
constexpr uint64_t BoneDic_ = 0x20;
constexpr uint64_t BoneDic_Entry = 0x18;
constexpr uint64_t Entry_BoneValue = 0x30;
constexpr uint64_t BoneValue_Trans = 0x10;

constexpr uint64_t clientTeam = 0x4F0;
constexpr uint64_t clientTeam_ID = 0x20;
constexpr uint64_t clientTeam_members = 0x18;
constexpr uint64_t members_size = 0x14;
//----------------------Belt
constexpr uint64_t cActItemID = 0x5A8;
constexpr uint64_t PlayerInventory = 0x460;
constexpr uint64_t ItemContainer = 0x28;
constexpr uint64_t list_item = 0x20;
constexpr uint64_t item_id = 0x78;
constexpr uint64_t item_Defintion = 0x10;
constexpr uint64_t list_item_count = 0x18;
constexpr uint64_t Phrase = 0x20;
constexpr uint64_t ItemName = 0x18;
constexpr uint64_t Name_len = 0x10;
constexpr uint64_t Name_char = 0x14;

namespace TOD_SKY {
	constexpr uint64_t TOD_CycleParameters = 0x18;
	constexpr uint64_t TOD_WorldParameters = 0x20;
	constexpr uint64_t TOD_AtmosphereParameters = 0x28;
	constexpr uint64_t TOD_DayParameters = 0x30;
	constexpr uint64_t TOD_NightParameters = 0x38;
	constexpr uint64_t TOD_SunParamteres = 0x40;
	constexpr uint64_t TOD_MoonParameters = 0x48;
	constexpr uint64_t TOD_StarParameters = 0x50;
	constexpr uint64_t TOD_CloudParameters = 0x58;
	constexpr uint64_t TOD_FogParameters = 0x68;
}

namespace TOD_SUN {
	constexpr uint64_t MeshSize = 0x10; //Set to Zero for No Sun (Still bright though, just no AIDS glare :D)
	constexpr uint64_t MeshBrightness = 0x14; //Set to Zero for No Sun
	constexpr uint64_t MeshContrast = 0x18;
}

namespace TOD_MOON {
	constexpr uint64_t MeshSize = 0x10; //Set to Zero for No Moon
	constexpr uint64_t MeshBrightness = 0x14; //Set to Zero for No Moon
	constexpr uint64_t MeshContrast = 0x18; //Set to Zero For No Glare 
	constexpr uint64_t HaloSize = 0x1C;
	constexpr uint64_t HaloBrightness = 0x20;
}

namespace TOD_CYCLE {
	constexpr uint64_t Hour = 0x10; //Control Time of Day
	constexpr uint64_t Day = 0x14;
	constexpr uint64_t Month = 0x18;
	constexpr uint64_t Year = 0x1C;
}

namespace TOD_ATMOSPHERE {
	constexpr uint64_t RayleighMultiplier = 0x10; //Adjust sky's "blue"
	constexpr uint64_t MeiMultiplier = 0x14;//Adjust white light scattering
	constexpr uint64_t Brightness = 0x18;//Adjust in game brightness 
	constexpr uint64_t Contrast = 0x1C;
	constexpr uint64_t Directionality = 0x20;
	constexpr uint64_t Fogginess = 0x24;//Set to Zero for no-fog
}

namespace TOD_CLOUD {
	constexpr uint64_t Size = 0x10;
	constexpr uint64_t Opacity = 0x14;
	constexpr uint64_t Coverage = 0x18;
	constexpr uint64_t Sharpness = 0x1C;
	constexpr uint64_t Coloring = 0x20;
	constexpr uint64_t Attenuation = 0x24;
	constexpr uint64_t Saturation = 0x28;
	constexpr uint64_t Scattering = 0x2C;
	constexpr uint64_t Brightness = 0x24;
}

namespace TOD_DAY {
	constexpr uint64_t LightIntensity = 0x48;
	constexpr uint64_t ShadowStrength = 0x4C;//Gets rid of shadows in game
	constexpr uint64_t AmbientMultiplier = 0x50; //Set to 1.00 for brightness in caves
	constexpr uint64_t ReflectionMultiplier = 0x54; //Set to 1.00 for brightness in caves
}

namespace TOD_NIGHT {
	constexpr uint64_t LightIntensity = 0x48;
	constexpr uint64_t ShadowStrength = 0x4C;
	constexpr uint64_t AmbientMultiplier = 0x50;
	constexpr uint64_t ReflectionMultiplier = 0x54;
}

//-----------------------------------------------------------------------------
// Call a virtual function
//-----------------------------------------------------------------------------
template<typename FuncType>
FuncType CallVFunction(void* ppClass, size_t index) {
	uintptr_t* pVTable = *(uintptr_t**)ppClass;
	uintptr_t dwAddress = pVTable[index];
	return (FuncType)(dwAddress);
}

#endif // SDK_H