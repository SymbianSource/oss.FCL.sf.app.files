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
* Description:  CDevEncPasswdDlg implementation.
*
*/

#include "DevEncPasswdDlg.h"
#include "DevEncUids.hrh"
#include "DevEncDef.h"
#include <eiksrvui.h>
#include <DevEncNotifPlugin.rsg>
#include <StringLoader.h>
#include <bautils.h>         // BAFL utils (for language file)
#include <data_caging_path_literals.hrh>
#include <utf.h>
#include "DevEncLog.h"

#define KErrorNoteTimeout   2000000
_LIT(KFileDrive,"z:");
_LIT(KResourceFileName, "DevEncNotifPlugin.rsc");

CDevEncPasswdDlg::CDevEncPasswdDlg() : 
    CActive( EPriorityStandard ),	// Standard priority
    iNeedToCompleteMessage( EFalse )
    {
    }

CDevEncPasswdDlg* CDevEncPasswdDlg::NewLC()
    {
	CDevEncPasswdDlg* self = new ( ELeave ) CDevEncPasswdDlg();
	CleanupStack::PushL( self );
	self->ConstructL();
	return self;
    }

CDevEncPasswdDlg* CDevEncPasswdDlg::NewL()
    {
	CDevEncPasswdDlg* self = CDevEncPasswdDlg::NewLC();
	CleanupStack::Pop(); // self;
	return self;
    }

void CDevEncPasswdDlg::ConstructL()
    {
    DFLOG( "CDevEncPasswdDlg::ConstructL Begin" );
	CActiveScheduler::Add( this );				// Add to scheduler
    
    iEikEnv = CEikonEnv::Static();
    iTimer.CreateLocal();
    TFileName filename;
    filename += KFileDrive;
    filename += KDC_RESOURCE_FILES_DIR; // From data_caging_path_literals.hrh
    filename += KResourceFileName;
    BaflUtils::NearestLanguageFile( iEikEnv->FsSession(), filename );
    iResourceFileFlag = iEikEnv->AddResourceFileL( filename );
    DFLOG( "CDevEncPasswdDlg::ConstructL End" );
    }

CDevEncPasswdDlg::~CDevEncPasswdDlg()
    {
    DFLOG( "CDevEncPasswdDlg::~CDevEncPasswdDlg Begin" );
    Cancel();
    iEikEnv->DeleteResourceFile( iResourceFileFlag );
    if ( iNeedToCompleteMessage )
        {
        iMessage.Complete( KErrDied );
        }
    iTimer.Close();
    DFLOG( "CDevEncPasswdDlg::~CDevEncPasswdDlg End" );
    }

void CDevEncPasswdDlg::DoCancel()
    {

    }

void CDevEncPasswdDlg::Release()
    {
    delete this;  
    }
   
CDevEncPasswdDlg::TNotifierInfo CDevEncPasswdDlg::RegisterL()
    {
    DFLOG( "CDevEncPasswdDlg::RegisterL Begin" );
    iInfo.iUid = TUid::Uid( KDevEncPasswdDlgUid );
    iInfo.iChannel = TUid::Uid( KDevEncPasswdDlgUid );
    iInfo.iPriority = ENotifierPriorityVHigh;
    DFLOG( "CDevEncPasswdDlg::RegisterL End" );
    return iInfo;
    }

CDevEncPasswdDlg::TNotifierInfo CDevEncPasswdDlg::Info() const
    {
    return iInfo;
    }

TPtrC8 CDevEncPasswdDlg::StartL(const TDesC8& /*aBuffer*/)
    {
    return TPtrC8();
    }

void CDevEncPasswdDlg::StartL(const TDesC8& /*aBuffer*/, TInt aReplySlot, const RMessagePtr2& aMessage)
    {
    DFLOG( "CDevEncPasswdDlg::StartL Begin" );
    iMessage = aMessage;
    iReplySlot = aReplySlot;
    iNeedToCompleteMessage = ETrue;
    
    SetActive();
    iStatus = KRequestPending;
    TRequestStatus* stat = &iStatus;
    User::RequestComplete( stat, KErrNone );
    DFLOG( "CDevEncPasswdDlg::StartL End" );
    }

void CDevEncPasswdDlg::Cancel()
    {
    CActive::Cancel();
    }

TPtrC8 CDevEncPasswdDlg::UpdateL(const TDesC8& /*aBuffer*/)
    {
    return TPtrC8();    
    }
   
   
void CDevEncPasswdDlg::RunL()
    {
    DFLOG( "CDevEncPasswdDlg::RunL Begin" );
    TBuf<KMaxPasswordLength> passwdIn;
    TBuf8<KMaxPasswordLength> passwdOut;

    STATIC_CAST( CEikServAppUi*, iEikEnv->AppUi())->
        SuppressAppSwitching( ETrue );
    
    iDlg = CAknTextQueryDialog::NewL( passwdIn );
    iDlg->SetMaxLength( KMaxPasswordLength );
    DFLOG( "CDevEncPasswdDlg: PasswdDlg: ExecuteLD" );
    TInt resp = iDlg->ExecuteLD( R_DEVENC_PASSWD_QUERY );
    iDlg = NULL;
    
    if ( resp == EEikBidCancel )
        {
        iMessage.Complete( KErrCancel );        
        iNeedToCompleteMessage = EFalse;
        }
    else
        {
        if ( CnvUtfConverter::ConvertFromUnicodeToUtf8( passwdOut, passwdIn ) )
            {
            iNote = new (ELeave) CAknErrorNote();
            HBufC* prompt = StringLoader::LoadLC( R_DEVENC_INVALID_PASSWD_STRING );
            iNote->ExecuteLD( *prompt );
            iNote = NULL;
            CleanupStack::PopAndDestroy( prompt );

            iTimer.After( iStatus, KErrorNoteTimeout );
            SetActive();
            return;
            }
        else
            {
            iMessage.WriteL( iReplySlot, passwdOut );
            iMessage.Complete( KErrNone );            
            iNeedToCompleteMessage = EFalse;
            }
        }
    STATIC_CAST( CEikServAppUi*, iEikEnv->AppUi())->
        SuppressAppSwitching( EFalse );
    DFLOG( "CDevEncPasswdDlg::RunL End" );
    }

TInt CDevEncPasswdDlg::RunError( TInt aError )
    {
    if( iNeedToCompleteMessage )
        {
        iMessage.Complete( aError );
        }
        
    iNeedToCompleteMessage = EFalse;
    iReplySlot = NULL;
    STATIC_CAST( CEikServAppUi*, iEikEnv->AppUi())->
        SuppressAppSwitching( EFalse );

	return aError;
    }

// End of file

