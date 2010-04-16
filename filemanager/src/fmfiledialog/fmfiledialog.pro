# Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
# Initial Contributors:
# Nokia Corporation - initial contribution.
# Contributors:
# Description:
# The project file of FmFileDialog

TARGET = 
CONFIG += hb
TEMPLATE = lib
DEFINES += FMFILEDIALOG_LIBRARY
include ( ../common.pri )
include ( ../inc/commoninc.pri )
include ( ../inc/commonutils.pri )
include ( ../../filemanager_plat/inc/filemanager_plat.pri )
include ( fmfiledialog.pri )
symbian { 
    LIBS += -lplatformenv \
            -lapmime \
            -lxqservice \
            -lxqutils \
            -lshareui
    TARGET.UID3 = 0x2002BCC2

    # export headers to app
#    EXPORT_PLATFORM_HEADERS += \
#                             ../../filemanager_plat/inc/fmglobal.h \
#                             ../../filemanager_plat/inc/fmfiledialog.h
#    
#    headers.sources = $$EXPORT_PLATFORM_HEADERS
#    for(header, headers.sources):BLD_INF_RULES.prj_exports += "./$$header $$APP_LAYER_PLATFORM_EXPORT_PATH($$basename(header))"
}
