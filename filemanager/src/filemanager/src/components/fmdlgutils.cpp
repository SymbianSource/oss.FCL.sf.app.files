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
 *     The source file of the query sdialog utility of file manager
 */

#include "fmdlgutils.h"
#include "fmcombinedquery.h"
#include "fmtimequery.h"
#include "fmsingletextquery.h"
#include "fmmultitextquery.h"
#include "fmutils.h"

#include <QString>
#include <QStringList>
#include <QMap>

#include <hblabel.h>
#include <hbaction.h>
#include <hbmessagebox.h>

FmDlgUtils::FmDlgUtils( void )
{
}

FmDlgUtils::~FmDlgUtils( void )
{
}

bool FmDlgUtils::showSingleSettingQuery(
        const QString &title,
        const QStringList &textList,
        int &selectedIndex )
{
   bool ret( false );

    int i = 0;
    QMap< int, QString > items;   // Maps list item with corresponding text in viewer
    for( QStringList::const_iterator it = textList.begin(); it!= textList.end(); ++it ){
        items.insert( i, *it );
        ++i;
    }
    bool ok ( false );                // SK return (out parameter)

    FmCombinedQuery *cQuery=new FmCombinedQuery;
    cQuery->setSelectMode( FmCombinedQuery::SingleSelectMode );
    cQuery->setHeadingWidget( new HbLabel( title ) );
    cQuery->setListItems( items );

    if( selectedIndex != -1 ){
	cQuery->setMarkedListItem( selectedIndex );
    }

    QString sk1 ( tr ( "ok" ) );
    QString sk2 ( tr ("cancel" ) );

    if(sk1.isEmpty() == false) {
        HbAction *primary = new HbAction( sk1 );
        cQuery->setPrimaryAction( primary );
    }

    if( sk2.isEmpty() == false ) {
        HbAction *secondary = new HbAction( sk2 );
        cQuery->setSecondaryAction( secondary );
    }
    HbAction* action = cQuery->exec();
    if( action == cQuery->secondaryAction() ){
        ok = false;
    }
    else{
        ok = true;
    }
    if( ok ){
        //get selected key list.
        QList<int> selectedKeyList = cQuery->selectedKeys();

        // Get new selection and notify changes
        int selectionResult( 0 );
        int i = 0;
        for ( QMap< int, QString >::const_iterator it = items.begin(); it != items.end(); ++it ){
            // if one item has been selected( judge if the key is in the selected key list)
            if ( selectedKeyList.indexOf( it.key() ) >= 0 ) {
                selectionResult = i;
				break;
            }
            ++i;
        }
        if ( selectionResult != selectedIndex ){
            selectedIndex = selectionResult;
            ret = true;
        }
    }
    delete cQuery;
    return ret;
}

bool FmDlgUtils::showMultiSettingQuery(
        const QString &title,
        const QStringList &textList,
        quint32 &selection,
        int dominantIndex )
{
    bool ret( false );

    int i = 0;
    QMap<int, QString> items;   // Maps list item with corresponding text in viewer
    for( QStringList::const_iterator it = textList.begin(); it!= textList.end(); ++it){
        items.insert( i, *it );
        ++i;
    }
    bool ok ( false );                // SK return (out parameter)

    FmCombinedQuery *cQuery=new FmCombinedQuery;
    cQuery->setSelectMode( FmCombinedQuery::MultiSelectMode );
    cQuery->setHeadingWidget( new HbLabel( title ) );
    cQuery->setListItems( items );
	cQuery->setDominantIndex( dominantIndex );

    // Create selection array
    int count( textList.count() );
    for ( int i( 0 ); i < count; ++i ){
        if ( selection & ( 1 << i ) ){
            cQuery->setMarkedListItem( i );
        }
    }


    QString sk1 ( tr( "ok" ) );
    QString sk2 ( tr( "cancel" ) );

    if(sk1.isEmpty() == false) {
        HbAction *primary = new HbAction( sk1 );
        cQuery->setPrimaryAction( primary );
    }

    if( sk2.isEmpty() == false ) {
        HbAction *secondary = new HbAction( sk2 );
        cQuery->setSecondaryAction( secondary );
    }
    HbAction* action = cQuery->exec();
    if( action == cQuery->secondaryAction() ){
        ok = false;
    }
    else{
        ok = true;
    }
    if( ok )
    {
        //get selected key list.
        QList<int> selectedKeyList = cQuery->selectedKeys();

        // Get new selection and notify changes
        quint32 selectionResult( 0 );
        int i = 0;
        for ( QMap< int, QString >::const_iterator it = items.begin(); it != items.end(); ++it ){
            // if one item has been selected( judge if the key is in the selected key list)
            if ( selectedKeyList.indexOf( it.key() ) >= 0 ) {
                selectionResult |= 1 << i;
            }
            ++i;
        }
        if ( selectionResult != selection ){
            selection = selectionResult;
            ret = true;
        }
    }
    delete cQuery;
    return ret;
}

bool FmDlgUtils::showTimeSettingQuery(
        const QString &title, QTime &time )
{
    bool ret( false );
    bool ok ( false );                // SK return (out parameter)

    FmTimeQuery *cQuery=new FmTimeQuery;
    cQuery->setHeadingWidget( new HbLabel( title ) );
    cQuery->setTime( time );

    QString sk1 ( tr( "ok" ) );
    QString sk2 ( tr( "cancel" ) );

    if(sk1.isEmpty() == false) {
        HbAction *primary = new HbAction( sk1 );
        cQuery->setPrimaryAction( primary );
    }

    if( sk2.isEmpty() == false ) {
        HbAction *secondary = new HbAction( sk2 );
        cQuery->setSecondaryAction( secondary );
    }
    HbAction* action = cQuery->exec();
    if( action == cQuery->secondaryAction() ){
        ok = false;
    }
    else{
        ok = true;
    }
    if( ok )
    {
        QTime resultTime = cQuery->getTime();
        if ( resultTime != time ){
            time = resultTime;
            ret = true;
        }
    }
    delete cQuery;
    return ret;
}
bool FmDlgUtils::showTextQuery(
        const QString& title, QString& driveName )
{
    bool ret( false );
    bool ok ( true );                // SK return (out parameter)

    FmSingleTextQuery *cQuery=new FmSingleTextQuery;
    cQuery->setHeadingWidget( new HbLabel( title ) );

    QString sk1 ( tr ( "ok" ) );
    QString sk2 ( tr ( "cancel" ) );

    if(sk1.isEmpty() == false) {
        HbAction *primary = new HbAction( sk1 );
        cQuery->setPrimaryAction( primary );
    }

    if( sk2.isEmpty() == false ) {
        HbAction *secondary = new HbAction( sk2 );
        cQuery->setSecondaryAction( secondary );
    }
    cQuery->setLineEditText( driveName );
    cQuery->checkActions();
    
    HbAction* action = cQuery->exec();
    if( action == cQuery->secondaryAction() ){
        ok = false;
    }
    else{
        ok = true;
    }

    if( ok ){
        QString newName = cQuery->getLineEditText();
        if ( newName != driveName ){
            driveName = newName;
            ret = true;
        }
    }
    delete cQuery;
    return ret;
}

bool FmDlgUtils::showConfirmPasswordQuery(
        const QString &title, const QString &driveName, QString &pwd )
{
    bool ret( false );
    bool ok ( true );               // SK return (out parameter)

    FmSingleTextQuery *cQuery = new FmSingleTextQuery( HbLineEdit::Password );
    cQuery->setHeadingWidget( new HbLabel( title ) );

    QString sk1 ( tr ("ok" ) );
    QString sk2 ( tr ("cancel" ) );

    if(sk1.isEmpty() == false) {
        HbAction *primary = new HbAction( sk1 );
        cQuery->setPrimaryAction( primary );
    }

    if( sk2.isEmpty() == false ) {
        HbAction *secondary = new HbAction( sk2 );
        cQuery->setSecondaryAction( secondary );
    }
    cQuery->checkActions();
    HbAction* action = 0;
    
    bool pwdOk( false );
    
    while( ok && !pwdOk ){
        action = cQuery->exec();
        if( action == cQuery->secondaryAction() ){
            ok = false;
        }
        else{
            ok = true;
        }

        if( ok ){
            QString inputPwd = cQuery->getLineEditText();
            if ( FmUtils::setDrivePwd( driveName, inputPwd, inputPwd ) == 0 ){
                pwd = inputPwd;
                pwdOk = true;
                ret = true;
            }
            else{
                HbMessageBox::information( tr( "The password is incorrect, try again!" ) );
            }
         }
    }
    delete cQuery;
    return ret;
}

bool FmDlgUtils::showChangePasswordQuery(     
       const QString &firstLabel, const QString &secondLabel, QString& newPassword )
{
    bool ret( false );
    bool ok ( true );                // SK return (out parameter)

    FmMultiTextQuery *cQuery = new FmMultiTextQuery( HbLineEdit::Password );
    cQuery->setFirstLabelText( firstLabel );
    cQuery->setSecondLabelText( secondLabel );

    QString sk1 = ( tr( "ok" ) );
    QString sk2 = ( tr( "cancel" ) );

    if(sk1.isEmpty() == false) {
        HbAction *primary = new HbAction( sk1 );
        cQuery->setPrimaryAction( primary );
    }

    if( sk2.isEmpty() == false ) {
        HbAction *secondary = new HbAction( sk2 );
        cQuery->setSecondaryAction( secondary );
    }
    cQuery->checkActions();
    HbAction* action = 0;
    
    bool pwdOk( false );
    
    while( ok && !pwdOk ){
        action = cQuery->exec();
        if( action == cQuery->secondaryAction() ){
            ok = false;
        }
        else{
            ok = true;
        }

        if( ok ){
            QString firstPwd = cQuery->getFirstEditText();
            QString secondPwd = cQuery->getSecondEditText();
            if ( firstPwd == secondPwd ){
                newPassword = secondPwd;
                pwdOk = true;
                ret = true;
            }
            else{
                HbMessageBox::information( tr( "The passwords do not match, try again!" ) );
            }
         }
    }
    delete cQuery;
    return ret;
}
