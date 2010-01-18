/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Memory scanning engine registry scanning
*
*/


#ifndef MSENGSISXINFO_H
#define MSENGSISXINFO_H

// CLASS DECLARATION

/**
* This class represents a sis registry entry.
*
* @since 3.2
*/
class CMsengSisxInfo : public CBase
    {
    public:  // Constructors and destructor
    
        /**
        * Constructor.
        * 
        * @param aEntry - Sis registry entry.
        * @param aDrive - Drive we are interested in
        */
        static CMsengSisxInfo* NewL( Swi::RSisRegistryEntry& aEntry, TDriveNumber aDrive );
        
        
        /**
        * Destructor.
        */
        virtual ~CMsengSisxInfo();
        
    public: // New functions

        
    public: // Functions from base classes
        
        /**
        * From CAppMngrAppInfo, Get location of the application.
        *
        * @since 3.2
        * @return Application location.
        */
        TBool RequestedLocation() const;

        
    private:
        
        /**
        * 2nd phase constructor.
        *
        * @since 3.2
        * @param aEntry - Sis registry entry.
        * @param aDrive - Drive scanning is requested for.
        */
        void ConstructL( Swi::RSisRegistryEntry& aEntry, TDriveNumber aDrive );
        
    private:    // Data
        HBufC* iFileName;  // Own      
        RPointerArray<HBufC> iFiles;

        TBool iRequestedLocation;
        TUid iUid;
    };

#endif      // MSENGSISXINFO_H
            
// End of File
