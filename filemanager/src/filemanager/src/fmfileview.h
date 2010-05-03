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
*     Zhiqiang Yang <zhiqiang.yang@nokia.com>
* 
* Description:
*     The header file of the file browse view of file manager
*
*/

#ifndef FMFILEVIEW_H
#define FMFILEVIEW_H

#include "fmcommon.h"
#include "fmfilebrowsewidget.h"
#include "fmviewbase.h"


class HbLineEdit;
class HbPushButton;
class HbMenu;
class FmOperationService;

class FmFileView : public FmViewBase
{
    Q_OBJECT
public:
    FmFileView();
    virtual ~FmFileView();

    void setRootLevelPath( const QString &pathName );
    void setFindDisabled( bool display );

public slots:
    void setRootPath( const QString &pathName );
    void refreshModel( const QString &path );

signals:
    // connected by viewmanager to close view
    void popViewAndShow();

private slots:

	void on_switchStyle_triggered();
	void on_setSelectable_triggered();
	void on_delete_triggered();
    void on_copy_triggered();
    void on_move_triggered();
    void on_newFolder_triggered();
	void on_upAction_triggered();
	void on_leftAction_triggered();
	void on_rightAction_triggered();
	void on_sortNameAction_triggered();
	void on_sortTimeAction_triggered();
	void on_sortSizeAction_triggered();
	void on_sortTypeAction_triggered();
	
    void startSearch( const QString &targetPath, const QString &criteria );
    void on_mainWidget_setEmptyMenu( bool isMenuEmpty );
    void on_mainWidget_setTitle( const QString &title );

#ifdef FM_CHANGE_ORIENT_ENABLE
    void on_rotateAction_triggered();
#endif

private:
    void initMenu();
    void initMainWidget();
    void initToolBar();

	void setStyle( FmFileBrowseWidget::Style style );
	void setSelectable( bool enable );
	void infoNoFileSelected();
	void removeToolBarAction();
	
private:
    FmFileBrowseWidget *mWidget;
    HbPushButton *mUpButton;
    //HbLineEdit *mLineEdit;
	HbAction *mStyleAction;
	HbAction *mSelectableAction;
    HbAction *mFindAction;
    QString  rootLevelPath;

    FmOperationService  *mOperationService;
    
    HbAction *mToolBarRightAction;
    HbMenu *mMenu;
};

#endif

