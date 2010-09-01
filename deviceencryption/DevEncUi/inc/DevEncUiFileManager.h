/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  AppUi class of the application.
*
*/

#ifndef CDEVENCUIFILEMANAGER_H_
#define CDEVENCUIFILEMANAGER_H_

#include <e32base.h>
#include <f32file.h>

class CDir;
class CFileStore;
//class RFs;

class CDevEncUiFileManager : public CBase
    {
    public:
        virtual ~CDevEncUiFileManager();
        void ConstructL();
        
        void InteractiveKeySaveL( const TDesC8& aPkcs5Key );
        void InteractiveKeyLoadL( HBufC8*& aPkcs5Key );

        TInt OpenFileStore( const TDesC& aFilename,
                            CFileStore*& aStore );

        void SaveKeyL( CFileStore* aStore,
                const TDesC8& aPkcs5Key ) const;

        void LoadKeyL( const TFileName& aFileName,
                       HBufC8*& aPkcs5Key );

        void GetKeyListL( CDir*& aList );

        TInt DriveToChar( TInt aDrive, TChar &aChar );

        TInt KeyFolder( TDes& aResult );

        TInt RemountMmc();

    private:
        RFs iFs;
    };

#endif /*CDEVENCUIFILEMANAGER_H_*/
