//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#if defined( WIN32 ) && !defined( _X360 )
#include <windows.h> // SRC only!!
#endif

#include "ModOptionsSubLoadout.h"
#include <stdio.h>

#include <vgui_controls/Button.h>
#include "tier1/KeyValues.h"
#include <vgui_controls/Label.h>
#include <vgui/ISystem.h>
#include <vgui/ISurface.h>
#include <vgui_controls/ComboBox.h>

#include "CvarTextEntry.h"
#include "CvarToggleCheckButton.h"
#include "LabeledCommandComboBox.h"
#include "EngineInterface.h"
#include "tier1/convar.h"

#include "GameUI_Interface.h"

#if defined( _X360 )
#include "xbox/xbox_win32stubs.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

const char* szMusicStrings[] =
{
	"valve_csgo_01", // the default one should be on top
	"amontobin_01",
	"austinwintory_01",
	"austinwintory_02",
	"awolnation_01",
	"beartooth_01",
	"beartooth_02",
	"blitzkids_01",
	"damjanmravunac_01",
	"danielsadowski_01",
	"danielsadowski_02",
	"danielsadowski_03",
	"danielsadowski_04",
	"darude_01",
	"dren_01",
	"dren_02",
	"feedme_01",
	"hades_01",
	"halflife_alyx_01",
	"halo_01",
	"hotlinemiami_01",
	"hundredth_01",
	"ianhultquist_01",
	"kellybailey_01",
	"kitheory_01",
	"lenniemoore_01",
	"mateomessina_01",
	"mattlange_01",
	"mattlevine_01",
	"michaelbross_01",
	"midnightriders_01",
	"mordfustang_01",
	"neckdeep_01",
	"neckdeep_02",
	"newbeatfund_01",
	"noisia_01",
	"proxy_01",
	"roam_01",
	"robertallaire_01",
	"sammarshall_01",
	"sasha_01",
	"scarlxrd_01",
	"scarlxrd_02",
	"seanmurray_01",
	"skog_01",
	"skog_02",
	"skog_03",
	"theverkkars_01",
	"timhuling_01",
	"treeadams_benbromfield_01",
	"troelsfolmann_01",
	"twinatlantic_01"
};


//-----------------------------------------------------------------------------
// Purpose: Basic help dialog
//-----------------------------------------------------------------------------
CModOptionsSubLoadout::CModOptionsSubLoadout(vgui::Panel *parent) : vgui::PropertyPage(parent, "ModOptionsSubLoadout") 
{
	Button *cancel = new Button( this, "Cancel", "#GameUI_Cancel" );
	cancel->SetCommand( "Close" );

	Button *ok = new Button( this, "OK", "#GameUI_OK" );
	ok->SetCommand( "Ok" );

	Button *apply = new Button( this, "Apply", "#GameUI_Apply" );
	apply->SetCommand( "Apply" );

	//=========

	m_pLoadoutM4ComboBox = new CLabeledCommandComboBox( this, "M4ComboBox" );
	m_pLoadoutM4ComboBox->AddItem( "#Cstrike_WPNHUD_M4A4", "loadout_slot_m4_weapon 0" );
	m_pLoadoutM4ComboBox->AddItem( "#Cstrike_WPNHUD_M4A1", "loadout_slot_m4_weapon 1" );
	m_pLoadoutM4ComboBox->AddActionSignalTarget( this );

	m_pLoadoutHKP2000ComboBox = new CLabeledCommandComboBox( this, "HKP2000ComboBox" );
	m_pLoadoutHKP2000ComboBox->AddItem( "#Cstrike_WPNHUD_HKP2000", "loadout_slot_hkp2000_weapon 0" );
	m_pLoadoutHKP2000ComboBox->AddItem( "#Cstrike_WPNHUD_USP45", "loadout_slot_hkp2000_weapon 1" );

	m_pLoadoutFiveSevenComboBox = new CLabeledCommandComboBox( this, "FiveSevenComboBox" );
	m_pLoadoutFiveSevenComboBox->AddItem( "#Cstrike_WPNHUD_FiveSeven", "loadout_slot_fiveseven_weapon 0" );
	m_pLoadoutFiveSevenComboBox->AddItem( "#Cstrike_WPNHUD_CZ75", "loadout_slot_fiveseven_weapon 1" );

	m_pLoadoutTec9ComboBox = new CLabeledCommandComboBox( this, "Tec9ComboBox" );
	m_pLoadoutTec9ComboBox->AddItem( "#Cstrike_WPNHUD_Tec9", "loadout_slot_tec9_weapon 0" );
	m_pLoadoutTec9ComboBox->AddItem( "#Cstrike_WPNHUD_CZ75", "loadout_slot_tec9_weapon 1" );

	m_pLoadoutMP7CTComboBox = new CLabeledCommandComboBox( this, "MP7CTComboBox" );
	m_pLoadoutMP7CTComboBox->AddItem( "#Cstrike_WPNHUD_MP7", "loadout_slot_mp7_weapon_ct 0" );
	m_pLoadoutMP7CTComboBox->AddItem( "#Cstrike_WPNHUD_MP5SD", "loadout_slot_mp7_weapon_ct 1" );

	m_pLoadoutMP7TComboBox = new CLabeledCommandComboBox( this, "MP7TComboBox" );
	m_pLoadoutMP7TComboBox->AddItem( "#Cstrike_WPNHUD_MP7", "loadout_slot_mp7_weapon_t 0" );
	m_pLoadoutMP7TComboBox->AddItem( "#Cstrike_WPNHUD_MP5SD", "loadout_slot_mp7_weapon_t 1" );

	m_pLoadoutDeagleCTComboBox = new CLabeledCommandComboBox( this, "DeagleCTComboBox" );
	m_pLoadoutDeagleCTComboBox->AddItem( "#Cstrike_WPNHUD_DesertEagle", "loadout_slot_deagle_weapon_ct 0" );
	m_pLoadoutDeagleCTComboBox->AddItem( "#Cstrike_WPNHUD_Revolver", "loadout_slot_deagle_weapon_ct 1" );

	m_pLoadoutDeagleTComboBox = new CLabeledCommandComboBox( this, "DeagleTComboBox" );
	m_pLoadoutDeagleTComboBox->AddItem( "#Cstrike_WPNHUD_DesertEagle", "loadout_slot_deagle_weapon_t 0" );
	m_pLoadoutDeagleTComboBox->AddItem( "#Cstrike_WPNHUD_Revolver", "loadout_slot_deagle_weapon_t 1" );
		
	m_pStatTrak = new CCvarToggleCheckButton( this, "EnableStatTrak", "#GameUI_Loadout_StatTrak", "loadout_stattrak" );
	m_pMusicSelection = new CLabeledCommandComboBox( this, "MusicSelectionComboBox" );

	for ( int i = 0; i < ARRAYSIZE( szMusicStrings ); i++ )
	{
		char command[128];
		char string[128];
		Q_snprintf( command, sizeof( command ), "snd_music_selection %s", szMusicStrings[i] );
		Q_snprintf( string, sizeof( string ), "#GameUI_Gameplay_MusicKit_%s", szMusicStrings[i] );
		m_pMusicSelection->AddItem( string, command );
	}

	m_pLoadoutM4ComboBox->AddActionSignalTarget( this );
	m_pLoadoutHKP2000ComboBox->AddActionSignalTarget( this );
	m_pLoadoutFiveSevenComboBox->AddActionSignalTarget( this );
	m_pLoadoutTec9ComboBox->AddActionSignalTarget( this );
	m_pLoadoutMP7CTComboBox->AddActionSignalTarget( this );
	m_pLoadoutMP7TComboBox->AddActionSignalTarget( this );
	m_pLoadoutDeagleCTComboBox->AddActionSignalTarget( this );
	m_pLoadoutDeagleTComboBox->AddActionSignalTarget( this );
	m_pStatTrak->AddActionSignalTarget( this );
	m_pMusicSelection->AddActionSignalTarget( this );

#if !INSTANT_MUSIC_CHANGE
	m_bNeedToWarnAboutMusic = true;
#endif

	LoadControlSettings("Resource/ModOptionsSubLoadout.res");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CModOptionsSubLoadout::~CModOptionsSubLoadout()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubLoadout::OnControlModified()
{
	PostMessage(GetParent(), new KeyValues("ApplyButtonEnable"));
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubLoadout::OnResetData()
{
	ConVarRef loadout_slot_m4_weapon( "loadout_slot_m4_weapon" );
	if ( loadout_slot_m4_weapon.IsValid() )
		m_pLoadoutM4ComboBox->SetInitialItem( loadout_slot_m4_weapon.GetInt() );

	ConVarRef loadout_slot_hkp2000_weapon( "loadout_slot_hkp2000_weapon" );
	if ( loadout_slot_hkp2000_weapon.IsValid() )
		m_pLoadoutHKP2000ComboBox->SetInitialItem( loadout_slot_hkp2000_weapon.GetInt() );

	ConVarRef loadout_slot_fiveseven_weapon( "loadout_slot_fiveseven_weapon" );
	if ( loadout_slot_fiveseven_weapon.IsValid() )
		m_pLoadoutFiveSevenComboBox->SetInitialItem( loadout_slot_fiveseven_weapon.GetInt() );

	ConVarRef loadout_slot_tec9_weapon( "loadout_slot_tec9_weapon" );
	if ( loadout_slot_tec9_weapon.IsValid() )
		m_pLoadoutTec9ComboBox->SetInitialItem( loadout_slot_tec9_weapon.GetInt() );

	ConVarRef loadout_slot_mp7_weapon_ct( "loadout_slot_mp7_weapon_ct" );
	if ( loadout_slot_mp7_weapon_ct.IsValid() )
		m_pLoadoutMP7CTComboBox->SetInitialItem( loadout_slot_mp7_weapon_ct.GetInt() );

	ConVarRef loadout_slot_mp7_weapon_t( "loadout_slot_mp7_weapon_t" );
	if ( loadout_slot_mp7_weapon_t.IsValid() )
		m_pLoadoutMP7TComboBox->SetInitialItem( loadout_slot_mp7_weapon_t.GetInt() );

	ConVarRef loadout_slot_deagle_weapon_ct( "loadout_slot_deagle_weapon_ct" );
	if ( loadout_slot_deagle_weapon_ct.IsValid() )
		m_pLoadoutDeagleCTComboBox->SetInitialItem( loadout_slot_deagle_weapon_ct.GetInt() );

	ConVarRef loadout_slot_deagle_weapon_t( "loadout_slot_deagle_weapon_t" );
	if ( loadout_slot_deagle_weapon_t.IsValid() )
		m_pLoadoutDeagleTComboBox->SetInitialItem( loadout_slot_deagle_weapon_t.GetInt() );

	m_pStatTrak->Reset();

	ConVarRef snd_music_selection( "snd_music_selection" );
	const char *pMusicName = snd_music_selection.GetString();
	for ( int i = 0; i < ARRAYSIZE( szMusicStrings ); i++ )
	{
		if ( !Q_strcmp( pMusicName, szMusicStrings[i] ) )
		{
			m_pMusicSelection->SetInitialItem( i );
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CModOptionsSubLoadout::OnApplyChanges()
{
	m_pLoadoutM4ComboBox->ApplyChanges();
	m_pLoadoutHKP2000ComboBox->ApplyChanges();
	m_pLoadoutFiveSevenComboBox->ApplyChanges();
	m_pLoadoutTec9ComboBox->ApplyChanges();
	m_pLoadoutMP7CTComboBox->ApplyChanges();
	m_pLoadoutMP7TComboBox->ApplyChanges();
	m_pLoadoutDeagleCTComboBox->ApplyChanges();
	m_pLoadoutDeagleTComboBox->ApplyChanges();
	m_pStatTrak->ApplyChanges();
	m_pMusicSelection->ApplyChanges();

	ConVarRef snd_music_selection( "snd_music_selection" );
#if INSTANT_MUSIC_CHANGE
	if ( Q_strcmp( snd_music_selection.GetString(), szMusicStrings[m_pMusicSelection->GetActiveItem()] ) )
#else
	if ( m_bNeedToWarnAboutMusic && Q_strcmp( snd_music_selection.GetString(), szMusicStrings[m_pMusicSelection->GetActiveItem()] ) )
#endif
	{
		// Bring up the confirmation dialog
#if INSTANT_MUSIC_CHANGE
		m_pMusicSelection->ApplyChanges();
		GameUI().ReleaseBackgroundMusic();
#else
		MessageBox *box = new MessageBox( "#GameUI_OptionsRestartRequired_Title", "#GameUI_Gameplay_MusicRestartHint", this );
		box->MoveToFront();
		box->DoModal();
		m_bNeedToWarnAboutMusic = false;
#endif
	}
}
