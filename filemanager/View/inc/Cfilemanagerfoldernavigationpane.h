/*
* Copyright (c) 2002-2006 Nokia Corporation and/or its subsidiary(-ies).
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



#ifndef CFILEMANAGERFOLDERNAVIGATIONPANE_H
#define CFILEMANAGERFOLDERNAVIGATIONPANE_H

// INCLUDES
#include <coecntrl.h>
#include <AknUtils.h> // TAknLayoutRect, TAknLayoutText
#include <aknlongtapdetector.h>
#include <AknsItemID.h>

// CONSTANTS
const TInt KFileManagerMaxFolderDepth = 5; // Includes the root

// FORWARD DECLARATIONS
class CFbsBitmap;
class MFileManagerFolderNaviObserver;

// CLASS DECLARATION

/**
 * Folder navigation pane for File Manager.
 * Custom navigation pane to show folder depth of the
 * current folder in navigation pane using folder icons.
 *
 *  @lib FileManagerView.lib
 *  @since S60 2.0
 */
class CFileManagerFolderNavigationPane :
        public CCoeControl,
        public MAknLongTapDetectorCallBack
    {
    private:
        /**
         * Folder indicator set.
         */
        class TIndicatorSet
            {
            public: // New functions
                /**
                 * Compares two indicator sets.
                 * @param aFirst First indicator set to compare.
                 * @param aSecond The other indicator set to compare.
                 * @return ETrue if sets are equal, EFalse otherwise.
                 */
                static TBool Compare(
                    const TIndicatorSet& aFirst,
                    const TIndicatorSet& aSecond );

            public: // Data
                /// id
                TInt iId;
                /// Bitmap file name
                TFileName iBmpFile;
                /// Icon bitmap id
                TInt iIconId;
                /// Mask bitmap id
                TInt iMaskId;
                /// Skin ID
                TAknsItemID iAknsItemIdRoot;

            };

    public: // Constructors and destructor

        /**
         * Two-phased constructor.
         * @param aRoot Id of the root indicator
         * @param aDepth Depth of the navigation
         * @param aReader Resource reader for reading navigation pane from
         *                resource file.
         * @return Newly created navigation pane.
         */
        IMPORT_C static CFileManagerFolderNavigationPane* NewL(
            const TInt aRoot,
            const TInt aDepth,
            TResourceReader& aReader );

        /**
         * Destructor.
         */
        IMPORT_C ~CFileManagerFolderNavigationPane();


    public: // New functions

        /**
         * Specifies how many subfolder icons are displayed.
         * Depth range is 0..n ( 0 = root, 1 = root+subfolder, etc. )
         * Depth over 4 is displayed just as tree dots at the end of navipane.
         * @param aDepth set current depth
         */
        IMPORT_C void SetFolderDepth( const TInt aDepth );

        /**
         * Return current folder depth.
         * @return aDepth return current depth
         */
        IMPORT_C TInt FolderDepth() const;

        /**
         * Changes the active root
         * @param aRoot Id of the root indicator
         */
        IMPORT_C void ChangeRootL( const TInt aRoot );

        /**
         * Handles resource change
         * @param aType Change type
         */
        IMPORT_C void HandleResourceChangeL( TInt aType );

        /**
         * Sets navigation observer
         * @param aObserver Pointer to navigation observer
         */
        IMPORT_C void SetObserver(
            MFileManagerFolderNaviObserver* aObserver );

    protected: // Functions From CCoeControl

        /**
         * @see CCoeControl
         */
        void SizeChanged();

        /**
         * @see CCoeControl
         */
        void Draw( const TRect& aRect ) const;

        /**
         * @see CCoeControl
         */
        void HandlePointerEventL(
            const TPointerEvent& aPointerEvent );

     private: // From MAknLongTapDetectorCallBack
        /**
         * @see MAknLongTapDetectorCallBack
         */
        void HandleLongTapEventL(
            const TPoint& aPenEventLocation, 
            const TPoint& aPenEventScreenLocation );

    private:

        /**
         * C++ default constructor.
         * @param aDepth Depth of the navigation
         */
        CFileManagerFolderNavigationPane(
            const TInt aRoot, const TInt aDepth );

        /**
         * By default Symbian 2nd phase constructor is private.
         * @param aRoot Id of the root indicator 
         * @param aReader Resource reader for reading navigation pane from
         *                resource file.
         */
        void ConstructL( TResourceReader& aReader );

    private: // New functions
        /**
         * Notifies CCoeControl to redraw the pane
         */
        void ReportChange();

        /**
         * Loads folder bitmaps
         */
        void LoadFolderBitmapL();

        /**
         * Checks does position map to drawn navigation level icon
         */
        TInt CheckHitFolderLevel( const TPoint& aPos );

    private: // Data

        // 0 = phone, 1 = MMC
        TInt iRoot;

        /// Own: Bitmap for subfolders
        CFbsBitmap* iFolderBitmap;

        /// Own: Bitmap mask for subfolders
        CFbsBitmap* iFolderMask;

        /// Own: Name of the bitmap file for folder icon and mask
        HBufC* iBmpFile;

        /// Id for folder icon bitmap
        TInt iFolderIconId;

        /// Id for folder icon mask
        TInt iFolderMaskId;

        /// Own: Separator character for folders
        HBufC* iSeparator;

        /// Own: The end character append to the indicator after iMaxDepth is reached
        HBufC* iEndChar;

        /// Id for folder skin
        TAknsItemID iAknsItemIdFolder;

        /// Maximum folder depth displayed in indicator
        TInt iMaxDepth;

        /// Own: Bitmap for rootfolder
        CFbsBitmap* iRootBitmap;

        /// Own: Bitmap mask for rootfolder
        CFbsBitmap* iRootMask;

        /// Rectangles where bitmaps are drawn ( 0 = rootbitmap, 1..max = defaultbitmaps )
        TFixedArray<TAknLayoutRect, KFileManagerMaxFolderDepth> iBitmapLayout;

        /// Rectangles where slashs are drawn ( 0..max = \, max + 1 = ... )
        TFixedArray<TAknLayoutText, KFileManagerMaxFolderDepth + 1> iTextLayout;

        /// Folder depth
        TInt iDepth;

        /// Array to store folder indicators
        RArray<TIndicatorSet> iIndicatorArray;

        /// Ref: Pointer to navigation observer
        MFileManagerFolderNaviObserver* iObserver;

        /// Own: For long tap detecting
        CAknLongTapDetector* iLongTapDetector;

        /// For long tap handling
        TBool iLongTap;
    };

#endif // CMGNAVIPANEOLDERINDICATOR_H

// End of File