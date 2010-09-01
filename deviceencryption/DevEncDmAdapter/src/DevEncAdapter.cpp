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
* Description:  SW Installer DM Adapter.
*
*/

// INCLUDE
#include <implementationproxy.h>
#include <smldmadapter.h>
#include <data_caging_path_literals.hrh>
#include <centralrepository.h>
#include <hwrmpowerstatesdkpskeys.h> // For power state
#include <DevEncExternalCRKeys.h>
#include <DevEncDef.h>
#include "DevEncLog.h" 
#include "DevEncAdapter.h"
#include "DevEncUids.hrh"


// CONSTANTS
_LIT8( KIntType,"int" ); // Leaf inputs
_LIT8( KTextType,"text/plain" ); // Leaf inputs
_LIT8( KDevEncAdapterVersion, "1.0" ); // Adapter version
_LIT8( KDevEncRootNode, "DevEnc" ); // root URI
_LIT8( KPhoneMemoryCmdNode, "PhoneMemoryCmd" ); // URI postfix
_LIT8( KMemoryCardCmdNode, "MemoryCardCmd" ); // URI postfix
_LIT8( KPhoneMemoryStatusNode, "PhoneMemoryStatus" ); // URI postfix
_LIT8( KMemoryCardStatusNode, "MemoryCardStatus" ); // URI postfix
_LIT8( KMemoryCardEncKeyNode, "MemoryCardEncKey" ); // URI postfix
_LIT8( KUIStateNode, "UIState" ); // URI postfix
_LIT8( KPhoneMemoryProgNode, "PhoneMemoryProgress" ); // URI postfix
_LIT8( KMemoryCardProgNode, "MemoryCardProgress" ); // URI postfix
_LIT8( KPhoneMemoryCmdDescription, "Phone memory encryption command" ); // Description
_LIT8( KMemoryCardCmdDescription, "Memory card encryption command" ); // Description
_LIT8( KPhoneMemoryStatusDescription, "Phone memory encryption status" ); // Description
_LIT8( KMemoryCardStatusDescription, "Memory card encryption status" ); // Description
_LIT8( KMemoryCardEncKeyDescription, "Memory card encryption key" ); // Description
_LIT8( KUIStateDescription, "State of encryption settings" ); // Description
_LIT8( KPhoneMemoryProgDescription, "En/Decryption progress in percents (phone)" ); // Description
_LIT8( KMemoryCardProgDescription, "En/Decryption progress in percents (memory card)" ); // Description
_LIT8( KSeparator, "/" );

static const TUint32 KSizeOfSettingId = 16; // Contanst size declaration
static const TInt KBufGranularity = 8;
static const TDriveNumber KPhoneMemoryDriveNum = EDriveC;
static const TDriveNumber KMemoryCardDriveNum = EDriveF;
static const TUid KPSCategory = { 0x101F9A02 }; // DM client SID
static const TInt KPkcs5PSKey = KDevEncOmaDmPluginImplUid;

// ============================= MEMBER FUNCTIONS =============================

// ----------------------------------------------------------------------------
// CDevEncAdapter::NewL
// Symbian 1st phase contructor
// (static, may leave)
// Status : Draft
// ----------------------------------------------------------------------------
//
CDevEncAdapter* CDevEncAdapter::NewL( MSmlDmCallback* aCallback )
    {
    CDevEncAdapter* self = NewLC( aCallback );
    CleanupStack::Pop( self );
    return self;
    }

// ----------------------------------------------------------------------------
// CDevEncAdapter::NewLC
// Symbian 1st phase contructor. Push object to cleanup-stack
// (static, may leave)
// Status : Draft
// ----------------------------------------------------------------------------
//
CDevEncAdapter* CDevEncAdapter::NewLC( MSmlDmCallback* aCallback )
    {
    CDevEncAdapter* self = new ( ELeave ) CDevEncAdapter( aCallback );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ----------------------------------------------------------------------------
// CDevEncAdapter::CDevEncAdapter()
// C++ Constructor
// Status : Draft
// ----------------------------------------------------------------------------
CDevEncAdapter::CDevEncAdapter( TAny* aEcomArguments )
    : CSmlDmAdapter::CSmlDmAdapter( aEcomArguments ),
        iPhoneMemOpPending( EFalse ),
        iMemCardOpPending( EFalse )
    {
    DFLOG( "CDevEncAdapter Constructor" );
    }

// ----------------------------------------------------------------------------
// CDevEncAdapter::ConstructL
// 2nd phase contructor
// (may leave)
// Status : Draft
// ----------------------------------------------------------------------------
//
void CDevEncAdapter::ConstructL()
    {
    DFLOG( "CDevEncAdapter::ConstructL Begin" );
    iDiskUtils = new (ELeave) CDevEncDiskUtils();
    iKeyUtils  = new (ELeave) CDevEncKeyUtils();
    iPhoneMemorySession = new (ELeave) CDevEncSession( KPhoneMemoryDriveNum );
    iMemoryCardSession  = new (ELeave) CDevEncSession( KMemoryCardDriveNum );
    User::LeaveIfError(iPhoneMemorySession->Connect());
    User::LeaveIfError(iMemoryCardSession->Connect());
    iTimer.CreateLocal();
    iUiCenRep = CRepository::NewL( TUid::Uid( KCRDevEncUiSettings ) );
    DFLOG( "CDevEncAdapter::ConstructL End" );
    }

// ----------------------------------------------------------------------------
// CDevEncAdapter::~CDevEncAdapter()
// C++ Destructor
// Status : Draft
// ----------------------------------------------------------------------------
//
CDevEncAdapter::~CDevEncAdapter()
    {
    DFLOG( "CDevEncAdapter Destructor Begin" );
    if (iDiskUtils)
        {
        delete iDiskUtils;
        iDiskUtils = NULL;
        }
    
    if (iKeyUtils)
        {
        delete iKeyUtils;
        iKeyUtils = NULL;
        }
    
    if (iPhoneMemorySession)
        {
        iPhoneMemorySession->Close();
        delete iPhoneMemorySession;
        iPhoneMemorySession = NULL;
        }
        
    if (iMemoryCardSession)
        {
        iMemoryCardSession->Close();
        delete iMemoryCardSession;
        iMemoryCardSession = NULL;
        }
    
    if ( iPkcs5Key )
        {
        delete iPkcs5Key;
        iPkcs5Key = NULL;
        }
    if (iTimer.Handle())
        iTimer.Close();
    
    if (iUiCenRep)
        {
        delete iUiCenRep;
        iUiCenRep = NULL;
        }
    DFLOG( "CDevEncAdapter Destructor End" );
    }

// ----------------------------------------------------------------------------
// CDevEncAdapter::DDFVersionL
// Return DM plug-in version
// (may leave)
// Status : Draft
// ----------------------------------------------------------------------------
//
void CDevEncAdapter::DDFVersionL( CBufBase& aDDFVersion )
    {
    // Insert version information
    DFLOG( "CDevEncAdapter::DDFVersionL Begin" );
    aDDFVersion.InsertL( 0, KDevEncAdapterVersion );
    DFLOG( "CDevEncAdapter::DDFVersionL End" );
    }

// ----------------------------------------------------------------------------
// CDevEncAdapter::DDFStructureL
// Return DM plug-in structure
// (may leave)
// Status : Draft
// ----------------------------------------------------------------------------
//
void CDevEncAdapter::DDFStructureL( MSmlDmDDFObject& aDDF )
    {
    // Declare accesses
    DFLOG( "CDevEncAdapter::DDFStructureL Begin" );
    TSmlDmAccessTypes accessTypes;
    accessTypes.SetGet();
    
    // Create root node 
    MSmlDmDDFObject& root = aDDF.AddChildObjectL( KDevEncRootNode );
    FillNodeInfoL( root,
                   accessTypes,
                   MSmlDmDDFObject::EPermanent,
                   MSmlDmDDFObject::ENode,
                   MSmlDmDDFObject::EOne,
                   KNullDesC8(),
                   KNullDesC8() );

    // Create leaf nodes

    accessTypes.Reset();
    accessTypes.SetReplace(); // Command nodes are write-only

    MSmlDmDDFObject& node1 = root.AddChildObjectL( KPhoneMemoryCmdNode );
    FillNodeInfoL( node1,
                   accessTypes,
                   MSmlDmDDFObject::EPermanent,
                   MSmlDmDDFObject::EInt,
                   MSmlDmDDFObject::EOne,
                   KPhoneMemoryCmdDescription,
                   KIntType );

    MSmlDmDDFObject& node2 = root.AddChildObjectL( KMemoryCardCmdNode );
    FillNodeInfoL( node2,
                   accessTypes,
                   MSmlDmDDFObject::EPermanent,
                   MSmlDmDDFObject::EInt,
                   MSmlDmDDFObject::EOne,
                   KMemoryCardCmdDescription,
                   KIntType );

    accessTypes.Reset();
    accessTypes.SetGet(); // Status nodes are read-only

    MSmlDmDDFObject& node3 = root.AddChildObjectL( KPhoneMemoryStatusNode );
    FillNodeInfoL( node3,
                   accessTypes,
                   MSmlDmDDFObject::EPermanent,
                   MSmlDmDDFObject::EInt,
                   MSmlDmDDFObject::EOne,
                   KPhoneMemoryStatusDescription,
                   KIntType );

    MSmlDmDDFObject& node4 = root.AddChildObjectL( KMemoryCardStatusNode );
    FillNodeInfoL( node4,
                   accessTypes,
                   MSmlDmDDFObject::EPermanent,
                   MSmlDmDDFObject::EInt,
                   MSmlDmDDFObject::EOne,
                   KMemoryCardStatusDescription,
                   KIntType );

    accessTypes.SetReplace();
    
    MSmlDmDDFObject& node5 = root.AddChildObjectL( KMemoryCardEncKeyNode );
    FillNodeInfoL( node5,
                   accessTypes,
                   MSmlDmDDFObject::EPermanent,
                   MSmlDmDDFObject::EB64,
                   MSmlDmDDFObject::EOne,
                   KMemoryCardEncKeyDescription,
                   KTextType );
    
    MSmlDmDDFObject& node6 = root.AddChildObjectL( KUIStateNode );
    FillNodeInfoL( node6,
                   accessTypes,
                   MSmlDmDDFObject::EPermanent,
                   MSmlDmDDFObject::EInt,
                   MSmlDmDDFObject::EOne,
                   KUIStateDescription,
                   KIntType );
    
    accessTypes.Reset();
    accessTypes.SetGet(); // Progress nodes are read-only

    MSmlDmDDFObject& node7 = root.AddChildObjectL( KPhoneMemoryProgNode );
    FillNodeInfoL( node7,
                   accessTypes,
                   MSmlDmDDFObject::EPermanent,
                   MSmlDmDDFObject::EInt,
                   MSmlDmDDFObject::EOne,
                   KPhoneMemoryProgDescription,
                   KIntType );
    
    MSmlDmDDFObject& node8 = root.AddChildObjectL( KMemoryCardProgNode );
    FillNodeInfoL( node8,
                   accessTypes,
                   MSmlDmDDFObject::EPermanent,
                   MSmlDmDDFObject::EInt,
                   MSmlDmDDFObject::EOne,
                   KMemoryCardProgDescription,
                   KIntType );
    DFLOG( "CDevEncAdapter::DDFStructureL End" );
    }

// ----------------------------------------------------------------------------
// CDevEncAdapter::StreamingSupport
// Return streaming support status, set supported item size
// Status : Draft
// ----------------------------------------------------------------------------
//
TBool CDevEncAdapter::StreamingSupport( TInt& /* aItemSize */ )
    {
    // Not supported
    DFLOG( "CDevEncAdapter::StreamingSupport" );
    return EFalse;
    }
    
// ----------------------------------------------------------------------------
// CDevEncAdapter::StreamCommittedL
// Commit stream buffer
// (may leave)
// Status : Draft
// ----------------------------------------------------------------------------
//
void CDevEncAdapter::StreamCommittedL()
    {        
    DFLOG( "CDevEncAdapter::StreamCommitted" );
    // Intentionally left empty 
    }

// ----------------------------------------------------------------------------
// CDevEncAdapter::CompleteOutstandingCmdsL
// Complete outstanding commands
// (may leave)
// Status : Draft
// ----------------------------------------------------------------------------
//
void CDevEncAdapter::CompleteOutstandingCmdsL()
    {
    DFLOG( "CDevEncAdapter::CompleteOutstandingCmdsL Begin" );
    MSmlDmAdapter::TError retValue = CSmlDmAdapter::EOk;
    TInt status;

    TRequestStatus reqStatus;
    TBool pending = ( iPhoneMemOpPending || iMemCardOpPending );
    DFLOG2( "CDevEncAdapter: pending %d", ( pending ? 1 : 0 ) );
    
    if ( iPhoneMemOpPending )
        {
        iPhoneMemOpPending = EFalse;
        DFLOG2( "CDevEncAdapter: iPendingCommand %d", iPendingCommand );
        switch ( iPendingCommand )
            {           
            case EDmDecrypt:
                iPhoneMemorySession->DiskStatus( status );
                if ( status == EEncrypted )
                    {
                    DFLOG( "CDevEncAdapter: phone memory decrypt" );
                    iPhoneMemorySession->StartDiskDecrypt();
                    }
                else
                    {
                    DFLOG2( "CDevEncAdapter: Cannot decrypt, phone mem status %d",
                            status );
                    retValue =  CSmlDmAdapter::EError;
                    }
                break;
            case EDmEncryptDefault:
                iPhoneMemorySession->DiskStatus( status );
                if ( status == EDecrypted )
                    {
                    // if FOTA couldn't be disabled then the encryption cannot be performed
                    if( retValue != CSmlDmAdapter::EError )
                    	{
                    	DFLOG( "CDevEncAdapter: phone memory encrypt" );
                        iPhoneMemorySession->StartDiskEncrypt();
                    	}
                    }
                else
                    {
                    DFLOG2( "CDevEncAdapter: Cannot encrypt, phone mem status %d",
                            status );
                    retValue = CSmlDmAdapter::EError;
                    }
                break;
            default:
                // Unknown or not valid command for phone memory
                DFLOG2( "CDevEncAdapter: Invalid command %d",
                        iPendingCommand );
                retValue = CSmlDmAdapter::EInvalidObject;
                break;
            }
        }

    if ( iMemCardOpPending )
        {
        iMemCardOpPending = EFalse;
        switch ( iPendingCommand )
            {
            case EDmDecrypt:
                iMemoryCardSession->DiskStatus( status );
                if ( status == EEncrypted )
                    {
                    DFLOG( "CDevEncAdapter: memory card decrypt" );
                    TInt opStatus = iMemoryCardSession->StartDiskDecrypt();
                    }
                else
                    {
                    DFLOG2( "CDevEncAdapter: Cannot decrypt, card status %d",
                            status );
                    retValue =  CSmlDmAdapter::EError;
                    }
                break;
            case EDmEncryptRestore:
            case EDmEncryptBackup:
                iMemoryCardSession->DiskStatus( status );
                if ( status == EDecrypted )
                    {                  
                    HBufC8* passwd = AskPassWordL();
                    if ( iPendingCommand == EDmEncryptBackup )
                        {
                        DFLOG( "CDevEncAdapter: memory card backup" );
                        _LIT_SECURITY_POLICY_S0( KPSSecurityPolicy, KPSCategory.iUid );

                        RProperty::Define( KPkcs5PSKey,
                                           RProperty::EByteArray,
                                           KPSSecurityPolicy,
                                           KPSSecurityPolicy );
                        iKeyUtils->CreateSetKey( reqStatus,
                                                 iPkcs5Key,
                                                 *passwd,
                                                 KEncryptionKeyLength );
                        User::WaitForRequest( reqStatus );
                        User::LeaveIfError( reqStatus.Int() );
                        TInt ret = RProperty::Set( KPSCategory,
                                                   KPkcs5PSKey,
                                                   *iPkcs5Key );
                        if ( ret != KErrNone )
                            {
                            DFLOG2( "CDevEncAdapter: Error storing key: %d", ret );
                            User::Leave( ret );
                            }
                        }
                    else if ( iPendingCommand == EDmEncryptRestore )
                        {
                        DFLOG( "CDevEncAdapter: memory card restore" );
                        iKeyUtils->SetKey( reqStatus, *iPkcs5Key, *passwd );
                        User::WaitForRequest( reqStatus );
                        DFLOG2( "CDevEncAdapter::CompleteOutstandingCmdsL: \
                                    SetKey: %d", reqStatus.Int() );
                        User::LeaveIfError( reqStatus.Int() );
                        }
                    DFLOG( "CDevEncAdapter: memory card encrypt" );
                    TInt opStatus = iMemoryCardSession->StartDiskEncrypt();
                    delete passwd;
                    passwd = NULL;
                    }
                else
                    {
                    DFLOG2( "CDevEncAdapter: Cannot encrypt, card status %d",
                            status );
                    retValue = CSmlDmAdapter::EError;
                    }
                break;
            case EDmEncryptDefault:
                iMemoryCardSession->DiskStatus( status );
                if ( status == EDecrypted )
                    {
                    DFLOG( "CDevEncAdapter: memory card encrypt" );
                    TInt opStatus = iMemoryCardSession->StartDiskEncrypt();
                    }
                else
                    {
                    DFLOG2( "CDevEncAdapter: Cannot encrypt, card status %d",
                            status );
                    retValue = CSmlDmAdapter::EError;
                    }
                break;
            default:
                // Unknown or not valid command for memory card
                DFLOG2( "CDevEncAdapter: Invalid command %d",
                        iPendingCommand );
                retValue = CSmlDmAdapter::EInvalidObject;
                break;
            }
        }

    if ( pending )
        {
        Callback().SetStatusL( iStatusRef, retValue );
        }
    DFLOG( "CDevEncAdapter::CompleteOutstandingCmdsL End" );
    }

// ----------------------------------------------------------------------------
// CDevEncAdapter::FillNodeInfoL
// Fill node info
// (may leave)
// Status : Draft
// ----------------------------------------------------------------------------
//
void CDevEncAdapter::FillNodeInfoL( MSmlDmDDFObject& aDDFObject, 
                                    TSmlDmAccessTypes& aAccessTypes, 
                                    MSmlDmDDFObject::TScope aScope,
                                    MSmlDmDDFObject::TDFFormat aFormat, 
                                    MSmlDmDDFObject::TOccurence aOccurence,
                                    const TDesC8& aDescription,
                                    const TDesC8& aMIMEType )
    {
    DFLOG( "CDevEncAdapter::FillNodeInfoL Begin" );
    aDDFObject.SetAccessTypesL( aAccessTypes );
    aDDFObject.SetScopeL( aScope );
    aDDFObject.SetOccurenceL( aOccurence );
    aDDFObject.SetDFFormatL( aFormat );
    aDDFObject.SetDescriptionL( aDescription );
    if ( aFormat != MSmlDmDDFObject::ENode )
        {
        aDDFObject.AddDFTypeMimeTypeL( aMIMEType );
        }
    DFLOG( "CDevEncAdapter::FillNodeInfoL End" );
    }

// ----------------------------------------------------------------------------
// CDevEncAdapter::CopyCommandL
// Copy object
// (may leave)
// Status : Draft
// ----------------------------------------------------------------------------
//
void CDevEncAdapter::CopyCommandL( const TDesC8& /*aTargetURI*/, 
                                   const TDesC8& /*aTargetLUID*/,
                                   const TDesC8& /*aSourceURI*/, 
                                   const TDesC8& /*aSourceLUID*/,
                                   const TDesC8& /*aType*/, 
                                   TInt aStatusRef )
    {
    // Not supported
    DFLOG( "CDevEncAdapter::CopyCommandL Begin" );
    Callback().SetStatusL( aStatusRef, CSmlDmAdapter::EError );
    DFLOG( "CDevEncAdapter::CopyCommandL End" );
    }

// ----------------------------------------------------------------------------
// DeleteObjectL
// Delete object
// (may leave)
// Status : Draft
// ----------------------------------------------------------------------------
//
void CDevEncAdapter::DeleteObjectL( const TDesC8& /* aURI */, 
                                    const TDesC8& /* aLUID */,
                                    TInt aStatusRef )

    {
    // Not supported
    DFLOG( "CDevEncAdapter::DeleteObjectL Begin" );
    Callback().SetStatusL( aStatusRef, CSmlDmAdapter::EError );
    DFLOG( "CDevEncAdapter::DeleteObjectL End" );
    }
    
// ----------------------------------------------------------------------------
// CDevEncAdapter::FetchLeafObjectL
// Fetch leaf
// (may leave)
// Status : Draft
// ----------------------------------------------------------------------------
//
void CDevEncAdapter::FetchLeafObjectL( const TDesC8& aURI, 
                                       const TDesC8& /* aLUID */,
								       const TDesC8& aType, 
								       TInt aResultsRef,
								       TInt aStatusRef )
    {
    DFLOG( "CDevEncAdapter::FetchLeafObjectL Begin" );

    MSmlDmAdapter::TError retValue = CSmlDmAdapter::EOk;
    CBufFlat* result = CBufFlat::NewL( KBufGranularity );
    CleanupStack::PushL( result );
    
    if ( aURI.Compare( KDevEncRootNode ) >= 0 )
        {
        TPtrC8 leaf = LastURISeg( aURI );
        TBuf8<KSizeOfSettingId> buf;
        
        if ( leaf == KPhoneMemoryStatusNode )
            {
            TInt status;
            iPhoneMemorySession->DiskStatus( status );
            buf.Num( status );
            result->InsertL( 0, buf );
            }
        else if ( leaf == KMemoryCardStatusNode )
            {
            TInt status;
            iMemoryCardSession->DiskStatus( status );
            buf.Num( status );
            result->InsertL( 0, buf );           
            }
        else if ( leaf == KMemoryCardEncKeyNode )
            {
            iPkcs5Key = HBufC8::NewL( RProperty::KMaxPropertySize );
            TPtr8 keyPtr = iPkcs5Key->Des();
            keyPtr.FillZ( RProperty::KMaxPropertySize );
            TInt ret = RProperty::Get( KPSCategory, 
                                       KPkcs5PSKey,
                                       keyPtr );
            if ( ret != KErrNone )
                {
                DFLOG2( "CDevEncAdapter::FetchLeafObjectL: Error reading key: %d", ret );
                User::Leave( ret );
                }
            result->InsertL( 0, *iPkcs5Key );                
            }
        else if ( leaf == KUIStateNode )
            {
            TInt status;
            iUiCenRep->Get( KDevEncUiDmControl, status );
            buf.Num( status );
            result->InsertL( 0, buf );
            }
        else if ( leaf == KPhoneMemoryProgNode )
            {
            TInt progress = -1;
            iPhoneMemorySession->Progress( progress );
            buf.Num( progress );
            result->InsertL( 0, buf );
            }
        else if ( leaf == KMemoryCardProgNode )
            {
            TInt progress = -1;
            iMemoryCardSession->Progress( progress );
            buf.Num( progress );
            result->InsertL( 0, buf );
            }
        else
            {
            retValue = CSmlDmAdapter::ENotFound;
            }
        }
    else
        {
        retValue = CSmlDmAdapter::ENotFound;
        }
    
    if ( retValue == CSmlDmAdapter::EOk )
        {
        result->Compress();
        Callback().SetResultsL( aResultsRef, *result, aType );                  
        }
    
    CleanupStack::PopAndDestroy( result );
    // Respond
    Callback().SetStatusL( aStatusRef, retValue );
    DFLOG( "CDevEncAdapter::FetchLeafObjectL End" );
    }    
    
// ----------------------------------------------------------------------------
// CDevEncAdapter::FetchLeafObjectSizeL
// Calculate leaf object size
// (may leave)
// Status : Draft
// ----------------------------------------------------------------------------
//
void CDevEncAdapter::FetchLeafObjectSizeL( const TDesC8& aURI, 
                                           const TDesC8& /* aLUID */,
                                           const TDesC8& /* aType */, 
                                           TInt aResultsRef,
									       TInt aStatusRef )
    {
    DFLOG( "CDevEncAdapter::FetchLeafObjectSizeL Begin" );

    MSmlDmAdapter::TError retValue = CSmlDmAdapter::EOk;
    CBufFlat* result = CBufFlat::NewL( KBufGranularity );
    CleanupStack::PushL( result );
    
    if ( aURI.Compare( KDevEncRootNode ) >= 0 )
        {
        TPtrC8 leaf = LastURISeg( aURI );
        TBuf8<KSizeOfSettingId> buf;
        
        if ( leaf == KPhoneMemoryStatusNode )
            {
            buf.AppendNum( sizeof(TInt) );
            result->InsertL( 0, buf );
            }
        else if ( leaf == KMemoryCardStatusNode )
            {
            buf.AppendNum( sizeof(TInt) );
            result->InsertL( 0, buf );           
            }
        else if ( leaf == KMemoryCardEncKeyNode )
            {
            buf.AppendNum( KEncryptionKeyLength );
            result->InsertL( 0, buf );  
            }
        else if ( leaf == KUIStateNode )
            {
            buf.AppendNum( sizeof(TInt) );
            result->InsertL( 0, buf );
            }
        else if ( leaf == KPhoneMemoryProgNode )
            {
            buf.AppendNum( sizeof(TInt) );
            result->InsertL( 0, buf );
            }
        else if ( leaf == KMemoryCardProgNode )
            {
            buf.AppendNum( sizeof(TInt) );
            result->InsertL( 0, buf );
            }
        else
            {
            retValue = CSmlDmAdapter::ENotFound;
            }
        }
    else
        {
        retValue = CSmlDmAdapter::ENotFound;
        }
    
    if ( retValue == CSmlDmAdapter::EOk )
        {
        result->Compress();
        Callback().SetResultsL( aResultsRef, *result, KIntType );                  
        }
    
    CleanupStack::PopAndDestroy( result );
    // Respond
    Callback().SetStatusL( aStatusRef, retValue );
    DFLOG( "CDevEncAdapter::FetchLeafObjectSizeL End" );
    }    

// ----------------------------------------------------------------------------
// CDevEncAdapter::ChildURIListL
// Create child URI list
// (may leave)
// Status : Draft
// ----------------------------------------------------------------------------
//
void CDevEncAdapter::ChildURIListL( const TDesC8& aURI, 
                                    const TDesC8& /* aLUID */,
                                    const CArrayFix<TSmlDmMappingInfo>& /* aPreviousURISegmentList */,
            				        TInt aResultsRef, 
            				        TInt aStatusRef )
    {
    DFLOG( "CDevEncAdapter::ChildURIListL Begin" );
    MSmlDmAdapter::TError retValue = CSmlDmAdapter::EOk;
    
    if ( aURI.Compare( KDevEncRootNode ) == 0 )
        {
        CBufFlat* buf = CBufFlat::NewL( KBufGranularity );
        CleanupStack::PushL( buf );
        buf->InsertL( 0, KPhoneMemoryCmdNode );
        buf->InsertL( buf->Size(), KSeparator );
        buf->InsertL( buf->Size(), KMemoryCardCmdNode );
        buf->InsertL( buf->Size(), KSeparator );
        buf->InsertL( buf->Size(), KPhoneMemoryStatusNode );
        buf->InsertL( buf->Size(), KSeparator );
        buf->InsertL( buf->Size(), KMemoryCardStatusNode );
        buf->InsertL( buf->Size(), KSeparator );
        buf->InsertL( buf->Size(), KMemoryCardEncKeyNode );
        buf->InsertL( buf->Size(), KSeparator );
        buf->InsertL( buf->Size(), KUIStateNode );
        buf->InsertL( buf->Size(), KSeparator );
        buf->InsertL( buf->Size(), KPhoneMemoryProgNode );
        buf->InsertL( buf->Size(), KSeparator );
        buf->InsertL( buf->Size(), KMemoryCardProgNode );
        
        buf->Compress();
        Callback().SetResultsL( aResultsRef, *buf, KNullDesC8 );
        CleanupStack::PopAndDestroy( buf );
        }
    else
        {
        retValue = CSmlDmAdapter::ENotFound;
        }

    // Respond
    Callback().SetStatusL( aStatusRef, retValue );
    DFLOG( "CDevEncAdapter::ChildURIListL End" );
    }    

// ----------------------------------------------------------------------------
// CDevEncAdapter::AddNodeObjectL
// Add node
// (may leave)
// Status : Draft
// ----------------------------------------------------------------------------
//
void CDevEncAdapter::AddNodeObjectL( const TDesC8& /* aURI */,
                                     const TDesC8& /* aParentLUID */,
								     TInt aStatusRef )
    {
    // Not supported
    DFLOG( "CDevEncAdapter::AddNodeObjectL Begin" );
    Callback().SetStatusL( aStatusRef, CSmlDmAdapter::EError );
    DFLOG( "CDevEncAdapter::AddNodeObjectL End" );
    }    

// ----------------------------------------------------------------------------
// CDevEncAdapter::ExecuteCommandL
// Execute command
// (may leave)
// Status : Draft
// ----------------------------------------------------------------------------
//
void CDevEncAdapter::ExecuteCommandL( const TDesC8& /* aURI */, 
                                      const TDesC8& /* aLUID */,
							          const TDesC8& /* aArgument */, 
							          const TDesC8& /* aType */,
								      TInt aStatusRef )
    {
    // Not supported
    DFLOG( "CDevEncAdapter::ExecuteCommandL Begin" );
    Callback().SetStatusL( aStatusRef, CSmlDmAdapter::EError );
    DFLOG( "CDevEncAdapter::ExecuteCommandL End" );
    }    

// ----------------------------------------------------------------------------
// CDevEncAdapter::ExecuteCommandL
// Execute command, streaming enabled
// (may leave)
// Status : Draft
// ----------------------------------------------------------------------------
//
void CDevEncAdapter::ExecuteCommandL( const TDesC8& /* aURI */, 
                                      const TDesC8& /* aLUID */,
								      RWriteStream*& /* aStream */,
								      const TDesC8& /* aType */,
								      TInt aStatusRef )
    {
    // Not supported
    DFLOG( "CDevEncAdapter::ExecuteCommandL Begin" );
    Callback().SetStatusL( aStatusRef, CSmlDmAdapter::EError );
    DFLOG( "CDevEncAdapter::ExecuteCommandL End" );
    }    
    
// ----------------------------------------------------------------------------
// CDevEncAdapter::UpdateLeafObjectL
// Update leaf object
// (may leave)
// Status : Draft
// ----------------------------------------------------------------------------
//
void CDevEncAdapter::UpdateLeafObjectL( const TDesC8& aURI, 
                                        const TDesC8& /* aLUID */,
                                        const TDesC8& aObject,
                                        const TDesC8& /* aType */,
                                        TInt aStatusRef )
    {
    DFLOG( "CDevEncAdapter::UpdateLeafObjectL Begin" );
    MSmlDmAdapter::TError retValue = CSmlDmAdapter::EOk;
    DFLOG2( "CDevEncAdapter: Object: %S", &aObject );
    
    if ( aURI.Compare( KDevEncRootNode ) >= 0 )
        {
        TPtrC8 leaf = LastURISeg( aURI );
        TLex8 lex;
        
        if ( leaf == KPhoneMemoryCmdNode )
            {
            DFLOG( "CDevEncAdapter: leaf: KPhoneMemoryCmdNode" );
            DFLOG2( "CDevEncAdapter: iMemCardOpPending %d", ( iMemCardOpPending ? 1 : 0 ) );
            if ( iMemCardOpPending )
                {
                retValue = CSmlDmAdapter::EObjectInUse;
                }
            else
                {
                if( !CheckBatteryL() ) 
                	{
                    retValue =  CSmlDmAdapter::EError;
                	}
                else
                	{
                    lex.Assign( aObject );
                    lex.Val( iPendingCommand );
                    iPhoneMemOpPending = ETrue;
                    iStatusRef = aStatusRef;
                    return; // Completed in CompleteOutstandingCmdsL
                	}
                }
            }
        else if ( leaf == KMemoryCardCmdNode )
            {
            DFLOG( "CDevEncAdapter: leaf: KMemoryCardCmdNode" );
            DFLOG2( "CDevEncAdapter: iPhoneMemOpPending %d", ( iPhoneMemOpPending ? 1 : 0 ) );
            if ( iPhoneMemOpPending )
                {
                retValue = CSmlDmAdapter::EObjectInUse;
                }
            else
                {
                if( !CheckBatteryL() ) 
                	{
                    retValue =  CSmlDmAdapter::EError;
                	}
                else
                	{
                    lex.Assign( aObject );
                    lex.Val( iPendingCommand );
                    iMemCardOpPending = ETrue;
                    iStatusRef = aStatusRef;
                    return; // Completed in CompleteOutstandingCmdsL
                	}
                }
            }
        else if ( leaf == KMemoryCardEncKeyNode )
            {
            DFLOG( "CDevEncAdapter: leaf: KMemoryCardEncKeyNode" );
            iPkcs5Key = aObject.AllocL();
            }
        else if ( leaf == KUIStateNode )
            {
            DFLOG( "CDevEncAdapter: leaf: KUIStateNode" );
            lex.Assign( aObject );
            TInt value;
            lex.Val( value );
            iUiCenRep->Set( KDevEncUiDmControl, value );
            }
        else
            {
            retValue = CSmlDmAdapter::ENotFound;
            DFLOG( "CDevEncAdapter: Not Found" );
            }
        }
    else
        {
        retValue = CSmlDmAdapter::ENotFound;
        DFLOG( "CDevEncAdapter: Not Found" );
        }
    Callback().SetStatusL( aStatusRef, retValue );           
    DFLOG( "CDevEncAdapter::UpdateLeafObjectL End" );
    }

// ----------------------------------------------------------------------------
// CDevEncAdapter::UpdateLeafObjectL
// Update leaf object, streaming enabled
// (may leave)
// Status : Draft
// ----------------------------------------------------------------------------
//
void CDevEncAdapter::UpdateLeafObjectL( const TDesC8& /* aURI */, 
                                         const TDesC8& /* aLUID */,
									     RWriteStream*& /* aStream */, 
									     const TDesC8& /* aType */,
		 							     TInt aStatusRef )
    {
    // Not supported
    DFLOG( "CDevEncAdapter::UpdateLeafObjectL Begin" );
    Callback().SetStatusL( aStatusRef, CSmlDmAdapter::EError );
    DFLOG( "CDevEncAdapter::UpdateLeafObjectL End" );
    }

// ----------------------------------------------------------------------------
// CDevEncAdapter::StartAtomicL
// Start atomic
// (may leave)
// Status : Draft
// ----------------------------------------------------------------------------
//
void CDevEncAdapter::StartAtomicL()
    {
    DFLOG( "CDevEncAdapter::StartAtomicL" );
    }
    
// ----------------------------------------------------------------------------
// CDevEncAdapter::CommitAtomicL
// Commit atomic commands
// (may leave)
// Status : Draft
// ----------------------------------------------------------------------------
//
void CDevEncAdapter::CommitAtomicL()
    {
    DFLOG( "CDevEncAdapter::CommitAtomicL" );
    }

// ----------------------------------------------------------------------------
// CDevEncAdapter::RollbackAtomicL
// Lose all modifications after 'StartAtomicL' command
// (may leave)
// Status : Draft
// ----------------------------------------------------------------------------
//
void CDevEncAdapter::RollbackAtomicL()
    {
    DFLOG( "CDevEncAdapter::RollbackAtomicL" );
    }

TPtrC8 CDevEncAdapter::LastURISeg( const TDesC8& aURI )
    {
    DFLOG( "CDevEncAdapter::LastURISeg Begin" );
    TInt i;
    for ( i = aURI.Length()-1; i >= 0; i-- )
        {
        if ( aURI[i] == '/' )
            {
            break;
            }
        }
    DFLOG( "CDevEncAdapter::LastURISeg End" );
    return aURI.Mid( i+1 );
    }

HBufC8* CDevEncAdapter::AskPassWordL()
    {
    DFLOG( "CDevEncAdapter::AskPassWordL Begin" );
    RNotifier notif;
    TInt err = notif.Connect();
    DFLOG2( "CDevEncAdapter::AskPassWordL notif.Connect(): %d", err );
    User::LeaveIfError( err );
    TBuf8<KMaxPasswordLength> passwd;
    passwd.SetLength(KMaxPasswordLength);
    TRequestStatus status = KRequestPending;
    DFLOG( "CDevEncAdapter::AskPassWordL StartNotifierAndGetResponse" );
    notif.StartNotifierAndGetResponse( 
            status, 
            TUid::Uid( KDevEncPasswdDlgUid ),
            KNullDesC8,
            passwd );
    User::WaitForRequest( status );
    notif.Close();
    err = status.Int();
    DFLOG2( "CDevEncAdapter::AskPassWordL status.Int(): %d", err );
    User::LeaveIfError( err );
    HBufC8* passwdBuf = passwd.AllocL();
    DFLOG( "CDevEncAdapter::AskPassWordL End" );
    return passwdBuf;
    }

// ---------------------------------------------------------------------------
// CDevEncAdapter::CheckBatteryL()
// Checks if there's enough battery power to update
// ---------------------------------------------------------------------------
//
TBool CDevEncAdapter::CheckBatteryL()
    {
#ifdef __WINS__

    // In the emulator, the battery level is always 0 and the charger is never
    // connected, so just return ETrue.
    return ETrue;

#else // __WINS__

    // Running on target. Check the real battery and charger status

    TInt chargingstatus( EChargingStatusError );
    TInt batterylevel( 1 );
    TBool enoughPower( EFalse );

    // Read battery
    DFLOG( "CDevEncAdapter::CheckBatteryL" );
    RProperty pw;
    User::LeaveIfError( pw.Attach( KPSUidHWRMPowerState, KHWRMBatteryLevel ) );
    User::LeaveIfError( pw.Get( batterylevel ) );
    pw.Close();

    User::LeaveIfError( pw.Attach( KPSUidHWRMPowerState, KHWRMChargingStatus ) );
    User::LeaveIfError( pw.Get( chargingstatus ));
    pw.Close();
    
    // Too low battery, power insufficient
    if ( batterylevel >= EBatteryLevelLevel4 )
        {
        enoughPower = ETrue;
        }
    // But charger is connected, power sufficient
    if ( ( chargingstatus != EChargingStatusError ) &&
         ( chargingstatus != EChargingStatusNotConnected ) )
        {
        enoughPower = ETrue;
        }

    DFLOG3( "CDevEncAdapter: Battery level: %d  (0..7), chargingstatus %d",
           batterylevel, chargingstatus );
    DFLOG2( "CDevEncAdapter: CheckBatteryL %d", ( enoughPower ? 1 : 0 ) );
    return enoughPower;

#endif // __WINS__
    }
    
// ========================= OTHER EXPORTED FUNCTIONS =========================

// End of File  
