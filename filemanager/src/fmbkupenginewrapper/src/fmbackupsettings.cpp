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
 *      The source file of the backup settings of file manager
 */

#include "fmbackupsettings.h"
#include "fmbkupengine.h"
#include "fmcommon.h"

#include <QString>
#include <QStringList>
#include <QTime>
#include <QSettings>

FmBackupSettings::FmBackupSettings( FmBkupEngine *aFmBkupEngine ) : mBkupEngine( aFmBkupEngine )
{
    
}

FmBackupSettings::~FmBackupSettings( void )
{   
   resetAndDestoryBackupEntry();
}

void FmBackupSettings::setContent( const quint32 aContent )
{
    mContent = aContent;
    save();
}

void FmBackupSettings::setScheduling( const TFileManagerBackupSchedule aScheduling )
{
    mScheduling = aScheduling;
    save();
}        

void FmBackupSettings::setWeekday( const TFileManagerBackupWeekday weekday )
{
    mWeekday = weekday;
    save();
}

void FmBackupSettings::setTime( const QTime &aTime )
{
    mTime = aTime;
    save();
}

void FmBackupSettings::setTargetDrive( const QString &aDrive )
{
    mTargetDrive = aDrive;
    save();
}

quint32 FmBackupSettings::content() const
{
    return mContent;
}

FmBackupSettings::TFileManagerBackupSchedule FmBackupSettings::scheduling() const
{
    return mScheduling;
}

FmBackupSettings::TFileManagerBackupWeekday FmBackupSettings::weekday() const
{
    return mWeekday;
}

const QTime& FmBackupSettings::time() const
{
    return mTime;
}

QString FmBackupSettings::targetDrive() const
{
    return mTargetDrive;
}

QString FmBackupSettings::availableTargetDrive() const
{
    QString targetDrive;
    QStringList driveList;
    mBkupEngine->getBackupDriveList( driveList );
    if( !driveList.isEmpty() ) {
        if( driveList.contains( mTargetDrive ) ) {
            targetDrive =  mTargetDrive;
        } else {
            targetDrive = driveList.first();
        }
    }
    return targetDrive;
}

int FmBackupSettings::contentsSelected() const
{
    int ret( 0 );
    // Count selected only
    quint32 mask( mContent );
    while ( mask ){
        if ( mask & 1 ){
            ++ret;
        }
        mask >>= 1;
    }
    return ret;
}

FmBackupEntry* FmBackupSettings::CreateEntry( const QString &title, const QString &tips, const FmBackupEntry::TSettingType type )
{
    FmBackupEntry *entry = new FmBackupEntry( title, tips, type );
    return entry;
}

FmBackupEntry* FmBackupSettings::createContentsEntry()
{
    QString title;
    QString tips;

    title = constFileManagerBackupSettingsTitleContents;

    int selected( contentsSelected() );
    if ( !( mContent & EFileManagerBackupContentAll ) && selected > 1 ){
        // create text as " 5 selected"
        tips = QString( QString::number(selected) + " selected" );
    }
    else
    {
        // create single content name
        tips = contentToString( mContent );
    }

    return CreateEntry( title, tips, FmBackupEntry::EContents );
}

FmBackupEntry* FmBackupSettings::createSchedulingEntry()
{
    QString title;
    QString tips;

    title = constFileManagerBackupSettingsTitleScheduling;
    tips  = schedulingToString( mScheduling );

    return CreateEntry( title, tips, FmBackupEntry::EScheduling );
}

FmBackupEntry* FmBackupSettings::createWeekdayEntry()
{
    QString title;
    QString tips;

    title = constFileManagerBackupSettingsTitleWeekday;
    tips  = weekdayToString( mWeekday );

    return CreateEntry( title, tips, FmBackupEntry::EWeekday );
}

FmBackupEntry* FmBackupSettings::createTimeEntry()
{
    QString title;
    QString tips;

    title = constFileManagerBackupSettingsTitleTime;
    tips  = mTime.toString( "hh:mm" );

    return CreateEntry( title, tips, FmBackupEntry::ETime );
}

FmBackupEntry* FmBackupSettings::createTargetDriveEntry()
{
    QString title;
    QString tips;

    title = constFileManagerBackupSettingsTitleTargetDrive;
    tips  = targetDriveToString( mTargetDrive );

    return CreateEntry( title, tips, FmBackupEntry::ETarget );
}

FmBackupEntry* FmBackupSettings::createBackupDateEntry()
{
    QString tips;
    if( mDate.isNull() )
        {
        tips =  "No previous backups created" ;
        }
    else
        {
        tips = QString( "Backup Created " + mDate.toString( "dd.MM.yyyy" ) );
        }    
    return CreateEntry( QString(""), tips, FmBackupEntry::EBackupdate);
}
QString FmBackupSettings::contentToString( const quint32 content )
{
    QString ret( constFileManagerBackupSettingsContentAll );

    if ( content & EFileManagerBackupContentAll ){
        ret = constFileManagerBackupSettingsContentAll;
    }
    else if ( content & EFileManagerBackupContentSettings ){
        ret = constFileManagerBackupSettingsContentSettings;
    }
    else if ( content & EFileManagerBackupContentMessages ){
        ret = constFileManagerBackupSettingsContentMessages;
    }
    else if ( content & EFileManagerBackupContentContacts ){
        ret = constFileManagerBackupSettingsContentContacts;
    }
    else if ( content & EFileManagerBackupContentCalendar ){
        ret = constFileManagerBackupSettingsContentCalendar;
    }
    else if ( content & EFileManagerBackupContentBookmarks ){
        ret = constFileManagerBackupSettingsContentBookmarks;
    }
    else if ( content & EFileManagerBackupContentUserFiles ){
        ret = constFileManagerBackupSettingsContentUserFiles;
    }
    return ret;
}

QString FmBackupSettings::weekdayToString( const TFileManagerBackupWeekday weekday )
{
    QString ret = "";
    switch (weekday)
    {
    case EFileManagerBackupWeekdayMonday:
        ret = constFileManagerBackupWeekdayMonday;
        break;
    case EFileManagerBackupWeekdayTuesday:
        ret = constFileManagerBackupWeekdayTuesday;
        break;
    case EFileManagerBackupWeekdayWednesday:
        ret = constFileManagerBackupWeekdayWednesday;
        break;
    case EFileManagerBackupWeekdayThursday:
        ret = constFileManagerBackupWeekdayThursday;
        break;
    case EFileManagerBackupWeekdayFriday:
        ret = constFileManagerBackupWeekdayFirday;
        break;
    case EFileManagerBackupWeekdaySaturday:
        ret = constFileManagerBackupWeekdaySaturday;
        break;
    case EFileManagerBackupWeekdaySunday:
        ret = constFileManagerBackupWeekdaySunday;
        break;
    }
    return ret;
}


QString FmBackupSettings::schedulingToString( const TFileManagerBackupSchedule scheduling )
{
    QString ret = "";
    switch (scheduling)
    {
    case EFileManagerBackupScheduleNever:
        ret = constFileManagerBackupScheduleNever;
        break;
    case EFileManagerBackupScheduleDaily:
        ret = constFileManagerBackupScheduleDaily;
        break;
    case EFileManagerBackupScheduleWeekly:
        ret = constFileManagerBackupScheduleWeekly;
        break;
    }
    return ret;
}

QString FmBackupSettings::targetDriveToString( const QString &targetDrive )
{
    //TODO: Get Volume Name and append to return value
    return targetDrive;
}

QList< FmBackupEntry* > FmBackupSettings::backupEntryList()
{
    return mBackupEntryList;
}

void FmBackupSettings::refreshList()
{
    resetAndDestoryBackupEntry();
    FmBackupEntry* entry = 0;

    entry = createContentsEntry();
    mBackupEntryList.push_back( entry );

    entry = createSchedulingEntry();
    mBackupEntryList.push_back( entry );

    if ( mScheduling == EFileManagerBackupScheduleWeekly )
    {
        entry = createWeekdayEntry();
        mBackupEntryList.push_back( entry );
    }

    if ( mScheduling == EFileManagerBackupScheduleWeekly ||
        mScheduling == EFileManagerBackupScheduleDaily )
    {
        entry = createTimeEntry();
        mBackupEntryList.push_back( entry );
    }

    QStringList driveList;
    mBkupEngine->getBackupDriveList( driveList );

    if( driveList.count() > 1 ) {
        entry = createTargetDriveEntry();
        mBackupEntryList.push_back( entry );
    }
    
    entry = createBackupDateEntry();
    mBackupEntryList.push_back( entry );
}

void FmBackupSettings::resetAndDestoryBackupEntry()
{
    for( QList< FmBackupEntry* >::iterator it = mBackupEntryList.begin(); 
        it!= mBackupEntryList.end(); ++it ){
        delete *it;
    }
    mBackupEntryList.clear();
}



// load and save

void FmBackupSettings::load()
{
   
    QStringList driveList;
    mBkupEngine->getBackupDriveList( driveList );
    QString defaultDrive( driveList.first() );
   
    QDate date;
    QSettings settings("Nokia", "FileManager");

    settings.beginGroup("BackupConfigure");

    mContent  = (settings.value("content", 1).toUInt()); // All for default value
    mScheduling = (TFileManagerBackupSchedule)(settings.value("scheduling", EFileManagerBackupScheduleNever ).toInt()); // Never schedule for default value
    mWeekday = (TFileManagerBackupWeekday)(settings.value("weekday", EFileManagerBackupWeekdayMonday ).toInt()); // monday for default value
    mTime = (settings.value("time", QTime::currentTime() ).toTime()); // empty for default
    mTargetDrive = (settings.value("targetDrive", defaultDrive ) ).toString();  // C for default
    mDate = (settings.value("backupDate", date)).toDate();
    settings.endGroup();
    refreshList();
}


void FmBackupSettings::save()
{

    QSettings settings( "Nokia", "FileManager" );
    settings.beginGroup( "BackupConfigure" );

    settings.setValue( "content", mContent );
    settings.setValue( "scheduling", mScheduling );
    settings.setValue( "weekday", mWeekday );
    settings.setValue( "time", mTime.toString() );
    settings.setValue( "targetDrive", mTargetDrive );
    settings.setValue( "backupDate", mDate.toString( Qt::ISODate ) );
    settings.endGroup();

    refreshList();
}

void FmBackupSettings::updateBackupDate()
{
    mDate.setDate( QDate::currentDate().year(), QDate::currentDate().month(), QDate::currentDate().day() );
    save();
}
