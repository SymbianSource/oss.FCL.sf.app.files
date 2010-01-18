/*
* Copyright (c) 2005-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Gets the item DRM information
*
*/



#ifndef CFILEMANAGERFULLOMADRMINFO_H
#define CFILEMANAGERFULLOMADRMINFO_H

// INCLUDES
#include <e32base.h>
#include <badesca.h>


// FORWARD DECLARATIONS
class CCoeEnv;
class DRM::CDrmUiHandling;

// CLASS DECLARATION
/**
 * Collects the drm info of item and puts strings to given array
 *
 *  @lib FileManagerView.lib
 *  @since S60 2.8
 */
NONSHARABLE_CLASS(CFileManagerFullOmaDrmInfo) : public CBase
    {
    public:  // Constructors and destructor
        
        /**
         * Two-phased constructor.
         * @param aArray Array which is filled with DRM information,
         *               to be displayed in info popup
         * @param aFullPath Full path to item which DRM info is needed
         * @param aCoeEnv Control environment
         * @return Newly created popup.
         */
        static CFileManagerFullOmaDrmInfo* NewL(
            CDesCArray& aArray, const TDesC& aFullPath, CCoeEnv& aCoeEnv );
        
        /**
         * Two-phased constructor.
         * @param aArray Array which is filled with DRM information,
         *               to be displayed in info popup
         * @param aFullPath Full path to item which DRM info is needed
         * @param aCoeEnv Control environment
         * @return Newly created popup.
         */
        static CFileManagerFullOmaDrmInfo* NewLC(
            CDesCArray& aArray, const TDesC& aFullPath, CCoeEnv& aCoeEnv );

        /**
         * Destructor.
         */
        ~CFileManagerFullOmaDrmInfo();

    public:  // New functions
#ifdef RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM
        /**
         * Two-phased constructor.
         * @param aFullPath Full path to item which DRM info is needed
         * @param aCoeEnv Control environment
         * @param aUiHandling DRM UI Handler
         */
        static void ViewDetailsL( const TDesC& aFullPath, DRM::CDrmUiHandling* aUiHandling );
#endif // RD_DRM_COMMON_INTERFACE_FOR_OMA_AND_WMDRM

    private:
        /**
         * By default Symbian 2nd phase constructor is private.
         */
        void ConstructL(
            CDesCArray& aArray,
            const TDesC& aFullPath, 
            CCoeEnv& aCoeEnv );

        /**
         * C++ default constructor.
         * @param aProperties Properties of the item that will be shown
         */
        CFileManagerFullOmaDrmInfo();

    };

#endif      // CFILEMANAGERFULLOMADRMINFO_H
            
// End of File
