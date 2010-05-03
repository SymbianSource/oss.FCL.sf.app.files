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
* Description: 
*
*/

#include "fmoperationformat.h"
#include "fmcommon.h"
#include "fmoperationbase.h"
#include "fmutils.h"

#include <QString>

#include <f32file.h>
#include <e32property.h>
#include <coreapplicationuisdomainpskeys.h>

FmOperationFormat::FmOperationFormat( QObject *parent, QString mDriverName ) : FmOperationBase( parent, FmOperationService::EOperationTypeFormat ),
    mDriverName( mDriverName )
{
}
FmOperationFormat::~FmOperationFormat()
{
}

QString FmOperationFormat::driverName()
{
    return mDriverName;
}

int FmOperationFormat::start()
{ 
    QString logString = "FmOperationFormat::start";
    FmLogger::log( logString );
    
    emit notifyPreparing( false );
    
    if( mDriverName.isEmpty() ) {
        return FmErrWrongParam;
    }
    
    RFormat format;
    
    RFs fs;
    int err = fs.Connect();
    
    if( err != KErrNone ){
        return FmErrTypeFormatFailed;
    }

    TInt drive = 0;
    drive = mDriverName[0].toUpper().toAscii() - 'A' + EDriveA;

    TDriveName formatDriveName( TDriveUnit( drive ).Name() );

    int finalValue = 0;
    
    format.Close();

    err = format.Open(
        fs, formatDriveName, EFullFormat | ESpecialFormat, finalValue );
    
    if( err == KErrNone ){
        logString = "emit notifyStart";
        FmLogger::log( logString );
        mTotalSteps = finalValue;
        emit notifyStart( false, finalValue );
    }
    
    logString = "Format open error:" + QString::number( err );
    FmLogger::log( logString );
    
    if( err == KErrLocked ){
        err = fs.ErasePassword( drive );
        
        logString = "ErasePassword error:" + QString::number( err );
        FmLogger::log( logString );
        
        if( err == KErrNone ){
            err = format.Open(
               fs, formatDriveName, EFullFormat, finalValue ); 
            
            if( err == KErrNone ){
                logString = "emit notifyStart";
                FmLogger::log( logString );
                mTotalSteps = finalValue;
                emit notifyStart( false, finalValue );
            }           
            logString = "Second format open error:" + QString::number( err );
            FmLogger::log( logString );
        }
    }
    
    if (err == KErrInUse){
        TBool reallyFormat = ETrue;
        if (reallyFormat){
            err = format.Open(
                    fs, formatDriveName, EFullFormat | EForceFormat, finalValue );
            
            if( err == KErrNone ){
                logString = "emit notifyStart";
                FmLogger::log( logString );
                mTotalSteps = finalValue;
                emit notifyStart( false, finalValue );
            } 
            
            logString = "Reallyformat open error:" + QString::number( err );
            FmLogger::log( logString );
        }
    }
    
    TFullName fsName;
    if ( err == KErrNone )
        {
        err = fs.FileSystemName( fsName, drive  );
        
        logString = "FileSystemName error:" + QString::number( err );
        FmLogger::log( logString );

        if ( err == KErrNone && fsName.Length() > 0 )
            {
            // Prevent SysAp shutting down applications
            RProperty::Set(
                    KPSUidCoreApplicationUIs,
                    KCoreAppUIsMmcRemovedWithoutEject,
                    ECoreAppUIsEjectCommandUsed );
            
            logString = "Prevent SysAp shutting down applications" ;
            FmLogger::log( logString );
            }
        else
            {
            // Don't continue with format if there is no file system name
            // or file system name could not be obtained.
            err = KErrCancel;
            
            logString = QString( "Format cancel" );
            FmLogger::log( logString );
            }
        }
    
    if( err == KErrNone &&  finalValue ){
    
        while ( finalValue ){        
            logString = "Format tracks:" + QString::number( finalValue );
            FmLogger::log( logString );
            err = format.Next( finalValue );
            
            if( err != KErrNone ){ 
                logString = "Format error:" + QString::number( err );
                FmLogger::log( logString );
                break; 
            }
            
            logString = "emit notifyProgress";
            FmLogger::log( logString );
            emit notifyProgress( mTotalSteps - finalValue );
        }
    }
    
    if( !finalValue || err != KErrNone ){
        format.Close();        
        fs.Close();
        
        FmUtils::createDefaultFolders( mDriverName );
    }

    if( err == KErrNone ){
        return FmErrNone;
    }
    else{
        return FmErrTypeFormatFailed;
    }
}
