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
*     The header file of the widget to browse files
*
*/

#ifndef FMFILEBROWSEWIDGET_H
#define FMFILEBROWSEWIDGET_H

#include "fmcommon.h"
#include "fmoperationservice.h"

#include <QFileInfo>

#include <hbwidget.h>
#include <QDir>

class QDirModel;
class QGraphicsLinearLayout;
class HbListView;
class HbTreeView;
class HbAbstractViewItem;
class HbSearchPanel;

class FmFileBrowseWidget : public HbWidget
{
    Q_OBJECT

public:
    enum Style {
        NoStyle,
        ListStyle,
        TreeStyle
    };
    
    enum TSortType{
        ESortByName,
        ESortByTime,
        ESortBySize,
        ESortByType
    };

    explicit FmFileBrowseWidget( HbWidget *parent = 0, FmFileBrowseWidget::Style style = ListStyle  );
    ~FmFileBrowseWidget();

    QFileInfo currentPath() const;
    QList<QFileInfo> checkedItems() const;
	Style style() const { return mStyle; }
    void setStyle( FmFileBrowseWidget::Style style );
	bool selectable() const;
	void setSelectable( bool enable );
	void clearSelection();

    bool rename( const QString &oldName, const QString &newName );
    void setModelFilter( QDir::Filters filters );
    
    void refreshModel( const QString& path );
    void sortFiles( TSortType sortType );
    void activeSearchPanel();

public slots:
    bool cdUp();
    void setRootPath( const QString &pathName );
    
    void on_searchPanel_searchOptionsClicked();
    void on_searchPanel_criteriaChanged( const QString &criteria );
    void on_searchPanel_exitClicked();

signals:
    void currentPathChanged( QString& );
    void startSearch( const QString &targetPath, const QString &criteria );

private slots:
    void on_list_activated( const QModelIndex &index );
    void on_tree_activated( const QModelIndex &index );
	void on_list_longPressed( HbAbstractViewItem *item, const QPointF &coords );
    void on_tree_longPressed( HbAbstractViewItem *item, const QPointF &coords );
    
    void on_viewAction_triggered();
    void on_copyAction_triggered();
    void on_moveAction_triggered();
    void on_deleteAction_triggered();
    void on_renameAction_triggered();
    void on_sendAction_triggered();

private:
    void initListView();
    void initTreeView();
    void initFileModel();
	void initLayout();
	void initSearchPanel();

    void changeRootIndex( const QModelIndex &index );
	bool isDriver(const QModelIndex &index) const;
	bool isFolder(const QModelIndex &index) const;
    
	HbTreeView *mTreeView;
    HbListView *mListView;
    QGraphicsLinearLayout *mLayout;
    QDirModel *mModel;
    
    bool mSelectable;
    Style mStyle;

	HbAbstractViewItem* mCurrentItem;	
	FmOperationService *mOperationService;
    
    QString mFindTargetPath;
    HbSearchPanel* mSearchPanel;
};

#endif
