/*
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0"
 * which accompanies this distribution, and is available
 * at the URL "http://www.eclipse.org/legal/epl-v10.html".
 *
 * Initial Contributors:
 * Nokia Corporation - initial contribution.
 * 
 * Contributors:
 *     
 * 
 * Description:
 *      The source file of the backup engine implement for symbian
 */

#include "fmbkupengine_p.h"

// INCLUDE FILES
/*#include <filemanagerengine.rsg>
#include "cfilemanagerremovabledrivehandler.h"
#include "mfilemanagerprocessobserver.h"
#include "cfilemanagerutils.h"
#include "cfilemanagerengine.h"
*/
#include <coreapplicationuisdomainpskeys.h>
#include <coemain.h>
#include <apgwgnam.h>
#include <apgtask.h>
#include <bautils.h>
#include <tz.h>
#include <babackup.h>
#include <pathinfo.h>
#include <sysutil.h>
#ifdef RD_MULTIPLE_DRIVE
#include <driveinfo.h>
#endif // RD_MULTIPLE_DRIVE
#include <e32property.h>
#include <centralrepository.h>
#include "CMMCScBkupEngine.h"
#include "MMCScBkupOperations.h"
#include "CMMCScBkupArchiveInfo.h"
#include "filemanagerprivatecrkeys.h"

#include <eikdef.h>
#include <eikenv.h>

#include "fmcommon.h"

/*
#include "cfilemanageritemproperties.h"
#include "cfilemanagerbackupsettings.h"
#include "filemanagerprivatecrkeys.h"
#include "bkupengine.hrh"
#include "filemanagerdebug.h"
#include "cfilemanagercommondefinitions.h"
#include "filemanageruid.h"
#include "filemanagerprivatepskeys.h"
*/

const TInt KFmgrSystemDrive = EDriveC;

FmBkupEnginePrivate::FmBkupEnginePrivate(FmBkupEngine* bkupEngine): q(bkupEngine),
    iError( KErrNone )
{
    iDrvAndOpList = new ( ELeave ) CArrayFixFlat<TBkupDrivesAndOperation> ( 10 ) ;
    iBkupCategoryList = new RPointerArray<CBkupCategory> ( 10 );
    iFs.Connect();
    iBkupEngine = CMMCScBkupEngine::NewL( iFs );
}

FmBkupEnginePrivate::~FmBkupEnginePrivate()
{
	delete iDrvAndOpList;
	
	iBkupCategoryList->ResetAndDestroy();
    delete iBkupCategoryList;
    
    delete iBkupEngine;
    iFs.Close();
}

TUint32 FmBkupEnginePrivate::FmgrToBkupMask(
        const TUint32 aFmrgMask )
    {
    TUint32 ret( 0 );

    for( TInt i( 0 ); i < KMaskLookupLen; ++i )
        {
        if ( ( aFmrgMask & FmBkupEngineCommon::EFileManagerBackupContentAll ) ||
            ( aFmrgMask & KMaskLookup[ i ].iFmgrMask ) )
            {
            ret |= KMaskLookup[ i ].iBkupMask;
            }
        }
    return ret;
    }

bool FmBkupEnginePrivate::startBackup(QList<FmBkupDrivesAndOperation* > drivesAndOperationList,
	    QList<FmBkupBackupCategory*> backupCategoryList,
	    QString drive, quint32 content)
{
    QString logString;
    logString = "startBackup";
    FmLogger::log( logString );
    iDrvAndOpList->Reset();
    iBkupCategoryList->ResetAndDestroy();
	
    logString = "startBackup_driveroperation count:" + QString::number(drivesAndOperationList.count());
    FmLogger::log( logString );
    
    for( QList<FmBkupDrivesAndOperation* >::iterator it = drivesAndOperationList.begin();
        it != drivesAndOperationList.end(); ++it ) {
		FmBkupDrivesAndOperation* fmDrvAndOp = *it;
		TBkupDrivesAndOperation drvAndOp;
		drvAndOp.setOwnerDataType( fmDrvAndOp->ownerDataType() );
		drvAndOp.setDrvCategories( fmDrvAndOp->drvCategories() );
		iDrvAndOpList->AppendL( drvAndOp );
		}

    logString = "startBackup_backupCategoryList count:" + QString::number(backupCategoryList.count());
    FmLogger::log( logString );

	
	for( QList<FmBkupBackupCategory* >::iterator it = backupCategoryList.begin();
		    it != backupCategoryList.end(); ++it ) {
        FmBkupBackupCategory* fmbkupCategory = *it;
        CBkupCategory* category = CBkupCategory::NewL();
        
        category->setCategory( fmbkupCategory->category() );
        
        HBufC *archiveName = HBufC::NewL( fmbkupCategory->archive_name().length() );
        *archiveName = fmbkupCategory->archive_name().utf16();
        category->setArchive_name( *archiveName );
        delete archiveName;
        
        category->setSpecial_flags( fmbkupCategory->special_flags() );
        category->setExclude_special_flags( fmbkupCategory->exclude_special_flags() );
        
        for( QList<unsigned int>::const_iterator it = fmbkupCategory->uids()->begin();
                it != fmbkupCategory->uids()->end(); ++it ) {
            unsigned int value = *it;
            category->addUids( value );	
        }
        
        
        for( QList<unsigned int>::const_iterator it = fmbkupCategory->exclude_uids()->begin();
                it != fmbkupCategory->exclude_uids()->end(); ++it ) {
            unsigned int value = *it; //( *ex_uidsList )[i];
            category->addExclude_uids( value );	
        }
        
        
        iBkupCategoryList->AppendL( category );
        }
	
	TUint32 bkupContent( FmgrToBkupMask( content ) );
	//TUint32 bkupContent = 63;
	
    logString = "startBackup_new param";
    FmLogger::log( logString );

	CMMCScBkupOpParamsBackupFull* params =
        CMMCScBkupOpParamsBackupFull::NewL(
            iDrvAndOpList,
            iBkupCategoryList,
            TDriveNumber( DriverNameToNumber( drive ) ),
            bkupContent );
    
    mProcess = FmBkupEngine::ProcessBackup;


    logString = "startBackup_param ok";
    FmLogger::log( logString );

    CCoeEnv* coeEnv = CCoeEnv::Static();
    CEikonEnv* eikonEnv = (STATIC_CAST(CEikonEnv*,coeEnv));
    eikonEnv->SetSystem(ETrue);

    logString = "startBackup_StartOperationL";
    FmLogger::log( logString );

    TRAPD( err, iBkupEngine->StartOperationL(
        EMMCScBkupOperationTypeFullBackup, *this, params ) );

    logString = "startBackup_end with error:" + QString::number(err) ;
    FmLogger::log( logString );
    return (err == KErrNone);
}

void FmBkupEnginePrivate::cancelBackup()
{
	QString logString  = "cancelBackup";
	FmLogger::log(logString);
	switch( mProcess )
    {
    case FmBkupEngine::ProcessBackup: // FALLTHROUGH
    case FmBkupEngine::ProcessRestore:
        {
//        PublishBurStatus( EFileManagerBkupStatusUnset );
        iBkupEngine->CancelOperation();
        break;
        }
    default:
        {
        break;
        }
    }
}


void FmBkupEnginePrivate::notifyPreparingInternal()
{
    iError = KErrNone;
    bool cancelable = false;
    switch( mProcess )
    {
        case FmBkupEngine::ProcessBackup:
            cancelable = false;
            break;
        case FmBkupEngine::ProcessRestore:
            cancelable = false;
            break;
        case FmBkupEngine::ProcessNone:
        default:
        Q_ASSERT( false );
    }
        
    emit notifyPreparing( cancelable );
}

void FmBkupEnginePrivate::notifyStartInternal( int aTotalCount )
{
    iError = KErrNone;
    
    bool cancelable = false;
    switch( mProcess )
    {
        case FmBkupEngine::ProcessBackup:
            cancelable = true;
            break;
        case FmBkupEngine::ProcessRestore:
            cancelable = false;
            break;
        case FmBkupEngine::ProcessNone:
        default:
        Q_ASSERT( false );
    }
    emit notifyStart( cancelable, aTotalCount );
}


void FmBkupEnginePrivate::notifyUpdateInternal( int aCount )
    {
    emit notifyUpdate( aCount ); 
    }

void FmBkupEnginePrivate::notifyFinishInternal()
    {
//    iEngine.ClearDriveInfo();
//    PublishBurStatus( EFileManagerBkupStatusUnset );
   /* CCoeEnv* coeEnv = CCoeEnv::Static();
    iEikonEnv->SetSystem( EFalse);
    */
    CCoeEnv* coeEnv = CCoeEnv::Static();
    CEikonEnv* eikonEnv = (STATIC_CAST(CEikonEnv*,coeEnv));
    eikonEnv->SetSystem(EFalse);
    mProcess = FmBkupEngine::ProcessNone;
    emit notifyFinish( error() );
    }
 

void FmBkupEnginePrivate::notifyMemoryLowInternal( int memoryValue, int &userError )
	{
	emit notifyMemoryLow( memoryValue, userError );
	}

TInt FmBkupEnginePrivate::HandleBkupEngineEventL(
        MMMCScBkupEngineObserver::TEvent aEvent, TInt aAssociatedData )
    {
    QString logString;
    TInt ret( KErrNone );
    switch( aEvent )
        {
        case MMMCScBkupEngineObserver::ECommonOperationStarting:
            {
            logString  = "ECommonOperationStarting";

            iFinalValue = KMaxTInt;
//            notifyStartInternal( KMaxTInt );
            notifyPreparingInternal();
            break;
            }
        case MMMCScBkupEngineObserver::ECommonSizeOfTaskUnderstood:
            {
            logString  = "ECommonSizeOfTaskUnderstood";
            iFinalValue = aAssociatedData;
            notifyStartInternal( aAssociatedData );
            break;
            }
        case MMMCScBkupEngineObserver::ECommonOperationPrepareEnded:
            {
            logString  = "ECommonOperationPrepareEnded";
//            PublishBurStatus( EFileManagerBkupStatusUnset );
            // In order to show finished dialog prior SysAp's note,
            // notify observer already at prepare-ended state.
            if( mProcess == FmBkupEngine::ProcessRestore )
                {
                if ( iFinalValue )
                    {
                    notifyUpdateInternal( iFinalValue );
                    }
                notifyFinishInternal();
                }
            break;
            }
        case MMMCScBkupEngineObserver::ECommonOperationEnded:
            {
            logString  = "ECommonOperationEnded";
//            PublishBurStatus( EFileManagerBkupStatusUnset );
            if( mProcess != FmBkupEngine::ProcessRestore && mProcess != FmBkupEngine::ProcessNone )
                {
                if ( iFinalValue )
                    {
                notifyUpdateInternal( iFinalValue );
                    }
                notifyFinishInternal();
                }
            break;
            }
        case MMMCScBkupEngineObserver::ECommonOperationError:
            {
            logString  = "ECommonOperationError:" + QString::number(aAssociatedData);
            iError = aAssociatedData;
            break;
            }
        case MMMCScBkupEngineObserver::ECommonProgress:
            {
            logString  = "ECommonProgress:" + QString::number( aAssociatedData );
            notifyUpdateInternal( aAssociatedData );
            break;
            }
        case MMMCScBkupEngineObserver::EBackupAnalysingData:
            {
            logString  = "EBackupAnalysingData:" + QString::number( aAssociatedData );
            if ( mProcess == FmBkupEngine::ProcessBackup )
                {
					int userError = FmErrNone;
					notifyMemoryLowInternal( aAssociatedData, userError );
                    switch( userError )
                    {
                    case FmErrDiskFull:
                        ret = KErrDiskFull;
                        break;
                    case FmErrCancel:
                        ret = KErrCancel;
                        break;
                    default:
                        break;
                    }
                }
            break;
            }
        default:
            {
            logString  = "default";
            break;
            }
        }
    FmLogger::log( logString );
    return ret;
    }

int FmBkupEnginePrivate::error()
{
    switch (iError) 
    {
    case KErrNone: 
        return FmErrNone;
    case KErrNotFound:
        return FmErrNotFound;
    case KErrDiskFull:
        return FmErrDiskFull;
    case KErrAlreadyExists:
        return FmErrAlreadyExists;
    case KErrCancel:
        return FmErrCancel;
    default: 
        return FmErrUnKnown;
    }    
}





int FmBkupEnginePrivate::deleteBackup( QList<FmBkupDrivesAndOperation* > drivesAndOperationList )
{
    iError = FmErrNone;

    QList< FmRestoreInfo > selection;
    FmRestoreSettings& rstSettings( *( q->RestoreSettingsL() ) );
    rstSettings.GetSelectionL( selection );

    iBkupCategoryList->ResetAndDestroy();
    for( QList<FmBkupDrivesAndOperation* >::iterator it = drivesAndOperationList.begin();
        it != drivesAndOperationList.end(); ++it )
        {
        FmBkupDrivesAndOperation* fmDrvAndOp = *it;
        TBkupDrivesAndOperation drvAndOp;
        drvAndOp.setOwnerDataType( fmDrvAndOp->ownerDataType() );
        drvAndOp.setDrvCategories( fmDrvAndOp->drvCategories() );
        iDrvAndOpList->AppendL( drvAndOp );
        }
    ////////

    CMMCScBkupOpParamsRestoreFull* params =
        CMMCScBkupOpParamsRestoreFull::NewL(
                iDrvAndOpList, FmBkupEngine::EBUCatAllSeparately );
    CleanupStack::PushL( params );

    // Get list of all archives
    RPointerArray< CMMCScBkupArchiveInfo > archives;
    TCleanupItem cleanupItem( ResetAndDestroyArchives, &archives );
    CleanupStack::PushL( cleanupItem );
    iBkupEngine->ListArchivesL(
        archives,
        params,
        AllowedDriveAttMatchMask());
    
    TInt i( 0 );
    while ( i < archives.Count() )
            {
            TBool remove( ETrue );

            // Compare archives category and drive
            CMMCScBkupArchiveInfo* archiveInfo = archives[ i ];
            TUint32 fmgrContent(
                BkupToFmgrMask( archiveInfo->Category().iFlags ) );
            TInt drive( archiveInfo->Drive() );

            TInt count( selection.count() );
            for( TInt j( 0 ); j < count; ++j )
                {
                const FmRestoreInfo& info( selection[ j ] );
                if ( ( drive == DriverNameToNumber( info.drive() ) ) && ( fmgrContent & info.content() ) )
                    {
                    // Found user selected archive
                    // Do not check this archive again
                    selection.removeAt( j );
                    remove = EFalse;
                    break;
                    }
                }
            if ( remove )
                {
                // Remove non selected archive
                archives.Remove( i );
                delete archiveInfo;
                }
            else
                {
                // Move to next archive
                ++i;
                }
            }
    
    iBkupEngine->DeleteArchivesL( archives );
    
    CleanupStack::Pop( &archives );
    archives.Close();
    CleanupStack::PopAndDestroy( params );
    
    return iError;
}


bool FmBkupEnginePrivate::StartRestoreL( QList<FmBkupDrivesAndOperation* > drivesAndOperationList )
    {
    TBool diskFull( SysUtil::DiskSpaceBelowCriticalLevelL(
        &iFs, 0, KFmgrSystemDrive ) );
    if ( diskFull )
        {
        mProcess = FmBkupEngine::ProcessRestore;
        notifyStartInternal( KMaxTInt );
        iError = KErrDiskFull;
        notifyFinishInternal();
        mProcess = FmBkupEngine::ProcessNone;
        return false;
        }
///////
    iDrvAndOpList->Reset();
    iBkupCategoryList->ResetAndDestroy();
    
    for( QList<FmBkupDrivesAndOperation* >::iterator it = drivesAndOperationList.begin();
        it != drivesAndOperationList.end(); ++it )
        {
        FmBkupDrivesAndOperation* fmDrvAndOp = *it;
        TBkupDrivesAndOperation drvAndOp;
        drvAndOp.setOwnerDataType( fmDrvAndOp->ownerDataType() );
        drvAndOp.setDrvCategories( fmDrvAndOp->drvCategories() );
        iDrvAndOpList->AppendL( drvAndOp );
        }
    ////////
    
   FmBackupSettings& bkupSettings( *( q->BackupSettingsL() ) );

    // Create restore params - ownership is transferred to
    // secure backup engine

#ifdef RD_FILE_MANAGER_BACKUP

    CMMCScBkupOpParamsRestoreFull* params =
        CMMCScBkupOpParamsRestoreFull::NewL(
                iDrvAndOpList,
            FmBkupEngine::EBUCatAllSeparately );
//    CleanupStack::PopAndDestroy(); // driveReader
    CleanupStack::PushL( params );

    // Get list of all archives
    RPointerArray< CMMCScBkupArchiveInfo > archives;
    TCleanupItem cleanupItem( ResetAndDestroyArchives, &archives );
    CleanupStack::PushL( cleanupItem );
    iBkupEngine->ListArchivesL(
        archives,
        params,
        AllowedDriveAttMatchMask() );

    // Get user set restore selection
    QList< FmRestoreInfo > selection;
//    CleanupClosePushL( selection );
    FmRestoreSettings& rstSettings( *( q->RestoreSettingsL() ) );
    rstSettings.GetSelectionL( selection );

    // Remove non user selected archives
    TInt i( 0 );
    while ( i < archives.Count() )
        {
        TBool remove( ETrue );

        // Compare archives category and drive
        CMMCScBkupArchiveInfo* archiveInfo = archives[ i ];
        TUint32 fmgrContent(
            BkupToFmgrMask( archiveInfo->Category().iFlags ) );
        TInt drive( archiveInfo->Drive() );

        TInt count( selection.count() );
        for( TInt j( 0 ); j < count; ++j )
            {
            const FmRestoreInfo& info( selection[ j ] );
            if ( ( drive == DriverNameToNumber( info.drive() ) ) && ( fmgrContent & info.content() ) )
                {
                // Found user selected archive
                // Do not check this archive again
                selection.removeAt( j );
                remove = EFalse;
                break;
                }
            }
        if ( remove )
            {
            // Remove non selected archive
            archives.Remove( i );
            delete archiveInfo;
            }
        else
            {
            // Move to next archive
            ++i;
            }
        }

//    CleanupStack::PopAndDestroy( &selection );
    params->SetArchiveInfosL( archives );
    CleanupStack::Pop( &archives );
    archives.Close();
    CleanupStack::Pop( params );

#else // RD_FILE_MANAGER_BACKUP

    CMMCScBkupOpParamsRestoreFull* params =
        CMMCScBkupOpParamsRestoreFull::NewL( driveReader, EBUCatAllInOne );
    CleanupStack::PopAndDestroy(); // driveReader

    // Get list of all archives
    RPointerArray< CMMCScBkupArchiveInfo > archives;
    TCleanupItem cleanupItem( ResetAndDestroyArchives, &archives );
    CleanupStack::PushL( cleanupItem );
    iBkupEngine->ListArchivesL(
        archives,
        params,
        bkupSettings.AllowedDriveAttMatchMask() );
    params->SetArchiveInfosL( archives );
    CleanupStack::Pop( &archives );

#endif // RD_FILE_MANAGER_BACKUP

    // Start the process - engine owns the parameters immediately
    mProcess = FmBkupEngine::ProcessRestore;
    PublishBurStatus( EFileManagerBkupStatusRestore );
    
    CCoeEnv* coeEnv = CCoeEnv::Static();
       CEikonEnv* eikonEnv = (STATIC_CAST(CEikonEnv*,coeEnv));
       eikonEnv->SetSystem(ETrue);
       
    TRAPD( err, iBkupEngine->StartOperationL(
        EMMCScBkupOperationTypeFullRestore, *this, params ) );
    if ( err != KErrNone )
        {
        PublishBurStatus( EFileManagerBkupStatusUnset );
        mProcess = FmBkupEngine::ProcessNone;
        User::Leave( err );
        }
    return true;
    }

void FmBkupEnginePrivate::GetRestoreInfoArray( QList<FmBkupDrivesAndOperation* > drivesAndOperationList,
        QList< FmRestoreInfo > &restoreInfoList,
        const TInt aDrive )
    {
    FmBackupSettings& settings( *( q->BackupSettingsL() ) );

    restoreInfoList.clear();
    
    ///////
    iDrvAndOpList->Reset();
    iBkupCategoryList->ResetAndDestroy();
    
    for( QList<FmBkupDrivesAndOperation* >::iterator it = drivesAndOperationList.begin();
        it != drivesAndOperationList.end(); ++it )
        {
        FmBkupDrivesAndOperation* fmDrvAndOp = *it;
        TBkupDrivesAndOperation drvAndOp;
        drvAndOp.setOwnerDataType( fmDrvAndOp->ownerDataType() );
        drvAndOp.setDrvCategories( fmDrvAndOp->drvCategories() );
        iDrvAndOpList->AppendL( drvAndOp );
        }
    ////////

    CMMCScBkupOpParamsRestoreFull* params =
        CMMCScBkupOpParamsRestoreFull::NewL(
                iDrvAndOpList, FmBkupEngine::EBUCatAllSeparately );
    CleanupStack::PushL( params );

    // Get list of all archives
    RPointerArray< CMMCScBkupArchiveInfo > archives;
    TCleanupItem cleanupItem( ResetAndDestroyArchives, &archives );
    CleanupStack::PushL( cleanupItem );
    iBkupEngine->ListArchivesL(
        archives,
        params,
        AllowedDriveAttMatchMask(),
        aDrive );

    // Fill restore info
    TInt count( archives.Count() );
//    restoreInfoList.ReserveL( count );

    for( TInt i( 0 ); i < count; ++i )
        {
        // Content
        CMMCScBkupArchiveInfo& archiveInfo( *archives[ i ] );
        
        TUint32 iContent = BkupToFmgrMask( archiveInfo.Category().iFlags );
        TTime iTime = archiveInfo.DateTime();
        TInt iDrive = archiveInfo.Drive();
        TDateTime iDateTime = iTime.DateTime();
        
        int h       = iDateTime.Hour();
        int m       = iDateTime.Minute();
        int s       = iDateTime.Second();
        int year    = iDateTime.Year();
        int month   = iDateTime.Month() + 1;
        int day     = iDateTime.Day();
        QTime time( h, m, s);
        QDate date( year, month, day );
        
        QDateTime dateTime( date, time );
        dateTime = dateTime.toLocalTime();
        
        FmRestoreInfo restoreInfo( iContent, dateTime, NumberToDriverName( iDrive ) );
        restoreInfoList.append( restoreInfo );
        }

    CleanupStack::PopAndDestroy( &archives );
    CleanupStack::PopAndDestroy( params );
    }

TUint32 FmBkupEnginePrivate::BkupToFmgrMask(
        const TUint32 aBkupMask )
    {
    TUint32 ret( 0 );

    for( TInt i( 0 ); i < KMaskLookupLen; ++i )
        {
        if ( aBkupMask & KMaskLookup[ i ].iBkupMask )
            {
            ret |= KMaskLookup[ i ].iFmgrMask;
            }
        }
    return ret;
    }

void FmBkupEnginePrivate::ResetAndDestroyArchives( TAny* aPtr )
    {
    RPointerArray< CMMCScBkupArchiveInfo >* archive = 
        static_cast< RPointerArray< CMMCScBkupArchiveInfo >* >( aPtr );
    archive->ResetAndDestroy();
    archive->Close();
    }

TUint32 FmBkupEnginePrivate::AllowedDriveAttMatchMask() const
    {
    return KDriveAttRemovable | KDriveAttInternal;//KDriveAttRemovable;
    }

TInt FmBkupEnginePrivate::DriverNameToNumber( QString driverName )
    {
        TInt drive = 0;
        drive = driverName[0].toUpper().toAscii() - 'A' + EDriveA;
        return drive;
    }
QString FmBkupEnginePrivate::NumberToDriverName( TInt driver )
    {
        QChar driverChar( driver - EDriveA + 'A' );
        QString driverName = QString( driverChar ) + ':';
        return driverName;
    }

// -----------------------------------------------------------------------------
// CFileManagerEngine::PublishBurStatus()
//
// -----------------------------------------------------------------------------
//  
void FmBkupEnginePrivate::PublishBurStatus( TInt aType )
    {
    Q_UNUSED( aType );
    /*
   _LIT_SECURITY_POLICY_S0( KFileManagerBkupWritePolicy, KFileManagerUID3 );
   _LIT_SECURITY_POLICY_PASS( KFileManagerBkupReadPolicy );

    TInt err( RProperty::Set(
        KPSUidFileManagerStatus, KFileManagerBkupStatus, aType ) );
    if ( err != KErrNone )
        {
        err = RProperty::Define(
            KPSUidFileManagerStatus, KFileManagerBkupStatus,
            RProperty::EInt, KFileManagerBkupReadPolicy,
            KFileManagerBkupWritePolicy );
        if ( err == KErrNone || err == KErrAlreadyExists )
            {
            err = RProperty::Set(
                KPSUidFileManagerStatus, KFileManagerBkupStatus, aType );
            }
        }
    LOG_IF_ERROR1(
        err, "FileManagerRemovableDriveHandler::PublishBurStatus-err=%d", err )
        */
    }
