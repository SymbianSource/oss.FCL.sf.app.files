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
* Description:  DeviceEncryptionGsPlugin implementation.
*
*/

#ifndef DEVENCGSPLUGIN_H
#define DEVENCGSPLUGIN_H

// User includes
#include <../../common/DevEncUids.hrh>

// System includes
#include <gsplugininterface.h>
#include <ConeResLoader.h>
#include <AknServerApp.h>
#ifdef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <eikdgfty.h>
#include <eiklibry.h>
#endif //SYMBIAN_ENABLE_SPLIT_HEADERS

// Classes referenced
class CAknNullService;
class CRepository;

// Constants
const TUid KDeviceEncryptionGsPluginUID       = { KDevEncGsPluginUid  };  // dll uid
const TUid KDeviceEncryptionGsPluginImplUID   = { KDevEncGsPluginImplUid };  // dll impl uid
const TUid KDeviceEncryptionAppUid          = { KDevEncUiUid };  // UI uid

_LIT( KDeviceEncryptionApp,    "z:\\sys\\bin\\DevEncUi.exe" );
_LIT( KDeviceEncryptionGsPluginResourceFileName, "z:devencGsPluginRsc.rsc" );
_LIT( KDeviceEncryptionGsPluginIconDirAndName, "z:devencgsplugin.mbm"); // Use KDC_BITMAP_DIR

// CLASS DECLARATION

//For embedding CC in CS
class TAppInfo
    {
    public:
        TAppInfo( TUid aUid, const TDesC& aFile )
            : iUid( aUid ), iFile( aFile )
            {}
        TUid iUid;
        TFileName iFile;
    };


/**
* CDeviceEncryptionGsPlugin.
*
* This class handles state and application logic of CDeviceEncryptionGsPlugin.
* The plugin is a type of EGSItemTypeSettingDialog and therefore the GS FW will
* call HandleSelection() instead of DoActivate(). No CAknView functionality is
* supported even though the base class is CAknView derived via
* CGSPluginInterface.
*
*/
class CDeviceEncryptionGsPlugin : public CGSPluginInterface,
                          public MAknServerAppExitObserver // Embedding
    {
    public: // Constructors and destructor

        /**
        * Symbian OS two-phased constructor
        * @return
        */
        static CDeviceEncryptionGsPlugin* NewL( TAny* aInitParams );

        /**
        * Destructor.
        */
        ~CDeviceEncryptionGsPlugin();

    public: // From CAknView

        /**
        * See base class.
        */
        TUid Id() const;

    public: // From CGSPluginInterface

        /**
        * See base class.
        */
        void GetCaptionL( TDes& aCaption ) const;

        /**
        * See base class.
        */
        TInt PluginProviderCategory() const;

        /**
        * See base class.
        */
        TGSListboxItemTypes ItemType();

        /**
        * See base class.
        */
        void GetValue( const TGSPluginValueKeys aKey,
                       TDes& aValue );

        /**
        * See base class.
        */
        void HandleSelection( const TGSSelectionTypes aSelectionType );
        
        /**
        * See base class.
        */
        CGulIcon* CreateIconL( const TUid aIconType );

    protected: // New

        /**
        * C++ default constructor.
        */
        CDeviceEncryptionGsPlugin();

        /**
        * Symbian OS default constructor.
        */
        void ConstructL();

    protected: // From CAknView

        /**
        * This implementation is empty because this class, being just a dialog,
        * does not implement the CAknView finctionality.
        */
        void DoActivateL( const TVwsViewId& aPrevViewId,
                          TUid aCustomMessageId,
                          const TDesC8& aCustomMessage );

        /**
        * This implementation is empty because this class, being just a dialog,
        * does not implement the CAknView finctionality.
        */
        void DoDeactivate();

    private:

        /**
        * Opens localized resource file.
        */
        void OpenLocalizedResourceFileL(
            const TDesC& aResourceFileName,
            RConeResourceLoader& aResourceLoader );

        /**
        * Launches provisioning application.
        */
        void LaunchDEAppL();
        
        /**
        * Launches application as embedded.
        */
        void EmbedAppL( const TAppInfo& aApp );
        
    protected:

        //Resource loader.
        RConeResourceLoader iResources;
        
        CAknNullService* iNullService;
        
        CApaDocument* iEmbedded;
    };

#endif // DEVENCGSPLUGIN_H
// End of File
