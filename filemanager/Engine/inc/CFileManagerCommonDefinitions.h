/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Common file manager definitions
*
*/


#ifndef CFILEMANAGERCOMMONDEFINITIONS_H
#define CFILEMANAGERCOMMONDEFINITIONS_H


// CONSTANTS
_LIT( KFmgrBackslash, "\\" );
const TInt KFmgrBackslashSpace = 1;

// Left to right and right to left markers
_LIT( KFmgrDirectionalChars, "\x202A\x202B\x202C\x202D\x200E\x200F" );

_LIT( KFmgrTab, "\t" );
_LIT( KFmgrLineFeed, "\n" );
_LIT( KFmgrParagraphSeparator, "\x2029" );
_LIT( KFmgrSpace, " " );

const TInt KFmgrDoubleMaxFileName = KMaxFileName * 2;

_LIT( KFmgrFatFSName, "Fat" );

#endif // CFILEMANAGERCOMMONDEFINITIONS_H

// End of File
