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
*     The source file of the file find widget of file manager
*
*/

#include "fmfindwidget.h"
#include "fmfindresultmodel.h"
#include "fmviewmanager.h"
#include "fmoperationservice.h"

#include <QGraphicsLinearLayout>
#include <QFileInfo>

#include <hblistview.h>
#include <hbsearchpanel.h>
#include <hblabel.h>
#include <hbwidget.h>

FmFindWidget::FmFindWidget( QGraphicsItem *parent )
    : HbWidget( parent )
{
    init();
}

FmFindWidget::~FmFindWidget()
{
}

void FmFindWidget::itemActivated(const QModelIndex &index)
{   
    FmOperationService *operationService = FmViewManager::viewManager()->operationService();
    if( !operationService ) {
        return;
    }

    QFileInfo fileInfo = mModel->fileInfo( index );
    if ( fileInfo.isFile() ) {
        operationService->syncLaunchFileOpen( fileInfo.filePath() );
    }else if( fileInfo.isDir() ) {
        emit activated( fileInfo.filePath() );
    }
}

void FmFindWidget::find( const QString &keyword, const QString &path )
{
    mModel->setFindPath( path );
    
    QRegExp regExp( '*' + keyword + '*' );
    regExp.setPatternSyntax( QRegExp::Wildcard );
    regExp.setCaseSensitivity( Qt::CaseInsensitive );
    mModel->setPattern( regExp );

    mModel->find();
}

void FmFindWidget::stopFind()
{
    mModel->stop();
}

void FmFindWidget::on_resultModel_finished()
{
    emit finished();
    //Since layout problem is found, refresh it
    on_resultModel_refresh();  
}

void FmFindWidget::on_resultModel_modelCountChanged( int count )
{
    if( count > 0 ) {
        activateContentWidget( ResultListView );
    } else {
        activateContentWidget( EmptyTipWidget );
    }
}

void FmFindWidget::activateContentWidget( ContentWidgetType contentWidgetType )
{
    switch( contentWidgetType )
    {
    case EmptyTipWidget:
        {
        if( mLayout->count() > 0 ) {
            if( mLayout->itemAt( 0 ) == mListView ) {
                mLayout->removeItem( mListView );
                mLayout->addItem( mEmptyTipWidget );
            } 
        } else {
            mLayout->addItem( mEmptyTipWidget );
        }
        mListView->hide();
        mEmptyTipWidget->show();
        deActiveSearchPanel();
        emit setEmptyMenu( true );
        }
        break;
    case ResultListView:
        {
        if( mLayout->count() > 0 ) {
            if( mLayout->itemAt( 0 ) == mEmptyTipWidget ) {
                mLayout->removeItem( mEmptyTipWidget );
                mLayout->addItem( mListView );
            } 
        } else {
            mLayout->addItem( mListView );
        }
        mEmptyTipWidget->hide();
        mListView->show();
        activeSearchPanel();
        emit setEmptyMenu( false );
        }
        break;
    }
}

void FmFindWidget::init()
{
    mLayout = new QGraphicsLinearLayout( this );
    mLayout->setOrientation( Qt::Vertical );

    mModel = new FmFindResultModel( this );
    mModel->setObjectName( "resultModel" );

    connect( mModel, SIGNAL(finished()), this, SLOT( on_resultModel_finished()) );

    connect( mModel, SIGNAL( modelCountChanged( int )),
        this, SLOT( on_resultModel_modelCountChanged( int )) );
    
    connect( mModel, SIGNAL( refresh()),
        this, SLOT( on_resultModel_refresh()) );

    mListView = new HbListView( this );
    mListView->setModel( mModel );

    mEmptyTipWidget = new HbWidget( this );
    QGraphicsLinearLayout *emptyTipLayout = new QGraphicsLinearLayout( mEmptyTipWidget );
    HbLabel *emptyTipLable = new HbLabel( hbTrId( "No found files or folders" ), mEmptyTipWidget );
    emptyTipLayout->addItem( emptyTipLable );
 
    initSearchPanel();
    activateContentWidget( EmptyTipWidget );
    
    setLayout( mLayout );

    connect( mListView, SIGNAL(activated(QModelIndex)), this, SLOT(itemActivated(QModelIndex)));
}

void FmFindWidget::initSearchPanel()
{
    mSearchPanel = new HbSearchPanel( this );
    mSearchPanel->setObjectName( "searchPanel" );
//    mSearchPanel->setSearchOptionsEnabled( true );
    mSearchPanel->setProgressive( false );
    mSearchPanel->hide();
    
    connect( mSearchPanel, SIGNAL( criteriaChanged( const QString & ) ),
        this, SLOT( on_searchPanel_criteriaChanged( const QString & ) ) );
    
    connect( mSearchPanel, SIGNAL( exitClicked() ),
        this, SLOT( on_searchPanel_exitClicked() ) );
}

void FmFindWidget::sortFiles( FmFindResultModel::SortFlag sortFlag )
{
    mModel->sort( sortFlag );
}

void FmFindWidget::activeSearchPanel()
{
    mLayout->addItem( mSearchPanel );
    mSearchPanel->show();
}

void FmFindWidget::on_searchPanel_criteriaChanged( const QString &criteria )
{
    mFindTargetPath.clear();
    emit startSearch( mFindTargetPath, criteria );
}

void FmFindWidget::on_searchPanel_exitClicked()
{
    mSearchPanel->hide();
    mLayout->removeItem( mSearchPanel );
}

void FmFindWidget::deActiveSearchPanel()
{
    mSearchPanel->hide();
    mLayout->removeItem( mSearchPanel );

}

void FmFindWidget::on_resultModel_refresh()
{
    mListView->setModel( 0 );
    mListView->setModel( mModel );  
}



