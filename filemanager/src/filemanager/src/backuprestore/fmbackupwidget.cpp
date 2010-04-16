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
 *     The header file of the back up widget of file manager
 */

#include "fmbackupwidget.h"
#include "fmbackupsettings.h"
#include "fmdlgutils.h"
#include "fmviewmanager.h"
#include "fmoperationservice.h"
#include "fmbkupengine.h"
#include "fmbackuprestorehandler.h"
#include "fmutils.h"


#include <QGraphicsLinearLayout>
#include <QDir>

#include <hbdataform.h>
#include <hbdataformmodel.h>
#include <hbdataformmodelitem.h>
#include <hbdataformviewitem.h>



FmBackupWidget::FmBackupWidget( QGraphicsItem *parent )
: HbWidget( parent ), mLastPressedItem( 0 ), mScrolled( 0 )
{
    init();
}

FmBackupWidget::~FmBackupWidget()
{
    mDataForm->setModel( 0 );
    delete mModel;
}

void FmBackupWidget::on_list_released( const QModelIndex &index )
{
    HbDataFormModelItem *item = mModel->itemFromIndex(index);
    if( item != mLastPressedItem || mDataForm->isScrolling() || mScrolled ) {
        mScrolled = false;
        return;
    }
    mScrolled = false;

    if( item == mContentsItem ){
        ChangeContents();
    } else if( item == mTargetItem ){
        ChangeTargetDrive();
    } else if( item == mSchedulingItem ){
        ChangeScheduling();
    } else if( item == mWeekdayItem ){
        ChangeWeekday();
    } else if( item == mTimeItem ){
        ChangeTime();
    }
}

void FmBackupWidget::on_list_pressed( const QModelIndex &index )
{
    mLastPressedItem = mModel->itemFromIndex(index);
}

void FmBackupWidget::on_list_scrollingStarted()
{
    mScrolled = true;
}

void FmBackupWidget::init()
{
    QGraphicsLinearLayout *vLayout = new QGraphicsLinearLayout( this );
    vLayout->setOrientation( Qt::Vertical );


    mModel = new HbDataFormModel();
    initModel();

    mDataForm = new HbDataForm( this );
    mDataForm->setModel( mModel );
    vLayout->addItem( mDataForm );
    
    connect( mDataForm, SIGNAL( released( const QModelIndex & ) ),
             this, SLOT( on_list_released( const QModelIndex & ) ) );

    connect( mDataForm, SIGNAL( pressed( const QModelIndex & ) ),
             this, SLOT( on_list_pressed( const QModelIndex & ) ) );

    connect( mDataForm, SIGNAL( scrollingStarted() ),
             this, SLOT( on_list_scrollingStarted() ) );

    connect( this, SIGNAL( doModelRefresh() ),
             this, SLOT( refreshModel() ), Qt::QueuedConnection );

    //mBackupSettings = new FmBackupSettings();
    mBackupSettings = FmViewManager::viewManager()->operationService()->backupRestoreHandler()->bkupEngine()->BackupSettingsL();
    mBackupSettings->load();

    emit doModelRefresh();

}

void FmBackupWidget::expandAllGroup()
{
    mDataForm->setModel( 0 );
    mDataForm->setModel( mModel );

    HbDataFormViewItem *itemScheduling = 
        (HbDataFormViewItem *)(mDataForm->itemByIndex(mModel->indexFromItem(mSchedulingGroup)));
    if( itemScheduling ){
        itemScheduling->setExpanded( true );
    }

    HbDataFormViewItem *itemGroup = 
        (HbDataFormViewItem *)(mDataForm->itemByIndex(mModel->indexFromItem(mContentsGroup)));
    if( itemGroup ){
        itemGroup->setExpanded( true );
    }
}

void FmBackupWidget::initModel()
{
    mContentsGroup = mModel->appendDataFormGroup(
        QString( tr( "Contents" ) ), mModel->invisibleRootItem());

    mContentsItem = mModel->appendDataFormItem(
        HbDataFormModelItem::TextItem, QString( "" ), mContentsGroup );
    mContentsItem->setContentWidgetData( QString("readOnly"), QString("true") );

    mTargetItem = mModel->appendDataFormItem(
        HbDataFormModelItem::TextItem, QString( "" ), mContentsGroup );
    mTargetItem->setContentWidgetData( QString("readOnly"), QString("true") );

    mSchedulingGroup = mModel->appendDataFormGroup(
        QString( tr( "Scheduling" ) ), mModel->invisibleRootItem());

    mSchedulingItem = mModel->appendDataFormItem(
        HbDataFormModelItem::TextItem, QString( "" ), mSchedulingGroup );
    mSchedulingItem->setContentWidgetData( QString("readOnly"), QString("true") );

    mWeekdayItem = mModel->appendDataFormItem(
        HbDataFormModelItem::TextItem, QString( "" ), mSchedulingGroup );
    mWeekdayItem->setContentWidgetData( QString("readOnly"), QString("true") );

    mTimeItem = mModel->appendDataFormItem(
        HbDataFormModelItem::TextItem, QString( "" ), mSchedulingGroup );
    mTimeItem->setContentWidgetData( QString("readOnly"), QString("true") );
}


void FmBackupWidget::refreshModel()
{
    mDataForm->setModel( 0 );

    mBackupSettings->load();
    
    bool isWeekdayExist = false;
    bool isTimeExist = false;
   
    QList< FmBackupEntry* > backupEntryList = mBackupSettings->backupEntryList();

    QList< FmBackupEntry* >::iterator it;

    for (  it = backupEntryList.begin(); it != backupEntryList.end(); ++it ){
    
        int type = ( *it )->type();

        QString title = ( *it )->title();
        QString tips  = ( *it )->tips();
      
        switch ( type )
        {
        case FmBackupEntry::EContents:
            {
            mContentsItem->setLabel(title);
            mContentsItem->setContentWidgetData( QString("text"), tips );
            break;
            }
        case FmBackupEntry::ETarget:
            {
            mTargetItem->setLabel(title);
            mTargetItem->setContentWidgetData( QString("text"), tips );
            break;
            }
        case FmBackupEntry::EScheduling:
            {
            mSchedulingItem->setLabel(title);
            mSchedulingItem->setContentWidgetData( QString("text"), tips );
            break;
            }
        case FmBackupEntry::EWeekday:
            {
            mWeekdayItem->setLabel(title);
            mWeekdayItem->setContentWidgetData( QString("text"), tips );
            isWeekdayExist = true;
            break;
            }
        case FmBackupEntry::ETime:
            {
            mTimeItem->setLabel(title);
            mTimeItem->setContentWidgetData( QString("text"), tips );
            isTimeExist = true;
            break;
            }
        }
    }
    int index = 0;

    index = mSchedulingGroup->indexOf(  mWeekdayItem );
    if( index >=0 ){
        mSchedulingGroup->removeChild( index );
    }

    index = mSchedulingGroup->indexOf(  mTimeItem );
    if( index >=0 ){
        mSchedulingGroup->removeChild( index );
    }

    if( isWeekdayExist ) {
            mSchedulingGroup->appendChild( mWeekdayItem );
        }

    if( isTimeExist ){
            mSchedulingGroup->appendChild( mTimeItem );
        }

    mDataForm->setModel( mModel );

    expandAllGroup();
}



void FmBackupWidget::ChangeContents()
{
    
    QString title = constFileManagerBackupSettingsTitleContents;  
    QStringList queryStringList;
    quint32 contentMask = FmBackupSettings::EFileManagerBackupContentAll;
    while ( contentMask <= FmBackupSettings::EFileManagerBackupContentLast)
    {
        queryStringList.push_back( mBackupSettings->contentToString( contentMask ) );
        contentMask = contentMask << 1;
    }
    quint32 select = mBackupSettings->content();
    if( FmDlgUtils::showMultiSettingQuery( title, queryStringList, select, 0 ) )
    {
        mBackupSettings->setContent( select );
        emit doModelRefresh();
    }
    
} 


void FmBackupWidget::ChangeScheduling()
{
    QString title = constFileManagerBackupSettingsTitleScheduling;  
    QStringList queryStringList;

    FmBackupSettings::TFileManagerBackupSchedule schedule = FmBackupSettings::EFileManagerBackupScheduleNever;
    while ( schedule <= FmBackupSettings::EFileManagerBackupScheduleWeekly )
    {
        queryStringList.push_back( mBackupSettings->schedulingToString( schedule ) );
        schedule = FmBackupSettings::TFileManagerBackupSchedule(((int)schedule)+1);
    }
    
    // adjust index offset against monday.
    int selectIndex = mBackupSettings->scheduling() - FmBackupSettings::EFileManagerBackupScheduleNever;
    if( FmDlgUtils::showSingleSettingQuery( title, queryStringList, selectIndex ) )
    {
        selectIndex += FmBackupSettings::EFileManagerBackupScheduleNever;
        mBackupSettings->setScheduling( ( FmBackupSettings::TFileManagerBackupSchedule )selectIndex );
        emit doModelRefresh();
    }
}

void FmBackupWidget::ChangeWeekday()
{
    QString title = constFileManagerBackupSettingsTitleWeekday;  
    QStringList queryStringList;

    FmBackupSettings::TFileManagerBackupWeekday weekday = FmBackupSettings::EFileManagerBackupWeekdayMonday;
    while ( weekday <= FmBackupSettings::EFileManagerBackupWeekdaySunday )
    {
        queryStringList.push_back( mBackupSettings->weekdayToString( weekday ) );
        weekday = FmBackupSettings::TFileManagerBackupWeekday(((int)weekday)+1);
    }
    
    // adjust index offset against monday.
    int selectIndex = mBackupSettings->weekday() - FmBackupSettings::EFileManagerBackupWeekdayMonday;
    if( FmDlgUtils::showSingleSettingQuery( title, queryStringList, selectIndex ) )
    {
        selectIndex += FmBackupSettings::EFileManagerBackupWeekdayMonday;
        mBackupSettings->setWeekday( (FmBackupSettings::TFileManagerBackupWeekday)selectIndex );
        emit doModelRefresh();
    }
}

void FmBackupWidget::ChangeTime()
{
    QString title = constFileManagerBackupSettingsTitleTime;  
    QTime queryTime = mBackupSettings->time();

    if( FmDlgUtils::showTimeSettingQuery( title, queryTime ) )
    {
        mBackupSettings->setTime( queryTime );
        emit doModelRefresh();
    }
}


void FmBackupWidget::ChangeTargetDrive()
{
    QString title = constFileManagerBackupSettingsTitleTargetDrive;  
    QStringList queryStringList;
    QStringList driveStringList;

    QStringList driveList;
    FmUtils::getDriveList( driveList, true );
    QString targetDrive =  mBackupSettings->targetDrive();
    int selectIndex = -1;

    int currentIndex = 0;
    for( QStringList::const_iterator it = driveList.begin(); it != driveList.end(); ++it )
    {
        QString drive = (*it);
        drive = FmUtils::removePathSplash( drive );
        QString driveWithVolume = FmUtils::fillDriveVolume( drive, true );

        driveStringList.push_back( drive );
        queryStringList.push_back( driveWithVolume );


        if( drive == targetDrive )
        {
            // adjust index offset against drive.
            selectIndex = currentIndex;
        }
        ++currentIndex;
    }
    
    if( FmDlgUtils::showSingleSettingQuery( title, queryStringList, selectIndex ) )
    {
        mBackupSettings->setTargetDrive( driveStringList.at( selectIndex ) );
        emit doModelRefresh();
    }
}
