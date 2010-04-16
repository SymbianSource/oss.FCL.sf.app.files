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
*     The header file of the file find widget of file manager
*
*/

#ifndef FMFINDWIDGET_H
#define FMFINDWIDGET_H

#include "fmcommon.h"
#include "fmfindresultmodel.h"

#include <hbwidget.h>

class HbListView;
class HbSearchPanel;
class HbWidget;

class QGraphicsLinearLayout;

class FmFindWidget : public HbWidget
{
    Q_OBJECT
public:
    enum ContentWidgetType
    {
        EmptyTipWidget,
        ResultListView
    };
    FmFindWidget( QGraphicsItem *parent = 0 );
    ~FmFindWidget();

    void find( const QString &keyword, const QString &path );
    void stopFind();
    void sortFiles( FmFindResultModel::SortFlag sortFlag );
    
    void activeSearchPanel();
    void deActiveSearchPanel();

signals:
    void finished();
    void activated( const QString &pathName );
    
    void startSearch( const QString &targetPath, const QString &criteria );

private slots:
    void itemActivated(const QModelIndex &index);
        
    void on_searchPanel_criteriaChanged( const QString &criteria );
    void on_searchPanel_exitClicked();

    void on_resultModel_finished();
    void on_resultModel_modelCountChanged( int count );
        
private:
    void init();
    void initSearchPanel();
    void activateContentWidget( ContentWidgetType contentWidgetType );

private:
    HbListView          *mListView;
    FmFindResultModel   *mModel;
    HbWidget            *mEmptyTipWidget;
    
    QGraphicsLinearLayout *mLayout;
    HbSearchPanel* mSearchPanel;
    QString mFindTargetPath;
};

#endif
