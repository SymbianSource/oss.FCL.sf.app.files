#
# Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# Nokia Corporation - initial contribution.
#
# Contributors:
#
# Description: filemanager source files
#

INCLUDEPATH += ./src
INCLUDEPATH += ./src/components
INCLUDEPATH += ./src/backuprestore
INCLUDEPATH += ./src/operationservice
INCLUDEPATH += ./src/fmdrivewatcher

win32 {
    INCLUDEPATH += ./src/fmdrivewatcher/private/win32
}


symbian {
    INCLUDEPATH += ./src/fmdrivewatcher/private/symbian
}

HEADERS += src/fmdriverlistwidget.h \
           src/listviewitems.h \
           src/fmfileview.h \
           src/fmfilebrowsewidget.h \
           src/fmdriverview.h \
           src/fmmainwindow.h \
           src/fmsplitview.h \
           src/fmfindthread.h \
           src/fmfindresultmodel.h \
           src/fmfindview.h \
           src/fmfindwidget.h \
           src/fmviewmanager.h \
           src/fmviewbase.h \
           src/operationservice/fmoperationservice.h \
           src/operationservice/fmoperationthread.h \
           src/operationservice/fmoperationbase.h \
           src/operationservice/fmoperationresultprocesser.h \
           src/operationservice/fmoperationcopy.h \
           src/operationservice/fmoperationmove.h \
           src/operationservice/fmoperationremove.h \
           src/backuprestore/fmbackupview.h \
           src/backuprestore/fmbackupwidget.h \
           src/backuprestore/fmrestoreview.h \
           src/backuprestore/fmrestorewidget.h \
           src/backuprestore/fmrestoreviewitem.h \
           src/backuprestore/fmbackupconfigloader.h \
           src/backuprestore/fmbackuprestorehandler.h \
           src/backuprestore/fmdeletebackupview.h  \
           src/backuprestore/fmdeletebackupwidget.h  \
           src/components/fmcombinedquery.h \
           src/components/fmdlgutils.h \
           src/components/fmtimequery.h \
           src/components/fmdatalistmodel.h \
           src/components/fmsingletextquery.h \
           src/components/fmmultitextquery.h \
           src/components/fmviewdetailsdialog.h \
           src/components/fmviewdetailsitem.h \
           src/components/fmdrivedetailstype.h \
           src/fmdrivewatcher/fmdrivewatcher.h \

win32 {
    HEADERS += src/fmdrivewatcher/private/win32/fmdrivewatcherprivate.h \

}
symbian {
    HEADERS += src/fmdrivewatcher/private/symbian/fmdrivewatcherprivate.h \
               src/fmdrivewatcher/private/symbian/fmdriveevent.h
}         

SOURCES += src/main.cpp \
           src/fmdriverlistwidget.cpp \
           src/listviewitems.cpp \
           src/fmfileview.cpp \
           src/fmfilebrowsewidget.cpp \
           src/fmdriverview.cpp \
           src/fmmainwindow.cpp \
           src/fmsplitview.cpp \
           src/fmfindthread.cpp \
           src/fmfindresultmodel.cpp \
           src/fmfindview.cpp \
           src/fmfindwidget.cpp \
           src/fmviewmanager.cpp \
           src/fmviewbase.cpp \
           src/operationservice/fmoperationservice.cpp \
           src/operationservice/fmoperationthread.cpp \
           src/operationservice/fmoperationbase.cpp \
           src/operationservice/fmoperationresultprocesser.cpp \
           src/operationservice/fmoperationcopy.cpp \
           src/operationservice/fmoperationmove.cpp \
           src/operationservice/fmoperationremove.cpp \
           src/backuprestore/fmbackupview.cpp \
           src/backuprestore/fmbackupwidget.cpp \
           src/backuprestore/fmrestoreview.cpp \
           src/backuprestore/fmrestorewidget.cpp \
           src/backuprestore/fmrestoreviewitem.cpp \
           src/backuprestore/fmbackupconfigloader.cpp \
           src/backuprestore/fmbackuprestorehandler.cpp \
           src/backuprestore/fmdeletebackupview.cpp  \
           src/backuprestore/fmdeletebackupwidget.cpp  \
           src/components/fmcombinedquery.cpp \
           src/components/fmdlgutils.cpp \
           src/components/fmtimequery.cpp \
           src/components/fmdatalistmodel.cpp \
           src/components/fmsingletextquery.cpp \
           src/components/fmmultitextquery.cpp \
           src/components/fmviewdetailsdialog.cpp \
           src/components/fmviewdetailsitem.cpp \
           src/components/fmdrivedetailstype.cpp \
           src/fmdrivewatcher/fmdrivewatcher.cpp


win32 {
    SOURCES += src/fmdrivewatcher/private/win32/fmdrivewatcherprivate.cpp
}
symbian {
    SOURCES += src/fmdrivewatcher/private/symbian/fmdrivewatcherprivate.cpp \
               src/fmdrivewatcher/private/symbian/fmdriveevent.cpp
}