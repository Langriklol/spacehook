#pragma once

#include "vector.h"
#include "../Utils/patternfinder.h"
#include "../Utils/xorstring.h"
#include "CCSGOAnimState.h"
#include "SDK.h"

#define MAX_SHOOT_SOUNDS 16
#define MAX_WEAPON_STRING 80
#define MAX_WEAPON_PREFIX 16
#define MAX_WEAPON_AMMO_NAME 32

enum WeaponSound_t
{
	EMPTY,
	SINGLE,
	SINGLE_NPC,
	WPN_DOUBLE, // Can't be "DOUBLE" because windows.h uses it.
	DOUBLE_NPC,
	BURST,
	RELOAD,
	RELOAD_NPC,
	MELEE_MISS,
	MELEE_HIT,
	MELEE_HIT_WORLD,
	SPECIAL1,
	SPECIAL2,
	SPECIAL3,
	TAUNT,
	FAST_RELOAD,

	// Add new shoot sound types here
	REVERSE_THE_NEW_SOUND,

	NUM_SHOOT_SOUND_TYPES,
};

enum MoveType_t
{
	MOVETYPE_NONE = 0,
	MOVETYPE_ISOMETRIC,
	MOVETYPE_WALK,
	MOVETYPE_STEP,
	MOVETYPE_FLY,
	MOVETYPE_FLYGRAVITY,
	MOVETYPE_VPHYSICS,
	MOVETYPE_PUSH,
	MOVETYPE_NOCLIP,
	MOVETYPE_LADDER,
	MOVETYPE_OBSERVER,
	MOVETYPE_CUSTOM,
	MOVETYPE_LAST = MOVETYPE_CUSTOM,
	MOVETYPE_MAX_BITS = 4
};

enum DataUpdateType_t
{
	DATA_UPDATE_CREATED = 0,
	DATA_UPDATE_DATATABLE_CHANGED,
};

class ICollideable
{
public:
	virtual void pad0();
	virtual const Vector& OBBMins() const;
	virtual const Vector& OBBMaxs() const;
};

class IHandleEntity
{
public:
	virtual ~IHandleEntity() {};
};

class IClientUnknown : public IHandleEntity {};
class IClientRenderable
{
public:
	virtual ~IClientRenderable() {};

	model_t* GetModel()
	{
		typedef model_t* (* oGetModel)(void*);
		return getvfunc<oGetModel>(this, 8)(this);
	}

	bool SetupBones(matrix3x4_t* pBoneMatrix, int nMaxBones, int nBoneMask, float flCurTime = 0)
	{
		typedef bool (* oSetupBones)(void*, matrix3x4_t*, int, int, float);
		return getvfunc<oSetupBones>(this, 13)(this, pBoneMatrix, nMaxBones, nBoneMask, flCurTime);
	}
};

class IClientNetworkable
{
public:
	virtual ~IClientNetworkable() {};

	void Release()
	{
		typedef void (* oRelease)(void*);
		return getvfunc<oRelease>(this, 1)(this);
	}

	ClientClass* GetClientClass()
	{
		typedef ClientClass* (* oGetClientClass)(void*);
		return getvfunc<oGetClientClass>(this, 2)(this);
	}

	void PreDataUpdate(DataUpdateType_t updateType)
	{
		typedef void (* oPreDataUpdate)(void*, DataUpdateType_t);
		return getvfunc<oPreDataUpdate>(this, 6)(this, updateType);
	}

	bool GetDormant()
	{
		typedef bool (* oGetDormant)(void*);
		return getvfunc<oGetDormant>(this, 9)(this);
	}

	int GetIndex()
	{
		typedef int (* oGetIndex)(void*);
		return getvfunc<oGetIndex>(this, 10)(this);
	}

	void SetDestroyedOnRecreateEntities()
	{
		typedef void (* oSetDestroyedOnRecreateEntities)(void*);
		return getvfunc<oSetDestroyedOnRecreateEntities>(this, 13)(this);
	}
};

class IClientThinkable
{
public:
	virtual ~IClientThinkable() {};
};

class IClientEntity : public IClientUnknown, public IClientRenderable, public IClientNetworkable, public IClientThinkable
{
public:
	virtual ~IClientEntity() {};
};

class C_BaseEntity : public IClientEntity
{
public:
	IClientNetworkable* GetNetworkable()
	{
		return (IClientNetworkable*)((uintptr_t)this + 0x10);
	}

	void SetModelIndex(int index)
	{
		typedef void (* oSetModelIndex)(void*, int);
		return getvfunc<oSetModelIndex>(this, 111)(this, index);
	}

	int* GetModelIndex()
	{
		return (int*)((uintptr_t)this + offsets.DT_BaseViewModel.m_nModelIndex);
	}

	float GetAnimTime()
	{
		return *(float*)((uintptr_t)this + offsets.DT_BaseEntity.m_flAnimTime);
	}

	float GetSimulationTime()
	{
		return *(float*)((uintptr_t)this + offsets.DT_BaseEntity.m_flSimulationTime);
	}

	TeamID GetTeam()
	{
		return *(TeamID*)((uintptr_t)this + offsets.DT_BaseEntity.m_iTeamNum);
	}

	Vector GetVecOrigin()
	{
		return *(Vector*)((uintptr_t)this + offsets.DT_BaseEntity.m_vecOrigin);
	}

	MoveType_t GetMoveType()
	{
		return *(MoveType_t*)((uintptr_t)this + offsets.DT_BaseEntity.m_MoveType);
	}

	ICollideable* GetCollideable()
	{
		return (ICollideable*)((uintptr_t)this + offsets.DT_BaseEntity.m_Collision);
	}

	bool* GetSpotted()
	{
		return (bool*)((uintptr_t)this + offsets.DT_BaseEntity.m_bSpotted);
	}

	void UpdateVisibility()
	{
		typedef void (*UpdateVisibilityFn) (void*);
		static UpdateVisibilityFn UpdateVisibility;

		if (!UpdateVisibility)
		{
			uintptr_t func_address = PatternFinder::FindPatternInModule((XORSTR("client_panorama_client.so")), // EntityUpdateVisibility -- xref: "c_baseentity.cpp"
																		(unsigned char*) XORSTR("\x55\x48\x89\xE5\x41\x54\x53\x48\x89\xFB\x48\x83\xEC\x30\x48\x81\xFF\x00\x00\x00\x00"),
																		XORSTR("xxxxxxxxxxxxxxxxx????"));

			UpdateVisibility = reinterpret_cast<UpdateVisibilityFn>(func_address);
		}

		UpdateVisibility(this);
	}

    void SetAbsOrigin(Vector origin) // up-to-date 25/01/2019
    {
        typedef void (*SetAbsOriginFn) (void*, const Vector&);
        static SetAbsOriginFn SetAbsOrigin;

        if (!SetAbsOrigin)
        {
            uintptr_t func_address = PatternFinder::FindPatternInModule((XORSTR("client_panorama_client.so")), // SetAbsOrigin xref: "ClientProjectile: No model %d!" - the 3rd sub after the 6 dwords
                                                                        (unsigned char*) XORSTR("\x55\x48\x89\xE5\x41\x55\x41\x54\x49\x89\xF4\x53\x48\x89\xFB\x48\x83\xEC\x08\xE8\x00\x00\x00\x00\xF3"),
                                                                        XORSTR("xxxxxxxxxxxxxxxxxxxx????x"));

            SetAbsOrigin = reinterpret_cast<SetAbsOriginFn>(func_address);
        }
        SetAbsOrigin(this, origin);
    }

    void SetAbsAngles(QAngle angles) // up-to-date 25/01/2019
    {
        typedef void (*SetAbsAnglesFn) (void*, const QAngle&);
        static SetAbsAnglesFn SetAbsAngles;

        if (!SetAbsAngles)
        {
            uintptr_t func_address = PatternFinder::FindPatternInModule((XORSTR("client_panorama_client.so")), // SetAbsAngles xref: "ClientProjectile: No model %d!" - the 2nd sub after the 6 dwords
                                                                        (unsigned char*) XORSTR("\x55\x48\x89\xE5\x41\x57\x41\x56\x41\x55\x41\x54\x49\x89\xF4\x53\x48\x89\xFB\x48\x83\xEC\x68\xE8\x00\x00\x00\x00\xF3"),
                                                                        XORSTR("xxxxxxxxxxxxxxxxxxxxxxxx????x"));

            SetAbsAngles = reinterpret_cast<SetAbsAnglesFn>(func_address);
        }

        SetAbsAngles(this, angles);
    }

	const Vector& GetAbsOrigin()
	{
		typedef Vector&(* oGetAbsOrigin)(void*);
		return getvfunc<oGetAbsOrigin>(this, 12)(this);
	}

	const QAngle& GetAbsAngles()
	{
		typedef QAngle&(* oGetAbsAngles)(void*);
		return getvfunc<oGetAbsAngles>(this, 13)(this);
	}

	void InvalidateBoneCache() // up-to-date 25/01/2019 -- get offsets from "80 3D ? ? ? ? ? 55 48 89 E5 74 1C"
	{
		static uintptr_t boneCounterAddr = 0;

		if (!boneCounterAddr)
		{
			boneCounterAddr = PatternFinder::FindPatternInModule((XORSTR("client_panorama_client.so")),
																 (unsigned char*) XORSTR("\x80\x3D\x00\x00\x00\x00\x00\x55\x48\x89\xE5\x74\x1C"), // the bool check in InvalidateBoneCache
																 XORSTR("xx?????xxxxxx"));

			boneCounterAddr = GetAbsoluteAddress(boneCounterAddr + 14, 2, 7);
		}

		unsigned long g_iModelBoneCounter = *(unsigned long*)(boneCounterAddr);

		*(unsigned long*)((uintptr_t)this + 0x2C58) = g_iModelBoneCounter - 1;
		*(unsigned long*)((uintptr_t)this + 0x2F90) = 0xFF7FFFFF;
	}
};

/* generic game classes */
class C_BasePlayer : public C_BaseEntity
{
public:
	QAngle* GetViewPunchAngle()
	{
		return (QAngle*)((uintptr_t)this + offsets.DT_BasePlayer.m_viewPunchAngle);
	}

	QAngle* GetAimPunchAngle()
	{
		return (QAngle*)((uintptr_t)this + offsets.DT_BasePlayer.m_aimPunchAngle);
	}

	Vector GetVecViewOffset()
	{
		return *(Vector*)((uintptr_t)this + offsets.DT_BasePlayer.m_vecViewOffset);
	}

	unsigned int GetTickBase()
	{
		return *(unsigned int*)((uintptr_t)this + offsets.DT_BasePlayer.m_nTickBase);
	}

	Vector GetVelocity()
	{
		return *(Vector*)((uintptr_t)this + offsets.DT_BasePlayer.m_vecVelocity);
	}

	int GetHealth()
	{
		return *(int*)((uintptr_t)this + offsets.DT_BasePlayer.m_iHealth);
	}

	unsigned char GetLifeState()
	{
		return *(unsigned char*)((uintptr_t)this + offsets.DT_BasePlayer.m_lifeState);
	}

	int GetFlags()
	{
		return *(int*)((uintptr_t)this + offsets.DT_BasePlayer.m_fFlags);
	}

	ObserverMode_t* GetObserverMode()
	{
		return (ObserverMode_t*)((uintptr_t)this + offsets.DT_BasePlayer.m_iObserverMode);
	}

	void* GetObserverTarget()
	{
		return (void*)((uintptr_t)this + offsets.DT_BasePlayer.m_hObserverTarget);
	}

	void* GetViewModel()
	{
		return (void*)((uintptr_t)this + offsets.DT_BasePlayer.m_hViewModel);
	}

	const char* GetLastPlaceName()
	{
		return (const char*)((uintptr_t)this + offsets.DT_BasePlayer.m_szLastPlaceName);
	}

	int GetShotsFired()
	{
		return *(int*)((uintptr_t)this + offsets.DT_CSPlayer.m_iShotsFired);
	}

	QAngle* GetEyeAngles()
	{
		return (QAngle*)((uintptr_t)this + offsets.DT_CSPlayer.m_angEyeAngles[0]);
	}

	int GetMoney()
	{
		return *(int*)((uintptr_t)this + offsets.DT_CSPlayer.m_iAccount);
	}

	int GetHits()
	{
		return *(int*)((uintptr_t)this + offsets.DT_CSPlayer.m_totalHitsOnServer);
	}

	int GetArmor()
	{
		return *(int*)((uintptr_t)this + offsets.DT_CSPlayer.m_ArmorValue);
	}

	int HasDefuser()
	{
		return *(int*)((uintptr_t)this + offsets.DT_CSPlayer.m_bHasDefuser);
	}

	bool IsDefusing()
	{
		return *(bool*)((uintptr_t)this + offsets.DT_CSPlayer.m_bIsDefusing);
	}

	bool IsGrabbingHostage()
	{
		return *(bool*)((uintptr_t)this + offsets.DT_CSPlayer.m_bIsGrabbingHostage);
	}

	bool IsScoped()
	{
		return *(bool*)((uintptr_t)this + offsets.DT_CSPlayer.m_bIsScoped);
	}

	bool GetImmune()
	{
		return *(bool*)((uintptr_t)this + offsets.DT_CSPlayer.m_bGunGameImmunity);
	}

	bool IsRescuing()
	{
		return *(bool*)((uintptr_t)this + offsets.DT_CSPlayer.m_bIsRescuing);
	}

	int HasHelmet()
	{
		return *(int*)((uintptr_t)this + offsets.DT_CSPlayer.m_bHasHelmet);
	}

	float GetFlashBangTime()
	{
		return *(float*)((uintptr_t)this + 0xABF4);
	}

	float GetFlashDuration()
	{
		return *(float*)((uintptr_t)this + offsets.DT_CSPlayer.m_flFlashDuration);
	}

	float* GetFlashMaxAlpha()
	{
		return (float*)((uintptr_t)this + offsets.DT_CSPlayer.m_flFlashMaxAlpha);
	}

	float* GetLowerBodyYawTarget()
	{
		return (float*)((uintptr_t)this + offsets.DT_CSPlayer.m_flLowerBodyYawTarget);
	}

	void* GetActiveWeapon()
	{
		return (void*)((uintptr_t)this + offsets.DT_BaseCombatCharacter.m_hActiveWeapon);
	}

	int* GetWeapons()
	{
		return (int*)((uintptr_t)this + offsets.DT_BaseCombatCharacter.m_hMyWeapons);
	}

	int* GetWearables()
	{
		return (int*)((uintptr_t)this + offsets.DT_BaseCombatCharacter.m_hMyWearables);
	}

	bool GetAlive()
	{
		return this->GetHealth() > 0 && this->GetLifeState() == LIFE_ALIVE;
	}

	Vector GetEyePosition()
	{
		return this->GetVecOrigin() + this->GetVecViewOffset();
	}

	inline matrix3x4_t* GetBoneMatrix()
	{
		matrix3x4_t BoneMatrix[MAXSTUDIOBONES];

		if (!this->SetupBones(BoneMatrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0))
			return nullptr;

		return BoneMatrix;
	}

	inline Vector GetBonePosition(int boneIndex)
	{
		matrix3x4_t BoneMatrix[MAXSTUDIOBONES];

		if (!this->SetupBones(BoneMatrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0))
			return this->GetVecOrigin();

		matrix3x4_t hitbox = BoneMatrix[boneIndex];

		return Vector(hitbox[0][3], hitbox[1][3], hitbox[2][3]);
	}

	QAngle* GetVAngles()
	{
		return (QAngle*)((uintptr_t)this + offsets.DT_BasePlayer.deadflag + 0x4);
	}

    CCSGOAnimState* GetAnimState()
    {
        return *reinterpret_cast<CCSGOAnimState**>((uintptr_t)this + Offsets::playerAnimStateOffset);
    }
};

class C_PlantedC4 : public C_BaseEntity
{
public:
	bool IsBombTicking()
	{
		return (bool)((uintptr_t)this + offsets.DT_PlantedC4.m_bBombTicking);
	}

	float GetBombTime()
	{
		return *(float*)((uintptr_t)this + offsets.DT_PlantedC4.m_flC4Blow);
	}

	bool IsBombDefused()
	{
		return *(bool*)((uintptr_t)this + offsets.DT_PlantedC4.m_bBombDefused);
	}

	int GetBombDefuser()
	{
		return *(int*)((uintptr_t)this + offsets.DT_PlantedC4.m_hBombDefuser);
	}
};

class C_BaseAttributableItem : public C_BaseEntity
{
public:
	ItemDefinitionIndex* GetItemDefinitionIndex()
	{
		return (ItemDefinitionIndex*)((uintptr_t)this + offsets.DT_BaseAttributableItem.m_iItemDefinitionIndex);
	}

	int* GetItemIDHigh()
	{
		return (int*)((uintptr_t)this + offsets.DT_BaseAttributableItem.m_iItemIDHigh);
	}

	int* GetEntityQuality()
	{
		return (int*)((uintptr_t)this + offsets.DT_BaseAttributableItem.m_iEntityQuality);
	}

	char* GetCustomName()
	{
		return (char*)((uintptr_t)this + offsets.DT_BaseAttributableItem.m_szCustomName);
	}

	int* GetFallbackPaintKit()
	{
		return (int*)((uintptr_t)this + offsets.DT_BaseAttributableItem.m_nFallbackPaintKit);
	}

	int* GetFallbackSeed()
	{
		return (int*)((uintptr_t)this + offsets.DT_BaseAttributableItem.m_nFallbackSeed);
	}

	float* GetFallbackWear()
	{
		return (float*)((uintptr_t)this + offsets.DT_BaseAttributableItem.m_flFallbackWear);
	}

	int* GetFallbackStatTrak()
	{
		return (int*)((uintptr_t)this + offsets.DT_BaseAttributableItem.m_nFallbackStatTrak);
	}

	int* GetAccountID()
	{
		return (int*)((uintptr_t)this + offsets.DT_BaseAttributableItem.m_iAccountID);
	}
};

class C_BaseViewModel: public C_BaseEntity
{
public:
	int GetWeapon()
	{
		return *(int*)((uintptr_t)this + offsets.DT_BaseViewModel.m_hWeapon);
	}

	int GetOwner()
	{
		return *(int*)((uintptr_t)this + offsets.DT_BaseViewModel.m_hOwner);
	}
};

class CHudTexture;

class FileWeaponInfo_t
{
public:
	FileWeaponInfo_t();

	// Each game can override this to get whatever values it wants from the script.
	virtual void Parse(KeyValues *pKeyValuesData, const char *szWeaponName);

	bool bParsedScript;
	bool bLoadedHudElements;

	char szClassName[MAX_WEAPON_STRING];
	char szPrintName[MAX_WEAPON_STRING];

	char szViewModel[MAX_WEAPON_STRING];
	char szWorldModel[MAX_WEAPON_STRING];
	char szAmmo1[MAX_WEAPON_AMMO_NAME];
	char szWorldDroppedModel[MAX_WEAPON_STRING];
	char szAnimationPrefix[MAX_WEAPON_PREFIX];
	int iSlot;
	int iPosition;
	int iMaxClip1;
	int iMaxClip2;
	int iDefaultClip1;
	int iDefaultClip2;
	int iWeight;
	int iRumbleEffect;
	bool bAutoSwitchTo;
	bool bAutoSwitchFrom;
	int iFlags;
	char szAmmo2[MAX_WEAPON_AMMO_NAME];
	char szAIAddOn[MAX_WEAPON_STRING];

	// Sound blocks
	char aShootSounds[NUM_SHOOT_SOUND_TYPES][MAX_WEAPON_STRING];

	int iAmmoType;
	int iAmmo2Type;
	bool m_bMeleeWeapon;

	// This tells if the weapon was built right-handed (defaults to true).
	// This helps cl_righthand make the decision about whether to flip the model or not.
	bool m_bBuiltRightHanded;
	bool m_bAllowFlipping;

	// Sprite data, read from the data file
	int iSpriteCount;
	CHudTexture* iconActive;
	CHudTexture* iconInactive;
	CHudTexture* iconAmmo;
	CHudTexture* iconAmmo2;
	CHudTexture* iconCrosshair;
	CHudTexture* iconAutoaim;
	CHudTexture* iconZoomedCrosshair;
	CHudTexture* iconZoomedAutoaim;
	CHudTexture* iconSmall;
};

class CCSWeaponInfo : public FileWeaponInfo_t {
public:
	char* GetConsoleName() {
		return *( char** ) ( ( uintptr_t )this + 0x8);
	}

	int GetClipSize() {
		return *( int* ) ( ( uintptr_t )this + 0x20);
	}

	CSWeaponType GetWeaponType() {
		return *( CSWeaponType* ) ( ( uintptr_t )this + 0x140);
	}

	int GetDamage() {
		return *( int* ) ( ( uintptr_t )this + 0x16C);
	}

	float GetWeaponArmorRatio() {
		return *( float* ) ( ( uintptr_t )this + 0x170);
	}

	float GetPenetration() {
		return *( float* ) ( ( uintptr_t )this + 0x178);
	}

	float GetRange() {
		return *( float* ) ( ( uintptr_t )this + 0x184);
	}

	float GetRangeModifier() {
		return *( float* ) ( ( uintptr_t )this + 0x188);
	}

	float GetMaxPlayerSpeed() {
		return *( float* ) ( ( uintptr_t )this + 0x1B0);
	}

	int GetZoomLevels() { // Doesn't work correctly on some weapons.
		return *( int* ) ( ( uintptr_t )this + 0x23C); // DT_WeaponCSBaseGun.m_zoomLevel ?
	}

	char* GetTracerEffect() {
		return *( char** ) ( ( uintptr_t )this + 0x280);
	}

	int* GetTracerFrequency() {
		return ( int* ) ( ( uintptr_t )this + 0x288);
	}
};

class C_BaseCombatWeapon: public C_BaseAttributableItem
{
public:
	int GetOwner()
	{
		return *(int*)((uintptr_t)this + offsets.DT_BaseCombatWeapon.m_hOwner);
	}

	int GetAmmo()
	{
		return *(int*)((uintptr_t)this + offsets.DT_BaseCombatWeapon.m_iClip1);
	}

	float GetNextPrimaryAttack()
	{
		return *(float*)((uintptr_t)this + offsets.DT_BaseCombatWeapon.m_flNextPrimaryAttack);
	}

	float GetPostponeFireReadyTime()
	{
		return *(float*)((uintptr_t)this + offsets.DT_WeaponCSBase.m_flPostponeFireReadyTime);
	}

	bool GetInReload()
	{
		return *(bool*)((uintptr_t)this + offsets.DT_BaseCombatWeapon.m_bInReload);
	}

	float GetAccuracyPenalty()
	{
		return *(float*)((uintptr_t)this + offsets.DT_WeaponCSBase.m_fAccuracyPenalty);
	}

	bool GetReloadVisuallyComplete()
	{
		return *(bool*)((uintptr_t)this + offsets.DT_WeaponCSBase.m_bReloadVisuallyComplete);
	}

		CCSWeaponInfo* GetCSWpnData() { // "script file not found"
		typedef CCSWeaponInfo* (* oGetCSWpnData)( void* );
		return getvfunc<oGetCSWpnData>( this, 522 )( this ); // 516
	}

	float GetSpread() {
		typedef float (* oGetSpread)( void* );
		return getvfunc<oGetSpread>( this, 514 )( this );
	}

	float GetInaccuracy() {
		typedef float (* oGetInaccuracy)( void* );
		return getvfunc<oGetInaccuracy>( this, 544 )( this );
	}

	void UpdateAccuracyPenalty() {
		typedef void (* oUpdateAccuracyPenalty)( void* );
		return getvfunc<oUpdateAccuracyPenalty>( this, 545 )( this );
	}
};

class C_WeaponC4 : C_BaseCombatWeapon
{
public:
	bool GetStartedArming()
	{
		return *(bool*)((uintptr_t)this + offsets.DT_WeaponC4.m_bStartedArming);
	}
};

class C_Chicken : C_BaseEntity
{
public:
	bool* GetShouldGlow()
	{
		return (bool*)((uintptr_t)this + offsets.DT_DynamicProp.m_bShouldGlow);
	}
};

class C_BaseCSGrenade : C_BaseCombatWeapon
{
public:
	bool IsHeldByPlayer()
	{
		return *(bool*)((uintptr_t)this + offsets.DT_BaseCSGrenade.m_bIsHeldByPlayer);
	}

	bool GetPinPulled()
	{
		return *(bool*)((uintptr_t)this + offsets.DT_BaseCSGrenade.m_bPinPulled);
	}

	float GetThrowTime()
	{
		return *(float*)((uintptr_t)this + offsets.DT_BaseCSGrenade.m_fThrowTime);
	}

	float GetThrowStrength()
	{
		return *(float*)((uintptr_t)this + offsets.DT_BaseCSGrenade.m_flThrowStrength);
	}
};
