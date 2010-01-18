/*
* Copyright (c) 2002-2008 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0""
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Navigation pane for file manager
*
*/


// INCLUDES
#include <coecntrl.h>
#include <avkon.rsg>
#include <aknconsts.h>
#include <StringLoader.h>
#include <bautils.h>
#include <featmgr.h>
#include <AknsSkinInstance.h>
#include <AknsUtils.h>
#include <aknlayoutscalable_avkon.cdl.h>
#include <FileManagerView.rsg>
#include <FileManagerEngine.hrh>
#include "Cfilemanagerfoldernavigationpane.h"
#include "MFileManagerFolderNaviObserver.h"
#ifdef RD_TACTILE_FEEDBACK
#include <touchfeedback.h>
#endif //RD_TACTILE_FEEDBACK

// CONSTANTS
const TUint KFmgrRoot = 0;
const TUint KFmgrFirstLevel = 1;

// needed because _LIT macro does not expand parameter, which is also macro
#define _CREATE_LIT( a, b ) _LIT( a, b )

_CREATE_LIT( KFileManagerMifFile, filemanager_mbm_file_location );


// -----------------------------------------------------------------------------
// CFileManagerFolderNavigationPane::TIndicatorSet::Compare
// 
// -----------------------------------------------------------------------------
// 
TBool CFileManagerFolderNavigationPane::TIndicatorSet::Compare( 
        const TIndicatorSet& aFirst,
        const TIndicatorSet& aSecond )
    { 
    return ( aFirst.iId == aSecond.iId ); 
    }

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFileManagerFolderNavigationPane::~CFileManagerFolderNavigationPane
// 
// -----------------------------------------------------------------------------
// 
EXPORT_C CFileManagerFolderNavigationPane::~CFileManagerFolderNavigationPane()
    {
    delete iLongTapDetector;
    iIndicatorArray.Reset();
    delete iFolderBitmap;
    delete iFolderMask;
    delete iRootBitmap;
    delete iRootMask;
    delete iSeparator;
    delete iBmpFile;
    delete iEndChar;
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderNavigationPane::CFileManagerFolderNavigationPane
// 
// -----------------------------------------------------------------------------
// 
CFileManagerFolderNavigationPane::CFileManagerFolderNavigationPane( 
        const TInt aRoot,
        const TInt aDepth ) :
    iRoot( aRoot ),
    iMaxDepth( KFileManagerMaxFolderDepth - 1 ), // Exclude the root
    iIndicatorArray( 1 )
    {
    // ensure that iDepth will be positive
    if ( aDepth < 0 )
        { 
        iDepth = 0;
        }
    else
        {
        iDepth = aDepth;
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderNavigationPane::ConstructL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFolderNavigationPane::ConstructL( TResourceReader& aReader )
    {
    // Get File Manager mif drive from exe location
    TFileName exeFileName( RProcess().FileName() );

    iBmpFile = aReader.ReadHBufCL();

    TPtr ptr( iBmpFile->Des() );
    if ( !ptr.CompareF( KFileManagerMifFile ) )
        {
        // Set correct File Manager mif drive
        ptr[ 0 ] = exeFileName[ 0 ];
        }

    iFolderIconId = aReader.ReadInt16();
    iFolderMaskId = aReader.ReadInt16();
    const TInt majorSkinId = static_cast< TInt >( aReader.ReadInt32() );
    const TInt minorSkinId = static_cast< TInt >( aReader.ReadInt32() );
    iSeparator = aReader.ReadHBufCL();
    iEndChar = aReader.ReadHBufCL();

    iAknsItemIdFolder.Set( majorSkinId, minorSkinId );

    TInt count( aReader.ReadInt16() );

    for ( TInt i( 0 ); i < count ; i++ )
        {
        TIndicatorSet set;
        set.iId = aReader.ReadInt16() ;

        HBufC* tmp = aReader.ReadHBufCL();
        TPtr ptr( tmp->Des() );
        if ( !ptr.CompareF( KFileManagerMifFile ) )
            {
            // Set correct File Manager mif drive
            ptr[ 0 ] = exeFileName[ 0 ];
            }
        set.iBmpFile = *tmp;
        delete tmp;

        set.iIconId = aReader.ReadInt16();
        set.iMaskId = aReader.ReadInt16();
        const TInt majorSkinIdRoot = static_cast< TInt >( aReader.ReadInt32() );
        const TInt minorSkinIdRoot = static_cast< TInt >( aReader.ReadInt32() );
        set.iAknsItemIdRoot.Set( majorSkinIdRoot, minorSkinIdRoot );

        iIndicatorArray.AppendL( set );
        }


    LoadFolderBitmapL();

    // Set the default root bitmap
    ChangeRootL( iRoot );

    iLongTapDetector = CAknLongTapDetector::NewL( this );
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderNavigationPane::NewL
// 
// -----------------------------------------------------------------------------
// 
EXPORT_C CFileManagerFolderNavigationPane* 
    CFileManagerFolderNavigationPane::NewL( const TInt aRoot,
                                            const TInt aDepth,
                                            TResourceReader& aReader )
    {
    CFileManagerFolderNavigationPane* self = 
        new( ELeave ) CFileManagerFolderNavigationPane( aRoot, aDepth );
    CleanupStack::PushL( self );
    self->ConstructL( aReader );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderNavigationPane::SetFolderDepth
// 
// -----------------------------------------------------------------------------
// 
EXPORT_C void CFileManagerFolderNavigationPane::SetFolderDepth( const TInt aDepth )
    {
    if ( aDepth < 0 )
        {
        iDepth = 0;
        }
    else
        {
        iDepth = aDepth;
        }

    // ReportChange() is used to notify CoeControl to redraw pane.
    ReportChange();
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderNavigationPane::FolderDepth
// 
// -----------------------------------------------------------------------------
// 
EXPORT_C TInt CFileManagerFolderNavigationPane::FolderDepth() const
    {
    return iDepth;
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderNavigationPane::SizeChanged
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFolderNavigationPane::SizeChanged()
    {

    TSize size(  TSize::EUninitialized );
    AknLayoutUtils::LayoutMetricsSize( AknLayoutUtils::ENaviPane, size );
    TRect parentRect( size );
    // Get pane icon and text layouts
    TAknWindowLineLayout paneIconLayout(
        AknLayoutScalable_Avkon::navi_icon_text_pane_g1().LayoutLine() );
    TAknTextLineLayout paneTextLayout(
        AknLayoutScalable_Avkon::navi_icon_text_pane_t1().LayoutLine() );
    TAknLayoutRect layoutRect;
    TAknLayoutText layoutText;
    layoutRect.LayoutRect( parentRect, paneIconLayout );
    layoutText.LayoutText( parentRect, paneTextLayout );
    // Setup pane items
    TRect itemsRect( layoutRect.Rect() );
    const CFont* font( layoutText.Font() ); // Not owned
    TInt bsWidth( font->MeasureText( *iSeparator ) );
    TInt dotsWidth( font->MeasureText( *iEndChar ) );
    TInt textWidth( KFileManagerMaxFolderDepth * bsWidth + dotsWidth );
    TInt iconWidth( ( itemsRect.Width() - textWidth )
        / KFileManagerMaxFolderDepth );
    TInt i( 0 );

    if ( AknLayoutUtils::LayoutMirrored() )
        {
        TInt offs( itemsRect.Width() );
        for ( i = 0 ; i < KFileManagerMaxFolderDepth; i++ )
            {
            // Root and Folder Icons
            offs -= iconWidth;
            iBitmapLayout[ i ].LayoutRect(
                parentRect,
                paneIconLayout.iC,
                paneIconLayout.il + offs,
                paneIconLayout.it,
                paneIconLayout.ir,
                paneIconLayout.ib,
                iconWidth,
                paneIconLayout.iH
                );
            // Backslashes
            offs -= bsWidth;
            iTextLayout[ i ].LayoutText(
                parentRect,
                paneTextLayout.iFont,
                paneTextLayout.iC,
                paneTextLayout.il + offs,
                paneTextLayout.ir,
                paneTextLayout.iB,
                paneTextLayout.iW,
                paneTextLayout.iJ
                );
            }
        // Dots
        offs -= dotsWidth;
        iTextLayout[ i ].LayoutText(
            parentRect,
            paneTextLayout.iFont,
            paneTextLayout.iC,
            paneTextLayout.il + offs,
            paneTextLayout.ir,
            paneTextLayout.iB,
            paneTextLayout.iW,
            paneTextLayout.iJ
            );
        }
    else
        {
        TInt offs( 0 );
        for ( i = 0 ; i < KFileManagerMaxFolderDepth; i++ )
            {
            // Root and Folder Icons
            iBitmapLayout[ i ].LayoutRect(
                parentRect,
                paneIconLayout.iC,
                paneIconLayout.il + offs,
                paneIconLayout.it,
                paneIconLayout.ir,
                paneIconLayout.ib,
                iconWidth,
                paneIconLayout.iH
                );
            offs += iconWidth;
            // Backslashes
            iTextLayout[ i ].LayoutText(
                parentRect,
                paneTextLayout.iFont,
                paneTextLayout.iC,
                paneTextLayout.il + offs,
                paneTextLayout.ir,
                paneTextLayout.iB,
                paneTextLayout.iW,
                paneTextLayout.iJ
                );
            offs += bsWidth;
            }
        // Dots
        iTextLayout[ i ].LayoutText(
            parentRect,
            paneTextLayout.iFont,
            paneTextLayout.iC,
            paneTextLayout.il + offs,
            paneTextLayout.ir,
            paneTextLayout.iB,
            paneTextLayout.iW,
            paneTextLayout.iJ
            );
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderNavigationPane::Draw
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFolderNavigationPane::Draw( const TRect& /*aRect*/ ) const
    {

    CWindowGc& gc = SystemGc();

    MAknsSkinInstance* skin = AknsUtils::SkinInstance();

    CFbsBitmap* bmp = NULL;
    TRgb color( KRgbWhite ); // Default never used
    bmp = AknsUtils::GetCachedBitmap( skin, KAknsIIDQsnComponentColorBmpCG2 );
    AknsUtils::GetCachedColor( skin, color, 
        KAknsIIDQsnComponentColors, EAknsCIQsnComponentColorsCG2 );

    AknIconUtils::SetSize( iRootMask, iBitmapLayout[ KFmgrRoot ].Rect().Size() );
    if ( bmp )
        {
        iBitmapLayout[ KFmgrRoot ].DrawImage( gc, bmp, iRootMask );
        }
    else
        {
        AknIconUtils::SetSize( iRootBitmap, iBitmapLayout[ KFmgrRoot ].Rect().Size() );
        iBitmapLayout[ KFmgrRoot ].DrawImage( gc, iRootBitmap, iRootMask );
        }

    if ( !bmp )
        {
        AknIconUtils::SetSize( iFolderBitmap, iBitmapLayout[ KFmgrFirstLevel ].Rect().Size() );
        bmp = iFolderBitmap;
        }

        
    // draw folder bitmaps depending on current folderdepth
    TInt count( iDepth > iMaxDepth ? iMaxDepth : iDepth );
    AknIconUtils::SetSize( iFolderMask, iBitmapLayout[ KFmgrFirstLevel ].Rect().Size() );
    for( TInt i( KFmgrFirstLevel ); i <= count; i++ )
        {
        iBitmapLayout[ i ].DrawImage( gc, bmp, iFolderMask );
        }
    
    // draw backslashes between folderbitmaps depending on current folderdepth
    TInt tc( iDepth > ( iMaxDepth + 1 ) ? ( iMaxDepth + 1 ) : iDepth );
    for( TInt j( 0 ); j < tc; j++ )
        {
        const CFont* font = iTextLayout[j].Font(); // Not owned
        gc.UseFont( font );
        gc.SetPenColor( color );
        gc.DrawText( *iSeparator, iTextLayout[j].TextRect(),
            font->AscentInPixels(), CGraphicsContext::ELeft, 0 );
        }
    
    // draw "..." at the end if necessary
    if ( iDepth > iMaxDepth )
        {
        const CFont* font = iTextLayout[iMaxDepth+1].Font(); // Not owned
        gc.UseFont( font );
        gc.SetPenColor( color );
        gc.DrawText( *iEndChar, iTextLayout[iMaxDepth+1].TextRect(),
            font->AscentInPixels(), CGraphicsContext::ELeft, 0 );        
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderNavigationPane::HandleResourceChangeL
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
EXPORT_C void CFileManagerFolderNavigationPane::HandleResourceChangeL( TInt aType )
    {
    // Skin has propably changed so load bitmaps again
    if( aType == KAknsMessageSkinChange )
        {
        LoadFolderBitmapL();

        // Delete old to make sure that bitmaps are reloaded
        delete iRootBitmap;
        iRootBitmap = NULL;
        delete iRootMask;
        iRootMask = NULL;

        ChangeRootL( iRoot );
        }
    }


// -----------------------------------------------------------------------------
// CFileManagerFolderNavigationPane::ReportChange
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFolderNavigationPane::ReportChange()
    {
    // Leave is trapped because it's not critical to functionality.
    // If leave occurs, navipane just may not be redrawn at that very moment.
    TRAP_IGNORE( ReportEventL( MCoeControlObserver::EEventStateChanged ) );
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderNavigationPane::ChangeRootL
// 
// -----------------------------------------------------------------------------
// 
EXPORT_C void CFileManagerFolderNavigationPane::ChangeRootL( const TInt aRoot )
    {
    // Do not reload icon if correct one is already in use
    if ( iRoot == aRoot && iRootBitmap && iRootMask )
        {
        return;
        }

    TIndicatorSet set;
    set.iId = aRoot;

    TInt index( iIndicatorArray.Find( set, TIndicatorSet::Compare ) );

    if ( index >= 0 )
        {
        set = iIndicatorArray[ index ];

        delete iRootBitmap;
        iRootBitmap = NULL;
        delete iRootMask;
        iRootMask = NULL;

        AknsUtils::CreateColorIconL( AknsUtils::SkinInstance(),
            set.iAknsItemIdRoot,
            KAknsIIDQsnIconColors,
            EAknsCIQsnIconColorsCG7,
            iRootBitmap, 
            iRootMask,
            set.iBmpFile,
            set.iIconId, 
            set.iMaskId,
            KRgbBlack );
        iRoot = aRoot;
        ReportChange();
        }

    }

// -----------------------------------------------------------------------------
// CFileManagerFolderNavigationPane::LoadFolderBitmapL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFolderNavigationPane::LoadFolderBitmapL()
    {

    delete iFolderBitmap;
    iFolderBitmap = NULL;
    delete iFolderMask;
    iFolderMask = NULL;
  
    AknsUtils::CreateColorIconL( AknsUtils::SkinInstance(),
        iAknsItemIdFolder,
        KAknsIIDQsnIconColors,
        EAknsCIQsnIconColorsCG7,
        iFolderBitmap, 
        iFolderMask,
        *iBmpFile,
        iFolderIconId, 
        iFolderMaskId,
        KRgbBlack );
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderNavigationPane::SetObserver
// 
// -----------------------------------------------------------------------------
// 
EXPORT_C void CFileManagerFolderNavigationPane::SetObserver(
        MFileManagerFolderNaviObserver* aObserver )
    {
    iObserver = aObserver;
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderNavigationPane::CheckHitFolderLevel
// 
// -----------------------------------------------------------------------------
// 
TInt CFileManagerFolderNavigationPane::CheckHitFolderLevel(
        const TPoint& aPos )
    {
    TInt ret( KErrNotFound );
    TInt count( iDepth + 1 );
    count = Min( KFileManagerMaxFolderDepth, count );
    for ( TInt i( 0 ); i < count; ++i )
        {
        const TRect& bmpRect( iBitmapLayout[ i ].Rect() );
        if ( bmpRect.Contains( aPos ) )
            {
            ret = i;
            break;
            }
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderNavigationPane::HandlePointerEventL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFolderNavigationPane::HandlePointerEventL(
        const TPointerEvent& aPointerEvent )
    {
    if ( !AknLayoutUtils::PenEnabled() )
        {
        return;
        }

    TInt level( KErrNotFound );

    switch ( aPointerEvent.iType )
        {
        case TPointerEvent::EButton1Down:
            {
            level = CheckHitFolderLevel( aPointerEvent.iPosition );
            if ( iObserver )
                {
#ifdef RD_TACTILE_FEEDBACK
				if (KErrNotFound != level)
				    {
				    MTouchFeedback* feedback = MTouchFeedback::Instance();
				    if ( feedback )
				        {
				        feedback->InstantFeedback( this, ETouchFeedbackBasic );
				        }
				    }
#endif //RD_TACTILE_FEEDBACK

                iObserver->HandleFolderNaviEventL(
                    MFileManagerFolderNaviObserver::ENaviTapDown,
                    level );
                }
            iLongTap = EFalse;
            break;
            }
        case TPointerEvent::EButton1Up:
            {
            level = CheckHitFolderLevel( aPointerEvent.iPosition );
            if ( !iLongTap )
                {
                if ( iObserver )
                    {
                    iObserver->HandleFolderNaviEventL(
                        MFileManagerFolderNaviObserver::ENaviTapUp,
                        level );
                    }
                }
            else
                {
                iLongTap = EFalse;
                }
            break;
            }
        default:
            {
            break;
            }
        }

    if ( iLongTapDetector && level != KErrNotFound )
        {
        iLongTapDetector->PointerEventL( aPointerEvent );
        }
    }

// -----------------------------------------------------------------------------
// CFileManagerFolderNavigationPane::HandleLongTapEventL
// 
// -----------------------------------------------------------------------------
// 
void CFileManagerFolderNavigationPane::HandleLongTapEventL(
        const TPoint& aPenEventLocation, 
        const TPoint& /*aPenEventScreenLocation*/ )
    {
    if ( iObserver )
        {
        TInt level( CheckHitFolderLevel( aPenEventLocation ) );
        iObserver->HandleFolderNaviEventL(
            MFileManagerFolderNaviObserver::ENaviLongTap,
            level );
        }
    iLongTap = ETrue;
    }

// End of File
