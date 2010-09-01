/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Wraps remote drive functionality
*
*/


#ifndef C_FILEMANAGERREMOTEDRIVEHANDLER_H
#define C_FILEMANAGERREMOTEDRIVEHANDLER_H


// INCLUDES
#include <e32base.h>
#include "MFileManagerPropertyObserver.h"


// FORWARD DECLARATIONS
class CRsfwMountMan;
class CFileManagerEngine;
class CFileManagerPropertySubscriber;
class CFileManagerUtils;


// CLASS DECLARATION
/**
 *  This class wraps remote drive functionality
 *
 *  @lib FileManagerEngine.lib
 *  @since S60 3.1
 */
NONSHARABLE_CLASS(CFileManagerRemoteDriveHandler) :
        public CBase,
        MFileManagerPropertyObserver
    {

public:
    /**
     * Two-phased constructor.
     */
    static CFileManagerRemoteDriveHandler* NewL(
        CFileManagerEngine& aEngine,
        CFileManagerUtils& aUtils );

    /**
     * Destructor
     */
    ~CFileManagerRemoteDriveHandler();

public: // New functions
    /**
     * Gets remote drive connection state
     * @param aDrive Drive number EDriveA...EDriveZ
     * @return ETrue if connected. Otherwise EFalse.
     */
    TBool IsConnected( const TInt aDrive );

    /**
     * Connects or disconnects remote drive
     * @param aDrive Drive number EDriveA...EDriveZ
     * @return System wide error code.
     */
    TInt SetConnection( TInt aDrive, TBool aConnect );

    /**
     * Deletes remote drive settings
     * @param aDrive Drive number EDriveA...EDriveZ
     * @return System wide error code.
     */
    TInt DeleteSettings( const TInt aDrive );

    /**
     * Ensures that contents of a remote directory are up to date
     * when directory is refreshed
     * @param aFullPath Full path to remote directory
     * @return error code.
     */
    TInt RefreshDirectory( const TDesC& aFullPath );

    /**
     * Checks does application have support for remote drives
     * @param aUid Application uid
     * @return ETrue if the app has support. Otherwise EFalse.
     */
    TBool HasAppRemoteDriveSupport( TUid aUid );

    /**
     * Cancels incoming or outgoing remote drive transfer
     * @param aFullPath Full path to remote file or directory
     */
    void CancelTransfer( const TDesC& aFullPath );

public: // From MFileManagerPropertyObserver
    void PropertyChangedL( const TUid& aCategory, const TUint aKey );

private:
    /**
     * Constructors
     */
    CFileManagerRemoteDriveHandler(
        CFileManagerEngine& aEngine,
        CFileManagerUtils& aUtils );

    void ConstructL();

private: // Data
    /**
     * For remote drive mounting and state handling
     * Own.
     */
    CRsfwMountMan* iMountMan;

    /**
     * Reference to file manager engine
     * Not own.
     */
    CFileManagerEngine& iEngine;

    /**
     * Reference to utils
     * Not own.
     */
    CFileManagerUtils& iUtils;

    /**
     * For PS subscribing
     * Own.
     */
    CFileManagerPropertySubscriber* iSubscriber;

    /**
     * Indicates if remote drives are supported or not
     */
    TBool iRemoteStorageFwSupported;

    };

#endif // C_FILEMANAGERREMOTEDRIVEHANDLER_H

// End of File
