/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: Inline definition for CMsengScannerBase
*
*
*/


inline void CMsengScannerBase::AddSizeByUidL(TUidTypes aUid, TInt aSize)
    {
    iInfoArray.AddSizeByUidL(aUid, aSize);
    }

inline void CMsengScannerBase::AddSizeByExtL(TExtTypes aExt, TInt aSize)
    {
    iInfoArray.AddSizeByExtL(aExt, aSize);
    }

inline TBool CMsengScannerBase::IsExcludedDir(const TDesC& aDirectory)
    {
    return iInfoArray.IsExcludedDir(aDirectory);
    }

inline TBool CMsengScannerBase::IsSpecialDir(const TDesC& aDirectory)
    {
    return iInfoArray.IsSpecialDir(aDirectory);
    }

inline RFs& CMsengScannerBase::FsSession() 
    {
    return iFsSession;
    }

inline CMsengInfoArray& CMsengScannerBase::InfoArray() 
    { 
    return iInfoArray; 
    }
inline MMsengScannerObserver& CMsengScannerBase::ScannerObserver() 
    { 
    return iObserver; 
    }

// End of File
