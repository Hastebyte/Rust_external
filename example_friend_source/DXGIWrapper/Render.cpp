#include "Headers.h"
#include "SDK.h"

static DWORD64 game_handle;
static DWORD64 unity_handle;

float s_width, s_height;

//Vector2
class Vector2
{
public:
	Vector2() : x(0.f), y(0.f)
	{

	}

	Vector2(float _x, float _y) : x(_x), y(_y)
	{

	}
	~Vector2()
	{

	}

	float x;
	float y;

	Vector2 operator+(Vector2 v)
	{
		return Vector2(x + v.x, y + v.y);
	}

	Vector2 operator-(Vector2 v)
	{
		return Vector2(x - v.x, y - v.y);
	}
};

//Vector3
class Vector3
{
public:
	Vector3() : x(0.f), y(0.f), z(0.f)
	{

	}

	Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
	{

	}
	~Vector3()
	{

	}

	float x;
	float y;
	float z;

	inline float Dot(Vector3 v)
	{
		return x * v.x + y * v.y + z * v.z;
	}

	inline float Distance(Vector3 v)
	{
		return float(sqrtf(powf(v.x - x, 2.0) + powf(v.y - y, 2.0) + powf(v.z - z, 2.0)));
	}

	inline float Length()
	{
		return sqrtf(Dot(*this));
	}

	Vector3 operator+(Vector3 v)
	{
		return Vector3(x + v.x, y + v.y, z + v.z);
	}

	Vector3 operator-(Vector3 v)
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}
};

//Vector4
class Vector4
{
public:
	Vector4() : x(0.f), y(0.f), z(0.f), w(0.f)
	{

	}

	Vector4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w)
	{

	}
	~Vector4()
	{

	}

	float x;
	float y;
	float z;
	float w;
};

namespace global
{
	DWORD64 pGOM = 0;
	DWORD64 pSky = 0;
	D3DXMATRIX viewMatrix;
	DWORD64 pLocal = 0;
	Vector3 LocalPos;
	Vector3 LocalCam;
	int CurrentItemId = 0;
	int LocalTeam = 0;
}

void CornerBox(Renderer* pRender, float x, float y, float w, float h, ImColor Color)
{
	int bWidth = w;
	int bHeight = h;                                                           // Box: Upper Left Corner    V   
	pRender->DrawLine(ImVec2(x, y), ImVec2(x, y + (bHeight / 5)), Color, 1.0f);     // Top To Bottom     
	pRender->DrawLine(ImVec2(x, y), ImVec2(x + (bWidth / 5), y), Color, 1.0f);     // Left To Right 

																				   //  Box: Upper Right Corner   V      
	pRender->DrawLine(ImVec2(x + bWidth, y), ImVec2(x + bWidth - (bWidth / 5), y), Color, 1.0f);     // Right To Left     
	pRender->DrawLine(ImVec2(x + bWidth, y), ImVec2(x + bWidth, y + (bHeight / 5)), Color, 1.0f);   // Top To Bottom   

																									// Box: Bottom Left Corner   V     
	pRender->DrawLine(ImVec2(x, y + bHeight), ImVec2(x + (bWidth / 5), y + bHeight), Color, 1.0f);            // Right To Left   
	pRender->DrawLine(ImVec2(x, y + bHeight), ImVec2(x, y + bHeight - (bHeight / 5)), Color, 1.0f);          // Bottom To Top  
																											 // Box: Bottom Right Corner  V       
	pRender->DrawLine(ImVec2(x + bWidth, y + bHeight), ImVec2(x + bWidth - (bWidth / 5), y + bHeight), Color, 1.0f);  // Right To Left         
	pRender->DrawLine(ImVec2(x + bWidth, y + bHeight), ImVec2(x + bWidth, y + bHeight - (bHeight / 5)), Color, 1.0f);// Bottom To Top
}

void CornerBoxOutline(Renderer* pRender, float x, float y, float w, float h, ImColor lineColor, ImColor Color)
{
	CornerBox(pRender, x - 1, y - 1, w + 2, h + 2, lineColor);
	CornerBox(pRender, x, y, w, h, Color);
}

void HealthBar_H(Renderer* pRender, float x, float y, int w, int h, float health, float healthmax) //health [0,100]
{
	if (health > 0.0f)
	{
		float progress = health / (healthmax / 100.0f);

		float healthlen = health / healthmax;

		float drawheight = h * healthlen;

		ImColor color = ImColor(1.0f, 0.0f, 0.0f, 0.8f);
		if (progress >= 20) color = ImColor(1.0f, 0.65f, 0.0f, 0.8f);
		if (progress >= 40) color = ImColor(1.0f, 1.0f, 0.0f, 0.8f);
		if (progress >= 60) color = ImColor(0.68f, 1.0f, 0.18f, 0.8f);
		if (progress >= 80) color = ImColor(0.0f, 1.0f, 0.0f, 0.8f);

		pRender->DrawRect(ImVec2(x - w - 3, y - 1), ImVec2(w + 1, h), ImColor(0.0f, 0.0f, 0.0f, 1.0f));
		pRender->DrawRect(ImVec2(x - w - 2, y), ImVec2(w - 1, drawheight - 1), color);
	}
}

bool WorldToScreen(Vector3 origin, Vector2* out)
{
	D3DXMATRIX temp;

	D3DXMatrixTranspose(&temp, &global::viewMatrix);

	Vector3 translationVector = Vector3(temp._41, temp._42, temp._43);
	Vector3 up = Vector3(temp._21, temp._22, temp._23);
	Vector3 right = Vector3(temp._11, temp._12, temp._13);

	float w = translationVector.Dot(origin) + temp._44;

	if (w < 0.098f)
		return false;

	float y = up.Dot(origin) + temp._24;
	float x = right.Dot(origin) + temp._14;

	out->x = (s_width / 2) * (1.f + x / w);
	out->y = (s_height / 2) * (1.f - y / w);

	return true;
}

enum Bones : int {

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

uintptr_t get_entity_bone(uintptr_t entity, int bone_index) {
	uintptr_t player_model = process->Read<DWORD64>((uintptr_t)entity + BasePlayer_PlayerModel);
	if (!player_model) {
		return 0;
	}

	uintptr_t multi_mesh = process->Read<DWORD64>((uintptr_t)player_model + PlayerModel_Skin);
	if (!multi_mesh) {
		return 0;
	}

	uintptr_t bone_dictionary = process->Read<DWORD64>((uintptr_t)multi_mesh + BoneDic_);
	if (!bone_dictionary) {
		return 0;
	}

	uintptr_t bone_values = process->Read<DWORD64>((uintptr_t)bone_dictionary + BoneDic_Entry);
	if (!bone_values) {
		return 0;
	}

	uintptr_t entity_bone = process->Read<DWORD64>((uintptr_t)bone_values + (Entry_BoneValue + ((bone_index - 1) * 0x8)));
	if (!entity_bone) {
		return 0;
	}

	uintptr_t bone_transform = process->Read<DWORD64>((uintptr_t)entity_bone + BoneValue_Trans);
	if (!bone_transform) {
		return 0;
	}

	return bone_transform;
}

struct TransformAccessReadOnly
{
	uintptr_t pTransformData;
};

struct TransformData
{
	uintptr_t pTransformArray;
	uintptr_t pTransformIndices;
};

struct Matrix34
{
	Vector4 vec0;
	Vector4 vec1;
	Vector4 vec2;
};

float vectorGetByIndex(__m128 V, int index) {
	union {
		__m128 v;
		float a[4];
	} converter;
	converter.v = V;
	return converter.a[index];
}

Vector3 get_bone_pos(uintptr_t pTransform)
{
	if (!pTransform) {
		return Vector3(0,0,0);
	}

	__m128 result;

	const __m128 mulVec0 = { -2.000, 2.000, -2.000, 0.000 };
	const __m128 mulVec1 = { 2.000, -2.000, -2.000, 0.000 };
	const __m128 mulVec2 = { -2.000, -2.000, 2.000, 0.000 };

	TransformAccessReadOnly pTransformAccessReadOnly = process->Read <TransformAccessReadOnly>(pTransform + 0x38);

	unsigned int index = process->Read<unsigned int>(pTransform + 0x40);
	TransformData transformData = process->Read <TransformData>(pTransformAccessReadOnly.pTransformData + 0x18);

	if (transformData.pTransformArray && transformData.pTransformIndices)
	{
		result = process->Read <__m128>(transformData.pTransformArray + 0x30 * index);
		int transformIndex = process->Read<int>(transformData.pTransformIndices + 0x4 * index);
		int pSafe = 0;
		while (transformIndex >= 0 && pSafe++ < 200)
		{
			Matrix34 matrix34 = process->Read <Matrix34>(transformData.pTransformArray + 0x30 * transformIndex);

			__m128 xxxx = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0x00));	// xxxx
			__m128 yyyy = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0x55));	// yyyy
			__m128 zwxy = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0x8E));	// zwxy
			__m128 wzyw = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0xDB));	// wzyw
			__m128 zzzz = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0xAA));	// zzzz
			__m128 yxwy = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0x71));	// yxwy
			__m128 tmp7 = _mm_mul_ps(*(__m128*)(&matrix34.vec2), result);

			result = _mm_add_ps(
				_mm_add_ps(
					_mm_add_ps(
						_mm_mul_ps(
							_mm_sub_ps(
								_mm_mul_ps(_mm_mul_ps(xxxx, mulVec1), zwxy),
								_mm_mul_ps(_mm_mul_ps(yyyy, mulVec2), wzyw)),
							_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(tmp7), 0xAA))),
						_mm_mul_ps(
							_mm_sub_ps(
								_mm_mul_ps(_mm_mul_ps(zzzz, mulVec2), wzyw),
								_mm_mul_ps(_mm_mul_ps(xxxx, mulVec0), yxwy)),
							_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(tmp7), 0x55)))),
					_mm_add_ps(
						_mm_mul_ps(
							_mm_sub_ps(
								_mm_mul_ps(_mm_mul_ps(yyyy, mulVec0), yxwy),
								_mm_mul_ps(_mm_mul_ps(zzzz, mulVec1), zwxy)),
							_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(tmp7), 0x00))),
						tmp7)), *(__m128*)(&matrix34.vec0));

			transformIndex = process->Read<int>(transformData.pTransformIndices + 0x4 * transformIndex);
		}
	}

	return Vector3(vectorGetByIndex(result, 0), vectorGetByIndex(result, 1), vectorGetByIndex(result, 2));
}


list<int> upper_part = { Bones::neck, Bones::head };
list<int> right_arm = { Bones::neck, Bones::r_clavicle, Bones::r_upperarm, Bones::r_forearm, Bones::r_hand };
list<int> left_arm = { Bones::neck, Bones::l_clavicle, Bones::l_upperarm , Bones::l_forearm, Bones::l_hand };
list<int> spine = { Bones::neck, Bones::spine3, Bones::spine2 };

list<int> lower_right = { Bones::spine2, Bones::r_hip, Bones::r_knee, Bones::r_foot };
list<int> lower_left = { Bones::spine2, Bones::l_hip, Bones::l_knee, Bones::l_foot};

list<list<int>> skeleton = { upper_part, right_arm, left_arm, spine, lower_right, lower_left };

void DrawSkeleton(Renderer* pRender, uint64_t base_entity)
{
	Vector3 neckpos = get_bone_pos(get_entity_bone(base_entity, Bones::neck));
	Vector3 pelvispos = get_bone_pos(get_entity_bone(base_entity, Bones::spine2));
	Vector3 previous(0, 0, 0);
	Vector3 current;
	Vector2 p1, c1;
	for (auto a : skeleton)
	{
		previous = Vector3(0, 0, 0);
		for (int bone : a)
		{
			current = bone == Bones::neck ? neckpos : (bone == Bones::spine2 ? pelvispos : get_bone_pos(get_entity_bone(base_entity, bone)));
			if (previous.x == 0.f)
			{
				previous = current;
				continue;
			}
			if (WorldToScreen(previous, &p1) && WorldToScreen(current, &c1))
			{
				pRender->DrawLine(ImVec2(p1.x, p1.y), ImVec2(c1.x, c1.y), ImColor(150, 255, 255, 0));
				previous = current;
			}
		}
	}
}

vector<DWORD64> playerList;
vector<DWORD64> itemList;

float Dist2D(float x1, float y1, float x2, float y2)
{
	return sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
}

float Dist3D(Vector3 v1, Vector3 v2)
{
	Vector3 Out;
	Out.x = v1.x - v2.x;
	Out.y = v1.y - v2.y;
	Out.z = v1.z - v2.z;
	return sqrt(Out.x*Out.x + Out.y*Out.y + Out.z*Out.z);
}

#define UCONST_Pi 3.141592654
#define CONST_RadToUnrRot 10430.3783504704527

void ClampAngles(Vector3& vAngles) //Pretty sure this isn't needed, but I like to be safe.
{
	if (vAngles.x > 180)
		vAngles.x -= 360;
	else if (vAngles.x < -180)
		vAngles.x += 360;

	if (vAngles.y > 180)
		vAngles.y -= 360;
	else if (vAngles.y < -180)
		vAngles.y += 360;

	if (vAngles.x < -74.9f)
		vAngles.x = -74.9f;
	if (vAngles.x > 74.9f)
		vAngles.x = 74.9f;

	while (vAngles.y < -180.0f)
		vAngles.y += 360.0f;
	while (vAngles.y > 180.0f)
		vAngles.y -= 360.0f;

	vAngles.z = 0.0f;
}

void AimAtPos_Mouse(float x, float y)
{
	int ScreenCenterX = s_width / 2;
	int ScreenCenterY = s_height / 2;

	float TargetX = 0;
	float TargetY = 0;


	//X Axis
	if (x != 0)
	{
		if (x > ScreenCenterX)
		{
			TargetX = -(ScreenCenterX - x);
			TargetX /= 2;
			if (TargetX + ScreenCenterX > ScreenCenterX * 2) TargetX = 0;
		}

		if (x < ScreenCenterX)
		{
			TargetX = x - ScreenCenterX;
			TargetX /= 2;
			if (TargetX + ScreenCenterX < 0) TargetX = 0;
		}
	}

	//Y Axis

	if (y != 0)
	{
		if (y > ScreenCenterX)
		{
			TargetY = -(ScreenCenterY - y);
			TargetY /= 2;
			if (TargetY + ScreenCenterY > ScreenCenterY * 2) TargetY = 0;
		}

		if (y < ScreenCenterX)
		{
			TargetY = y - ScreenCenterY;
			TargetY /= 2;
			if (TargetY + ScreenCenterY < 0) TargetY = 0;
		}
	}

	mouse_event(MOUSEEVENTF_MOVE, (DWORD)TargetX, (DWORD)TargetY, NULL, NULL);
}

void AimAt(Renderer* pRender, Vector3 AimPos, int AimType)
{
	int ScreenCenterY = s_height * 0.5f;
	int ScreenCenterX = s_width * 0.5f;

	Vector2 vScr;
	if (!WorldToScreen(AimPos, &vScr))
	{
		return;
	}

	if (AimType == 0)
	{
		AimAtPos_Mouse(vScr.x, vScr.y);
	}
	else if (AimType == 1)
	{
		Vector3 vecAng;
		Vector3 vecDelta = global::LocalCam - AimPos;
		float magnitude = vecDelta.Length();

		vecAng.x = (asinf(vecDelta.y / magnitude) * 180.0f / 3.14159265f); //Pitch
		vecAng.y = (atan2(vecDelta.x, vecDelta.z) * 180.0f / 3.14159265f) + 180.0f; //Yaw
		vecAng.z = 0.0f;

		ClampAngles(vecAng);

		DWORD64 gameObject = process->Read<DWORD64>(global::pLocal + 0x10);
		if (!gameObject) return;

		DWORD64 objectClass = process->Read<DWORD64>(gameObject + CorrespondingObject);
		if (!objectClass) return;

		DWORD64 entity_ptr = process->Read<DWORD64>(objectClass + EntityRef_Value);
		if (!entity_ptr) return;

		DWORD64 base_entity = process->Read<DWORD64>(entity_ptr + Base_Entitym);
		if (!base_entity) return;

		DWORD64 pPlayerInput = process->Read<DWORD64>(base_entity + BasePlayerInput);
		if (!pPlayerInput) return;

		process->Write<Vector3>(pPlayerInput + 0x44, vecAng);
	}
}

int TargetClientNum = -1;
uint64_t iAimAt = 0;

void GetClosestClientByXhair(Renderer* pRender) {

	float ScreenX = s_width / 2;
	float ScreenY = s_height / 2;

	float MaxDistance = FLT_MAX;

	TargetClientNum = -1;
	iAimAt = 0;

	for (int i = 0; i < playerList.size(); i++)
	{
		DWORD64 taggedObject = playerList.at(i);
		if (!taggedObject) continue;

		DWORD64 gameObject = process->Read<DWORD64>(taggedObject + 0x10);
		if (!gameObject) continue;

		DWORD64 objectClass = process->Read<DWORD64>(gameObject + CorrespondingObject);
		if (!objectClass) {
			continue;
		}

		DWORD64 entity_ptr = process->Read<DWORD64>(objectClass + EntityRef_Value);
		if (!entity_ptr) {
			continue;
		}

		DWORD64 base_entity = process->Read<DWORD64>(entity_ptr + Base_Entitym);
		if (!base_entity) {
			continue;
		}

		DWORD64 player_model = process->Read<DWORD64>(base_entity + BasePlayer_PlayerModel);
		if (!player_model) {
			continue;
		}

		DWORD64 object_transform = process->Read<DWORD64>(objectClass + LocalEntity_Transform);
		if (!object_transform) {
			continue;
		}

		DWORD64 visual_state = process->Read<DWORD64>(object_transform + Transform_VisualState);
		if (!visual_state) {
			continue;
		}

		float fHealth = process->Read<float>(base_entity + BaseCombatEntity_Health);

		BYTE bFlag = process->Read<BYTE>(base_entity + BasePlayer_PlayerFlags);

		BYTE bSleeping = process->Read<BYTE>(base_entity + BasePlayer_isSleeping);

		bool bIsEnemy = true;

		int iTeam = process->Read<int>(base_entity + BasePlayer_TeamNumber);

		if (iTeam != 0 && global::LocalTeam != 0 && (iTeam == global::LocalTeam))
		{
			bIsEnemy = false;
		}

		if (!bIsEnemy) continue;

		BYTE bVisible = process->Read<BYTE>(player_model + PlayerModel_visible);

		bool isVisable = bVisible;

		if (bSleeping) continue;

		uint64_t u64SteamId = process->Read<uint64_t>(base_entity + BasePlayer_SteamID);

		std::string playerName = "";
		DWORD64 pName = process->Read<DWORD64>(base_entity + BasePlayer_Name);
		if (pName)
		{
			int nameLen = process->Read<int>(pName + Name_len);

			wchar_t wname[255] = { NULL };
			if (process->Read(pName + Name_char, &wname, sizeof(wname)))
			{
				char aname[255];
				sprintf_s(aname, "%ws", wname);
				playerName = aname;
			}
		}

		Vector3 vPos = process->Read <Vector3>(visual_state + VisualState_Position);
		Vector3 vHead = get_bone_pos(get_entity_bone(base_entity, head));

		float fDist = vPos.Distance(global::LocalPos);

		Vector2 vScrHead;
		if (!WorldToScreen(vHead, &vScrHead)) continue;

		float distToCrossHair = Dist2D(ScreenX, ScreenY, vScrHead.x, vScrHead.y);

		if (isVisable)
		{
			float Radius = 30.0f *(s_width / 180);

			if (distToCrossHair <= Radius)
			{
				if (distToCrossHair < MaxDistance)
				{
					MaxDistance = distToCrossHair;
					TargetClientNum = i;
					iAimAt = taggedObject;
				}
			}
		}
	}

	if (TargetClientNum != -1 && iAimAt != 0)
	{
		DWORD64 gameObject = process->Read<DWORD64>(iAimAt + 0x10);
		if (gameObject)
		{
			DWORD64 objectClass = process->Read<DWORD64>(gameObject + CorrespondingObject);
			if (objectClass)
			{
				DWORD64 entity_ptr = process->Read<DWORD64>(objectClass + EntityRef_Value);
				if (entity_ptr) {
					DWORD64 base_entity = process->Read<DWORD64>(entity_ptr + Base_Entitym);
					if (base_entity) {
						Vector3 AimPosition = get_bone_pos(get_entity_bone(base_entity, Bones::head));
						Vector3 AimCorrect = AimPosition;

						Vector2 vScreen, vAimScreen;

						if(WorldToScreen(AimPosition, &vScreen))
						{
							pRender->DrawRect(ImVec2(vScreen.x, vScreen.y), ImVec2(2.0f, 2.0f), ImColor(1.0f, 1.0f, 0.0f, 1.0f));
						}
						if (WorldToScreen(AimCorrect, &vAimScreen))
						{
							pRender->DrawRect(ImVec2(vAimScreen.x, vAimScreen.y), ImVec2(2.0f, 2.0f), ImColor(1.0f, 1.0f, 0.0f, 1.0f));
						}
						pRender->DrawLine(ImVec2(vScreen.x, vScreen.y), ImVec2(vAimScreen.x, vAimScreen.y), ImColor(1.0f, 1.0f, 0.0f, 1.0f));


						bool bVisible = true;
						if (bVisible)
						{
							if ((GetAsyncKeyState(0x2) & 0x8000))
							{
								AimAt(pRender, AimCorrect, 1);
							}
						}
					}
				}

			}
		}
	}
}

void PlayerAimbot(Renderer* pRender)
{
	GetClosestClientByXhair(pRender);
}

void PlayerESP(Renderer* pRender) {
	
	for (int i = 0; i < playerList.size(); i++)
	{
		DWORD64 taggedObject = playerList.at(i);
		if (!taggedObject) continue;

		DWORD64 gameObject = process->Read<DWORD64>(taggedObject + 0x10);
		if (!gameObject) continue;

		DWORD64 objectClass = process->Read<DWORD64>(gameObject + CorrespondingObject);
		if (!objectClass) {
			continue;
		}

		DWORD64 entity_ptr = process->Read<DWORD64>(objectClass + EntityRef_Value);
		if (!entity_ptr) {
			continue;
		}

		DWORD64 base_entity = process->Read<DWORD64>(entity_ptr + Base_Entitym);
		if (!base_entity) {
			continue;
		}

		DWORD64 player_model = process->Read<DWORD64>(base_entity + BasePlayer_PlayerModel);
		if (!player_model) {
			continue;
		}

		DWORD64 object_transform = process->Read<DWORD64>(objectClass + LocalEntity_Transform);
		if (!object_transform) {
			continue;
		}

		DWORD64 visual_state = process->Read<DWORD64>(object_transform + Transform_VisualState);
		if (!visual_state) {
			continue;
		}

		float fHealth = process->Read<float>(base_entity + BaseCombatEntity_Health);

		BYTE bFlag = process->Read<BYTE>(base_entity + BasePlayer_PlayerFlags);

		BYTE bSleeping = process->Read<BYTE>(base_entity + BasePlayer_isSleeping);

		ImColor espColor = ImColor(1.0f, 0.5f, 0.0f, 0.8f);

		bool bIsEnemy = true;

		int iTeam = process->Read<int>(base_entity + BasePlayer_TeamNumber);

		if (iTeam != 0 && global::LocalTeam != 0 && (iTeam == global::LocalTeam))
		{
			bIsEnemy = false;
		}

		if (!bIsEnemy) continue;

		BYTE bVisible = process->Read<BYTE>(player_model + PlayerModel_visible);

		bool isVisable = bVisible;

		if (!bIsEnemy)
		{
			espColor = ImColor(0.0f, 1.0f, 0.0f, 0.8f);
		}

		if (bSleeping && bIsEnemy)
		{
			espColor = ImColor(1.0f, 0.68f, 0.78f, 0.8f);
		}

		if (isVisable && bIsEnemy)
		{
			espColor = ImColor(1.0f, 0.0f, 0.0f, 0.8f);
		}

		if (isVisable && bSleeping && bIsEnemy)
		{
			espColor = ImColor(0.5f, 1.0f, 1.0f, 0.8f);	
		}

		uint64_t u64SteamId = process->Read<uint64_t>(base_entity + BasePlayer_SteamID);

		std::string playerName = "";
		DWORD64 pName = process->Read<DWORD64>(base_entity + BasePlayer_Name);
		if (pName)
		{
			int nameLen = process->Read<int>(pName + Name_len);

			wchar_t wname[255] = { NULL };
			if (process->Read(pName + Name_char, &wname, sizeof(wname)))
			{
				char aname[255];
				sprintf_s(aname, "%ws", wname);
				playerName = aname;
			}
		}

		Vector3 vPos = process->Read <Vector3>(visual_state + VisualState_Position);
		Vector3 vHead = get_bone_pos(get_entity_bone(base_entity, head));

		float fDist = vPos.Distance(global::LocalPos);

		DrawSkeleton(pRender, base_entity);

		Vector2 vScr, vScrHead;
		if (WorldToScreen(vPos, &vScr) && WorldToScreen(vHead, &vScrHead)) {
			float h = vScr.y - vScrHead.y;
			float w = h / 4.5f;
			float h2 = vPos.y - vHead.y;

			int cline = 0;

			CornerBoxOutline(pRender, vScrHead.x - w, vScrHead.y, w * 2, h, ImColor(0.0f, 0.0f, 0.0f, 0.8f), espColor);
			pRender->DrawRect(ImVec2(vScrHead.x, vScrHead.y), ImVec2(2.0f, 2.0f), ImColor(1.0f, 0.0f, 0.0f, 0.8f));

			pRender->RenderText(ImVec2(vScr.x, vScr.y + 15.0f * cline), 15.0f, espColor, false, true, "%s", playerName.c_str());
			cline++;

			pRender->RenderText(ImVec2(vScr.x, vScr.y + 15.0f * cline), 15.0f, espColor, false, true, "[%.1fm]", fDist);
			cline++;

			HealthBar_H(pRender, vScrHead.x - w, vScrHead.y, 4, h, fHealth, 100.0f);
			pRender->RenderText(ImVec2(vScr.x, vScr.y + 15.0f * cline), 15.0f, espColor, false, true, "[HP: %.1f]", fHealth);
			cline++;

			pRender->RenderText(ImVec2(vScr.x, vScr.y + 15.0f * cline), 15.0f, espColor, false, true, "Player %d: 0x%llX", i, taggedObject);
			cline++;
		}
	}
}

bool bRenderInit;
void Render(IDXGISwapChain *m_pSwapChain, Renderer* pRender)
{
	if (!bRenderInit)
	{
		game_handle = (DWORD64)GetModuleHandleA("RustClient.exe");
		unity_handle = (DWORD64)GetModuleHandleA("UnityPlayer.dll");

		if (!unity_handle) return;

		bRenderInit = true;
	}

	if (!bRenderInit) return;

	global::pGOM = process->Read<DWORD64>(unity_handle + 0x15FC218);
	if (!global::pGOM)
		return;

	DWORD64 lasttaggedObject = process->Read<DWORD64>(global::pGOM + 0x0);
	if (!lasttaggedObject)
		return;

	DWORD64 taggedObject = process->Read<DWORD64>(global::pGOM + 0x8);
	if (!taggedObject)
		return;

	DWORD64 lastactiveObject = process->Read<DWORD64>(global::pGOM + 0x10);
	if (!lastactiveObject)
		return;

	DWORD64 activeObject = process->Read<DWORD64>(global::pGOM + 0x18);
	if (!activeObject)
		return;

	int tagEntityCount = 0;
	int activeEntityCount = 0;
	playerList.clear();

	int playerCount = 0;

	for (int i = 0; taggedObject != lasttaggedObject; i++) {

		if (tagEntityCount > 50000) break;

		DWORD64 gameObject = process->Read<DWORD64>(taggedObject + 0x10);
		if (!gameObject) break;

		INT16 tag = process->Read<INT16>(gameObject + Object_Tag);
		INT16 ulayer = process->Read<INT16>(gameObject + Object_Layer);

		if (tag == 0)
		{
			break;
		}

		if (ulayer != 30)
		{
			if (tag == 5)
			{
				//camera
				DWORD64 objectClass = process->Read<DWORD64>(gameObject + CameraObject);

				if (objectClass) {
					DWORD64 cameraObject = process->Read<DWORD64>(objectClass + LocalEntity_MainCamera);

					if (cameraObject) {
						global::viewMatrix = process->Read<D3DXMATRIX>(cameraObject + CameraViewMatrix);
						global::LocalCam = process->Read<Vector3>(cameraObject + CameraPos);
					}
				}
			}
			if (tag == 20011)
			{				
			//sky
				global::pSky = taggedObject;

				DWORD64 objectClass = process->Read<DWORD64>(gameObject + CorrespondingObject);
				if (objectClass) {

					DWORD64 entity_ptr = process->Read<DWORD64>(objectClass + EntityRef_Value);
					if (entity_ptr) {

						DWORD64 base_entity = process->Read<DWORD64>(entity_ptr + Base_Entitym);
						if (base_entity) {

							DWORD64 pCycle = process->Read<DWORD64>(base_entity + TOD_SKY::TOD_CycleParameters);
							if (pCycle)
							{
								process->Write<float>(pCycle + TOD_CYCLE::Hour, 12.0f);	//set to noon
							}
						}
					}
				}
			}

			if (tag == 6)
			{
				DWORD64 namePtr = process->Read<DWORD64>(gameObject + Object_Name);
				if (namePtr)
				{
					char entityname[255] = { NULL };
					if (process->Read(namePtr, &entityname, sizeof(entityname)))
					{
						if (strcmp(entityname, "LocalPlayer") == 0)
						{
							global::pLocal = taggedObject;

							DWORD64 objectClass = process->Read<DWORD64>(gameObject + CorrespondingObject);
							if (objectClass) {
								DWORD64 object_transform = process->Read<DWORD64>(objectClass + LocalEntity_Transform);

								if (object_transform) {
									DWORD64 visual_state = process->Read<DWORD64>(object_transform + Transform_VisualState);

									if (visual_state) {
										global::LocalPos = process->Read<Vector3>(visual_state + VisualState_Position);
									}
								}


								DWORD64 entity_ptr = process->Read<DWORD64>(objectClass + EntityRef_Value);

								if (entity_ptr) {
									DWORD64 base_entity = process->Read<DWORD64>(entity_ptr + Base_Entitym);

									if (base_entity) {

										global::CurrentItemId = process->Read<int>(base_entity + ActiveItemId);

										global::LocalTeam = process->Read<int>(base_entity + BasePlayer_TeamNumber);

										DWORD64 pPlayerMovement = process->Read<DWORD64>(base_entity + BasePlayerMovement);
										if (pPlayerMovement)
										{
											if (true)	//No fall damage
											{
												//UnityEngine.Vector3 previousVelocity; //0x00B4
												process->Write<Vector3>(pPlayerMovement + 0xB4, Vector3(0.0f, 0.0f, 0.0f));
											}
											
											if (true)	//gravity 1.0 - 2.5;
											{
												//System.Single gravityMultiplier; //0x007C
												process->Write<float>(pPlayerMovement + 0x7C, 1.0);
											}
										}

										DWORD64 pPlayerInventory = process->Read<DWORD64>(base_entity + PlayerInventory);
										if (pPlayerInventory) {

											DWORD64 pItemContainer = process->Read<DWORD64>(pPlayerInventory + ItemContainer);
											if (pItemContainer) {

												DWORD64 plist_item = process->Read<DWORD64>(pItemContainer + list_item);
												if (plist_item) {

													DWORD64 plist = process->Read<DWORD64>(plist_item + 0x10);
													int itemCount = process->Read<int>(plist_item + 0x18);

													if (plist && itemCount > 0)
													{
														for (int k = 0; k < itemCount; k++)
														{
															DWORD64 a_item = process->Read<DWORD64>(plist + 0x20 + 0x8 * k);
															if (a_item)
															{
																int uid = process->Read<int>(a_item + 0x78);

																if (uid != 0 && global::CurrentItemId == uid)	//item holding
																{
																	DWORD64 pItemDefinition = process->Read<int>(a_item + 0x10);
																	if (pItemDefinition)
																	{
																		DWORD64 pcName = process->Read<DWORD64>(pItemDefinition + 0x20);

																		if (pcName)
																		{
																			DWORD64 pDisplayName = process->Read<DWORD64>(pcName + 0x18);

																			int nameLen = process->Read<int>(pDisplayName + Name_len);

																			wchar_t wname[255] = { NULL };
																			if (process->Read(pDisplayName + Name_char, &wname, sizeof(wname)))
																			{
																				char aname[255];
																				sprintf_s(aname, "%ws", wname);
																			}
																		}

																		int dwItemCategory = process->Read<int>(pItemDefinition + 0xBC);
																		if (dwItemCategory == 0)	//holding a weapon
																		{
																			DWORD64 pProjectile = process->Read<DWORD64>(a_item + 0x58);
																			if (pProjectile)
																			{
																				DWORD64 pRecoil = process->Read<DWORD64>(pProjectile + 0x248);
																				if (pRecoil)
																				{
																					if (true)	//No recoil
																					{
																						process->Write<float>(pProjectile + 0x290, 0.0f);	//aimSway
																						process->Write<float>(pProjectile + 0x294, 0.0f);	//aimSwaySpeed

																						process->Write<float>(pRecoil + 0x28, 0.0f);	//recoilYawMin
																						process->Write<float>(pRecoil + 0x2C, 0.0f);
																						process->Write<float>(pRecoil + 0x30, 0.0f);
																						process->Write<float>(pRecoil + 0x34, 0.0f);
																						process->Write<float>(pRecoil + 0x3C, 0.0f);
																					}
																				}
																			}
																		}
																	}
																}

															}
														}

													}

												}
											}
										}
									}
								}
							}
						}
					}
				}

				if (global::pLocal != taggedObject)
				{
					DWORD64 objectClass = process->Read<DWORD64>(gameObject + CorrespondingObject);
					if (objectClass) {
						DWORD64 entity_ptr = process->Read<DWORD64>(objectClass + EntityRef_Value);

						if (entity_ptr) {
							DWORD64 base_entity = process->Read<DWORD64>(entity_ptr + Base_Entitym);

							if (base_entity) {
								DWORD64 object_transform = process->Read<DWORD64>(objectClass + LocalEntity_Transform);

								if (object_transform) {
									DWORD64 visual_state = process->Read<DWORD64>(object_transform + Transform_VisualState);

									if (visual_state) {

										playerList.push_back(taggedObject);
										playerCount++;
									}
								}
							}
						}
					}
				}

			}
		}

		lasttaggedObject = process->Read<DWORD64>(global::pGOM + 0x0);
		taggedObject = process->Read<DWORD64>(taggedObject + 0x8);
		tagEntityCount++;
	}

	PlayerAimbot(pRender);
	PlayerESP(pRender);

	tagEntityCount = 0;
	activeEntityCount = 0;
	playerList.clear();

	playerCount = 0;
}

static float firstTickCount = 0.0f;
float currentTickCount = 0.0f;

bool bFirstRun = false;
void CRender::DrawPresent(IDXGISwapChain *m_pSwapChain, Renderer* pRender)
{
	ImVec2 ScrSize = pRender->GetSize();
	s_width = ScrSize.x;
	s_height = ScrSize.y;

	Render(m_pSwapChain, pRender);
}

void CRender::DrawPMenu(IDXGISwapChain *m_pSwapChain, Renderer* pRender)
{

}
