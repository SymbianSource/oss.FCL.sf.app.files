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
*     The source file of the file dialog
*
*/

#include "fmfiledialog.h"
#include "fmfiledialog_p.h"
#include "fmfilewidget.h"
#include "fmlogger.h"
#include "fmutils.h"
#include "fmdrivemodel.h"
#include "fmcommon.h"

#include "hbwidget.h"
#include "hblabel.h"
#include "hbaction.h"
#include "hbpushbutton.h"
#include "hblineedit.h"

#include <QGraphicsLinearLayout>

FmFileDialog::FmFileDialog( QGraphicsItem *parent ) : 
    HbDialog( parent ), d_ptr( new FmFileDialogPrivate( this ) )
{
}

FmFileDialog::~FmFileDialog()
{
    delete d_ptr;
}

QString FmFileDialog::getExistingDirectory( HbWidget *parent,
                                    const QString &title,
                                    const QString &dir,
                                    const QStringList &nameFilters,
                                    Options options )
{

    QString ret;

    FmFileDialogPrivate::FmFileDialogArgs args;
    args.mDialogMode = FmFileDialogPrivate::GetDirMode;
    args.mTitle = title;
    args.mDirectory = dir;
    if( options & DisplayAllDirs ) {
        args.mDirFilters = QDir::AllDirs | QDir::NoDotAndDotDot;
    } else {
        args.mDirFilters = QDir::Dirs | QDir::NoDotAndDotDot;
    }
    args.mNameFilters = nameFilters;
    args.mOptions = options;

    FmFileDialog dialog( parent );
    dialog.d_ptr->init( args );
    if( dialog.exec() ) {
        ret = FmUtils::fillPathWithSplash( dialog.d_ptr->currentPath() );
    }
    return ret;
}

QString FmFileDialog::getOpenFileName( HbWidget *parent,
                               const QString &title,
                               const QString &dir,
                               const QStringList &nameFilters,
                               Options options )
{

    QString ret;

    FmFileDialogPrivate::FmFileDialogArgs args;
    args.mDialogMode = FmFileDialogPrivate::GetFileMode;
    args.mTitle = title;
    args.mDirectory = dir;
    if( options & DisplayAllDirs ) {
        args.mDirFilters = QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files;
    } else {
        args.mDirFilters = QDir::Dirs | QDir::NoDotAndDotDot | QDir::Files;
    }
    args.mNameFilters = nameFilters;
    args.mOptions = options;

    FmFileDialog dialog( parent );
    dialog.d_ptr->init( args );
    if( dialog.exec() ) {
        ret = FmUtils::fillPathWithSplash( dialog.d_ptr->currentPath() )
            + dialog.d_ptr->selectedFile();
    }
    return ret;
}


QString FmFileDialog::getSaveFileName( HbWidget * parent,
                               const QString &title,
                               const QString &dir,
                               const QStringList &nameFilters,
                               Options options )
{
    QString ret;

    FmFileDialogPrivate::FmFileDialogArgs args;
    args.mDialogMode = FmFileDialogPrivate::SaveFileMode;
    args.mTitle = title;
    args.mDirectory = dir;
    if( options & DisplayAllDirs ) {
        args.mDirFilters = QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Files;
    } else {
        args.mDirFilters = QDir::Dirs | QDir::NoDotAndDotDot | QDir::Files;
    }
    args.mNameFilters = nameFilters;
    args.mOptions = options;

    FmFileDialog dialog( parent );
    dialog.d_ptr->init( args );
    if( dialog.exec() ) {
        ret = FmUtils::fillPathWithSplash( dialog.d_ptr->currentPath() )
            + dialog.d_ptr->selectedFile();
    }
    return ret;
}



bool FmFileDialog::exec()
{
    if ( d_ptr->isOkAction( HbDialog::exec() ) ) {
        return true;
    } else {
        return false ;
    }
}




