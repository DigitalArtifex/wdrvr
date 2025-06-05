#include "locationmodel.h"

LocationModel::LocationModel(QObject *parent)
    : QAbstractListModel{parent}
{
}

LocationModel::~LocationModel()
{

}

int LocationModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_filteredData.count();
}

QVariant LocationModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    QVariant data;

    switch(static_cast<LocationDataRole>(role))
    {
    case LocationRole:
        data = QVariant::fromValue<QGeoCoordinate>(m_filteredData[index.row()].coordinates);
        break;
    case NameRole:
        data = QVariant::fromValue<QString>(m_filteredData[index.row()].name);
        break;
    case StyleRole:
        data = QVariant::fromValue<QString>(m_filteredData[index.row()].styleTag);
        break;
    case DescriptionRole:
        data = QVariant::fromValue<QString>(m_filteredData[index.row()].description);
        break;
    case OpenNetworksRole:
        data = QVariant::fromValue<int>(m_filteredData[index.row()].open);
        break;
    }

    return data;
}

QHash<int, QByteArray> LocationModel::roleNames() const
{
    QHash<int, QByteArray> roleMap {
        { LocationRole, "location" },
        { NameRole, "name" },
        { StyleRole, "style" },
        { DescriptionRole, "description" },
        { OpenNetworksRole, "open" }
    };

    return roleMap;
}

void LocationModel::setLocations(const QVector<LocationData> &locations)
{
    clear();

    // qint64 startData = m_data.count() > 0 ? m_data.count() - 1 : 0;
    // qint64 endData = (m_data.count() + locations.count()) > 0 ? (m_data.count() + locations.count()) - 1 : 0;

    // beginInsertRows(QModelIndex(), startData, endData);
    //m_data.append(locations);
    // endInsertRows();

    // QList<int> roles { LocationRole, NameRole, StyleRole, DescriptionRole, OpenNetworksRole };
    // const QModelIndex start = index(0, 0);
    // const QModelIndex end = index(m_data.count() - 1, 0);

    // emit dataChanged(start, end, roles);
}

void LocationModel::setLocation(const LocationData &location)
{
    *&m_lastNode = new LocationDataNode;
}

void LocationModel::getLocations(const QGeoCoordinate &center, const qreal &distanceFrom, const qreal &precision)
{
    auto result = QtConcurrent::run([this, center, distanceFrom, precision] {
        if(!m_threadMutex.tryLock(QDeadlineTimer(10)))
            return;

        qreal timeStart = QDateTime::currentMSecsSinceEpoch();

        //reset the previous data
        beginResetModel();
        m_filteredData.clear();
        endResetModel();

        //TODO: Group data to viewport with distance from first
        LocationDataNode *currentNode = m_headNode;
        LocationDataNode *nodesInViewport = nullptr;
        LocationDataNode *viewportNodesHead = nullptr;

        quint64 nodesInViewportLength = 0;

        while(currentNode)
        {
            QGeoCoordinate dataCoordinate(currentNode->data.coordinates.latitude(), currentNode->data.coordinates.longitude());
            qreal distance = center.distanceTo(dataCoordinate);

            if(distance <= distanceFrom)
            {
                if(!viewportNodesHead)
                {
                    viewportNodesHead = new LocationDataNode;
                    nodesInViewport = viewportNodesHead;
                }
                else
                {
                    nodesInViewport->next = new LocationDataNode;
                    nodesInViewport = nodesInViewport->next;
                }

                nodesInViewport->data = currentNode->data;
                ++nodesInViewportLength;
            }

            currentNode = currentNode->next;
        }

        //reset head for nodes in viewport
        nodesInViewport = viewportNodesHead;

        //verify (DEBUG)
        if(m_debug)
        {
            quint64 verified = 0;
            while(nodesInViewport)
            {
                ++verified;
                nodesInViewport = nodesInViewport->next;
            }

            //reset head for use
            nodesInViewport = viewportNodesHead;

            //report status
            qDebug() << "Verified" << verified << "of" << nodesInViewportLength;
        }

        //group POI clusters
        qreal clusterDistance = logScale(precision);
        LocationDataNode *viewportNodeIteratorA = nodesInViewport;
        quint64 viewportNodeRemovals = 0;

        while(viewportNodeIteratorA)
        {
            LocationDataNode *viewportNodeIteratorB = viewportNodeIteratorA->next;
            LocationDataNode *viewportNodeIteratorC = viewportNodeIteratorA; //last referenced for splicing

            while(viewportNodeIteratorB)
            {
                QGeoCoordinate coordA(viewportNodeIteratorA->data.coordinates.latitude(), viewportNodeIteratorA->data.coordinates.longitude());
                QGeoCoordinate coordB(viewportNodeIteratorB->data.coordinates.latitude(), viewportNodeIteratorB->data.coordinates.longitude());
                qreal coordDistance = coordA.distanceTo(coordB);

                if(coordDistance <= clusterDistance)
                {
                    viewportNodeIteratorC->next = viewportNodeIteratorB->next;
                    delete viewportNodeIteratorB;

                    --nodesInViewportLength;
                    viewportNodeIteratorB = viewportNodeIteratorC->next;

                    continue;
                }

                viewportNodeIteratorC = viewportNodeIteratorB;
                viewportNodeIteratorB = viewportNodeIteratorB->next;
            }

            viewportNodeIteratorA = viewportNodeIteratorA->next;
        }

        //reset head for nodes in viewport
        nodesInViewport = viewportNodesHead;

        //verify (DEBUG)
        if(m_debug)
        {
            quint64 verified = 0;
            while(nodesInViewport)
            {
                ++verified;
                nodesInViewport = nodesInViewport->next;
            }

            //reset head for use
            nodesInViewport = viewportNodesHead;

            //report status
            qDebug() << "Groups:" << verified << nodesInViewportLength;
        }

        //move filtered items to the vector and clean up the nodes along the way
        quint64 row = 0;
        beginInsertRows(QModelIndex(), 0, nodesInViewportLength - 1);
        while(nodesInViewport)
        {
            // beginInsertRows(QModelIndex(), row, row);
            LocationData data = nodesInViewport->data;
            m_filteredData.append(data);

            LocationDataNode *tempNode = nodesInViewport;
            nodesInViewport = nodesInViewport->next;
            delete tempNode;
            // ++row;
        }
        endInsertRows();

        m_threadMutex.unlock();

        qreal endTime = QDateTime::currentMSecsSinceEpoch();

        qDebug() << "Sorted nodes in" << endTime - timeStart << "ms";
    });
}

void LocationModel::clear()
{
    resetDataModel();
}

double LocationModel::logScale(double percentage, double min, double max)
{
    if (percentage < 0.0001) percentage = 0.0001;
    if (percentage > 1.0) percentage = 1.0;

    return max * std::pow(min / max, percentage);
}

void LocationModel::parseKML(QString fileName, bool append)
{

    auto result = QtConcurrent::run([this, fileName, append] {

        if(!m_threadMutex.tryLock(QDeadlineTimer(1500)))
            return;

        QFile file(fileName);
        qDebug() << "Opening file" << fileName;

        if(!file.exists())
        {
            qDebug() << "Couldnt open file " + file.errorString();
            emit error("File Error", "Could not open file due to invalid filename or path.");
            return;
        }

        if(!file.open(QFile::ReadOnly))
        {
            qDebug() << "Couldnt open file " + file.errorString();
            emit error("File Error", "Could not open file. " + file.errorString());
            return;
        }

        if(!append)
            resetDataModel();

        startUpdateTimer();

        QXmlStreamReader xml(&file);
        quint64 pois = 0;
        quint64 bluetooth = 0;
        quint64 cellular = 0;
        quint64 wifi = 0;

        while(!xml.atEnd())
        {
            xml.readNextStartElement();
            QString elementName = xml.name().toString().toLower();
            if(elementName.toLower() == "placemark")
            {
                LocationData data;

                bool placemark = true;
                while(placemark)
                {
                    //read by element type to catch the end of the placemark
                    xml.readNext();
                    elementName = xml.name().toString().toLower();

                    if(xml.isEndElement() && elementName == "placemark")
                    {
                        placemark = false;
                        continue;
                    }
                    else if(!xml.isStartElement())
                        continue;

                    QString markerAttribute = xml.name().toString().toLower();

                    if(markerAttribute == "name")
                        data.name = xml.readElementText();

                    else if(markerAttribute == "description")
                    {
                        QString description = xml.readElementText();

                        QStringList lines = description.split(QString("\n"), Qt::SkipEmptyParts);

                        for(const QString &line : std::as_const(lines))
                        {
                            QStringList descriptorParts = line.split(QString(": "), Qt::SkipEmptyParts);

                            if(descriptorParts.count() > 1)
                            {
                                QString key = descriptorParts.takeAt(0).toLower();
                                QString value = descriptorParts.join(':');

                                if(key == "type")
                                {
                                    if(value.toLower() == "wifi")
                                        ++wifi;
                                    else if(value.toLower() == "bluetooth")
                                        ++bluetooth;
                                    else if(value.toLower() == "cellular")
                                        ++cellular;

                                    data.type = value;
                                }
                                else if(key == "encryption")
                                    data.encryption = value;
                                else if(key == "capabilities")
                                    data.encryption = value;
                                else if(key == "frequency")
                                    data.encryption = value;
                                else if(key == "time")
                                    data.encryption = value;
                                else if(key == "signal")
                                    data.encryption = value;
                                else if(key == "roamingcois")
                                    data.encryption = value;
                            }
                        }

                        data.description = description;
                    }

                    else if(markerAttribute == "styleurl")
                        data.styleTag = xml.readElementText();

                    else if(markerAttribute == "point")
                    {
                        bool position = true;
                        while(position)
                        {
                            xml.readNext();
                            elementName = xml.name().toString().toLower();

                            if(xml.isEndElement() && elementName == "point")
                            {
                                position = false;
                                continue;
                            }
                            else if(!xml.isStartElement())
                                continue;

                            QString positionAttribute = xml.name().toString().toLower();

                            if(positionAttribute == "coordinates")
                            {
                                QString coordinateString = xml.readElementText();
                                QStringList coordinateList = coordinateString.split(',');

                                if(coordinateList.count() == 2)
                                {
                                    //wiggle has them backwards
                                    qreal x = coordinateList[1].toDouble();
                                    qreal y = coordinateList[0].toDouble();

                                    data.coordinates = QGeoCoordinate(x, y);
                                }
                            }
                        }
                    }

                    else if(markerAttribute == "open")
                        data.open = xml.readElementText().toInt();
                }

                if(!m_headNode)
                {
                    m_headNode = new LocationDataNode;
                    m_lastNode = m_headNode;
                }
                else
                {
                    m_lastNode->next = new LocationDataNode;
                    m_lastNode = m_lastNode->next;
                }

                m_lastNode->data = data;

                // this->append(data);

                ++m_nodeLength;
            }
        }

        m_updateTimer->stop();
        file.close();

        setTotalPointsOfInterest(m_nodeLength);
        setBluetoothPointsOfInterest(bluetooth);
        setCellularPointsOfInterest(cellular);
        setWifiPointsOfInterest(wifi);

        if(xml.hasError())
        {
            qDebug() << xml.errorString();
        }

        qDebug() << "Parsed" << m_nodeLength << "POIs";

        if(m_debug)
        {
            quint64 verified = 0;
            LocationDataNode *node = m_headNode;

            while(node)
            {
                ++verified;
                node = node->next;
            }

            qDebug() << "Verified" << verified << "POIs";
        }

        m_threadMutex.unlock();
    });

}

void LocationModel::openFile(QString fileName, bool append)
{
    if(!fileName.startsWith("file://", Qt::CaseInsensitive))
        return;

    fileName.remove(0,7);

    if(fileName.endsWith(".kml", Qt::CaseInsensitive))
        parseKML(fileName, append);
}

qreal LocationModel::progress() const
{
    return m_progress;
}

void LocationModel::setProgress(qreal progress)
{
    if (qFuzzyCompare(m_progress, progress))
        return;

    if(progress > 100)
        progress = 100;
    else if(progress < 0)
        progress = 0;

    m_progress = progress;
}

void LocationModel::append(const LocationData &data)
{
    // int latitudeIndex = std::round(data.coordinates.latitude() + 90);
    // int longitudeIndex = std::round(data.coordinates.longitude() + 180);

    // ++m_sectors[latitudeIndex][longitudeIndex].locations;

    // //construct the first POI for the sector
    // if(!m_sectors[latitudeIndex][longitudeIndex].head)
    // {
    //     m_sectors[latitudeIndex][longitudeIndex].head = new LocationDataNode;
    //     m_sectors[latitudeIndex][longitudeIndex].last = m_sectors[longitudeIndex][latitudeIndex].head;

    //     m_sectors[latitudeIndex][longitudeIndex].head->data = data;

    //     return;
    // }

    // QGeoCoordinate dataCoordinate(data.coordinates.latitude(),data.coordinates.longitude());
    // qreal dataDistance = QGeoCoordinate(-90, -180).distanceTo(dataCoordinate);

    // LocationDataNode *node = m_sectors[latitudeIndex][longitudeIndex].head;
    // qreal nodeDistance;

    // while(node->next)
    // {
    //     nodeDistance = QGeoCoordinate(-90, -180).distanceTo(node->next->data.coordinates);

    //     if(nodeDistance > dataDistance)
    //     {
    //         LocationDataNode *nextNode = node->next;
    //         node->next = new LocationDataNode(data);
    //         node->next->next = nextNode;

    //         return;
    //     }
    // }

    // //new data point is the furthest
    // node->next = new LocationDataNode(data);
}

void LocationModel::resetDataModel()
{
    //signal model reset
    beginResetModel();

    setTotalPointsOfInterest(0);
    setBluetoothPointsOfInterest(0);
    setCellularPointsOfInterest(0);

    //iterate over nodes and delete them as we go
    while(m_headNode)
    {
        LocationDataNode *currentNode = m_headNode;
        m_headNode = currentNode->next;

        delete currentNode;

        //decrease node length as a way to verify we were successful
        --m_nodeLength;
    }

    //sectored data
    // for(int latitude = 0; latitude < 180; ++latitude)
    // {
    //     for(int longitude = 0; longitude < 360; ++longitude)
    //     {
    //         LocationDataNode *node = m_sectors[longitude][latitude].head;

    //         while(node)
    //         {
    //             LocationDataNode *currentNode = node;
    //             node = node->next;

    //             delete currentNode;
    //         }
    //     }
    // }

    //clear the filtered data vector
    m_filteredData.clear();

    //signal end model reset
    endResetModel();

    //whoopsie
    if(m_nodeLength)
    {
        qDebug() << "Location Model Reset Error";
        errno = EIO;
    }
}

void LocationModel::startUpdateTimer()
{
    if(!m_updateTimer)
    {
        m_updateTimer = new QTimer(this);
        m_updateTimer->setInterval(10);

        connect(m_updateTimer, &QTimer::timeout, this, [this] {emit progressChanged();});
    }

    m_updateTimer->start();
}

quint64 LocationModel::wifiPointsOfInterest() const
{
    return m_wifiPointsOfInterest;
}

void LocationModel::setWifiPointsOfInterest(quint64 wifiPointsOfInterest)
{
    if (m_wifiPointsOfInterest == wifiPointsOfInterest)
        return;
    m_wifiPointsOfInterest = wifiPointsOfInterest;
    emit wifiPointsOfInterestChanged();
}

quint64 LocationModel::cellularPointsOfInterest() const
{
    return m_cellularPointsOfInterest;
}

void LocationModel::setCellularPointsOfInterest(quint64 cellularPointsOfInterest)
{
    if (m_cellularPointsOfInterest == cellularPointsOfInterest)
        return;
    m_cellularPointsOfInterest = cellularPointsOfInterest;
    emit cellularPointsOfInterestChanged();
}

quint64 LocationModel::bluetoothPointsOfInterest() const
{
    return m_bluetoothPointsOfInterest;
}

void LocationModel::setBluetoothPointsOfInterest(quint64 bluetoothPointsOfInterest)
{
    if (m_bluetoothPointsOfInterest == bluetoothPointsOfInterest)
        return;
    m_bluetoothPointsOfInterest = bluetoothPointsOfInterest;
    emit bluetoothPointsOfInterestChanged();
}

quint64 LocationModel::totalPointsOfInterest() const
{
    return m_totalPointsOfInterest;
}

void LocationModel::setTotalPointsOfInterest(quint64 totalPointsOfInterest)
{
    if (m_totalPointsOfInterest == totalPointsOfInterest)
        return;
    m_totalPointsOfInterest = totalPointsOfInterest;
    emit totalPointsOfInterestChanged();
}
