/*
*
* This file is part of QMapControl,
* an open-source cross-platform map widget
*
* Copyright (C) 2007 - 2008 Kai Winter
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with QMapControl. If not, see <http://www.gnu.org/licenses/>.
*
* Contact e-mail: kaiwinter@gmx.de
* Program URL   : http://qmapcontrol.sourceforge.net/
*
*/

#include "mapnetwork.h"
#include <QWaitCondition>
namespace qmapcontrol
{
    MapNetwork::MapNetwork(ImageManager* parent)
        :parent(parent), http(new QNetworkAccessManager(this)), loaded(0)
    {
        connect(http, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(processReply(QNetworkReply*)));
    }

    MapNetwork::~MapNetwork()
    {
        //http->clearPendingRequests();
        delete http;
    }


    void MapNetwork::loadImage(const QString& host, const QString& url)
    {
        qDebug() << "getting: " << QString(host).append(url);
        // http->setHost(host);
        // int getId = http->get(url);

        request.setUrl(QUrl(QString(host).prepend("http://").append(url)));
        request.setRawHeader("User-Agent", "TerraGearGUI");
        request.setRawHeader("Host", host.toLatin1());
        /* send the request */
        http->get(request);

        if (vectorMutex.tryLock())
        {
            loadingMap.insert(getId(url), url);
            vectorMutex.unlock();
        }
    }

    void MapNetwork::processReply(QNetworkReply* reply)
    {
        // sleep(1);
        qDebug() << "MapNetwork::requestFinished";
        if (reply->error() != QNetworkReply::NoError)
        {
            qDebug() << "network error: " << reply->errorString();
            //restart query

        }
        else if (vectorMutex.tryLock())
        {
            QString uid = getId(reply->request().url().toString());
            // check if id is in map?
            if (loadingMap.contains(uid))
            {

                QString url = loadingMap.value(uid);
                loadingMap.remove(uid);
                vectorMutex.unlock();
                // qDebug() << "request finished for id: " << id << ", belongs to: " << notifier.url << endl;
                QByteArray ax;

                if (reply->bytesAvailable()>0)
                {
                    QPixmap pm;
                    ax = reply->readAll();

                    if (pm.loadFromData(ax))
                    {
                        loaded += pm.size().width()*pm.size().height()*pm.depth()/8/1024;
                        // qDebug() << "Network loaded: " << (loaded);
                        parent->receivedImage(pm, url);
                    }
                    else
                    {
                        qDebug() << "NETWORK_PIXMAP_ERROR: " << ax;
                    }
                }

            }
            else
                vectorMutex.unlock();

        }
        if (loadingMap.size() == 0)
        {
            // qDebug () << "all loaded";
            parent->loadingQueueEmpty();
        }
    }

    QString MapNetwork::getId(QString url)
    {
        QString uid;
        foreach(QString s, QString(url).remove(".png").split("/", QString::SkipEmptyParts))
        {
            bool check = false;
            s.toInt(&check, 10);
            if(!check){
                continue;
            } else {
                uid.append(s);
            }
        }
        return uid;
    }

    void MapNetwork::abortLoading()
    {
    //http->clearPendingRequests();
        if (vectorMutex.tryLock())
        {
            loadingMap.clear();
            vectorMutex.unlock();
        }
    }

    bool MapNetwork::imageIsLoading(QString url)
    {
        return loadingMap.values().contains(url);
    }

    void MapNetwork::setProxy(QString host, int port)
    {
#ifndef Q_WS_QWS
        // do not set proxy on qt/extended
        //http->setProxy(host, port);
#endif
    }
}
