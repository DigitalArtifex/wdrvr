#include "locationmodel.h"

LocationModel::LocationModel(QObject *parent)
    : QAbstractListModel{parent}
{

    m_updateTimer = new QTimer;
    m_updateTimer->setInterval(10);

    connect(m_updateTimer, &QTimer::timeout, this, &LocationModel::updateProgress);

    QDir databaseDirectory(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    QStringList databases = databaseDirectory.entryList(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDir::Name);

    if(!databases.contains("default"))
    {
        createDatabase("default");
        databases.append("default");
    }

    setAvailableDatabases(databases);
}

LocationModel::~LocationModel()
{
    if(m_updateTimer)
        delete m_updateTimer;

    for(int i = 0; i < 360; ++i)
    {
        for(int p = 0; p < 180; ++p)
        {
            LocationDataNode *node = m_sectors[i][p].head;

            while(node)
            {
                LocationDataNode *temp = node;
                node = node->next;

                delete temp;
            }
        }
    }
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
    case TypeRole:
        data = QVariant::fromValue<QString>(m_filteredData[index.row()].type);
        break;
    case EncryptionRole:
        data = QVariant::fromValue<QString>(m_filteredData[index.row()].encryption);
        break;
    case TimestampRole:
        data = QVariant::fromValue<qreal>(m_filteredData[index.row()].timestamp.toMSecsSinceEpoch());
        break;
    case AccuracyRole:
        data = QVariant::fromValue<qreal>(m_filteredData[index.row()].accuracy);
        break;
    case SignalRole:
        data = QVariant::fromValue<qreal>(m_filteredData[index.row()].signal);
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
        { OpenNetworksRole, "open" },
        { TypeRole, "type" },
        { EncryptionRole, "encryption" },
        { TimestampRole, "timestamp" },
        { AccuracyRole, "accuracy" },
        { SignalRole, "signal" }
    };

    return roleMap;
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

void LocationModel::parseCSV(QString fileName)
{
    startLoading(QString("Importing file into database `%1`").arg(m_loadedDatabase));

    m_totalPointsOfInterestTemp = m_totalPointsOfInterest;
    m_bluetoothPointsOfInterestTemp = m_bluetoothPointsOfInterest;
    m_cellularPointsOfInterestTemp = m_cellularPointsOfInterest;
    m_wifiPointsOfInterestTemp = m_wifiPointsOfInterest;

    auto result = QtConcurrent::run([this, fileName] {
        QDateTime first = QDateTime::fromMSecsSinceEpoch(0);
        QDateTime last = QDateTime::fromMSecsSinceEpoch(0);

        quint64 imported = 0;

        QFile file(fileName);

        qDebug() << "Opening file" << fileName;

        if(!file.exists())
        {
            qDebug() << "Couldnt open file " + file.errorString();
            errorOccurred("File Error", "Could not open file due to invalid filename or path.");
            return;
        }

        if(!file.open(QFile::ReadOnly))
        {
            qDebug() << "Couldnt open file " + file.errorString();
            errorOccurred("File Error", "Could not open file. " + file.errorString());
            return;
        }

        bool valid = false;

        while(!file.atEnd())
        {
            QString line = file.readLine();

            //pre-header and header information not currently being used
            if(line.startsWith("wigle", Qt::CaseInsensitive) || line.startsWith("mac,ssid", Qt::CaseInsensitive))
            {
                if(line.startsWith("wigle", Qt::CaseInsensitive))
                    valid = true; //eventually change this to properly validate versions

                setProgress(file.pos() / file.size());
                continue;
            }

            // if(!valid)
            // {
            //     errorOccurred("CSV Error", "File is not a valid Wigle CSV file");
            //     return;
            // }

            QStringList segments = line.split(',');

            if(segments.count() != 14)
                continue;

            QString type = segments[13];

            QString capabilitiesString = segments[2];
            capabilitiesString.replace("][", " ");
            capabilitiesString.remove("[");
            capabilitiesString.remove("]");

            QStringList capabilities = capabilitiesString.split(' ', Qt::SkipEmptyParts);
            LocationData data;

            //[BSSID],[SSID],[Capabilities],[First timestamp seen],[Channel],[Frequency],[RSSI],[Latitude],[Longitude],[Altitude],[Accuracy],[RCOIs],[MfgrId],[Type]
            if(wifiTypeKeys.contains(type))
            {
                data = {
                    segments[10].toDouble(),
                    1,
                    QGeoCoordinate(segments[7].toDouble(), segments[8].toDouble(), segments[9].toDouble()),
                    "",
                    segments[2],
                    segments[0],
                    segments[1],
                    segments[4].toInt(),
                    segments[6].toDouble(),
                    "", //don't know how wigle calculates style tags
                    segments[13],
                    QDateTime::fromString(segments[3]),
                    segments[12],
                    segments[5].toDouble(),
                    capabilities,
                    segments[11].split(' ', Qt::SkipEmptyParts)
                };
                ++m_wifiPointsOfInterestTemp;
            }

            //[BD_ADDR],[Device Name],[Capabilities],[First timestamp seen],[Channel],[Frequency],[RSSI],[Latitude],[Longitude],[Altitude],[Accuracy],[RCOIs],[MfgrId],[Type]
            else if(cellTypeKeys.contains(type))
            {
                data = {
                    segments[10].toDouble(),
                    1,
                    QGeoCoordinate(segments[7].toDouble(), segments[8].toDouble(), segments[9].toDouble()),
                    "",
                    segments[2],
                    segments[0],
                    segments[1],
                    segments[4].toInt(),
                    segments[6].toDouble(),
                    "", //don't know how wigle calculates style tags
                    segments[13],
                    QDateTime::fromString(segments[3]),
                    segments[12],
                    segments[5].toDouble(),
                    capabilities,
                    segments[11].split(' ', Qt::SkipEmptyParts)
                };
                ++m_cellularPointsOfInterestTemp;

                if(data.type.toLower() == "lte")
                    ++m_lteStats;
                else if(data.type.toLower() == "nr")
                    ++m_nrStats;
                else if(data.type.toLower() == "gsm")
                    ++m_gsmStats;
                else if(data.type.toLower() == "wcdma")
                    ++m_wcdmaStats;
                else if(data.type.toLower() == "cdma")
                    ++m_cdmaStats;
            }

            //[BD_ADDR],[Device Name],[Capabilities],[First timestamp seen],[Channel],[Frequency],[RSSI],[Latitude],[Longitude],[Altitude],[Accuracy],[RCOIs],[MfgrId],[Type]
            else if(bluetoothTypeKeys.contains(type))
            {
                data = {
                    segments[10].toDouble(),
                    1,
                    QGeoCoordinate(segments[7].toDouble(), segments[8].toDouble(), segments[9].toDouble()),
                    "",
                    segments[2],
                    segments[0],
                    segments[1],
                    segments[4].toInt(),
                    segments[6].toDouble(),
                    "", //don't know how wigle calculates style tags
                    segments[13],
                    QDateTime::fromString(segments[3]),
                    segments[12],
                    segments[5].toDouble(),
                    capabilities,
                    segments[11].split(' ', Qt::SkipEmptyParts)
                };

                ++m_bluetoothPointsOfInterestTemp;

                if(data.type.toLower() == "bt")
                    ++m_bluetoothStats;
                else if(data.type.toLower() == "ble")
                    ++m_bluetoothLEStats;

            }

            if(first == QDateTime::fromMSecsSinceEpoch(0) || data.timestamp < first)
                first = data.timestamp;
            if(last == QDateTime::fromMSecsSinceEpoch(0) || data.timestamp > last)
                last = data.timestamp;

            append(data);
            setProgress(file.pos() / file.size());

            ++imported; //for mps
        }

        quint64 totalSecs = last.toSecsSinceEpoch() - first.toSecsSinceEpoch();

        m_mps.append(static_cast<qreal>(imported) / totalSecs);

        file.close();
    });

    watcher.setFuture(result);

    connect(&watcher, &QFutureWatcher<void>::finished, this, [this](){

        calculateMPS();

        emit lteStatsChanged();
        emit bluetoothStatsChanged();
        emit bluetoothLEStatsChanged();
        emit gsmStatsChanged();
        emit cdmaStatsChanged();
        emit wcdmaStatsChanged();
        emit lteStatsChanged();
        emit nrStatsChanged();
        emit wifiStatsChanged();

        setTotalPointsOfInterest(m_totalPointsOfInterestTemp);
        setBluetoothPointsOfInterest(m_bluetoothPointsOfInterestTemp);
        setWifiPointsOfInterest(m_wifiPointsOfInterestTemp);
        setCellularPointsOfInterest(m_cellularPointsOfInterestTemp);
        endLoading();
        save();
    });
}

void LocationModel::parseKML(QString fileName)
{
    startLoading(QString("Importing file into database `%1`").arg(m_loadedDatabase));

    auto result = QtConcurrent::run([this, fileName] {
        QDateTime first = QDateTime::fromMSecsSinceEpoch(0);
        QDateTime last = QDateTime::fromMSecsSinceEpoch(0);

        if(!m_threadMutex.tryLock(QDeadlineTimer(1500)))
            return;

        QFile file(fileName);
        qDebug() << "Opening file" << fileName;

        if(!file.exists())
        {
            qDebug() << "Couldnt open file " + file.errorString();
            errorOccurred("File Error", "Could not open file due to invalid filename or path.");
            return;
        }

        if(!file.open(QFile::ReadOnly))
        {
            qDebug() << "Couldnt open file " + file.errorString();
            errorOccurred("File Error", "Could not open file. " + file.errorString());
            return;
        }

        QXmlStreamReader xml(&file);
        quint64 loop = 0;

        m_totalPointsOfInterestTemp = m_totalPointsOfInterest;
        m_bluetoothPointsOfInterestTemp = m_bluetoothPointsOfInterest;
        m_cellularPointsOfInterestTemp = m_cellularPointsOfInterest;
        m_wifiPointsOfInterestTemp = m_wifiPointsOfInterest;

        quint64 imported = 0;

        while(!xml.atEnd())
        {
            setProgress(static_cast<qreal>(file.pos()) / file.size());

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
                        QString descriptionValue = description;

                        //remove newlines, they are not a reliable separator for this block
                        description.replace('\n', ' ');
                        QStringList lines = description.split(m_kmlDescriptionSeparator);

                        for(const QString &line : std::as_const(lines))
                        {
                            QStringList descriptorParts = line.split(QString(": "), Qt::SkipEmptyParts);

                            if(descriptorParts.count() > 1)
                            {
                                QString key = descriptorParts.takeAt(0).toLower();
                                QString value = descriptorParts.join(':');

                                if(key == "type")
                                {
                                    data.type = value;

                                    if(bluetoothTypeKeys.contains(data.type))
                                    {
                                        if(data.type.toLower() == "bt")
                                            ++m_bluetoothStats;
                                        else if(data.type.toLower() == "ble")
                                            ++m_bluetoothLEStats;

                                        ++m_bluetoothPointsOfInterestTemp;
                                    }

                                    else if(cellTypeKeys.contains(data.type))
                                    {
                                        if(data.type.toLower() == "lte")
                                            ++m_lteStats;
                                        else if(data.type.toLower() == "nr")
                                            ++m_nrStats;
                                        else if(data.type.toLower() == "gsm")
                                            ++m_gsmStats;
                                        else if(data.type.toLower() == "wcdma")
                                            ++m_wcdmaStats;
                                        else if(data.type.toLower() == "cdma")
                                            ++m_cdmaStats;

                                        ++m_cellularPointsOfInterestTemp;
                                    }

                                    else if(wifiTypeKeys.contains(data.type) || data.type.isEmpty())
                                    {
                                        ++m_wifiStats;
                                        ++m_wifiPointsOfInterestTemp;
                                    }
                                }
                                else if(key == "encryption")
                                    data.encryption = value;
                                else if(key == "capabilities")
                                {
                                    QString capabilitiesString = value;
                                    capabilitiesString.remove("[");
                                    capabilitiesString.remove("]");

                                    data.capabilities = capabilitiesString.split("][", Qt::SkipEmptyParts);
                                }
                                else if(key == "frequency")
                                    data.frequency = value.toDouble();
                                else if(key == "time")//"2025-05-29T08:45:33.000-07:00"
                                {
                                    data.timestamp = QDateTime::fromString(value,QString("yyyy-MM-ddThh:mm:ss.zzzt"));
                                    data.timestamp;
                                }
                                else if(key == "signal")
                                    data.signal = value.toDouble();
                                else if(key == "network id")
                                    data.id = value;
                            }
                        }

                        data.description = descriptionValue;
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

                if(first == QDateTime::fromMSecsSinceEpoch(0) || data.timestamp < first)
                    first = data.timestamp;
                if(last == QDateTime::fromMSecsSinceEpoch(0) || data.timestamp > last)
                    last = data.timestamp;

                this->append(data);
                ++m_totalPointsOfInterestTemp;
                ++imported;
            }
        }

        file.close();

        quint64 totalSecs = last.toSecsSinceEpoch() - first.toSecsSinceEpoch();

        m_mps.append(static_cast<qreal>(imported) / totalSecs);

        if(xml.hasError())
        {
            qDebug() << xml.errorString();
        }

        qDebug() << "Parsed" << m_totalPointsOfInterestTemp - m_totalPointsOfInterest << "POIs";

        if(m_debug)
        {
            // need an updated verification
        }

        m_threadMutex.unlock();
    });
    watcher.setFuture(result);

    connect(&watcher, &QFutureWatcher<void>::finished, this, [this](){
        calculateMPS();

        emit lteStatsChanged();
        emit bluetoothStatsChanged();
        emit bluetoothLEStatsChanged();
        emit gsmStatsChanged();
        emit cdmaStatsChanged();
        emit wcdmaStatsChanged();
        emit lteStatsChanged();
        emit nrStatsChanged();
        emit wifiStatsChanged();

        setTotalPointsOfInterest(m_totalPointsOfInterestTemp);
        setBluetoothPointsOfInterest(m_bluetoothPointsOfInterestTemp);
        setWifiPointsOfInterest(m_wifiPointsOfInterestTemp);
        setCellularPointsOfInterest(m_cellularPointsOfInterestTemp);
        endLoading();
        save();
    });
}

void LocationModel::openFile(QString fileName)
{
    //damn windows
    if(fileName.startsWith("file:///", Qt::CaseInsensitive))
        fileName.remove(0,8);

    if(fileName.startsWith("file://", Qt::CaseInsensitive))
        fileName.remove(0,7);

    if(fileName.endsWith(".kml", Qt::CaseInsensitive))
        parseKML(fileName);
    else if(fileName.endsWith(".csv", Qt::CaseInsensitive))
        parseCSV(fileName);
}

qreal LocationModel::progress() const
{
    return m_progress;
}

void LocationModel::setProgress(qreal progress)
{
    if (qFuzzyCompare(m_progress, progress))
        return;

    if(progress > 1)
        progress = 1;
    else if(progress < 0)
        progress = 0;

    m_progress = progress;
}

void LocationModel::append(const LocationData &data)
{
    int latitudeIndex = std::floor(data.coordinates.latitude() + 90);
    int longitudeIndex = std::floor(data.coordinates.longitude() + 180);

    m_sectors[longitudeIndex][latitudeIndex].mutex.lock();

    ++m_sectors[longitudeIndex][latitudeIndex].locations;

    //construct the first POI for the sector
    if(!m_sectors[longitudeIndex][latitudeIndex].head)
    {
        m_sectors[longitudeIndex][latitudeIndex].head = new LocationDataNode(data);
        m_sectors[longitudeIndex][latitudeIndex].last = m_sectors[longitudeIndex][latitudeIndex].head;
    }

    //otherwise add it to the end of the element
    else
    {
        m_sectors[longitudeIndex][latitudeIndex].last->next = new LocationDataNode(data);
        m_sectors[longitudeIndex][latitudeIndex].last = m_sectors[longitudeIndex][latitudeIndex].last->next;
    }

    m_sectors[longitudeIndex][latitudeIndex].updated = true;
    m_sectors[longitudeIndex][latitudeIndex].mutex.unlock();
}

void LocationModel::sort()
{
    startLoading("Sorting");

    watcher.disconnect();
    watcher.setFuture(QtConcurrent::run([this](){
        for(int longitudeIndex = 0; longitudeIndex < 360; ++longitudeIndex)
        {
            for(int latitudeIndex = 0; latitudeIndex < 180; ++latitudeIndex)
            {
                if(!m_sectors[longitudeIndex][latitudeIndex].updated || !m_sectors[longitudeIndex][latitudeIndex].head)
                    continue;

                m_sectors[longitudeIndex][latitudeIndex].mutex.lock();
                setLoadingTitle(QString("Sorting Sector [%1][%2]").arg(QString::number(longitudeIndex - 180), QString::number(latitudeIndex - 90)));

                bool op = true;
                while(op)
                {
                    LocationDataNode *node = m_sectors[longitudeIndex][latitudeIndex].head;
                    op = false;

                    while(node && node->next)
                    {
                        LocationDataNode *nextNode = node->next;
                        qreal nodeDistance = QGeoCoordinate(-90,-180).distanceTo(node->data.coordinates);
                        qreal nextNodeDistance = QGeoCoordinate(-90,-180).distanceTo(node->next->data.coordinates);

                        if(nodeDistance > nextNodeDistance)
                        {
                            node->next = nextNode->next;
                            nextNode->next = node;
                            op = true;
                        }

                        node = node->next;
                    }
                }

                m_sectors[longitudeIndex][latitudeIndex].mutex.unlock();
            }
        }
    }));

    watcher.connect(&watcher, &QFutureWatcher<void>::finished, this, [this](){
        endLoading();
    });
}

void LocationModel::save()
{
    startLoading("Saving");

    watcher.disconnect();
    watcher.setFuture(QtConcurrent::run([this](){
        QDir databaseDirectory(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + m_loadedDatabase);

        QFile mpsFile(databaseDirectory.absolutePath() + QDir::separator() + "mps.dat");

        if(mpsFile.open(QFile::ReadWrite))
        {
            for(const qreal &value : std::as_const(m_mps))
                mpsFile.write(QString::number(value).toUtf8() + "\n");
        }

        mpsFile.close();

        quint64 totalOps = 360 * 180;
        for(int longitudeIndex = 0; longitudeIndex < 360; ++longitudeIndex)
        {
            for(int latitudeIndex = 0; latitudeIndex < 180; ++latitudeIndex)
            {
                if(!m_sectors[longitudeIndex][latitudeIndex].updated || !m_sectors[longitudeIndex][latitudeIndex].head)
                    continue;

                if(!m_sectors[longitudeIndex][latitudeIndex].mutex.tryLock(1250))
                    return;

                QFile file(databaseDirectory.absolutePath() + QDir::separator() + QString::number(longitudeIndex) + QDir::separator() + QString::number(latitudeIndex) + QDir::separator() + "0.csv");

                if(!file.open(QFile::ReadWrite))
                {
                    errorOccurred("Save Database Error", file.errorString());
                    return;
                }

                LocationDataNode *node = m_sectors[longitudeIndex][latitudeIndex].head;

                while(node)
                {
                    LocationData data = node->data;
                    QString fileData;

                    QString capabilities;

                    for(const QString &capability : std::as_const(data.capabilities))
                        capabilities += "[" + capability + "]";

                    QString rois;

                    for(const QString &roi : std::as_const(data.rois))
                        rois += "[" + roi + "]";

                    fileData =  data.id + "," + QString::number(data.coordinates.longitude()) + "," + QString::number(data.coordinates.latitude()) + ",";
                    fileData += data.encryption + "," + data.description + "," + data.name + "," + data.type + ",";
                    fileData += QString::number(data.accuracy) + "," + QString::number(data.signal) + ",";
                    fileData += QString::number(data.open) + "," + QString::number(data.timestamp.toMSecsSinceEpoch()) + "," + data.styleTag + ",";
                    fileData += capabilities + "," + rois + "," + data.mfgid + "," + QString::number(data.frequency);

                    node = node->next;

                    file.write(QUrl::toPercentEncoding(fileData) + "\n");
                }

                file.close();

                setProgress((longitudeIndex + latitudeIndex) / totalOps);

                m_sectors[longitudeIndex][latitudeIndex].mutex.unlock();
            }
        }
    }));

    watcher.connect(&watcher, &QFutureWatcher<void>::finished, this, [this](){
        endLoading();
    });
}

void LocationModel::load(QString database)
{
    startLoading("Loading");

    resetSectorData();

    m_bluetoothPointsOfInterestTemp = 0;
    m_wifiPointsOfInterestTemp = 0;
    m_totalPointsOfInterestTemp = 0;
    m_cellularPointsOfInterestTemp = 0;

    m_bluetoothStats = 0;
    m_bluetoothLEStats = 0;
    m_gsmStats = 0;
    m_cdmaStats = 0;
    m_wcdmaStats = 0;
    m_lteStats = 0;
    m_nrStats = 0;
    m_wifiStats = 0;

    watcher.disconnect();
    watcher.setFuture(QtConcurrent::run([this, database](){
        QDir databaseDirectory(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + database);

        QFile mpsFile(databaseDirectory.absolutePath() + QDir::separator() + "mps.dat");

        if(mpsFile.open(QFile::ReadOnly))
        {
            while(!mpsFile.atEnd())
            {
                QString line = mpsFile.readLine();
                bool converted = false;

                qreal value = line.toDouble(&converted);

                if(converted)
                    m_mps.append(value);
            }
        }

        mpsFile.close();

        calculateMPS();

        quint128 totalOps = 0;

        for(int longitudeIndex = 0; longitudeIndex < 360; ++longitudeIndex)
        {
            for(int latitudeIndex = 0; latitudeIndex < 180; ++latitudeIndex)
            {
                QFile file(databaseDirectory.absolutePath() + QDir::separator() + QString::number(longitudeIndex) + QDir::separator() + QString::number(latitudeIndex) + QDir::separator() + "0.csv");
                totalOps += file.size();
            }
        }

        quint128 completedOps = 0;

        for(int longitudeIndex = 0; longitudeIndex < 360; ++longitudeIndex)
        {
            for(int latitudeIndex = 0; latitudeIndex < 180; ++latitudeIndex)
            {
                m_sectors[longitudeIndex][latitudeIndex].mutex.lock();

                QFile file(databaseDirectory.absolutePath() + QDir::separator() + QString::number(longitudeIndex) + QDir::separator() + QString::number(latitudeIndex) + QDir::separator() + "0.csv");

                if(!file.open(QFile::ReadWrite))
                {
                    errorOccurred("Load Database Error", file.fileName() + "\n" + file.errorString());
                    return;
                }

                LocationDataNode *node = m_sectors[longitudeIndex][latitudeIndex].head;
                while(!file.atEnd())
                {
                    QString line = QUrl::fromPercentEncoding(file.readLine());
                    QStringList segments = line.split(',');

                    QString capabilitiesString = segments[12];
                    capabilitiesString.remove("[");
                    capabilitiesString.remove("]");

                    QString roiString = segments[13];
                    roiString.remove("[");
                    roiString.remove("]");

                    //fileData += capabilities + "," + rois + "," + data.mfgid + "," + QString::number(data.frequency);
                    LocationData data {
                        segments[7].toDouble(), //accuracy
                        1, //cluster count
                        QGeoCoordinate(segments[2].toDouble(), segments[1].toDouble()),
                        segments[4],
                        segments[3],
                        segments[0],
                        segments[5],
                        segments[9].toInt(),
                        segments[8].toDouble(),
                        segments[11],
                        segments[6],
                        QDateTime::fromMSecsSinceEpoch(segments[10].toLongLong()),
                        segments[14],
                        segments[15].toDouble(),
                        capabilitiesString.split("][", Qt::SkipEmptyParts),
                        roiString.split("][", Qt::SkipEmptyParts)
                    };

                    if(wifiTypeKeys.contains(data.type) || data.type.isEmpty())
                    {
                        ++m_wifiPointsOfInterestTemp;
                        ++m_wifiStats;
                    }
                    else if(bluetoothTypeKeys.contains(data.type))
                    {
                        if(data.type.toLower() == "bt")
                            ++m_bluetoothStats;
                        else if(data.type.toLower() == "ble")
                            ++m_bluetoothLEStats;

                        ++m_bluetoothPointsOfInterestTemp;
                    }
                    else if(cellTypeKeys.contains(data.type))
                    {
                        if(data.type.toLower() == "lte")
                            ++m_lteStats;
                        else if(data.type.toLower() == "nr")
                            ++m_nrStats;
                        else if(data.type.toLower() == "gsm")
                            ++m_gsmStats;
                        else if(data.type.toLower() == "wcdma")
                            ++m_wcdmaStats;
                        else if(data.type.toLower() == "cdma")
                            ++m_cdmaStats;

                        ++m_cellularPointsOfInterestTemp;
                    }
                    else
                        qDebug() << "Uknown Type Key" << data.type;

                    ++m_totalPointsOfInterestTemp;

                    if(!node)
                    {
                        m_sectors[longitudeIndex][latitudeIndex].head = new LocationDataNode(data);
                        node = m_sectors[longitudeIndex][latitudeIndex].head;
                    }

                    else
                    {
                        node->next = new LocationDataNode(data);
                        node = node->next;
                    }

                    completedOps += file.pos();
                }

                file.close();

                if(totalOps > 0)
                    setProgress(completedOps / totalOps);

                m_sectors[longitudeIndex][latitudeIndex].mutex.unlock();
                setLoadedDatabase(database);
            }
        }
    }));

    watcher.connect(&watcher, &QFutureWatcher<void>::finished, this, [this](){

        emit lteStatsChanged();
        emit bluetoothStatsChanged();
        emit bluetoothLEStatsChanged();
        emit gsmStatsChanged();
        emit cdmaStatsChanged();
        emit wcdmaStatsChanged();
        emit lteStatsChanged();
        emit nrStatsChanged();
        emit wifiStatsChanged();

        setTotalPointsOfInterest(m_totalPointsOfInterestTemp);
        setBluetoothPointsOfInterest(m_bluetoothPointsOfInterestTemp);
        setWifiPointsOfInterest(m_wifiPointsOfInterestTemp);
        setCellularPointsOfInterest(m_cellularPointsOfInterestTemp);
        endLoading();
    });
}

//this should be able to go into the class, but QtConcurrent::run
LocationDataNode *groupPoints(LocationDataNode *node, QGeoShape area,  qreal clusterDistance, quint64 &count)
{
    //copy for clustering
    LocationDataNode *nodeToCopy = node;
    LocationDataNode *nodeToWrite = nullptr;
    LocationDataNode *copiedHead = nullptr;

    while(nodeToCopy)
    {
        if(area.contains(nodeToCopy->data.coordinates))
        {
            if(!nodeToWrite)
            {
                nodeToWrite = new LocationDataNode(nodeToCopy->data);
                copiedHead = nodeToWrite;
            }
            else
            {
                nodeToWrite->next = new LocationDataNode(nodeToCopy->data);
                nodeToWrite = nodeToWrite->next;
            }

            ++count;
        }

        nodeToCopy = nodeToCopy->next;
    }

    //group POI clusters
    LocationDataNode *viewportNodeIteratorA = copiedHead;
    quint64 groups = 0;

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

                viewportNodeIteratorB = viewportNodeIteratorC->next;

                --count;
                ++groups;

                continue;
            }

            viewportNodeIteratorC = viewportNodeIteratorB;
            viewportNodeIteratorB = viewportNodeIteratorB->next;
        }

        viewportNodeIteratorA = viewportNodeIteratorA->next;
    }

    // qDebug() << "Grouped" << groups << "within" << clusterDistance;

    return copiedHead;
}

void LocationModel::getPointsInRect(QGeoShape area, qreal zoomLevel)
{
    auto result = QtConcurrent::run([this, area, zoomLevel]
    {
        if(!m_threadMutex.tryLock(QDeadlineTimer(1250)))
            return;

        qreal timeStart = QDateTime::currentMSecsSinceEpoch();

        // QFutureSynchronizer<LocationDataNode*> watcher;
        // QList<QFuture<LocationDataNode*>> futures;

        QGeoPolygon poly = area;

        //QGeoPolygon seems to be constructed from bottom right, to bottom left, to top left to top right
        int latitudeStartIndex = poly.coordinateAt(1).latitude() + 90;
        int latitudeEndIndex = poly.coordinateAt(2).latitude() + 90;
        int longitudeStartIndex = poly.coordinateAt(1).longitude() + 180;
        int longitudeEndIndex = poly.coordinateAt(0).longitude() + 180;

        LocationDataNode *nodes = nullptr;
        LocationDataNode *headNode = nullptr;

        quint64 totalNodes = 0;

        //get points in the bounding rect
        for(int x = 0; x < 360; ++x)
        {
            for(int y = 0; y < 180; ++y)
            {
                if(x >= longitudeStartIndex && x <= longitudeEndIndex && y >= latitudeStartIndex && y <= latitudeEndIndex)
                {
                    quint64 count = 0;
                    if(!nodes)
                    {
                        nodes = groupPoints(m_sectors[x][y].head, area, logScale(zoomLevel), count);
                        headNode = nodes;
                    }

                    else
                        nodes->next = groupPoints(m_sectors[x][y].head, area, logScale(zoomLevel), count);

                    while(nodes && nodes->next)
                        nodes = nodes->next;

                    totalNodes += count;

                    // QFuture<LocationDataNode*> future = QtConcurrent::run(groupPoints, m_sectors[x][y].head, area, logScale(zoomLevel));
                    // watcher.addFuture(future);
                }
            }
        }

        watcher.waitForFinished();

        resetDataModel();

        quint64 index = 0;
        beginInsertRows(QModelIndex(), 0, totalNodes - 1);
        while(headNode)
        {
            LocationDataNode *nodeToDelete = headNode;
            m_filteredData.append(headNode->data);
            headNode = headNode->next;

            delete nodeToDelete;
        }
        endInsertRows();

        // for(const QFuture<LocationDataNode*> &future : std::as_const(futures))
        // {
        //     LocationDataNode *futureNodes = future.result();

        //     if(!futureNodes)
        //         continue;

        //     while(futureNodes)
        //     {
        //         quint64 start = m_filteredData.count() > 0 ? m_filteredData.count() - 1 : 0;
        //         quint64 end = m_filteredData.count() > 0 ? m_filteredData.count() : 1;

        //         beginInsertRows(QModelIndex(), start, end);
        //         m_filteredData.append(futureNodes->data);
        //         endInsertRows();

        //         LocationDataNode *tempNode = futureNodes;
        //         futureNodes = futureNodes->next;

        //         delete tempNode;
        //     }
        // }

        qreal endTime = QDateTime::currentMSecsSinceEpoch();

        m_threadMutex.unlock();

        qDebug() << "Sorted nodes" << totalNodes << "in" << endTime - timeStart << "ms";
    });
}

void LocationModel::updateProgress()
{
    if(!m_loading)
    {
        qDebug() << "Stopping";
        m_updateTimer->stop();
        emit loadingFinished();
    }

    emit progressChanged();
}

void LocationModel::errorOccurred(QString title, QString message)
{
    m_errorMessage = message;
    m_errorTitle = title;

    emit error();
}

void LocationModel::calculateMPS()
{
    qreal total = 0;

    for(const qreal &value : std::as_const(m_mps))
        total += value;

    setMpsAverage(total / m_mps.count());
}

qreal LocationModel::mpsAverage() const
{
    return m_mpsAverage;
}

void LocationModel::setMpsAverage(qreal mpsAverage)
{
    if (qFuzzyCompare(m_mpsAverage, mpsAverage))
        return;
    m_mpsAverage = mpsAverage;
    emit mpsAverageChanged();
}

QString LocationModel::currentPage() const
{
    return m_currentPage;
}

void LocationModel::setCurrentPage(const QString &currentPage)
{
    if (m_currentPage == currentPage)
        return;

    m_currentPage = currentPage;
    emit currentPageChanged();
}

QString LocationModel::loadedDatabase() const
{
    return m_loadedDatabase;
}

void LocationModel::setLoadedDatabase(const QString &loadedDatabase)
{
    if (m_loadedDatabase == loadedDatabase)
        return;

    setDatabase(loadedDatabase);
    m_loadedDatabase = loadedDatabase;
    emit loadedDatabaseChanged();
}

QStringList LocationModel::availableDatabases() const
{
    return m_availableDatabases;
}

void LocationModel::setAvailableDatabases(const QStringList &availableDatabases)
{
    if (m_availableDatabases == availableDatabases)
        return;

    m_availableDatabases = availableDatabases;
    emit availableDatabasesChanged();
}

QString LocationModel::database() const
{
    return m_database;
}

void LocationModel::setDatabase(const QString &database)
{
    if (m_database == database)
        return;

    m_database = database;
    emit databaseChanged();
}

void LocationModel::resetDataModel()
{
    //signal model reset
    beginResetModel();

    //clear the filtered data vector
    m_filteredData.clear();

    //signal end model reset
    endResetModel();
}

void LocationModel::resetSectorData()
{
    //clear sectored data
    for(int longitude = 0; longitude < 360; ++longitude)
    {
        for(int latitude = 0; latitude < 180; ++latitude)
        {
            LocationDataNode *node = m_sectors[longitude][latitude].head;

            while(node)
            {
                LocationDataNode *currentNode = node;
                node = node->next;

                delete currentNode;
            }

            m_sectors[longitude][latitude].last = nullptr;
            m_sectors[longitude][latitude].updated = false;
            m_sectors[longitude][latitude].locations = 0;
        }
    }

    setTotalPointsOfInterest(0);
    setBluetoothPointsOfInterest(0);
    setCellularPointsOfInterest(0);
    setWifiPointsOfInterest(0);
}

void LocationModel::startLoading(QString title)
{
    m_loading = true;
    m_progress = -1;
    setLoadingTitle(title);
    startUpdateTimer();
    emit loadingStarted();
}

void LocationModel::endLoading()
{
    m_loading = false;
    m_progress = -1;
    setLoadingTitle("Loading");
    stopUpdateTimer();
    emit loadingFinished();
}

void LocationModel::resetDatabase()
{
    startLoading("Resetting Database");
    auto result = QtConcurrent::run([this](){
        if(!m_threadMutex.tryLock(1250))
            return;

        //clear filtered data
        if(m_filteredData.count())
            resetDataModel();

        //determine how much we have to do for progress
        quint64 fileOps = 0;
        quint64 memOps = 0;

        for(int latitude = 0; latitude < 180; ++latitude)
        {
            for(int longitude = 0; longitude < 360; ++longitude)
            {
                LocationDataNode *node = m_sectors[longitude][latitude].head;

                //only alter files with data in them for default database
                if(node && m_loadedDatabase == "default")
                {
                    ++fileOps;

                    memOps += m_sectors[longitude][latitude].locations;
                }
                else if(m_loadedDatabase != "default")
                {
                    ++fileOps;

                    memOps += m_sectors[longitude][latitude].locations;
                }
            }
        }

        quint64 totalOps = fileOps + memOps;
        quint64 completedFileOps = 0;
        quint64 completedMemOps = 0;
        setProgress(0);

        QDir databaseDirectory(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + m_loadedDatabase);

        //clear sectored data
        for(int longitude = 0; longitude < 360; ++longitude)
        {
            QDir longitudeDirectory = databaseDirectory;
            longitudeDirectory.cd(QString::number(longitude));

            for(int latitude = 0; latitude < 180; ++latitude)
            {
                QDir latitudeDirectory = longitudeDirectory;
                latitudeDirectory.cd(QString::number(latitude));

                LocationDataNode *node = m_sectors[longitude][latitude].head;

                //make sure only files with data are altered for the default database
                if(node && m_loadedDatabase == "default")
                {
                    QFile file(latitudeDirectory.absolutePath() + QDir::separator() + QString("0.csv"));

                    if(file.exists())
                        file.remove();

                    if(file.open(QFile::ReadWrite))
                    {
                        file.write("");
                        file.flush();
                        file.close();
                    }
                }

                else if(m_loadedDatabase != "default")
                {
                    QFile file(latitudeDirectory.absolutePath() + QDir::separator() + QString("0.csv"));

                    if(file.exists())
                        file.remove();
                }

                //remove the directory if its not the default database
                if(m_loadedDatabase != "default" && latitudeDirectory.exists())
                    latitudeDirectory.rmdir(latitudeDirectory.absolutePath());

                //progress = 50% file ops 50% mem ops = (completeFops / fops) * 0.5 + (completedMops / mops) * 0.5
                setProgress(((completedFileOps / fileOps) * 0.5) + ((completedMemOps / memOps) * 0.5));

                while(node)
                {
                    LocationDataNode *currentNode = node;
                    node = node->next;

                    delete currentNode;

                    setProgress(((completedFileOps / fileOps) * 0.5) + ((completedMemOps / memOps) * 0.5));
                }

                m_sectors[longitude][latitude].last = nullptr;
                m_sectors[longitude][latitude].updated = false;
                m_sectors[longitude][latitude].locations = 0;
            }

            //remove the directory if its not the default database
            if(m_loadedDatabase != "default" && longitudeDirectory.exists())
                longitudeDirectory.rmdir(longitudeDirectory.absolutePath());
        }

        m_threadMutex.unlock();

    });

    watcher.disconnect();
    watcher.setFuture(result);

    connect(&watcher, &QFutureWatcher<void>::finished, this, [this](){
        endLoading();
    });
}

void LocationModel::createDatabase(QString name)
{
    startLoading("Creating Database");

    auto result = QtConcurrent::run([this, name](){
        QDir databaseDirectory(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + name);

        if(databaseDirectory.exists())
        {
            errorOccurred("Create Database Error", "Database already exists");
            return;
        }

        //create the root directory
        databaseDirectory.mkpath(databaseDirectory.absolutePath());

        //create sector directories
        for(quint64 longitude = 0; longitude < 360; ++longitude)
        {
            for(quint64 latitude = 0; latitude < 180; ++latitude)
            {
                if(!databaseDirectory.mkpath(QString("%1%2%3").arg(QString::number(longitude), QDir::separator(), QString::number(latitude))))
                {
                    errorOccurred("Create Database Error", "Could not create required directories");

                    if(longitude || latitude)
                        databaseDirectory.rmdir(databaseDirectory.absolutePath());

                    return;
                }

                QFile file(databaseDirectory.absolutePath() + QDir::separator() + QString("%1%2%3").arg(QString::number(longitude), QDir::separator(), QString::number(latitude)) + QDir::separator() + "0.csv");
                if(!file.open(QFile::ReadWrite))
                {
                    errorOccurred("Create Database Error", "Could not create required files");

                    if(longitude || latitude)
                        databaseDirectory.rmdir(databaseDirectory.absolutePath());
                }

                file.write("");
                file.flush();
                file.close();
            }
        }

        return;
    });

    watcher.disconnect();
    watcher.setFuture(result);

    connect(&watcher, &QFutureWatcher<void>::finished, this, [this](){
        endLoading();
    });
}

void LocationModel::startUpdateTimer()
{
    if(!m_updateTimer->isActive())
    {
        QThread *origin = m_updateTimer->thread();
        m_updateTimer->moveToThread(QThread::currentThread());
        m_updateTimer->start();
        m_updateTimer->moveToThread(origin);
    }
}

void LocationModel::stopUpdateTimer()
{
    if(m_updateTimer->isActive())
        m_updateTimer->stop();
}

QTimer *LocationModel::createUpdateTimer()
{
    QTimer *timer = new QTimer;
    timer->setInterval(10);
    timer->connect(timer, &QTimer::timeout,this, [this](){updateProgress();});

    return timer;
}

quint64 LocationModel::wifiStats() const
{
    return m_wifiStats;
}

void LocationModel::setWifiStats(quint64 wifiStats)
{
    if (m_wifiStats == wifiStats)
        return;

    m_wifiStats = wifiStats;
    emit wifiStatsChanged();
}

quint64 LocationModel::nrStats() const
{
    return m_nrStats;
}

void LocationModel::setNrStats(quint64 nrStats)
{
    if (m_nrStats == nrStats)
        return;
    m_nrStats = nrStats;
    emit nrStatsChanged();
}

quint64 LocationModel::lteStats() const
{
    return m_lteStats;
}

void LocationModel::setLteStats(quint64 lteStats)
{
    if (m_lteStats == lteStats)
        return;
    m_lteStats = lteStats;
    emit lteStatsChanged();
}

quint64 LocationModel::wcdmaStats() const
{
    return m_wcdmaStats;
}

void LocationModel::setWcdmaStats(quint64 wcdmaStats)
{
    if (m_wcdmaStats == wcdmaStats)
        return;
    m_wcdmaStats = wcdmaStats;
    emit wcdmaStatsChanged();
}

quint64 LocationModel::cdmaStats() const
{
    return m_cdmaStats;
}

void LocationModel::setCdmaStats(quint64 cdmaStats)
{
    if (m_cdmaStats == cdmaStats)
        return;
    m_cdmaStats = cdmaStats;
    emit cdmaStatsChanged();
}

quint64 LocationModel::gsmStats() const
{
    return m_gsmStats;
}

void LocationModel::setGsmStats(quint64 gsmStats)
{
    if (m_gsmStats == gsmStats)
        return;
    m_gsmStats = gsmStats;
    emit gsmStatsChanged();
}

quint64 LocationModel::bluetoothLEStats() const
{
    return m_bluetoothLEStats;
}

void LocationModel::setBluetoothLEStats(quint64 bluetoothLEStats)
{
    if (m_bluetoothLEStats == bluetoothLEStats)
        return;
    m_bluetoothLEStats = bluetoothLEStats;
    emit bluetoothLEStatsChanged();
}

quint64 LocationModel::bluetoothStats() const
{
    return m_bluetoothStats;
}

void LocationModel::setBluetoothStats(quint64 bluetoothStats)
{
    if (m_bluetoothStats == bluetoothStats)
        return;
    m_bluetoothStats = bluetoothStats;
    emit bluetoothStatsChanged();
}

QString LocationModel::loadingTitle() const
{
    return m_loadingTitle;
}

void LocationModel::setLoadingTitle(const QString &loadingTitle)
{
    if (m_loadingTitle == loadingTitle)
        return;
    m_loadingTitle = loadingTitle;
    emit loadingTitleChanged();
}

QString LocationModel::errorTitle() const
{
    return m_errorTitle;
}

void LocationModel::setErrorTitle(const QString &errorTitle)
{
    if (m_errorTitle == errorTitle)
        return;
    m_errorTitle = errorTitle;
    emit errorTitleChanged();
}

QString LocationModel::errorMessage() const
{
    return m_errorMessage;
}

void LocationModel::setErrorMessage(const QString &errorMessage)
{
    if (m_errorMessage == errorMessage)
        return;
    m_errorMessage = errorMessage;
    emit errorMessageChanged();
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
