//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include <KeyValues.h>
#include "cs_weapon_parse.h"
#include "cs_shareddefs.h"
#include "weapon_csbase.h"
#include "icvar.h"
#include "cs_gamerules.h"
#include "cs_blackmarket.h"


//--------------------------------------------------------------------------------------------------------
struct WeaponTypeInfo
{
	CSWeaponType type;
	const char * name;
};


//--------------------------------------------------------------------------------------------------------
WeaponTypeInfo s_weaponTypeInfo[] =
{
	{ WEAPONTYPE_KNIFE,			"Knife" },
	{ WEAPONTYPE_PISTOL,		"Pistol" },
	{ WEAPONTYPE_SUBMACHINEGUN,	"Submachine Gun" },	// First match is printable
	{ WEAPONTYPE_SUBMACHINEGUN,	"submachinegun" },
	{ WEAPONTYPE_SUBMACHINEGUN,	"smg" },
	{ WEAPONTYPE_RIFLE,			"Rifle" },
	{ WEAPONTYPE_SHOTGUN,		"Shotgun" },
	{ WEAPONTYPE_SNIPER_RIFLE,	"Sniper Rifle" },	// First match is printable
	{ WEAPONTYPE_SNIPER_RIFLE,	"SniperRifle" },
	{ WEAPONTYPE_MACHINEGUN,	"Machine Gun" },		// First match is printable
	{ WEAPONTYPE_MACHINEGUN,	"machinegun" },
	{ WEAPONTYPE_MACHINEGUN,	"mg" },
	{ WEAPONTYPE_C4,			"C4" },
	{ WEAPONTYPE_GRENADE,		"Grenade" },
	{ WEAPONTYPE_EQUIPMENT,		"Equipment" },
	{ WEAPONTYPE_STACKABLEITEM,	"StackableItem" },
};


struct WeaponNameInfo
{
	CSWeaponID id;
	const char *name;
};

WeaponNameInfo s_weaponNameInfo[] =
{
	{ WEAPON_P250,				"weapon_p250" },
	{ WEAPON_GLOCK,				"weapon_glock" },
	{ WEAPON_SSG08,				"weapon_ssg08" },
	{ WEAPON_HEGRENADE,			"weapon_hegrenade" },
	{ WEAPON_XM1014,			"weapon_xm1014" },
	{ WEAPON_C4,				"weapon_c4" },
	{ WEAPON_MAC10,				"weapon_mac10" },
	{ WEAPON_AUG,				"weapon_aug" },
	{ WEAPON_SMOKEGRENADE,		"weapon_smokegrenade" },
	{ WEAPON_ELITE,				"weapon_elite" },
	{ WEAPON_FIVESEVEN,			"weapon_fiveseven" },
	{ WEAPON_UMP45,				"weapon_ump45" },
	{ WEAPON_SCAR20,			"weapon_scar20" },

	{ WEAPON_GALILAR,			"weapon_galilar" },
	{ WEAPON_FAMAS,				"weapon_famas" },
	{ WEAPON_USP,				"weapon_usp_silencer" },
	{ WEAPON_AWP,				"weapon_awp" },
	{ WEAPON_MP5SD,				"weapon_mp5sd" },
	{ WEAPON_M249,				"weapon_m249" },
	{ WEAPON_NOVA,				"weapon_nova" },
	{ WEAPON_M4A1,				"weapon_m4a1_silencer" },
	{ WEAPON_MP9,				"weapon_mp9" },
	{ WEAPON_G3SG1,				"weapon_g3sg1" },
	{ WEAPON_FLASHBANG,			"weapon_flashbang" },
	{ WEAPON_DEAGLE,			"weapon_deagle" },
	{ WEAPON_SG556,				"weapon_sg556" },
	{ WEAPON_AK47,				"weapon_ak47" },
	{ WEAPON_KNIFE,				"weapon_knife" },
	{ WEAPON_KNIFE_T,			"weapon_knife_t" },
	{ WEAPON_KNIFE_CSS,			"weapon_knife_css" },
	{ WEAPON_KNIFE_KARAMBIT,	"weapon_knife_karambit" },
	{ WEAPON_KNIFE_FLIP,		"weapon_knife_flip" },
	{ WEAPON_KNIFE_BAYONET,		"weapon_knife_bayonet" },
	{ WEAPON_KNIFE_M9_BAYONET,	"weapon_knife_m9_bayonet" },
	{ WEAPON_KNIFE_BUTTERFLY,	"weapon_knife_butterfly" },
	{ WEAPON_KNIFE_GUT,			"weapon_knife_gut" },
	{ WEAPON_KNIFE_TACTICAL,	"weapon_knife_tactical" },
	{ WEAPON_KNIFE_FALCHION,	"weapon_knife_falchion" },
	{ WEAPON_KNIFE_SURVIVAL_BOWIE,"weapon_knife_survival_bowie" },
	{ WEAPON_KNIFE_CANIS,		"weapon_knife_canis" },
	{ WEAPON_KNIFE_CORD,		"weapon_knife_cord" },
	{ WEAPON_KNIFE_GYPSY,		"weapon_knife_gypsy_jackknife" },
	{ WEAPON_KNIFE_OUTDOOR,		"weapon_knife_outdoor" },
	{ WEAPON_KNIFE_SKELETON,	"weapon_knife_skeleton" },
	{ WEAPON_KNIFE_STILETTO,	"weapon_knife_stiletto" },
	{ WEAPON_KNIFE_URSUS,		"weapon_knife_ursus" },
	{ WEAPON_KNIFE_WIDOWMAKER,	"weapon_knife_widowmaker" },
	{ WEAPON_KNIFE_PUSH,		"weapon_knife_push" },
	{ WEAPON_P90,				"weapon_p90" },

	// new weapons
	{ WEAPON_HKP2000,			"weapon_hkp2000" },
	{ WEAPON_TEC9,				"weapon_tec9" },
	{ WEAPON_M4A4,				"weapon_m4a4" },
	{ WEAPON_REVOLVER,			"weapon_revolver" },
	{ WEAPON_CZ75,				"weapon_cz75" },
	{ WEAPON_MAG7,				"weapon_mag7" },
	{ WEAPON_SAWEDOFF,			"weapon_sawedoff" },
	{ WEAPON_NEGEV,				"weapon_negev" },
	{ WEAPON_MP7,				"weapon_mp7" },
	{ WEAPON_BIZON,				"weapon_bizon" },
	{ WEAPON_DECOY,				"weapon_decoy" },
	{ WEAPON_MOLOTOV,			"weapon_molotov" },
	{ WEAPON_INCGRENADE,		"weapon_incgrenade" },
	{ WEAPON_TASER,				"weapon_taser" },

	{ WEAPON_HEALTHSHOT,		"weapon_healthshot" },

	// not sure any of these are needed
	{ WEAPON_SHIELDGUN,			"weapon_shieldgun" },
	{ WEAPON_KEVLAR,			"weapon_kevlar" },
	{ WEAPON_ASSAULTSUIT,		"weapon_assaultsuit" },
	{ WEAPON_NVG,				"weapon_nvg" },

	{ WEAPON_NONE,				"weapon_none" },
};



//--------------------------------------------------------------------------------------------------------------


CCSWeaponInfo g_EquipmentInfo[MAX_EQUIPMENT];

void PrepareEquipmentInfo( void )
{
	memset( g_EquipmentInfo, 0, ARRAYSIZE( g_EquipmentInfo ) );

	g_EquipmentInfo[2].SetWeaponPrice( CSGameRules()->GetBlackMarketPriceForWeapon( WEAPON_KEVLAR ) );
	g_EquipmentInfo[2].SetDefaultPrice( KEVLAR_PRICE );
	g_EquipmentInfo[2].SetPreviousPrice( CSGameRules()->GetBlackMarketPreviousPriceForWeapon( WEAPON_KEVLAR ) );
	g_EquipmentInfo[2].m_iTeam = TEAM_UNASSIGNED;
	Q_strcpy( g_EquipmentInfo[2].szClassName, "weapon_vest" );

#ifdef CLIENT_DLL
	g_EquipmentInfo[2].iconActive = new CHudTexture;
	g_EquipmentInfo[2].iconActive->cCharacterInFont = 't';
#endif

	g_EquipmentInfo[1].SetWeaponPrice( CSGameRules()->GetBlackMarketPriceForWeapon( WEAPON_ASSAULTSUIT ) );
	g_EquipmentInfo[1].SetDefaultPrice( ASSAULTSUIT_PRICE );
	g_EquipmentInfo[1].SetPreviousPrice( CSGameRules()->GetBlackMarketPreviousPriceForWeapon( WEAPON_ASSAULTSUIT ) );
	g_EquipmentInfo[1].m_iTeam = TEAM_UNASSIGNED;
	Q_strcpy( g_EquipmentInfo[1].szClassName, "weapon_vesthelm" );

#ifdef CLIENT_DLL
	g_EquipmentInfo[1].iconActive = new CHudTexture;
	g_EquipmentInfo[1].iconActive->cCharacterInFont = 'u';
#endif

	g_EquipmentInfo[0].SetWeaponPrice( CSGameRules()->GetBlackMarketPriceForWeapon( WEAPON_NVG ) );
	g_EquipmentInfo[0].SetPreviousPrice( CSGameRules()->GetBlackMarketPreviousPriceForWeapon( WEAPON_NVG ) );
	g_EquipmentInfo[0].SetDefaultPrice( NVG_PRICE );
	g_EquipmentInfo[0].m_iTeam = TEAM_UNASSIGNED;
	Q_strcpy( g_EquipmentInfo[0].szClassName, "weapon_nvgs" );

#ifdef CLIENT_DLL
	g_EquipmentInfo[0].iconActive = new CHudTexture;
	g_EquipmentInfo[0].iconActive->cCharacterInFont = 's';
#endif

}

//--------------------------------------------------------------------------------------------------------------
CCSWeaponInfo * GetWeaponInfo( CSWeaponID weaponID )
{
	if ( weaponID == WEAPON_NONE )
		return NULL;

	if ( weaponID >= WEAPON_KEVLAR )
	{
		int iIndex = (WEAPON_MAX - weaponID) - 1;

		return &g_EquipmentInfo[iIndex];

	}

	const char *weaponName = WeaponIdAsString(weaponID);
	WEAPON_FILE_INFO_HANDLE	hWpnInfo = LookupWeaponInfoSlot( weaponName );
	if ( hWpnInfo == GetInvalidWeaponInfoHandle() )
	{
		return NULL;
	}

	CCSWeaponInfo *pWeaponInfo = dynamic_cast< CCSWeaponInfo* >( GetFileWeaponInfoFromHandle( hWpnInfo ) );

	return pWeaponInfo;
}

//--------------------------------------------------------------------------------------------------------
const char* WeaponClassAsString( CSWeaponType weaponType )
{
	for ( int i = 0; i < ARRAYSIZE(s_weaponTypeInfo); ++i )
	{
		if ( s_weaponTypeInfo[i].type == weaponType )
		{
			return s_weaponTypeInfo[i].name;
		}
	}

	return NULL;
}


//--------------------------------------------------------------------------------------------------------
CSWeaponType WeaponClassFromString( const char* weaponType )
{
	for ( int i = 0; i < ARRAYSIZE(s_weaponTypeInfo); ++i )
	{
		if ( !Q_stricmp( s_weaponTypeInfo[i].name, weaponType ) )
		{
			return s_weaponTypeInfo[i].type;
		}
	}

	return WEAPONTYPE_UNKNOWN;
}


//--------------------------------------------------------------------------------------------------------
CSWeaponType WeaponClassFromWeaponID( CSWeaponID weaponID )
{
	const char *weaponStr = WeaponIDToAlias( weaponID );
	const char *translatedAlias = GetTranslatedWeaponAlias( weaponStr );

	char wpnName[128];
	Q_snprintf( wpnName, sizeof( wpnName ), "weapon_%s", translatedAlias );
	WEAPON_FILE_INFO_HANDLE	hWpnInfo = LookupWeaponInfoSlot( wpnName );
	if ( hWpnInfo != GetInvalidWeaponInfoHandle() )
	{
		CCSWeaponInfo *pWeaponInfo = dynamic_cast< CCSWeaponInfo* >( GetFileWeaponInfoFromHandle( hWpnInfo ) );
		if ( pWeaponInfo )
		{
			return pWeaponInfo->m_WeaponType;
		}
	}

	return WEAPONTYPE_UNKNOWN;
}


//--------------------------------------------------------------------------------------------------------
const char * WeaponIdAsString( CSWeaponID weaponID )
{
	for ( int i = 0; i < ARRAYSIZE(s_weaponNameInfo); ++i )
	{
		if (s_weaponNameInfo[i].id == weaponID )
			return s_weaponNameInfo[i].name;
	}

	return NULL;
}


//--------------------------------------------------------------------------------------------------------
CSWeaponID WeaponIdFromString( const char *szWeaponName )
{
	for ( int i = 0; i < ARRAYSIZE(s_weaponNameInfo); ++i )
	{
		if ( Q_stricmp(s_weaponNameInfo[i].name, szWeaponName) == 0 )
			return s_weaponNameInfo[i].id;
	}

	return WEAPON_NONE;
}


//--------------------------------------------------------------------------------------------------------
void ParseVector( KeyValues *keyValues, const char *keyName, Vector& vec )
{
	vec.x = vec.y = vec.z = 0.0f;

	if ( !keyValues || !keyName )
		return;

	const char *vecString = keyValues->GetString( keyName, "0 0 0" );
	if ( vecString && *vecString )
	{
		float x = 0.0f, y = 0.0f, z = 0.0f;
		if ( 3 == sscanf( vecString, "%f %f %f", &x, &y, &z ) )
		{
			vec.x = x;
			vec.y = y;
			vec.z = z;
		}
	}
}


FileWeaponInfo_t* CreateWeaponInfo()
{
	return new CCSWeaponInfo;
}


template <typename T>
void ZeroObject( T* p )
{
	memset( p, 0x0, sizeof(T) );
}

CCSWeaponInfo::CCSWeaponInfo()
{
	m_flMaxSpeed[0] = m_flMaxSpeed[1] = 1; // This should always be set in the script.
	m_szAddonModel[0] = 0;
	m_szMagModel[0] = 0;
	m_szAddonLocation[0] = '\0';
	m_fThrowVelocity = 0.0f;
	m_iKillAward = 0;
	m_vecIronsightEyePos.Init();
	m_angIronsightPivotAngle.Init();
	ZeroObject(m_fSpread);
	ZeroObject(m_fInaccuracyCrouch);
	ZeroObject(m_fInaccuracyStand);
	ZeroObject(m_fInaccuracyJump);
	ZeroObject(m_fInaccuracyLand);
	ZeroObject(m_fInaccuracyLadder);
	ZeroObject(m_fInaccuracyImpulseFire);
	ZeroObject(m_fInaccuracyMove);
	ZeroObject(m_fRecoilAngle);
	ZeroObject(m_fRecoilAngleVariance);
	ZeroObject(m_fRecoilMagnitude);
	ZeroObject(m_fRecoilMagnitudeVariance);
	m_iRecoilSeed = 0;
}

int	CCSWeaponInfo::GetKillAward( void ) const
{
	return m_iKillAward;
}

int	CCSWeaponInfo::GetWeaponPrice( void ) const
{
	return m_iWeaponPrice;
}

int	CCSWeaponInfo::GetDefaultPrice( void )
{
	return m_iDefaultPrice;
}

int	CCSWeaponInfo::GetPrevousPrice( void )
{
	return m_iPreviousPrice;
}


void CCSWeaponInfo::Parse( KeyValues *pKeyValuesData, const char *szWeaponName )
{
	BaseClass::Parse( pKeyValuesData, szWeaponName );

	m_flMaxSpeed[0] = (float)pKeyValuesData->GetInt( "MaxPlayerSpeed", 1 );
	m_flMaxSpeed[1] = (float)pKeyValuesData->GetInt( "MaxPlayerSpeedAlt", m_flMaxSpeed[0] );

	m_iKillAward = pKeyValuesData->GetInt( "KillAward", 300 );

	m_iDefaultPrice = m_iWeaponPrice = pKeyValuesData->GetInt( "WeaponPrice", -1 );
	if ( m_iWeaponPrice == -1 )
	{
		// This weapon should have the price in its script.
		Assert( false );
	}

	if ( CSGameRules()->IsBlackMarket() )
	{
		CSWeaponID iWeaponID = AliasToWeaponID( GetTranslatedWeaponAlias ( szWeaponName ) );

		m_iDefaultPrice = m_iWeaponPrice;
		m_iPreviousPrice = CSGameRules()->GetBlackMarketPreviousPriceForWeapon( iWeaponID );
		m_iWeaponPrice = CSGameRules()->GetBlackMarketPriceForWeapon( iWeaponID );
	}
		
	m_flArmorRatio				= pKeyValuesData->GetFloat( "WeaponArmorRatio", 1 );
	m_iCrosshairMinDistance		= pKeyValuesData->GetInt( "CrosshairMinDistance", 4 );
	m_iCrosshairDeltaDistance	= pKeyValuesData->GetInt( "CrosshairDeltaDistance", 3 );
	m_bCanUseWithShield			= !!pKeyValuesData->GetInt( "CanEquipWithShield", false );
	m_flAddonScale				= pKeyValuesData->GetFloat( "AddonScale", 1 );

	// muzzle flash
	Q_strncpy( m_szMuzzleFlash1stPerson, pKeyValuesData->GetString( "MuzzleFlash1stPerson" ), sizeof( m_szMuzzleFlash1stPerson ) );
	Q_strncpy( m_szMuzzleFlash1stPersonAlt, pKeyValuesData->GetString( "MuzzleFlash1stPersonAlt" ), sizeof( m_szMuzzleFlash1stPersonAlt ) );
	Q_strncpy( m_szMuzzleFlash3rdPerson, pKeyValuesData->GetString( "MuzzleFlash3rdPerson" ), sizeof( m_szMuzzleFlash3rdPerson ) );
	Q_strncpy( m_szMuzzleFlash3rdPersonAlt, pKeyValuesData->GetString( "MuzzleFlash3rdPersonAlt" ), sizeof( m_szMuzzleFlash3rdPersonAlt ) );

	m_iPenetration		= pKeyValuesData->GetFloat( "Penetration", 1 );
	m_iDamage			= pKeyValuesData->GetInt( "Damage", 42 ); // Douglas Adams 1952 - 2001
	m_flRange			= pKeyValuesData->GetFloat( "Range", 8192.0f );
	m_flRangeModifier	= pKeyValuesData->GetFloat( "RangeModifier", 0.98f );
	m_iBullets			= pKeyValuesData->GetInt( "Bullets", 1 );
	m_flCycleTime[0]	= pKeyValuesData->GetFloat( "CycleTime", 0.15 );
	m_flCycleTime[1]	= pKeyValuesData->GetFloat( "CycleTimeAlt", m_flCycleTime[0] );

	// new accuracy model parameters
	m_fSpread[0]				= pKeyValuesData->GetFloat("Spread", 0.0f);
	m_fInaccuracyCrouch[0]		= pKeyValuesData->GetFloat("InaccuracyCrouch", 0.0f);
	m_fInaccuracyStand[0]		= pKeyValuesData->GetFloat("InaccuracyStand", 0.0f);
	m_fInaccuracyJump[0]		= pKeyValuesData->GetFloat("InaccuracyJump", 0.0f);
	m_fInaccuracyJumpInitial	= pKeyValuesData->GetFloat("InaccuracyJumpInitial", 0.0f);
	m_fInaccuracyLand[0]		= pKeyValuesData->GetFloat("InaccuracyLand", 0.0f);
	m_fInaccuracyLadder[0]		= pKeyValuesData->GetFloat("InaccuracyLadder", 0.0f);
	m_fInaccuracyImpulseFire[0]	= pKeyValuesData->GetFloat("InaccuracyFire", 0.0f);
	m_fInaccuracyMove[0]		= pKeyValuesData->GetFloat("InaccuracyMove", 0.0f);

	m_fSpread[1]				= pKeyValuesData->GetFloat( "SpreadAlt", m_fSpread[0] );
	m_fInaccuracyCrouch[1]		= pKeyValuesData->GetFloat( "InaccuracyCrouchAlt", m_fInaccuracyCrouch[0] );
	m_fInaccuracyStand[1]		= pKeyValuesData->GetFloat( "InaccuracyStandAlt", m_fInaccuracyStand[0] );
	m_fInaccuracyJump[1]		= pKeyValuesData->GetFloat( "InaccuracyJumpAlt", m_fInaccuracyJump[0] );
	m_fInaccuracyLand[1]		= pKeyValuesData->GetFloat( "InaccuracyLandAlt", m_fInaccuracyLand[0] );
	m_fInaccuracyLadder[1]		= pKeyValuesData->GetFloat( "InaccuracyLadderAlt", m_fInaccuracyLadder[0] );
	m_fInaccuracyImpulseFire[1] = pKeyValuesData->GetFloat( "InaccuracyFireAlt", m_fInaccuracyImpulseFire[0] );
	m_fInaccuracyMove[1]		= pKeyValuesData->GetFloat( "InaccuracyMoveAlt", m_fInaccuracyMove[0] );

	m_fRecoilAngle[0]				= pKeyValuesData->GetFloat( "RecoilAngle", 0.0f );
	m_fRecoilAngleVariance[0]		= pKeyValuesData->GetFloat( "RecoilAngleVariance", 0.0f );
	m_fRecoilMagnitude[0]			= pKeyValuesData->GetFloat( "RecoilMagnitude", 0.0f );
	m_fRecoilMagnitudeVariance[0]	= pKeyValuesData->GetFloat( "RecoilMagnitudeVariance", 0.0f );
	m_fRecoilAngle[1]				= pKeyValuesData->GetFloat( "RecoilAngleAlt",				m_fRecoilAngle[0] );
	m_fRecoilAngleVariance[1]		= pKeyValuesData->GetFloat( "RecoilAngleVarianceAlt",		m_fRecoilAngleVariance[0] );
	m_fRecoilMagnitude[1]			= pKeyValuesData->GetFloat( "RecoilMagnitudeAlt",			m_fRecoilMagnitude[0] );
	m_fRecoilMagnitudeVariance[1]	= pKeyValuesData->GetFloat( "RecoilMagnitudeVarianceAlt",	m_fRecoilMagnitudeVariance[0] );
	m_iRecoilSeed					= pKeyValuesData->GetInt( "RecoilSeed", 0 );

	m_fInaccuracyReload			= pKeyValuesData->GetFloat("InaccuracyReload", 0.0f);
	m_fInaccuracyAltSwitch		= pKeyValuesData->GetFloat("InaccuracyAltSwitch", 0.0f);

	m_fRecoveryTimeCrouch		= pKeyValuesData->GetFloat("RecoveryTimeCrouch", 1.0f);
	m_fRecoveryTimeCrouchFinal	= pKeyValuesData->GetFloat("RecoveryTimeCrouchFinal", m_fRecoveryTimeCrouch);
	m_fRecoveryTimeStand		= pKeyValuesData->GetFloat("RecoveryTimeStand", 1.0f);
	m_fRecoveryTimeStandFinal	= pKeyValuesData->GetFloat("RecoveryTimeStandFinal", m_fRecoveryTimeStand);

	m_iRecoveryTransitionStartBullet	= pKeyValuesData->GetInt("RecoveryTransitionStartBullet", 0);
	m_iRecoveryTransitionEndBullet		= pKeyValuesData->GetInt("RecoveryTransitionEndBullet", 0);

	m_flTimeToIdleAfterFire	= pKeyValuesData->GetFloat( "TimeToIdle", 2 );
	m_flIdleInterval	= pKeyValuesData->GetFloat( "IdleInterval", 20 );

	// grenade parameters
	m_fThrowVelocity	= pKeyValuesData->GetFloat( "ThrowVelocity", 0.0f );

#if 0
	// eject brass variables
	Q_strncpy( m_szEjectBrassEffect, pKeyValuesData->GetString( "EjectBrassEffect" ), sizeof( m_szEjectBrassEffect ) );
#endif

	// tracer variables
	m_iTracerFrequency[0] = pKeyValuesData->GetInt( "TracerFrequency", 0 );
	m_iTracerFrequency[1] = pKeyValuesData->GetInt( "TracerFrequencyAlt", m_iTracerFrequency[0] );
	Q_strncpy( m_szTracerEffect, pKeyValuesData->GetString( "TracerEffect" ), sizeof( m_szTracerEffect ) );

	// heat variables
	m_flHeatPerShot = pKeyValuesData->GetFloat( "HeatPerShot", 0.0f );
	Q_strncpy( m_szHeatEffect, pKeyValuesData->GetString( "HeatEffect" ), sizeof( m_szHeatEffect ) );

	// ironsight variables
	m_bIronsightCapable = pKeyValuesData->GetBool( "IronsightCapable", false );
	m_flIronsightSpeedUp = pKeyValuesData->GetFloat( "IronsightSpeedUp", 0.0f );
	m_flIronsightSpeedDown = pKeyValuesData->GetFloat( "IronsightSpeedDown", 0.0f );
	m_flIronsightLooseness = pKeyValuesData->GetFloat( "IronsightLooseness", 0.0f );
	m_flIronsightFOV = pKeyValuesData->GetFloat( "IronsightFOV", 0.0f );
	m_flIronsightPivotForward = pKeyValuesData->GetFloat( "IronsightPivotForward", 0.0f );

	const char* srcString = pKeyValuesData->GetString( "IronsightEyePos", "0.0 0.0 0.0" );
	if ( (sscanf( srcString, "%f %f %f", &(m_vecIronsightEyePos.x), &(m_vecIronsightEyePos.y), &(m_vecIronsightEyePos.z) ) != 3) )
		m_vecIronsightEyePos.Init( 0, 0, 0 );
	srcString = pKeyValuesData->GetString( "IronsightPivotAngle", "0.0 0.0 0.0" );
	if ( (sscanf( srcString, "%f %f %f", &(m_angIronsightPivotAngle.x), &(m_angIronsightPivotAngle.y), &(m_angIronsightPivotAngle.z) ) != 3) )
		m_angIronsightPivotAngle.Init( 0, 0, 0 );

	Q_strncpy( m_szIronsightDotMaterial, pKeyValuesData->GetString( "IronsightDotMaterial" ), sizeof( m_szIronsightDotMaterial ) );

	// Figure out what team can have this weapon.
	m_iTeam = TEAM_UNASSIGNED;
	const char *pTeam = pKeyValuesData->GetString( "Team", NULL );
	if ( pTeam )
	{
		if ( Q_stricmp( pTeam, "CT" ) == 0 )
		{
			m_iTeam = TEAM_CT;
		}
		else if ( Q_stricmp( pTeam, "TERRORIST" ) == 0 )
		{
			m_iTeam = TEAM_TERRORIST;
		}
		else if ( Q_stricmp( pTeam, "ANY" ) == 0 )
		{
			m_iTeam = TEAM_UNASSIGNED;
		}
		else
		{
			Assert( false );
		}
	}
	else
	{
		Assert( false );
	}

	
	Q_strncpy( m_WrongTeamMsg, pKeyValuesData->GetString( "WrongTeamMsg" ), sizeof( m_WrongTeamMsg ) );

	Q_strncpy( m_szShieldViewModel, pKeyValuesData->GetString( "shieldviewmodel" ), sizeof( m_szShieldViewModel ) );
	
	Q_strncpy( m_szAnimExtension, pKeyValuesData->GetString( "PlayerAnimationExtension", "m4" ), sizeof( m_szAnimExtension ) );
	
	Q_strncpy( m_szUIAnimExtension, pKeyValuesData->GetString( "UIPlayerAnimationExtension", m_szAnimExtension ), sizeof( m_szUIAnimExtension ) );

	// Default is 2000.
	m_flBotAudibleRange = pKeyValuesData->GetFloat( "BotAudibleRange", 2000.0f );
	
	const char *pTypeString = pKeyValuesData->GetString( "WeaponType", "" );
	m_WeaponType = WeaponClassFromString(pTypeString);

	m_bFullAuto = pKeyValuesData->GetBool("FullAuto");

	// Read the addon model.
	Q_strncpy( m_szAddonModel, pKeyValuesData->GetString( "AddonModel" ), sizeof( m_szAddonModel ) );

	// Read the magazine model.
	Q_strncpy( m_szMagModel, pKeyValuesData->GetString( "magazine_model" ), sizeof( m_szMagModel ) );

	// Read a special addon attachment location if not the default location
	Q_strncpy( m_szAddonLocation, pKeyValuesData->GetString( "AddonLocation" ), sizeof( m_szAddonLocation ) );

	// Read the StatTrak model.
	Q_strncpy( m_szStatTrakModel, pKeyValuesData->GetString( "StatTrakModel" ), sizeof( m_szStatTrakModel ) );

	// Read the dropped model.
	Q_strncpy( m_szDroppedModel, pKeyValuesData->GetString( "DroppedModel" ), sizeof( m_szDroppedModel ) );

	// Read the silencer model.
	//Q_strncpy( m_szSilencerModel, pKeyValuesData->GetString( "SilencerModel" ), sizeof( m_szSilencerModel ) );

#ifndef CLIENT_DLL
	// Enforce consistency for the weapon here, since that way we don't need to save off the model bounds
	// for all time.
	// Moved to pure_server_minimal.txt
//	engine->ForceExactFile( UTIL_VarArgs("scripts/%s.ctx", szWeaponName ) );

	// Model bounds are rounded to the nearest integer, then extended by 1
	engine->ForceModelBounds( szWorldModel, Vector( -20, -12, -18 ), Vector( 50, 16, 19 ) );
	if ( m_szAddonModel[0] )
	{
		engine->ForceModelBounds( m_szAddonModel, Vector( -5, -5, -6 ), Vector( 13, 5, 7 ) );
	}/*
	if ( m_szSilencerModel[0] )
	{
		engine->ForceModelBounds( m_szSilencerModel, Vector( -20, -12, -18 ), Vector( 50, 16, 19 ) );
	}*/
#endif // !CLIENT_DLL
}

ConVar weapon_recoil_suppression_shots( "weapon_recoil_suppression_shots", "4", FCVAR_CHEAT |  FCVAR_REPLICATED, "Number of shots before weapon uses full recoil" );
ConVar weapon_recoil_suppression_factor( "weapon_recoil_suppression_factor", "0.75", FCVAR_CHEAT |  FCVAR_REPLICATED, "Initial recoil suppression factor (first suppressed shot will use this factor * standard recoil, lerping to 1 for later shots" );
ConVar weapon_recoil_variance("weapon_recoil_variance", "0.55", FCVAR_CHEAT | FCVAR_REPLICATED, "Amount of variance per recoil impulse", true, 0.0f, true, 1.0f );

WeaponRecoilData::WeaponRecoilData()
{
	m_mapRecoilTables.SetLessFunc( DefLessFunc( CSWeaponID ) );
}

WeaponRecoilData::~WeaponRecoilData()
{
	m_mapRecoilTables.PurgeAndDeleteElements();
}

void WeaponRecoilData::GenerateRecoilTable( RecoilData *data )
{
	const int iSuppressionShots = weapon_recoil_suppression_shots.GetInt();
	const float fBaseSuppressionFactor = weapon_recoil_suppression_factor.GetFloat();
	const float fRecoilVariance = weapon_recoil_variance.GetFloat();
	CUniformRandomStream recoilRandom;

	if ( !data )
		return;

	CCSWeaponInfo *pWeaponInfo = GetWeaponInfo( data->iWeaponID );

	// Walk the attributes to determine all things that we need
	int iSeed = 0;
	bool bFullAuto = false;
	float flRecoilAngle[2] = {};
	float flRecoilAngleVariance[2] = {};
	float flRecoilMagnitude[2] = {};
	float flRecoilMagnitudeVariance[2] = {};

	if ( !pWeaponInfo && pWeaponInfo->szClassName )
	{
		char const *szItemClass = pWeaponInfo->szClassName;
		CSWeaponID wpnId = WeaponIdFromString( szItemClass );
		if ( wpnId != WEAPON_NONE )
		{
			pWeaponInfo = GetWeaponInfo( wpnId );
		}
	}
	if ( pWeaponInfo )
	{
		iSeed = pWeaponInfo->m_iRecoilSeed;
		bFullAuto = pWeaponInfo->m_bFullAuto;
		for ( int iMode = 0; iMode < 2; ++ iMode )
		{
			flRecoilAngle[iMode] = pWeaponInfo->m_fRecoilAngle[iMode];
			flRecoilAngleVariance[iMode] = pWeaponInfo->m_fRecoilAngleVariance[iMode];
			flRecoilMagnitude[iMode] = pWeaponInfo->m_fRecoilMagnitude[iMode];
			flRecoilMagnitudeVariance[iMode] = pWeaponInfo->m_fRecoilMagnitudeVariance[iMode];
		}
	}
	
	for ( int iMode = 0; iMode < 2; ++iMode )
	{
		Assert( pWeaponInfo );

		recoilRandom.SetSeed( iSeed );

		float fAngle = 0.0f;
		float fMagnitude = 0.0f;

		for ( int j = 0; j < ARRAYSIZE( data->recoilTable[iMode] ); ++j )
		{
			float fAngleNew = flRecoilAngle[iMode] + recoilRandom.RandomFloat(- flRecoilAngleVariance[iMode], + flRecoilAngleVariance[iMode] );
			float fMagnitudeNew = flRecoilMagnitude[iMode] + recoilRandom.RandomFloat(- flRecoilMagnitudeVariance[iMode], + flRecoilMagnitudeVariance[iMode] );

			if ( bFullAuto && ( j > 0 ) )
			{
				fAngle = Lerp( fRecoilVariance, fAngle, fAngleNew );
				fMagnitude = Lerp( fRecoilVariance, fMagnitude, fMagnitudeNew );
			}
			else
			{
				fAngle = fAngleNew;
				fMagnitude = fMagnitudeNew;
			}

			if ( bFullAuto && ( j < iSuppressionShots ) )
			{
				float fSuppressionFactor = Lerp( (float)j / (float)iSuppressionShots, fBaseSuppressionFactor, 1.0f );
				fMagnitude *= fSuppressionFactor;
			}

			data->recoilTable[iMode][j].fAngle = fAngle;
			data->recoilTable[iMode][j].fMagnitude = fMagnitude;
		}
	}
}

void WeaponRecoilData::GetRecoilOffsets( CWeaponCSBase *pWeapon, int iMode, int iIndex, float& fAngle, float &fMagnitude )
{
	// Recoil offset tables are indexed by a weapon's definition index.
	// Look for the existing table, otherwise generate it.

	CSWeaponID id = pWeapon->GetCSWeaponID();

	RecoilData *wepData = NULL;
	CUtlMap< CSWeaponID, RecoilData* >::IndexType_t iMapLocation = m_mapRecoilTables.Find( id );
	if ( iMapLocation == m_mapRecoilTables.InvalidIndex() )
	{
		Assert( !"Generating recoil table too late" ); // failed to find recoil table, need to re-generate!
		wepData = new RecoilData;
		wepData->iWeaponID = id;
		iMapLocation = m_mapRecoilTables.InsertOrReplace( id, wepData );
		GenerateRecoilTable( wepData );
	}
	else
	{
		wepData = m_mapRecoilTables.Element( iMapLocation );
		Assert( wepData );
	}

	iIndex = iIndex % ARRAYSIZE( wepData->recoilTable[iMode] );
	fAngle = wepData->recoilTable[iMode][iIndex].fAngle;
	fMagnitude = wepData->recoilTable[iMode][iIndex].fMagnitude;
}

void WeaponRecoilData::GenerateRecoilPattern( CSWeaponID id )
{
	CUtlMap< CSWeaponID, RecoilData* >::IndexType_t iMapLocation = m_mapRecoilTables.Find( id );
	if ( iMapLocation == m_mapRecoilTables.InvalidIndex() )
	{
		RecoilData *wepData = new RecoilData;
		wepData->iWeaponID = id;
		iMapLocation = m_mapRecoilTables.InsertOrReplace( id, wepData );
		GenerateRecoilTable( wepData );
	}
}

WeaponRecoilData g_WeaponRecoilData;

void GenerateWeaponRecoilPattern( CSWeaponID idx )
{
	g_WeaponRecoilData.GenerateRecoilPattern( idx );
}
