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
* Description:  Implementation of CDevEncUiFileManager.
*
*/

#include "DevEncLog.h"
#include "DevEncUids.hrh"
#include "DevEncUiFileManager.h"

#include <pathinfo.h>
#include <s32file.h>
#include <s32std.h>

// --------------------------------------------------------------------------
// CDevEncUiFileManager::CDevEncUiFileManager()
//
// --------------------------------------------------------------------------
void CDevEncUiFileManager::ConstructL()
    {
    User::LeaveIfError( iFs.Connect() );
    }

// --------------------------------------------------------------------------
// CDevEncUiFileManager::CDevEncUiFileManager()
//
// --------------------------------------------------------------------------
CDevEncUiFileManager::~CDevEncUiFileManager()
    {
    iFs.Close();
    }

// --------------------------------------------------------------------------
// CDevEncUiFileManager::SaveKeyL()
//
// --------------------------------------------------------------------------
void CDevEncUiFileManager::SaveKeyL( CFileStore* aStore,
                                     const TDesC8& aPkcs5Key ) const
    {
    RStoreWriteStream write;
    
    aStore->SetTypeL( aStore->Layout() );
    
    //write the encryption key to a new stream
    write.CreateLC( *aStore );
    write << aPkcs5Key;
    write.CommitL();
    CleanupStack::PopAndDestroy(); //CreateLC()

    aStore->Commit();
    }

// --------------------------------------------------------------------------
// CDevEncUiFileManager::LoadKeyL()
//
// --------------------------------------------------------------------------
void CDevEncUiFileManager::LoadKeyL( const TFileName& aFileName,
                                     HBufC8*& aPkcs5Key )
    {
    DFLOG2( "CDevEncUiFileManager::LoadKeyL Filename %S", &aFileName );
    //prepare to read the streams back in, creating a new TPBEncryptionData
    RStoreReadStream read;
     
    // open the next PFS
    CFileStore *store = CPermanentFileStore::OpenLC( iFs,
                                                     aFileName,
                                                     EFileRead );
    
    DFLOG("CPermanentFileStore::OpenLC passed");
    
    TStreamId dataStreamId( 1 ); // we know it was the first stream written
    read.OpenLC( *store, dataStreamId );
    DFLOG("RStoreReadStream::OpenLC passed");
    //read in Encryption key
    aPkcs5Key = HBufC8::NewL( read, KMaxTInt );
    DFLOG("HBufC8::NewL passed");
    CleanupStack::Pop(); // read
    read.Close();
    CleanupStack::PopAndDestroy( store );
    }

// --------------------------------------------------------------------------
// CDevEncUiFileManager::OpenFileStore()
//
// --------------------------------------------------------------------------
TInt CDevEncUiFileManager::OpenFileStore( const TDesC& aFilename,
                                          CFileStore*& aStore )
    {
    // Leaves with KErrAlreadyExists if file exists from before
    TInt error( KErrNone );
    CFileStore* store( NULL );
    TRAP( error, store = CPermanentFileStore::CreateL( iFs,
                                                aFilename,
                                                EFileRead | EFileWrite ) );
    if ( !error )
        {
        aStore = store;
        }
    return error;
    }

// --------------------------------------------------------------------------
// CDevEncUiFileManager::GetKeyListL()
//
// --------------------------------------------------------------------------
void CDevEncUiFileManager::GetKeyListL( CDir*& aList )
    {
    // Construct file path
    _LIT( KAsterisk, "*" );
    _LIT( KDevEncKeyFileExtension, ".pk5");
    
    TBuf<KMaxFileName> filePath;
    User::LeaveIfError( KeyFolder( filePath ) );
    filePath.Append( KAsterisk );
    filePath.Append( KDevEncKeyFileExtension );
    
    TInt error = iFs.GetDir( filePath,
                    KEntryAttNormal, // Any file, not hidden and system files
                    ESortByName,
                    aList );
    if ( error )
        {
        DFLOG2( "Could not get dir listing, error %d", error );
        User::Leave( error );
        }
    }

// --------------------------------------------------------------------------
// CDevEncUiFileManager::DriveToChar()
//
// --------------------------------------------------------------------------
TInt CDevEncUiFileManager::DriveToChar( TInt aDrive, TChar &aChar )
    {
    return iFs.DriveToChar( aDrive, aChar );
    }

// --------------------------------------------------------------------------
// CDevEncUiFileManager::KeyFolder()
//
// --------------------------------------------------------------------------
TInt CDevEncUiFileManager::KeyFolder( TDes& aResult )
    {
    DFLOG("KeyFolder()");
    TInt result( KErrNone );
    if ( !result )
        {
        DFLOG("1st Append");
        aResult.Append( PathInfo::PhoneMemoryRootPath() );
        DFLOG("2nd Append");
        aResult.Append( PathInfo::OthersPath() );
        }
    DFLOG2( "CDevEncUiFileManager::KeyFolder %S", &aResult );
    return result;
    }

// --------------------------------------------------------------------------
// CDevEncUiFileManager::RemountMmc()
//
// --------------------------------------------------------------------------
TInt CDevEncUiFileManager::RemountMmc()
    {
    return iFs.RemountDrive( /*EDriveE*/EDriveF );
    }

// End of file
