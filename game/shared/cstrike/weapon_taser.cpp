//====== Copyright � 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "weapon_csbasegun.h"
#include "cs_gamerules.h"

#if defined( CLIENT_DLL )
	#define CWeaponTaser C_WeaponTaser
	#include "c_cs_player.h"
#else
	#include "cs_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#define TASER_BIRTHDAY_PARTICLES	"weapon_confetti"
#define TASER_BIRTHDAY_SOUND		"Weapon_PartyHorn.Single"

class CWeaponTaser : public CWeaponCSBaseGun
{
public:
	DECLARE_CLASS( CWeaponTaser, CWeaponCSBaseGun );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CWeaponTaser();

	virtual void Precache();
	virtual void PrimaryAttack( void );

#if defined( GAME_DLL )
	virtual bool Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void ItemPostFrame();
#endif

#ifdef CLIENT_DLL
	const char* GetMuzzleFlashEffectName( bool bThirdPerson );
#endif

	virtual CSWeaponID GetCSWeaponID( void ) const		{ return WEAPON_TASER; }

private:
	CWeaponTaser( const CWeaponTaser& );
	float	m_fFireTime;
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponTaser, DT_WeaponTaser )

BEGIN_NETWORK_TABLE( CWeaponTaser, DT_WeaponTaser )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponTaser )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_taser, CWeaponTaser );
PRECACHE_WEAPON_REGISTER( weapon_taser );

CWeaponTaser::CWeaponTaser() :
	m_fFireTime(0.0f)
{
}

void CWeaponTaser::Precache()
{
	BaseClass::Precache();

	PrecacheParticleSystem( TASER_BIRTHDAY_PARTICLES );
	PrecacheScriptSound( TASER_BIRTHDAY_SOUND );
}

void CWeaponTaser::PrimaryAttack( void )
{
	if ( !CSBaseGunFire( GetCSWpnData().m_flCycleTime[m_weaponMode], Primary_Mode ) )
		return;

	m_fFireTime = gpGlobals->curtime;

	if ( UTIL_IsCSSOBirthday() )
	{
		//CPASAttenuationFilter filter( this, params.soundlevel );
		//EmitSound( filter, entindex(), TASER_BIRTHDAY_SOUND, &GetLocalOrigin(), 0.0f );

		CPASAttenuationFilter filter( this );
		filter.UsePredictionRules();
		EmitSound( filter, entindex(), TASER_BIRTHDAY_SOUND );
	}
}

#if defined( GAME_DLL )

bool CWeaponTaser::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	if ( HasAmmo() == false )
	{
		// just drop it if it's out of ammo and we're trying to switch away
		GetPlayerOwner()->CSWeaponDrop( this );
	}

	return BaseClass::Holster(pSwitchingTo);
}

void CWeaponTaser::ItemPostFrame()
{
	const float kTaserDropDelay = 0.5f;
	BaseClass::ItemPostFrame();

	if ( HasAmmo() == false && gpGlobals->curtime >= m_fFireTime + kTaserDropDelay )
	{
		GetPlayerOwner()->CSWeaponDrop( this );
	}
}

#endif

#ifdef CLIENT_DLL
const char* CWeaponTaser::GetMuzzleFlashEffectName( bool bThirdPerson )
{
	if ( UTIL_IsCSSOBirthday() )
		return TASER_BIRTHDAY_PARTICLES;

	return BaseClass::GetMuzzleFlashEffectName( bThirdPerson );
}
#endif