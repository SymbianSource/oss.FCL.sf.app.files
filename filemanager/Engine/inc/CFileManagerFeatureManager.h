/*
* Copyright (c) 2007-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This class manages the runtime variated features
*
*/



#ifndef C_FILEMANAGERFEATUREMANAGER_H
#define C_FILEMANAGERFEATUREMANAGER_H

#include <e32std.h>

/**
*  This class manages the runtime variated features
*
*  @lib FileManagerEngine.lib
*  @since 3.2
*/
NONSHARABLE_CLASS(CFileManagerFeatureManager) : public CBase
    {
public:
    /**
    * Two-phased constructor.
    */
    static CFileManagerFeatureManager* NewL();
    
    /**
    * Destructor.
    */
    virtual ~CFileManagerFeatureManager();

    /**
     * Checks if feature is supported
     * @since 3.2
     */
    IMPORT_C TBool IsDrmFullSupported() const;

    /**
     * Checks if feature is supported
     * @since 3.2
     */
    IMPORT_C TBool IsHelpSupported() const;

    /**
     * Checks if feature is supported
     * @since 3.2
     */
    IMPORT_C TBool IsIrdaSupported() const;

    /**
     * Checks if feature is supported
     * @since 3.2
     */
    IMPORT_C TBool IsRemoteStorageFwSupported() const;

    /**
     * Checks if feature is supported
     * @since 3.2
     */
    IMPORT_C TBool IsMmcPassWdSupported() const;

    /**
     * Checks if feature is supported
     * @since 3.2
     */
    IMPORT_C TBool IsWesternVariant() const;

    /**
     * Checks if feature is supported
     * @since 3.2
     */
    IMPORT_C TBool IsMmcSwEjectSupported() const;

    /**
     * Checks if feature specified by TFileManagerFeatures is supported
     * @since 3.2
     */
    IMPORT_C TBool IsFeatureSupported( TInt aFileManagerFeature ) const;

    /**
     * Checks if the application is in embedded mode
     * @since 3.2
     */
    IMPORT_C TBool IsEmbedded() const;

private:
    /**
    * C++ default constructor.
    */
    CFileManagerFeatureManager();

    /**
    * By default Symbian 2nd phase constructor is private.
    */
    void ConstructL();

private: // Data
    TBool iDrmFullSupported;
    TBool iHelpSupported;
    TBool iIrdaSupported;
    TBool iRemoteStorageFwSupported;
    TBool iMmcPassWdSupported;
    TBool iWesternVariant;
    TBool iMmcSwEjectSupported;
    TBool iEmbedded;
    TInt iFileManagerFeatures;
    };

#endif // C_FILEMANAGERFEATUREMANAGER_H
