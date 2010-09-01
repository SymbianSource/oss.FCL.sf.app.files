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
* Description: Inline definition for CMsengScanner
*
*
*/


inline const CArrayFix<TInt64>* CMsengScanner::UidResults() const
    {
    return iScanArray->UidResults();
    }

inline const CArrayFix<TInt64>* CMsengScanner::ExtResults() const
    {
    return iScanArray->ExtResults();
    }

inline const CArrayFix<TInt64>* CMsengScanner::GroupResults() const
    {
    return iScanArray->GroupResults();
    }

inline const TDriveNumber CMsengScanner::CurrentDrive() const
    {
    return iScanArray->CurrentDrive();
    }

//  End of File
