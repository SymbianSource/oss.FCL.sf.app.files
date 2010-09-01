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
* Description: Inline definition for CMsengInfoArray
*
*
*/


inline CArrayFix<TInt64>* CMsengInfoArray::UidResults() const
    {
    return iUidResultArray;
    }

inline CArrayFix<TInt64>* CMsengInfoArray::ExtResults() const
    {
    return iExtResultArray;
    }

inline CArrayFix<TInt64>* CMsengInfoArray::GroupResults() const
    {
    return iGroupResultArray;
    }

inline const CArrayFix<TUid>& CMsengInfoArray::Uids() const
    {
    return *iUidArray;
    }

inline const CDesCArray& CMsengInfoArray::Exts() const
    {
    return *iExtArray;
    }

inline const CDesCArray& CMsengInfoArray::Dirs() const
    {
    return *iDirArray;
    }

inline const CDesCArray& CMsengInfoArray::DataDirs() const
    {
    return *iDataDirArray;
    }

inline const CArrayFix<TInt>& CMsengInfoArray::DataDirGroups() const
    {
    return *iDataDirGroupArray;
    }

inline const CArrayPtrFlat<CDesCArray>& CMsengInfoArray::DataDirExcludedFiles() const
    {
    return *iDataDirExclArray;
    }

inline const TDriveNumber CMsengInfoArray::CurrentDrive() const
    {
    return iCurrentScannedDrive;
    }

inline void CMsengInfoArray::AddSizeByUidL(TInt aUid, TInt64 aSize)
    {
    // The array must have been initialized to contain zeros!
    (*iUidResultArray)[aUid] += aSize;
    }

inline void CMsengInfoArray::AddSizeByExtL(TInt aExt, TInt64 aSize)
    {
    // The array must have been initialized to contain zeros!
    (*iExtResultArray)[aExt] += aSize;
    }

inline void CMsengInfoArray::AddSizeByGroupL(TInt aGroupIndex, TInt64 aSize)
    {
    // The array must have been initialized to contain zeros!
    (*iGroupResultArray)[aGroupIndex] += aSize;
    }

// End of File
