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
        data = QVariant::fromValue<QPointF>(m_filteredData[index.row()].coordinates);
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
    auto result = QtConcurrent::run([this, center, distanceFrom, precision]{
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
            QGeoCoordinate dataCoordinate(currentNode->data.coordinates.x(), currentNode->data.coordinates.y());
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
                QGeoCoordinate coordA(viewportNodeIteratorA->data.coordinates.x(), viewportNodeIteratorA->data.coordinates.y());
                QGeoCoordinate coordB(viewportNodeIteratorB->data.coordinates.x(), viewportNodeIteratorB->data.coordinates.y());
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
        beginInsertRows(QModelIndex(), 0, nodesInViewportLength - 1);
        while(nodesInViewport)
        {
            LocationData data = nodesInViewport->data;
            m_filteredData.append(data);

            LocationDataNode *tempNode = nodesInViewport;
            nodesInViewport = nodesInViewport->next;
            delete tempNode;
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

double LocationModel::logScale(double percentage, double min, double max, double base)
{
    if (percentage < 0.00001) percentage = 0.00001;
    if (percentage > 1.0) percentage = 1.0;

    // Step 1: Make input percentage logarithmic
    return max * std::pow(min / max, percentage);
}

void LocationModel::parseKML(QString fileName, bool append)
{
    if(!fileName.startsWith("file://"))
        return;

    fileName.remove(0,7);

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

    resetDataModel();
    startUpdateTimer();

    QXmlStreamReader xml(&file);

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
                    data.description = xml.readElementText();

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

                                data.coordinates = QPointF(x, y);
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

            ++m_nodeLength;
        }
    }

    m_updateTimer->stop();
    file.close();

    if(xml.hasError())
    {
        qDebug() << xml.errorString();
    }

    //setLocations(locations);

    // QList<int> roles { LocationRole, NameRole, StyleRole, DescriptionRole, OpenNetworksRole };
    // const QModelIndex start = index(0, 0);
    // const QModelIndex end = index(m_data.count() > 0 ? m_data.count() - 1 : 0, 0);

    // emit dataChanged(start, end, roles);

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

void LocationModel::resetDataModel()
{
    //signal model reset
    beginResetModel();

    //iterate over nodes and delete them as we go
    while(m_headNode)
    {
        LocationDataNode *currentNode = m_headNode;
        m_headNode = currentNode->next;

        delete currentNode;

        //decrease node length as a way to verify we were successful
        --m_nodeLength;
    }

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
