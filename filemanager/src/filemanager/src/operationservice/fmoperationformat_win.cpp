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
    FM_LOG( logString );
    
    if( mDriverName.isEmpty() ) {
        return FmErrWrongParam;
    }
    int totalCount( 100 );
    emit notifyStart( totalCount, false );
    for( int i = 0; i < totalCount; i++ ) {
        emit notifyProgress( i );
    }

    return FmErrNone;

}
