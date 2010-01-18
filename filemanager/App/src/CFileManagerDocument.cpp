/*
* Copyright (c) 2002-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Document class of the file manager
*
*/


// INCLUDE FILES
#include <gulicon.h>        // to make iIconArray->ResetAndDestroy work
#include <coemain.h>
#include <apgwgnam.h>
#include <data_caging_path_literals.hrh> 
#include <FileManagerUID.h>
#include <CFileManagerIconArray.h>
#include <CFileManagerEngine.h>
#include <FileManagerDebug.h>
#include <CFileManagerUtils.h>
#include "CFileManagerDocument.h"
#include "CFileManagerAppUi.h"
#include "CFileManagerStringCache.h"


// CONSTANTS
_LIT( KFileManagerEngineResource, "filemanagerengine.rsc" );
_LIT( KFileManagerViewResource, "filemanagerview.rsc" );
_LIT_SECURE_ID( KFileManagerSchBkupSID, KFileManagerSchBkupUID3 );


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerDocument::CFileManagerDocument
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFileManagerDocument::CFileManagerDocument( CEikApplication& aApp ) :
    CAiwGenericParamConsumer( aApp ),
    iViewResourceLoader( *CCoeEnv::Static() ),
    iEngineResourceLoader( *CCoeEnv::Static() )
    {
    FUNC_LOG
    }

// -----------------------------------------------------------------------------
// CFileManagerDocument::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CFileManagerDocument::ConstructL()
    {
    FUNC_LOG

    // Get resource drive from exe location
    TFileName exeFileName( RProcess().FileName() );
    TParsePtrC exeParse( exeFileName );
    TPtrC exeDrive( exeParse.Drive() );

    TFileName fileName;
    fileName.Copy( exeDrive );
    fileName.Append( KDC_RESOURCE_FILES_DIR );
    CFileManagerUtils::EnsureFinalBackslash( fileName );
    fileName.Append( KFileManagerViewResource );
    iViewResourceLoader.OpenL( fileName );

    fileName.Copy( exeDrive );
    fileName.Append( KDC_RESOURCE_FILES_DIR );
    CFileManagerUtils::EnsureFinalBackslash( fileName );
    fileName.Append( KFileManagerEngineResource );
    iEngineResourceLoader.OpenL( fileName );

    INFO_LOG( "CFileManagerDocument::ConstructL()-Create engine" )
    RFs& fs( CCoeEnv::Static()->FsSession() );
    User::LeaveIfError( fs.ShareProtected() ); // Make shareable
    iEngine = CFileManagerEngine::NewL( fs );

    INFO_LOG( "CFileManagerDocument::ConstructL()-Create icon array" )
    iIconArray = CFileManagerIconArray::NewL();

    INFO_LOG( "CFileManagerDocument::ConstructL()-Create string cache" )
    iStringCache = CFileManagerStringCache::NewL( *iEngine, *iIconArray );
    }

// -----------------------------------------------------------------------------
// CFileManagerDocument::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFileManagerDocument* CFileManagerDocument::NewL( CEikApplication& aApp )
    {
    CFileManagerDocument* self = new( ELeave ) CFileManagerDocument( aApp );
    
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerDocument::~CFileManagerDocument
// Destructor
// -----------------------------------------------------------------------------
// 
CFileManagerDocument::~CFileManagerDocument()
    {
    FUNC_LOG

    DeletePlugins();
    iPluginArray.Close();
    REComSession::FinalClose();
    iViewResourceLoader.Close();
    iEngineResourceLoader.Close();
    if ( iIconArray )
        {
        iIconArray->ResetAndDestroy();
        delete iIconArray;
        }
    delete iStringCache;
    delete iEngine;
    }

// -----------------------------------------------------------------------------
// CFileManagerDocument::CreateAppUiL
// 
// -----------------------------------------------------------------------------
// 
CEikAppUi* CFileManagerDocument::CreateAppUiL()
    {
    return new( ELeave ) CFileManagerAppUi;
    }

// -----------------------------------------------------------------------------
// CFileManagerDocument::IconArray
// 
// -----------------------------------------------------------------------------
// 
CFileManagerIconArray* CFileManagerDocument::IconArray() const
    {
    return iIconArray;
    }

// -----------------------------------------------------------------------------
// CFileManagerDocument::Engine
// 
// -----------------------------------------------------------------------------
// 
CFileManagerEngine& CFileManagerDocument::Engine() const
    {
    return *iEngine;
    }

// -----------------------------------------------------------------------------
// CFileManagerDocument::ClearStringCache
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerDocument::ClearStringCache()
    {
    iStringCache->Clear();
    }

// -----------------------------------------------------------------------------
// CFileManagerDocument::FileList
// 
// -----------------------------------------------------------------------------
// 
MDesCArray* CFileManagerDocument::FileList() const
    {
    return iStringCache;
    }

// -----------------------------------------------------------------------------
// CFileManagerDocument::LastError
// 
// -----------------------------------------------------------------------------
// 
TInt CFileManagerDocument::LastError() const
    {
    return iStringCache->LastError();
    }

// -----------------------------------------------------------------------------
// CFileManagerDocument::UpdateTaskNameL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerDocument::UpdateTaskNameL( CApaWindowGroupName* aWgName )
	{
    CAknDocument::UpdateTaskNameL( aWgName );
#ifdef RD_FILE_MANAGER_BACKUP
    if ( IsScheduledBackup() )
        {
        aWgName->SetHidden( ETrue );
        }
#endif // RD_FILE_MANAGER_BACKUP
	}

// -----------------------------------------------------------------------------
// CFileManagerDocument::IsScheduledBackup
// 
// -----------------------------------------------------------------------------
// 
TBool CFileManagerDocument::IsScheduledBackup()
    {
    // Check is started by file manager schedule starter
    return ( User::CreatorSecureId() == KFileManagerSchBkupSID );
    }

// -----------------------------------------------------------------------------
// CFileManagerDocument::StorePluginL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerDocument::StorePluginL( const TUid& aUid )
    {
    iPluginArray.AppendL( aUid );
    }

// -----------------------------------------------------------------------------
// CFileManagerDocument::DeletePlugins
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerDocument::DeletePlugins()
    {
    TInt count( iPluginArray.Count() );
    for ( TInt i( 0 ); i < count; ++i )
        {
        REComSession::DestroyedImplementation( iPluginArray[ i ] );
        }
    iPluginArray.Reset();
    }

// -----------------------------------------------------------------------------
// CFileManagerDocument::OpenFileL()
//
// -----------------------------------------------------------------------------
CFileStore* CFileManagerDocument::OpenFileL(
        TBool /*aDoOpen*/, const TDesC& /*aFilename*/, RFs& /*aFs*/ )
    {
    const CAiwGenericParamList* inParams = GetInputParameters();
    CFileManagerAppUi* appUi = static_cast< CFileManagerAppUi* >( iAppUi );
    if ( appUi && inParams )
        {
        appUi->ProcessAiwParamListL( *inParams );
        }
     return NULL;
    }

//  End of File  
