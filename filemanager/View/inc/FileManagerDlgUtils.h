/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Dialog utilities
*
*/



#ifndef FILEMANAGERVIEWUTILS_H
#define FILEMANAGERVIEWUTILS_H

// INCLUDES
#include <e32base.h>


// CONSTANTS
const TInt KIndexNotUsed = -1;


// FORWARD DECLARATIONS
class TFileManagerDriveInfo;
class CFileManagerEngine;
class CFileManagerItemProperties;
class CFileManagerFeatureManager;


// CLASS DECLARATION
/**
 *  This class is used for static dialog utilities
 *
 *  @lib FileManagerView.lib
 *  @since S60 3.1
 */
class FileManagerDlgUtils
    {

public:
    /**
     * Shows one of many setting page
     *
     * @since S60 3.1
     * @param aTitleId Title text id
     * @param aTextArray Text array for available settings
     * @param aSelectedIndex For storing selected index
     * @return ETrue if selection made, otherwise EFalse
     */
    IMPORT_C static TBool ShowOOfMSettingQueryL(
        const TInt aTitleId,
        const MDesCArray* aTextArray,
        TInt& aSelectedIndex );

    /**
     * Shows one of many setting page
     *
     * @since S60 3.1
     * @param aTitleId Title text id
     * @param aTextIds Text id array for available settings
     * @param aSelectedIndex For storing selected index
     * @return ETrue if selection made, otherwise EFalse
     */
    IMPORT_C static TBool ShowOOfMSettingQueryL(
        const TInt aTitleId,
        const TInt aTextIds,
        TInt& aSelectedIndex );

    /**
     * Shows weekday selection setting page
     *
     * @since S60 3.1
     * @param aTitleId Title text id
     * @param aDay For storing selected weekday
     * @return ETrue if selection made, otherwise EFalse
     */
    IMPORT_C static TBool ShowWeekdayQueryL(
        const TInt aTitleId,
        TDay& aDay );

    /**
     * Shows n of many setting page
     *
     * @since S60 3.1
     * @param aTitleId Title text id
     * @param aTextIds Text id array for available settings
     * @param aSelection For storing selected indexes as bitmask
     * @param aDominantIndex Dominant index for select all behaviour
     * @return ETrue if selection made, otherwise EFalse
     */
    IMPORT_C static TBool ShowNOfMSettingQueryL(
        const TInt aTitleId,
        const TInt aTextIds,
        TUint32& aSelection,
        const TInt aDominantIndex = KIndexNotUsed );

    /**
     * Shows time setting page
     *
     * @since S60 3.1
     * @param aTitleId Title text id
     * @param aTime Selected time
     * @return ETrue if selection made, otherwise EFalse
     */
    IMPORT_C static TBool ShowTimeSettingQueryL(
        const TInt aTitleId, TTime& aTime );

    /**
     * Shows memory store info popup
     *
     * @since S60 3.1
     * @param aInfo Memory store info
     */
    IMPORT_C static void ShowMemoryStoreInfoPopupL(
        const TFileManagerDriveInfo& aInfo );

    /**
     * Shows info query in message query format
     *
     * @since S60 3.1
     * @param aText Text to display
     */
    IMPORT_C static void ShowInfoQueryL(
        const TDesC& aText );

    /**
     * Shows info query in message query format
     *
     * @since S60 3.1
     * @param aTextId TextId to display
     * @param aValue Text value to display
     */
    IMPORT_C static void ShowInfoQueryL(
        const TInt aTextId,
        const TDesC& aValue = KNullDesC );

    /**
     * Shows info query in message query format
     *
     * @since S60 3.1
     * @param aTextId TextId to display
     * @param aValue Integer value to display
     */
    IMPORT_C static void ShowInfoQueryL(
        const TInt aTextId,
        const TInt aValue );

    /**
     * Shows error note
     *
     * @since S60 3.1
     * @param aTextId TextId to display
     * @param aValue Text value to display
     */
    IMPORT_C static void ShowErrorNoteL(
        const TInt aTextId,
        const TDesC& aValue = KNullDesC );

    /**
     * Shows confirm note
     *
     * @since S60 3.1
     * @param aTextId TextId to display
     */
    IMPORT_C static void ShowConfirmNoteL( const TInt aTextId );

    /**
     * Shows warning note
     *
     * @since S60 3.1
     * @param aTextId TextId to display
     */
    IMPORT_C static void ShowWarningNoteL( const TInt aTextId );

    /**
     * Shows confirm query with yes no softkey
     *
     * @since S60 3.1
     * @param aTextId TextId to display
     * @param aValue Text value to display
     * @return ETrue is Yes selected, otherwise EFalse
     */
    IMPORT_C static TBool ShowConfirmQueryWithYesNoL(
        const TInt aTextId,
        const TDesC& aValue = KNullDesC );

    /**
     * Shows confirm query with yes no softkey
     *
     * @since S60 3.1
     * @param aText Text to display
     * @return ETrue is Yes selected, otherwise EFalse
     */
    IMPORT_C static TBool ShowConfirmQueryWithYesNoL(
        const TDesC& aText );

    // Dialog icon types
    enum TIcons
        {
        EInfoIcons = 0,
        EErrorIcons
        };

    /**
     * Shows confirm query with ok softkey
     *
     * @since S60 3.1
     * @param aIcons Icons to display
     * @param aText Text to display
     * @param aValue Text value to display
     */
    IMPORT_C static void ShowConfirmQueryWithOkL(
        const TIcons aIcons,
        const TInt aTextId,
        const TDesC& aValue = KNullDesC );

    /**
     * Shows confirm query with ok softkey
     *
     * @since S60 3.1
     * @param aIcons Icons to display
     * @param aText Text to display
     * @param aValue Integer value to display
     */
    IMPORT_C static void ShowConfirmQueryWithOkL(
        const TIcons aIcons,
        const TInt aTextId,
        const TInt aValue );

    /**
     * Shows confirm query with ok softkey
     *
     * @since S60 3.1
     * @param aIcons Icons to display
     * @param aText Text to display
     */
    IMPORT_C static void ShowConfirmQueryWithOkL(
        const TIcons aIcons,
        const TDesC& aText );

    /**
     * Shows confirm query with ok cancel softkeys
     *
     * @since S60 3.1
     * @param aTextId TextId to display
     * @param aValue Text value to display
     * @return ETrue is Cancel selected, otherwise EFalse
     */
    IMPORT_C static TBool ShowConfirmQueryWithOkCancelL(
        const TInt aTextId,
        const TDesC& aValue = KNullDesC );

    /**
     * Shows confirm query with ok cancel softkeys
     *
     * @since S60 3.1
     * @param aText Text to display
     * @return ETrue is Cancel selected, otherwise EFalse
     */
    IMPORT_C static TBool ShowConfirmQueryWithOkCancelL(
        const TDesC& aText );

    /**
     * Shows info note
     *
     * @since S60 3.1
     * @param aText Text to display
     */
    IMPORT_C static void ShowInfoNoteL(
        const TDesC& aText );

    /**
     * Shows info note
     *
     * @since S60 3.1
     * @param aTextId TextId to display
     * @param aValue Text value to display
     */
    IMPORT_C static void ShowInfoNoteL(
        const TInt aTextId,
        const TDesC& aValue = KNullDesC );

    /**
     * Shows info note
     *
     * @since S60 3.1
     * @param aTextId TextId to display
     * @param aValue Integer value to display
     */
    IMPORT_C static void ShowInfoNoteL(
        const TInt aTextId,
        const TInt aValue );

    /**
     * Shows simple password query
     *
     * @since S60 3.1
     * @param aTitleId Title to display
     * @param aPwd For storing given password
     * @return ETrue if password is given, otherwise EFalse
     */
    IMPORT_C static TBool ShowSimplePasswordQueryL(
        const TInt aTitleId, TDes& aPwd );

    /**
     * Shows password query
     *
     * @since S60 3.1
     * @param aPwd For storing given password
     * @return ETrue if password is given correctly, otherwise EFalse
     */
    IMPORT_C static TBool ShowPasswordQueryL( TDes& aPwd );

    /**
     * Shows file name query
     *
     * @since S60 3.2
     * @param aTitleId Title text id
     * @param aOldName Old file name
     * @param aNewName User given new name
     * @param aEngine Reference to the engine
     * @return ETrue if name is given, otherwise EFalse
     */
    IMPORT_C static TBool ShowFileNameQueryL(
        const TInt aTitleId,
        const TDesC& aOldName,
        TDes& aNewName,
        CFileManagerEngine& aEngine );

    /**
     * Shows folder name query
     *
     * @since S60 3.2
     * @param aTitleId Title text id
     * @param aName Old folder name, this will be the default for new
     * @param aEngine Reference to the engine
     * @param aNameGeneration ETrue if name generation will be used,
     *                        EFalse otherwise
     * @return ETrue if name is given, otherwise EFalse
     */
    IMPORT_C static TBool ShowFolderNameQueryL(
        const TInt aTitleId,
        TDes& aName,
        CFileManagerEngine& aEngine,
        const TBool aNameGeneration = EFalse );

    /**
     * Shows item info popup
     *
     * @since S60 3.2
     * @param aProperties Item properties
     * @param aFeatureManager Reference to the feature manager
     */
    IMPORT_C static void ShowItemInfoPopupL(
        CFileManagerItemProperties& aProperties,
        const CFileManagerFeatureManager& aFeatureManager );

    /**
     * Shows simple password query with drive name
     *
     * @since S60 3.2
     * @param aText Name to display
     * @param aPwd For storing given password
     * @return ETrue if password is given, otherwise EFalse
     */
    IMPORT_C static TBool ShowSimplePasswordQueryL(
        const TDesC& aText, TDes& aPwd );

    /**
     * Shows error note
     *
     * @since S60 3.2
     * @param aText Text to display
     */
    IMPORT_C static void ShowErrorNoteL( const TDesC& aText );

    };

#endif // FILEMANAGERVIEWUTILS_H

// End of File
