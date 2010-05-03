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
* Description: filemanager file icon provider source file 
*
*/

#include "fmfileiconprovider.h"
#include "fmutils.h"

#include <QDir>
#include <QFileInfo>

#define mmcIcon         ":image/qgn_prop_mmc_memc.svg"
#define mmcNoneIcon     ":image/qgn_prop_fmgr_mmc_no_sub.svg"
#define mmcLockedIcon   ":image/qgn_prop_mmc_locked.svg"
#define phoneMemoryIcon ":image/qgn_prop_phone_memc.svg"
#define usbMemoryIcon   ":image/qgn_prop_usb_memc.svg"
#define massMemoryIcon  ":image/qgn_prop_fmgr_ms.svg"

FmFileIconProvider::FmFileIconProvider()
{

}

FmFileIconProvider::~FmFileIconProvider()
{
}

QIcon FmFileIconProvider::icon(const QFileInfo &info) const
{
    QString filePath( info.path() );
    if( FmUtils::isDrive( filePath ) ) {
        FmDriverInfo::DriveState driveState = FmUtils::queryDriverInfo( filePath ).driveState();
        if( driveState & FmDriverInfo::EDriveAvailable ){
            if( driveState & FmDriverInfo::EDriveRemovable ) {
                if( driveState & FmDriverInfo::EDriveMassStorage ) {
                    // Mass Storage
                    return QIcon( massMemoryIcon );
                }
                else{
                    //Memory Card
                    return QIcon( mmcIcon );
                }
            } else{
                //Phone Memory
                return QIcon( phoneMemoryIcon );
            }
        } else if( driveState & FmDriverInfo::EDriveLocked ) {
            return QIcon( mmcLockedIcon );
        } else if( driveState & FmDriverInfo::EDriveCorrupted ) {
            return QIcon( mmcNoneIcon );
        } else if( driveState & FmDriverInfo::EDriveNotPresent ){
            return QIcon( mmcNoneIcon );
        } else {
            return QIcon( mmcNoneIcon );
        }
    } else {
        return QFileIconProvider::icon( info );
    }
}
