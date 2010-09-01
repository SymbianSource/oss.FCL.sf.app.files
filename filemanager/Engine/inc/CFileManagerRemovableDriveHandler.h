/*
* Copyright (c) 2006-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Wraps removable drive functionality
*
*/


#ifndef C_FILEMANAGERREMOVABLEDRIVEHANDLER_H
#define C_FILEMANAGERREMOVABLEDRIVEHANDLER_H


// INCLUDES
#include <e32base.h>
#include <f32file.h>
#include "MFileManagerProcessObserver.h"
#include "MMMCScBkupEngineObserver.h"
#include "CMMCScBkupArchiveInfo.h"
#include "CFileManagerRestoreSettings.h"


// FORWARD DECLARATIONS
class CMMCScBkupEngine;
class CFileManagerUtils;
class CFileManagerEngine;
class CBaBackupSessionWrapper;
class CFileManagerBackupSettings;


// CLASS DECLARATION
/**
*  This class wraps removable drive functionality
*
*  @lib FileManagerEngine.lib
*  @since S60 3.1
*/
NONSHARABLE_CLASS(CFileManagerRemovableDriveHandler) :
        public CActive,
        public MMMCScBkupEngineObserver
    {

public:
    /**
     * Two-phased constructor.
     */
    static CFileManagerRemovableDriveHandler* NewL(
        RFs& aFs,
        CFileManagerUtils& aUtils,
        CFileManagerEngine& aEngine );

    /**
     * Destructor
     */
    virtual ~CFileManagerRemovableDriveHandler();

public: // New methods
    /**
     * Sets process observer for memory card operations
     * @param aObserver Pointer to observer or NULL,
     *                  ownership is not transferred
     */
    void SetObserver( MFileManagerProcessObserver* aObserver );

    /**
     * Starts eject process
     */
    void StartEjectL();

    /**
     * Starts format process
     * @param aDrive Drive number EDriveA...EDriveZ
     */
    void StartFormatL( const TInt aDrive );

    /**
     * Cancels ongoing process
     */
    void CancelProcess();

    /**
     * Checks does backup file exists
     * @return ETrue if exists. Otherwise EFalse.
     */
    TBool BackupFileExistsL( const TInt aDrive );

    /**
    * Starts backup process
    * @param aProcess Backup process to start
    */
    void StartBackupL(
        MFileManagerProcessObserver::TFileManagerProcess aProcess );

    /**
    * Starts restore process
    */
    void StartRestoreL();

    /**
    * Gets restore info array
    * @param aArray For storing restore info
    * @param aDrive For getting info from specific drive
    */
    void GetRestoreInfoArrayL(
        RArray< CFileManagerRestoreSettings::TInfo >& aArray,
        const TInt aDrive );

    /**
    * Checks is process ongoing
    */
    TBool IsProcessOngoing() const;

    /**
    * Gets create time of the latest backup archive
    * @param aBackupTime Stores the time
    */
    void LatestBackupTimeL( TTime& aBackupTime );

    /**
    * Deletes selected backups
    */
    void DeleteBackupsL();

private: // From CActive
    /**
     * @see CActive
     */
    void DoCancel();

    /**
     * @see CActive
     */
    void RunL();

    /**
     * @see CActive
     */
    TInt RunError( TInt aError );

private:
    /**
    * Constructors
    */
    CFileManagerRemovableDriveHandler(
        RFs& aFs,
        CFileManagerUtils& aUtils,
        CFileManagerEngine& aEngine );

    void ConstructL();

    /**
    * For periodic eject scan
    */
    static TInt EjectScanAndShutdownApps( TAny* ptr );

    /**
    * For periodic eject scan
    */
    void DoEjectScanAndShutdownL();

    /**
    * For periodic eject scan
    */
    void EjectComplete( TInt aErr );

    /**
    * Gets backup file name
    */
    HBufC* BackupFileNameLC( TBool aFullPath ) const;

    /**
    * Starts eject scan
    */
    void StartEjectScanL();

    void InformStartL( TInt aTotalCount );

    void InformUpdateL( TInt aCount );

    void InformFinishL( );

    void InformError( TInt aErr );

    void StartFormatProcessL();

    void EndFormatProcessL( TInt aErr );

    void CloseAppsL();

    void RestartAppsL();

    static void ResetAndDestroyArchives( TAny* aPtr );

    static TUint32 BkupToFmgrMask( const TUint32 aBkupMask );

    static TUint32 FmgrToBkupMask( const TUint32 aFmrgMask );

    void PublishBurStatus( TInt aType );

    TBool IsInternalMassStorage( TInt aDrive );

    void StoreVolumeNameL( TInt aDrive );

    void RestoreVolumeNameL( TInt aDrive );
    
    void ListArchivesL(
        RPointerArray< CMMCScBkupArchiveInfo >& aArchives,
        const CFileManagerBackupSettings& aBackupSettings );

private: // From MMMCScBkupEngineObserver
    TInt HandleBkupEngineEventL(
        MMMCScBkupEngineObserver::TEvent aEvent, TInt aAssociatedData );

private:    // Data
    /**
     * Backup engine
     * Own.
     */
    CMMCScBkupEngine* iBkupEngine;

    /**
     * Pointer to observer
     * Not own.
     */
    MFileManagerProcessObserver* iObserver;

    /**
     * Observed process
     */
    MFileManagerProcessObserver::TFileManagerProcess iProcess;

    /**
     * For file operations
     * Not own.
     */
    RFs& iFs;

    /**
     * For utility access
     * Not own.
     */
    CFileManagerUtils& iUtils;

    /**
     * For file manager engine access
     * Not own.
     */
    CFileManagerEngine& iEngine;

    /**
     * Last process error
     */
    TInt iLastError;

    /**
     * Final progress value
     */
    TInt iFinalValue;

    /**
     * For timed eject scanning steps
     * Own.
     */
    CPeriodic* iEjectScanPeriodic;

    /**
     * For eject scan round counting
     */
    TInt iEjectScanRounds;

    /**
     * Indicates if backup or restore is in progress
     */
    TBool iBackupRestoreInprogress;

    /**
     * For formatting drives
     */
    RFormat iFormatter;

    /**
     * For keeping track of formatting process progress
     */
    TPckgBuf< TInt > iFormatCountBuf;

    /**
     * For closing and restarting open files while formatting
     */
    CBaBackupSessionWrapper* iBSWrapper;

    /**
     * Target drive of the process
     */
    TInt iDrive;

    };

#endif // C_FILEMANAGERREMOVABLEDRIVEHANDLER_H
            
// End of File
