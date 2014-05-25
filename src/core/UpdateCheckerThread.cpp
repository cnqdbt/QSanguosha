/********************************************************************
	Copyright (c) 2013-2014 - QSanguosha-Hegemony Team

  This file is part of QSanguosha-Hegemony.

  This game is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3.0 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  See the LICENSE file for more details.

  QSanguosha-Hegemony Team	
*********************************************************************/
#include "UpdateCheckerThread.h"
#include "mainwindow.h"

#include <QNetworkReply>
#include <QRegExp>
#include <QEventLoop>
#include <QTextCodec>
#include <QFile>

UpdateCheckerThread::UpdateCheckerThread()
{
}

void UpdateCheckerThread::run() {
    QNetworkAccessManager *mgr = new QNetworkAccessManager;
    const QString URL = "https://raw.githubusercontent.com/QSanguosha-Rara/QSanguosha-For-Hegemony/Qt-4.8/info/UpdateInfo";
    const QString URL2 = "https://raw.githubusercontent.com/QSanguosha-Rara/QSanguosha-For-Hegemony/Qt-4.8/info/whatsnew.html";
    QEventLoop loop;
    QNetworkReply *reply = mgr->get(QNetworkRequest(QUrl(URL)));
    QNetworkReply *reply2 = mgr->get(QNetworkRequest(QUrl(URL2)));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
    connect(this, SIGNAL(terminated()), reply, SLOT(deleteLater()));
    connect(this, SIGNAL(terminated()), reply2, SLOT(deleteLater()));
    connect(this, SIGNAL(terminated()), mgr, SLOT(deleteLater()));

    loop.exec();

    bool is_comment = false;

    while (!reply->atEnd()) {
        QString line = reply->readLine();

        //simple comment support
        if (line.startsWith("//")) continue;
        if (!is_comment && line.startsWith("/*")) 
            is_comment = true;
        if (is_comment) {
            line.trimmed();
            if (line.contains("*/")) 
                //I wanna use QString::endsWith here, but the mothod always returns false.
                //@@todo:Refine It Later
                is_comment = false;
            continue;
        }

        QStringList texts = line.split("|", QString::SkipEmptyParts);

        Q_ASSERT(texts.size() == 2);

        const QString key = texts.at(0);
        const QString value = texts.at(1);
        emit storeKeyAndValue(key, value);
    }
    const QString FILE_NAME = "info.html";
    QFile file(FILE_NAME);
    if( !file.open(QIODevice::WriteOnly | QIODevice::Text) )  
    {  
        qDebug() << "Cannot open the file: " << FILE_NAME;  
        return;  
    }
    QTextStream out(&file);    
    QString codeContent = reply2->readAll();    

    QTextCodec *codec = QTextCodec::codecForHtml(codeContent.toLatin1());    
    codeContent = codec->toUnicode(codeContent.toLatin1());    
    out.setCodec(codec);  
    out << codeContent << endl;    
    file.close();

    terminate();
}