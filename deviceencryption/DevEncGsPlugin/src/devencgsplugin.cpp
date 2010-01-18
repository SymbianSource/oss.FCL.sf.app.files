/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0""
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  GSProfilesPlugin implementation.
*
*/


// User includes
#include    "devencgsplugin.h"
#include    <GSParentPlugin.h>
#include    <GSCommon.hrh>
#include    <devencgspluginrsc.rsg> // GUI Resource
//#include    <DevEncUi.rsg>
#include    <devencgsplugin.mbg>
#include    <GSPrivatePluginProviderIds.h>
#include    <hwrmvibrasdkcrkeys.h>
#include    <AknLaunchAppService.h>
#include    <AiwCommon.h>
// System includes
#include    <AknNullService.h>
#include    <bautils.h>
#include    <StringLoader.h>

// ========================= MEMBER FUNCTIONS ================================

// ---------------------------------------------------------------------------
// CDeviceEncryptionGsPlugin::CDeviceEncryptionGsPlugin
//
// ---------------------------------------------------------------------------
//
CDeviceEncryptionGsPlugin::CDeviceEncryptionGsPlugin()
    : iResources( *iCoeEnv ), iNullService(NULL)
    {
    }


// ---------------------------------------------------------------------------
// CDeviceEncryptionGsPlugin::~CDeviceEncryptionGsPlugin
//
// ---------------------------------------------------------------------------
//
CDeviceEncryptionGsPlugin::~CDeviceEncryptionGsPlugin()
    {
    iResources.Close();

    if ( iNullService )
        {
        delete iNullService;
        }
    }


// ---------------------------------------------------------------------------
// CDeviceEncryptionGsPlugin::ConstructL
//
// ---------------------------------------------------------------------------
//
void CDeviceEncryptionGsPlugin::ConstructL()
    {
    OpenLocalizedResourceFileL( KDeviceEncryptionGsPluginResourceFileName, iResources );
    }


// ---------------------------------------------------------------------------
// CDeviceEncryptionGsPlugin::NewL
//
// ---------------------------------------------------------------------------
//
CDeviceEncryptionGsPlugin* CDeviceEncryptionGsPlugin::NewL( TAny* /*aInitParams*/ )
    {
    CDeviceEncryptionGsPlugin* self = new ( ELeave ) CDeviceEncryptionGsPlugin();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }


// -----------------------------------------------------------------------------
// CDeviceEncryptionGsPlugin::Id (from CGSPluginInterface)
//
// -----------------------------------------------------------------------------
//
TUid CDeviceEncryptionGsPlugin::Id() const
    {
    return KDeviceEncryptionGsPluginImplUID;
    }


// -----------------------------------------------------------------------------
// CDeviceEncryptionGsPlugin::DoActivateL (from CGSPluginInterface)
//
// -----------------------------------------------------------------------------
//
void CDeviceEncryptionGsPlugin::DoActivateL( const TVwsViewId& /*aPrevViewId*/,
                                  TUid /*aCustomMessageId*/,
                                  const TDesC8& /*aCustomMessage*/ )
    {
    }


// -----------------------------------------------------------------------------
// CDeviceEncryptionGsPlugin::DoDeactivate (from CGSPluginInterface)
//
// -----------------------------------------------------------------------------
//
void CDeviceEncryptionGsPlugin::DoDeactivate()
    {
    }



// -----------------------------------------------------------------------------
// CDeviceEncryptionGsPlugin::GetCaptionL (from CGSPluginInterface)
//
// -----------------------------------------------------------------------------
//
void CDeviceEncryptionGsPlugin::GetCaptionL( TDes& aCaption ) const
    {
//    HBufC* result = iEikonEnv->AllocReadResourceAsDes16L( R_GS_DE_PLUGIN_CAPTION );
    HBufC* result = StringLoader::LoadL( R_GS_DE_PLUGIN_CAPTION );
    
    if (result->Des().Length() < aCaption.MaxLength())
        {
        aCaption.Copy( *result );
        }
    else
        {
        aCaption = KNullDesC;
        }
   
    delete result;
//    aCaption.Copy( _L("Device Encryption") );
    }


// -----------------------------------------------------------------------------
// CDeviceEncryptionGsPlugin::PluginProviderCategory (from CGSPluginInterface)
//
// -----------------------------------------------------------------------------
//
TInt CDeviceEncryptionGsPlugin::PluginProviderCategory() const
    {
    return KGSPluginProviderInternal;
    }


// -----------------------------------------------------------------------------
// CDeviceEncryptionGsPlugin::ItemType (from CGSPluginInterface)
//
// -----------------------------------------------------------------------------
//
TGSListboxItemTypes CDeviceEncryptionGsPlugin::ItemType()
    {
    return EGSItemTypeSettingDialog;
    }


// -----------------------------------------------------------------------------
// CDeviceEncryptionGsPlugin::GetValue (from CGSPluginInterface)
//
// -----------------------------------------------------------------------------
//
void CDeviceEncryptionGsPlugin::GetValue( const TGSPluginValueKeys /*aKey*/,
                                      TDes& /*aValue*/ )
    {
    }


// -----------------------------------------------------------------------------
// CDeviceEncryptionGsPlugin::HandleSelection (from CGSPluginInterface)
//
// -----------------------------------------------------------------------------
//
void CDeviceEncryptionGsPlugin::HandleSelection(
    const TGSSelectionTypes /*aSelectionType*/ )
    {
    //TRAP_IGNORE( LaunchDEAppL() );
    LaunchDEAppL();
    }

// ---------------------------------------------------------------------------
// CDeviceEncryptionGsPlugin::CreateIconL (from CGSPluginInterface)
//
// ---------------------------------------------------------------------------
//

CGulIcon* CDeviceEncryptionGsPlugin::CreateIconL( const TUid aIconType )
    {
    CGulIcon* icon;
    TParse* fp = new ( ELeave ) TParse();
    CleanupStack::PushL( fp );
    fp->Set( KDeviceEncryptionGsPluginIconDirAndName, &KDC_BITMAP_DIR, NULL );

    if ( aIconType == KGSIconTypeLbxItem )
        {
        icon = AknsUtils::CreateGulIconL(
        AknsUtils::SkinInstance(),
        KAknsIIDQgnPropCpPersoProf,
        //KAknsIIDQgnPropCpDevenc,
        fp->FullName(),
        EMbmDevencgspluginQgn_prop_cp_devenc,
        EMbmDevencgspluginQgn_prop_cp_devenc_mask );
        }    
    else
        {
        icon = CGSPluginInterface::CreateIconL( aIconType );
        }

    CleanupStack::PopAndDestroy( fp );

    return icon;
    }


// -----------------------------------------------------------------------------
// CDeviceEncryptionGsPlugin::OpenLocalizedResourceFileL
//
// -----------------------------------------------------------------------------

void CDeviceEncryptionGsPlugin::OpenLocalizedResourceFileL(
    const TDesC& aResourceFileName,
    RConeResourceLoader& aResourceLoader )
    {
    RFs fsSession;
    User::LeaveIfError( fsSession.Connect() );
    CleanupClosePushL(fsSession);

    // Find the resource file:
    TParse parse;
    parse.Set( aResourceFileName, &KDC_RESOURCE_FILES_DIR, NULL );
    TFileName fileName( parse.FullName() );

    // Get language of resource file:
    BaflUtils::NearestLanguageFile( fsSession, fileName );

    // Open resource file:
    aResourceLoader.OpenL( fileName );

    CleanupStack::PopAndDestroy(&fsSession);
    }


// -----------------------------------------------------------------------------
// CDeviceEncryptionGsPlugin::LaunchProfilesAppL
//
// -----------------------------------------------------------------------------
//
void CDeviceEncryptionGsPlugin::LaunchDEAppL()
    {
    // Get the correct application data
    RWsSession ws;
    User::LeaveIfError(ws.Connect());
    CleanupClosePushL(ws);

    // Find the task with uid
    TApaTaskList taskList(ws);
    TApaTask task = taskList.FindApp( KDeviceEncryptionAppUid );

    if ( task.Exists() )
        {
        task.BringToForeground();
        }
    else
        {
        //Launch application as embedded
        TAppInfo app( KDeviceEncryptionAppUid, KDeviceEncryptionApp );
        iEmbedded=NULL;
        EmbedAppL( app );
        }
    CleanupStack::PopAndDestroy(&ws);
    }


// -----------------------------------------------------------------------------
// CDeviceEncryptionGsPlugin::EmbedAppL
//
// -----------------------------------------------------------------------------
//
void CDeviceEncryptionGsPlugin::EmbedAppL( const TAppInfo& aApp )
    {
    if ( iNullService )
        {
        delete iNullService;
        iNullService = NULL;
        }
    iNullService = CAknNullService::NewL( aApp.iUid, this );
    }

// End of file
