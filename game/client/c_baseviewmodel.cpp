//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side view model implementation. Responsible for drawing
//			the view model.
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_baseviewmodel.h"
#include "model_types.h"
#include "hud.h"
#include "view_shared.h"
#include "iviewrender.h"
#include "view.h"
#include "mathlib/vmatrix.h"
#include "cl_animevent.h"
#include "eventlist.h"
#include "tools/bonelist.h"
#include <KeyValues.h>
#include "hltvcamera.h"
#ifdef TF_CLIENT_DLL
	#include "tf_weaponbase.h"
#endif
#ifdef CSTRIKE_DLL
	#include "weapon_csbase.h"
	#include "weapon_basecsgrenade.h"
	#include "cs_shareddefs.h"
	#include "c_cs_player.h"
	#include "cs_loadout.h"
#endif

#if defined( REPLAY_ENABLED )
#include "replay/replaycamera.h"
#include "replay/ireplaysystem.h"
#include "replay/ienginereplay.h"
#endif

// NVNT haptics system interface
#include "haptics/ihaptics.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CSTRIKE_DLL
	ConVar cl_righthand( "cl_righthand", "1", FCVAR_ARCHIVE, "Use right-handed view models." );

	extern ConVar loadout_slot_gloves_ct;
	extern ConVar loadout_slot_gloves_t;
	extern ConVar loadout_stattrak;
#endif

extern ConVar r_drawviewmodel;

#ifdef TF_CLIENT_DLL
	ConVar cl_flipviewmodels( "cl_flipviewmodels", "0", FCVAR_USERINFO | FCVAR_ARCHIVE | FCVAR_NOT_CONNECTED, "Flip view models." );
#endif

void PostToolMessage( HTOOLHANDLE hEntity, KeyValues *msg );

void C_BaseViewModel::UpdateStatTrakGlow( void )
{
	//approach the ideal in 2 seconds
	m_flStatTrakGlowMultiplier = Approach( m_flStatTrakGlowMultiplierIdeal, m_flStatTrakGlowMultiplier, (gpGlobals->frametime * 0.5) );
}

void C_BaseViewModel::OnNewParticleEffect( const char *pszParticleName, CNewParticleEffect *pNewParticleEffect )
{
	if ( FStrEq( pszParticleName, MOLOTOV_PARTICLE_EFFECT_NAME ) )
	{
		m_viewmodelParticleEffect = pNewParticleEffect;
	}
}

void C_BaseViewModel::OnParticleEffectDeleted( CNewParticleEffect *pParticleEffect )
{
	BaseClass::OnParticleEffectDeleted( pParticleEffect );

	if ( m_viewmodelParticleEffect == pParticleEffect )
	{
		m_viewmodelParticleEffect = NULL;
	}
}

void C_BaseViewModel::UpdateParticles()
{
	C_BasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( !pPlayer )
		return;

	if ( pPlayer->IsPlayerDead() )
		return;

	// Otherwise pass the event to our associated weapon
	C_BaseCombatWeapon *pWeapon = GetOwningWeapon();
	if ( !pWeapon )
		return;

	CWeaponCSBase *pCSWeapon = ( CWeaponCSBase* )pPlayer->GetActiveWeapon();
	if ( !pCSWeapon )
		return;

	int iWeaponId = pCSWeapon->GetCSWeaponID();

	bool shouldDrawPlayer = ( pPlayer->ShouldDraw() );
	bool visible = r_drawviewmodel.GetBool() && pPlayer && !shouldDrawPlayer;

	if ( visible && iWeaponId == WEAPON_MOLOTOV )
	{
		CBaseCSGrenade *pGren = dynamic_cast<CBaseCSGrenade*>( pPlayer->GetActiveWeapon() );

		if ( pGren->IsPinPulled() )
		{
			//if ( !pGren->IsLoopingSoundPlaying() )
			//{
			//	pGren->SetLoopingSoundPlaying( true );
			//	EmitSound( "Molotov.IdleLoop" );
			//	//DevMsg( 1, "++++++++++>Playing Molotov.IdleLoop 2\n" );
			//}

			// TEST: [mlowrance] This is to test for attachment.
			int iAttachment = -1;
			if ( pWeapon && pWeapon->GetBaseAnimating() )
				iAttachment = pWeapon->GetBaseAnimating()->LookupAttachment( "Wick" );

			if ( iAttachment >= 0 )
			{
				if ( !m_viewmodelParticleEffect )
				{
					DispatchParticleEffect( MOLOTOV_PARTICLE_EFFECT_NAME, PATTACH_POINT_FOLLOW, this, "Wick" );
				}
			}
		}
	}
	else
	{
		if ( m_viewmodelParticleEffect )
		{
			StopSound( "Molotov.IdleLoop" );
			//DevMsg( 1, "---------->Stopping Molotov.IdleLoop 3\n" );
			m_viewmodelParticleEffect->StopEmission( false, true );
			m_viewmodelParticleEffect->SetRemoveFlag();
			m_viewmodelParticleEffect = NULL;
		}
	}
}

void C_BaseViewModel::Simulate()
{
	UpdateParticles();
	UpdateStatTrakGlow();
	BaseClass::Simulate();
	return;
}

void FormatViewModelAttachment( Vector &vOrigin, bool bInverse )
{
	// Presumably, SetUpView has been called so we know our FOV and render origin.
	const CViewSetup *pViewSetup = view->GetPlayerViewSetup();
	
	float worldx = tan( pViewSetup->fov * M_PI/360.0 );
	float viewx = tan( pViewSetup->fovViewmodel * M_PI/360.0 );

	// aspect ratio cancels out, so only need one factor
	// the difference between the screen coordinates of the 2 systems is the ratio
	// of the coefficients of the projection matrices (tan (fov/2) is that coefficient)
	// NOTE: viewx was coming in as 0 when folks set their viewmodel_fov to 0 and show their weapon.
	float factorX = viewx ? ( worldx / viewx ) : 0.0f;
	float factorY = factorX;
	
	// Get the coordinates in the viewer's space.
	Vector tmp = vOrigin - pViewSetup->origin;
	Vector vTransformed( MainViewRight().Dot( tmp ), MainViewUp().Dot( tmp ), MainViewForward().Dot( tmp ) );

	// Now squash X and Y.
	if ( bInverse )
	{
		if ( factorX != 0 && factorY != 0 )
		{
			vTransformed.x /= factorX;
			vTransformed.y /= factorY;
		}
		else
		{
			vTransformed.x = 0.0f;
			vTransformed.y = 0.0f;
		}
	}
	else
	{
		vTransformed.x *= factorX;
		vTransformed.y *= factorY;
	}



	// Transform back to world space.
	Vector vOut = (MainViewRight() * vTransformed.x) + (MainViewUp() * vTransformed.y) + (MainViewForward() * vTransformed.z);
	vOrigin = pViewSetup->origin + vOut;
}


void C_BaseViewModel::FormatViewModelAttachment( int nAttachment, matrix3x4_t &attachmentToWorld )
{
	Vector vecOrigin;
	MatrixPosition( attachmentToWorld, vecOrigin );
	::FormatViewModelAttachment( vecOrigin, false );
	PositionMatrix( vecOrigin, attachmentToWorld );
}


bool C_BaseViewModel::IsViewModel() const
{
	return true;
}

void C_BaseViewModel::UncorrectViewModelAttachment( Vector &vOrigin )
{
	// Unformat the attachment.
	::FormatViewModelAttachment( vOrigin, true );
}


//-----------------------------------------------------------------------------
// Purpose
//-----------------------------------------------------------------------------
void C_BaseViewModel::FireEvent( const Vector& origin, const QAngle& angles, int event, const char *options )
{
	// We override sound requests so that we can play them locally on the owning player
	if ( ( event == AE_CL_PLAYSOUND ) || ( event == CL_EVENT_SOUND ) )
	{
		// Only do this if we're owned by someone
		if ( GetOwner() != NULL )
		{
			CLocalPlayerFilter filter;
			EmitSound( filter, GetOwner()->GetSoundSourceIndex(), options, &GetAbsOrigin() );
			return;
		}
	}

	// Otherwise pass the event to our associated weapon
	C_BaseCombatWeapon *pWeapon = GetActiveWeapon();
	if ( pWeapon )
	{
		// NVNT notify the haptics system of our viewmodel's event
		if ( haptics )
			haptics->ProcessHapticEvent(4,"Weapons",pWeapon->GetName(),"AnimationEvents",VarArgs("%i",event));

		bool bResult = pWeapon->OnFireEvent( this, origin, angles, event, options );
		if ( !bResult )
		{
			BaseClass::FireEvent( origin, angles, event, options );
		}
	}
}

bool C_BaseViewModel::Interpolate( float currentTime )
{
	CStudioHdr *pStudioHdr = GetModelPtr();
	// Make sure we reset our animation information if we've switch sequences
	UpdateAnimationParity();

	bool bret = BaseClass::Interpolate( currentTime );

	// Hack to extrapolate cycle counter for view model
	float elapsed_time = currentTime - m_flAnimTime;
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	// Predicted viewmodels have fixed up interval
	if ( GetPredictable() || IsClientCreated() )
	{
		Assert( pPlayer );
		float curtime = pPlayer ? pPlayer->GetFinalPredictedTime() : gpGlobals->curtime;
		elapsed_time = curtime - m_flAnimTime;
		// Adjust for interpolated partial frame
		if ( !engine->IsPaused() )
		{
			elapsed_time += ( gpGlobals->interpolation_amount * TICK_INTERVAL );
		}
	}

	// Prediction errors?	
	if ( elapsed_time < 0 )
	{
		elapsed_time = 0;
	}

	float dt = elapsed_time * (GetPlaybackRate() * GetSequenceCycleRate( pStudioHdr, GetSequence() )) + m_fCycleOffset;

	if ( dt < 0.0f )
	{
		dt = 0.0f;
	}

	if ( dt >= 1.0f )
	{
		if ( !IsSequenceLooping( GetSequence() ) )
		{
			dt = 0.999f;
		}
		else
		{
			dt = fmod( dt, 1.0f );
		}
	}

	SetCycle( dt );
	return bret;
}


bool C_BaseViewModel::ShouldFlipViewModel()
{
#ifdef CSTRIKE_DLL
	// If cl_righthand is set, then we want them all right-handed.
	CBaseCombatWeapon *pWeapon = m_hWeapon.Get();
	if ( pWeapon )
	{
		const FileWeaponInfo_t *pInfo = &pWeapon->GetWpnData();
		return pInfo->m_bAllowFlipping && pInfo->m_bBuiltRightHanded != cl_righthand.GetBool();
	}
#endif

#ifdef TF_CLIENT_DLL
	CBaseCombatWeapon *pWeapon = m_hWeapon.Get();
	if ( pWeapon )
	{
		return pWeapon->m_bFlipViewModel != cl_flipviewmodels.GetBool();
	}
#endif

	return false;
}


void C_BaseViewModel::ApplyBoneMatrixTransform( matrix3x4_t& transform )
{
	if ( ShouldFlipViewModel() )
	{
		matrix3x4_t viewMatrix, viewMatrixInverse;

		// We could get MATERIAL_VIEW here, but this is called sometimes before the renderer
		// has set that matrix. Luckily, this is called AFTER the CViewSetup has been initialized.
		const CViewSetup *pSetup = view->GetPlayerViewSetup();
		AngleMatrix( pSetup->angles, pSetup->origin, viewMatrixInverse );
		MatrixInvert( viewMatrixInverse, viewMatrix );

		// Transform into view space.
		matrix3x4_t temp, temp2;
		ConcatTransforms( viewMatrix, transform, temp );
		
		// Flip it along X.
		
		// (This is the slower way to do it, and it equates to negating the top row).
		//matrix3x4_t mScale;
		//SetIdentityMatrix( mScale );
		//mScale[0][0] = 1;
		//mScale[1][1] = -1;
		//mScale[2][2] = 1;
		//ConcatTransforms( mScale, temp, temp2 );
		temp[1][0] = -temp[1][0];
		temp[1][1] = -temp[1][1];
		temp[1][2] = -temp[1][2];
		temp[1][3] = -temp[1][3];

		// Transform back out of view space.
		ConcatTransforms( viewMatrixInverse, temp, transform );
	}
}

//-----------------------------------------------------------------------------
// Purpose: check if weapon viewmodel should be drawn
//-----------------------------------------------------------------------------
bool C_BaseViewModel::ShouldDraw()
{
	if ( engine->IsHLTV() )
	{
		return ( HLTVCamera()->GetMode() == OBS_MODE_IN_EYE &&
				 HLTVCamera()->GetPrimaryTarget() == GetOwner()	);
	}
#if defined( REPLAY_ENABLED )
	else if ( g_pEngineClientReplay->IsPlayingReplayDemo() )
	{
		return ( ReplayCamera()->GetMode() == OBS_MODE_IN_EYE &&
				 ReplayCamera()->GetPrimaryTarget() == GetOwner() );
	}
#endif
	else
	{
		return BaseClass::ShouldDraw();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Render the weapon. Draw the Viewmodel if the weapon's being carried
//			by this player, otherwise draw the worldmodel.
//-----------------------------------------------------------------------------
int C_BaseViewModel::DrawModel( int flags )
{
	if ( !m_bReadyToDraw )
		return 0;

	CMatRenderContextPtr pRenderContext( materials );

	if ( flags & STUDIO_RENDER )
	{
		// Determine blending amount and tell engine
		float blend = (float)( GetFxBlend() / 255.0f );

		// Totally gone
		if ( blend <= 0.0f )
			return 0;

		// Tell engine
		render->SetBlend( blend );

		float color[3];
		GetColorModulation( color );
		render->SetColorModulation(	color );
	}

	if ( ShouldFlipViewModel() )
		pRenderContext->CullMode( MATERIAL_CULLMODE_CW );
		
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	C_BaseCombatWeapon *pWeapon = GetOwningWeapon();
	int ret;
	// If the local player's overriding the viewmodel rendering, let him do it
	if ( pPlayer && pPlayer->IsOverridingViewmodel() )
	{
		ret = pPlayer->DrawOverriddenViewmodel( this, flags );
	}
	else if ( pWeapon && pWeapon->IsOverridingViewmodel() )
	{
		ret = pWeapon->DrawOverriddenViewmodel( this, flags );
	}
	else
	{
		ret = BaseClass::DrawModel( flags );
	}

	pRenderContext->CullMode( MATERIAL_CULLMODE_CCW );

	// Now that we've rendered, reset the animation restart flag
	if ( flags & STUDIO_RENDER )
	{
		if ( m_nOldAnimationParity != m_nAnimationParity )
		{
			m_nOldAnimationParity = m_nAnimationParity;
		}
		// Tell the weapon itself that we've rendered, in case it wants to do something
		if ( pWeapon )
		{
			pWeapon->ViewModelDrawn( this );
		}
	}

	if ( flags )
	{
		FOR_EACH_VEC( m_vecViewmodelArmModels, i )
		{
			if ( m_vecViewmodelArmModels[i] )
			{

				if ( m_vecViewmodelArmModels[i]->GetMoveParent() != this )
				{
					m_vecViewmodelArmModels[i]->SetEFlags( EF_BONEMERGE );
					m_vecViewmodelArmModels[i]->SetParent( this );
				}

				m_vecViewmodelArmModels[i]->DrawModel( flags );
			}
		}
		if ( m_viewmodelStatTrakAddon )
		{
			m_viewmodelStatTrakAddon->DrawModel( flags );
		}
	}

	return ret;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int C_BaseViewModel::InternalDrawModel( int flags )
{
	CMatRenderContextPtr pRenderContext( materials );
	if ( ShouldFlipViewModel() )
		pRenderContext->CullMode( MATERIAL_CULLMODE_CW );

	int ret = BaseClass::InternalDrawModel( flags );

	pRenderContext->CullMode( MATERIAL_CULLMODE_CCW );

	return ret;
}

//-----------------------------------------------------------------------------
// Purpose: Called by the player when the player's overriding the viewmodel drawing. Avoids infinite recursion.
//-----------------------------------------------------------------------------
int C_BaseViewModel::DrawOverriddenViewmodel( int flags )
{
	return BaseClass::DrawModel( flags );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
int C_BaseViewModel::GetFxBlend( void )
{
	// See if the local player wants to override the viewmodel's rendering
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( pPlayer && pPlayer->IsOverridingViewmodel() )
	{
		pPlayer->ComputeFxBlend();
		return pPlayer->GetFxBlend();
	}

	C_BaseCombatWeapon *pWeapon = GetOwningWeapon();
	if ( pWeapon && pWeapon->IsOverridingViewmodel() )
	{
		pWeapon->ComputeFxBlend();
		return pWeapon->GetFxBlend();
	}

	return BaseClass::GetFxBlend();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_BaseViewModel::IsTransparent( void )
{
	// See if the local player wants to override the viewmodel's rendering
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( pPlayer && pPlayer->IsOverridingViewmodel() )
	{
		return pPlayer->ViewModel_IsTransparent();
	}

	C_BaseCombatWeapon *pWeapon = GetOwningWeapon();
	if ( pWeapon && pWeapon->IsOverridingViewmodel() )
		return pWeapon->ViewModel_IsTransparent();

	return BaseClass::IsTransparent();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool C_BaseViewModel::UsesPowerOfTwoFrameBufferTexture( void )
{
	// See if the local player wants to override the viewmodel's rendering
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( pPlayer && pPlayer->IsOverridingViewmodel() )
	{
		return pPlayer->ViewModel_IsUsingFBTexture();
	}

	C_BaseCombatWeapon *pWeapon = GetOwningWeapon();
	if ( pWeapon && pWeapon->IsOverridingViewmodel() )
	{
		return pWeapon->ViewModel_IsUsingFBTexture();
	}

	return BaseClass::UsesPowerOfTwoFrameBufferTexture();
}

//-----------------------------------------------------------------------------
// Purpose: If the animation parity of the weapon has changed, we reset cycle to avoid popping
//-----------------------------------------------------------------------------
void C_BaseViewModel::UpdateAnimationParity( void )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	
	// If we're predicting, then we don't use animation parity because we change the animations on the clientside
	// while predicting. When not predicting, only the server changes the animations, so a parity mismatch
	// tells us if we need to reset the animation.
	if ( m_nOldAnimationParity != m_nAnimationParity && !GetPredictable() )
	{
		float curtime = (pPlayer && IsIntermediateDataAllocated()) ? pPlayer->GetFinalPredictedTime() : gpGlobals->curtime;
		// FIXME: this is bad
		// Simulate a networked m_flAnimTime and m_flCycle
		// FIXME:  Do we need the magic 0.1?
		SetCycle( 0.0f ); // GetSequenceCycleRate( GetSequence() ) * 0.1;
		m_flAnimTime = curtime;
		m_fCycleOffset = 0.0f;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Update global map state based on data received
// Input  : bnewentity - 
//-----------------------------------------------------------------------------
void C_BaseViewModel::OnDataChanged( DataUpdateType_t updateType )
{
	SetPredictionEligible( true );
	BaseClass::OnDataChanged(updateType);
}

void C_BaseViewModel::PostDataUpdate( DataUpdateType_t updateType )
{
	BaseClass::PostDataUpdate(updateType);
	OnLatchInterpolatedVariables( LATCH_ANIMATION_VAR );
}


//-----------------------------------------------------------------------------
// Purpose: Add entity to visible view models list
//-----------------------------------------------------------------------------
void C_BaseViewModel::AddEntity( void )
{
	// Server says don't interpolate this frame, so set previous info to new info.
	if ( IsNoInterpolationFrame() )
	{
		ResetLatched();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BaseViewModel::GetBoneControllers(float controllers[MAXSTUDIOBONECTRLS])
{
	BaseClass::GetBoneControllers( controllers );

	// Tell the weapon itself that we've rendered, in case it wants to do something
	C_BaseCombatWeapon *pWeapon = GetActiveWeapon();
	if ( pWeapon )
	{
		pWeapon->GetViewmodelBoneControllers( this, controllers );
	}
}

void C_BaseViewModel::UpdateAllViewmodelAddons( void )
{
	C_CSPlayer *pPlayer = dynamic_cast<C_CSPlayer*>( GetOwner() );

	// Remove any view model add ons if we're spectating.
	if ( !pPlayer )
	{
		RemoveViewmodelArmModels();
		RemoveViewmodelStatTrak();
		return;
	}

	CWeaponCSBase* pCSWeapon = dynamic_cast<CWeaponCSBase*>( pPlayer->GetActiveWeapon() );
	if ( !pCSWeapon )
	{
		RemoveViewmodelArmModels();
		RemoveViewmodelStatTrak();
		return;
	}

	CStudioHdr *pHdr = pPlayer->GetModelPtr();
#if 0
	if ( pHdr )
	{
		// PiMoN: help me
		// p.s. I bet this thing will fucking destroy everyone's performance
		// BUT I HAVE NO OTHER CHOICE BECAUSE FUCK THOSE ASSHOLES AT VALVE
		// WHY IS A FUCKING player_spawn EVENT NOT BEING RECEIVED BY CLIENT
		if ( pPlayer->m_pViewmodelArmConfig != GetPlayerViewmodelArmConfigForPlayerModel( pHdr->pszName() ) )
			pPlayer->m_pViewmodelArmConfig = NULL;
	}
#endif

	if ( pPlayer->m_pViewmodelArmConfig == NULL )
	{
		RemoveViewmodelArmModels();
		RemoveViewmodelStatTrak();

		if ( pHdr )
		{
			pPlayer->m_pViewmodelArmConfig = GetPlayerViewmodelArmConfigForPlayerModel( pHdr->pszName() );
		}
	}

	if ( pPlayer->m_bNeedToChangeGloves )
		RemoveViewmodelArmModels();
	
	// add gloves and sleeves
	if ( m_vecViewmodelArmModels.Count() == 0 )
	{
		if ( CSLoadout()->HasGlovesSet( pPlayer, pPlayer->GetTeamNumber() ) )
		{
			AddViewmodelArmModel( GetGlovesInfo( CSLoadout()->GetGlovesForPlayer( pPlayer, pPlayer->GetTeamNumber() ) )->szViewModel, pPlayer->m_pViewmodelArmConfig->iSkintoneIndex, pPlayer->m_pViewmodelArmConfig->bHideBareArms );
			if ( pPlayer->m_pViewmodelArmConfig->szAssociatedSleeveModelGloveOverride[0] != NULL )
				AddViewmodelArmModel( pPlayer->m_pViewmodelArmConfig->szAssociatedSleeveModelGloveOverride );
			else
				AddViewmodelArmModel( pPlayer->m_pViewmodelArmConfig->szAssociatedSleeveModel );
		}
		else
		{
			AddViewmodelArmModel( pPlayer->m_pViewmodelArmConfig->szAssociatedGloveModel, pPlayer->m_pViewmodelArmConfig->iSkintoneIndex, pPlayer->m_pViewmodelArmConfig->bHideBareArms );
			AddViewmodelArmModel( pPlayer->m_pViewmodelArmConfig->szAssociatedSleeveModel );
		}
	}


	// verify stattrak module and add if necessary
	if ( loadout_stattrak.GetBool() && pCSWeapon->GetCSWpnData().m_szStatTrakModel && pCSWeapon->GetCSWpnData().m_szStatTrakModel[0] )
	{
		AddViewmodelStatTrak( pCSWeapon );
	}
	else
		RemoveViewmodelStatTrak();
}

//--------------------------------------------------------------------------------------------------------
C_ViewmodelAttachmentModel* C_BaseViewModel::AddViewmodelArmModel( const char *pszArmsModel, int nSkintoneIndex, bool bHideBareArms )
{
	// Only create the view model attachment if we have a valid arm model
	if ( pszArmsModel == NULL || pszArmsModel[0] == '\0' || modelinfo->GetModelIndex( pszArmsModel ) == -1 )
		return NULL;

	C_ViewmodelAttachmentModel *pEnt = new class C_ViewmodelAttachmentModel;
	if ( pEnt && pEnt->InitializeAsClientEntity( pszArmsModel, RENDER_GROUP_VIEW_MODEL_OPAQUE ) )
	{
		m_vecViewmodelArmModels[ m_vecViewmodelArmModels.AddToTail() ] = pEnt;

		if ( nSkintoneIndex != -1 )
			pEnt->m_nSkin = nSkintoneIndex;

		// magic tricks to get 0.01 more fps on potato PCs
		if ( pEnt->FindBodygroupByName( "bare" ) != -1 )
			pEnt->SetBodygroup( pEnt->FindBodygroupByName( "bare" ), bHideBareArms );

		pEnt->SetParent( this );
		pEnt->SetLocalOrigin( vec3_origin );
		pEnt->UpdatePartitionListEntry();
		pEnt->CollisionProp()->MarkPartitionHandleDirty();
		pEnt->UpdateVisibility();
		RemoveEffects( EF_NODRAW );
		return pEnt;
	}	

	return NULL;
}

void C_BaseViewModel::AddViewmodelStatTrak( CWeaponCSBase *pWeapon )
{
	// PiMoN: commenting this out to pretend that it "fixes" a bug with stattrak model not updating in time on weapon switch
	/*if ( m_viewmodelStatTrakAddon && m_viewmodelStatTrakAddon.Get() && m_viewmodelStatTrakAddon->GetMoveParent() )
		return;*/

	RemoveViewmodelStatTrak();

	if (!pWeapon)
		return;

	if ( !pWeapon->GetCSWpnData().m_szStatTrakModel && !pWeapon->GetCSWpnData().m_szStatTrakModel[0] )
		return;

	C_ViewmodelAttachmentModel *pStatTrakEnt = new class C_ViewmodelAttachmentModel;
	if ( pStatTrakEnt && pStatTrakEnt->InitializeAsClientEntity( pWeapon->GetCSWpnData().m_szStatTrakModel, RENDER_GROUP_VIEW_MODEL_OPAQUE ) )
	{
		m_viewmodelStatTrakAddon = pStatTrakEnt;
		pStatTrakEnt->SetParent( this );
		pStatTrakEnt->SetLocalOrigin( vec3_origin );
		pStatTrakEnt->UpdatePartitionListEntry();
		pStatTrakEnt->CollisionProp()->MarkPartitionHandleDirty();
		pStatTrakEnt->UpdateVisibility();
		pStatTrakEnt->AddEffects( EF_BONEMERGE );
		pStatTrakEnt->AddEffects( EF_BONEMERGE_FASTCULL );
		pStatTrakEnt->AddEffects( EF_NODRAW );

		if ( !cl_righthand.GetBool() )
		{
			pStatTrakEnt->SetBodygroup( 0, 1 ); // use a special mirror-image stattrak module that appears correct for lefties
		}
	}
}

void C_BaseViewModel::RemoveViewmodelArmModels( void )
{
	FOR_EACH_VEC_BACK( m_vecViewmodelArmModels, i )
	{
		C_ViewmodelAttachmentModel *pEnt = m_vecViewmodelArmModels[i].Get();
		if ( pEnt )
		{
			pEnt->Remove();
		}
	}
	m_vecViewmodelArmModels.RemoveAll();
}

void C_BaseViewModel::RemoveViewmodelStatTrak( void )
{
	C_ViewmodelAttachmentModel *pStatTrakEnt = m_viewmodelStatTrakAddon.Get();
	if ( pStatTrakEnt )
	{
		pStatTrakEnt->Remove();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : RenderGroup_t
//-----------------------------------------------------------------------------
RenderGroup_t C_BaseViewModel::GetRenderGroup()
{
	return RENDER_GROUP_VIEW_MODEL_OPAQUE;
}


bool C_ViewmodelAttachmentModel::InitializeAsClientEntity( const char *pszModelName, RenderGroup_t renderGroup )
{
	if ( !BaseClass::InitializeAsClientEntity( pszModelName, renderGroup ) )
		return false;

	AddEffects( EF_BONEMERGE );
	AddEffects( EF_BONEMERGE_FASTCULL );
	AddEffects( EF_NODRAW );
	return true;
}

int C_ViewmodelAttachmentModel::InternalDrawModel( int flags )
{
	CMatRenderContextPtr pRenderContext( materials );

	C_BaseViewModel *pViewmodel = (C_BaseViewModel*)GetFollowedEntity();
	if ( pViewmodel && pViewmodel->ShouldFlipViewModel() )
		pRenderContext->CullMode( MATERIAL_CULLMODE_CW );

	int r = BaseClass::InternalDrawModel( flags );

	pRenderContext->CullMode( MATERIAL_CULLMODE_CCW );

	return r;
}