#include "locationmodel.h"


LocationData::~LocationData()
{
    // while(children)
    // {
    //     LocationDataNode *node = children;
    //     children = children->next;

    //     delete node;
    // }
}

LocationModel::LocationModel(QObject *parent)
    : QAbstractListModel{parent}
{

    m_updateTimer = new QTimer;
    m_updateTimer->setInterval(10);

    connect(m_updateTimer, &QTimer::timeout, this, &LocationModel::updateProgress);

    m_sqlDatabase = QSqlDatabase::addDatabase("QSQLITE");

    QDir databaseDirectory(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    QStringList databases = databaseDirectory.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    if(!databases.contains("default"))
    {
        createDatabase("default");
        databases.insert(0, "default");
    }

    setAvailableDatabases(databases);
}

LocationModel::~LocationModel()
{
    if(m_updateTimer)
        delete m_updateTimer;

    resetSectorData();
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
    case ColorRole:
        data = QVariant::fromValue<QColor>(m_filteredData[index.row()].color);
        break;
    case SizeRole:
        data = QVariant::fromValue<qreal>(m_filteredData[index.row()].dotSize);
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
        { SignalRole, "signal" },
        { ColorRole, "dotColor" },
        { SizeRole, "dotSize" }
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

            QString capabilitiesString = segments[2];
            capabilitiesString.replace("][", " ");
            capabilitiesString.remove("[");
            capabilitiesString.remove("]");

            QStringList capabilities = capabilitiesString.split(' ', Qt::SkipEmptyParts);
            QDateTime timestamp;

            bool okay = false;
            quint64 ms = segments[3].toLongLong(&okay);

            if(okay)
                timestamp = QDateTime::fromMSecsSinceEpoch(ms);

            if(!timestamp.isValid()) //fallback attempt
                timestamp = QDateTime::fromString(segments[3],QString("yyyy-MM-ddThh:mm:sstt"));

            if(!timestamp.isValid()) //fallback attempt 2
                timestamp = QDateTime::fromString(segments[3],QString("yyyy-MM-ddThh:mm:ss.zzzt"));

            if(!timestamp.isValid()) //well crap
            {
                errorOccurred("Document Parsing Error", "Document uses an invalid timestamp format. Aborting.");
                file.close();

                return;
            }

            LocationData data {
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
                timestamp,
                segments[12],
                segments[5].toDouble(),
                capabilities,
                segments[11].split(' ', Qt::SkipEmptyParts)
            };

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
                                    data.type = value;
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
                                else if(key == "time")//"2025-05-29T08:45:33.000-07:00" OR MS Since Epoch
                                {

                                    bool okay = false;
                                    quint64 ms = value.toLongLong(&okay);

                                    if(okay)
                                        data.timestamp = QDateTime::fromMSecsSinceEpoch(ms);

                                    if(!okay || !data.timestamp.isValid()) //fallback attempt
                                        data.timestamp = QDateTime::fromString(value,QString("yyyy-MM-ddThh:mm:sstt"));

                                    if(!data.timestamp.isValid()) //fallback attempt 2
                                        data.timestamp = QDateTime::fromString(value,QString("yyyy-MM-ddThh:mm:ss.zzzt"));

                                    if(!data.timestamp.isValid()) //well crap
                                    {
                                        errorOccurred("Document Parsing Error", "Document uses an invalid timestamp format. Aborting.");
                                        file.close();

                                        return;
                                    }
                                }
                                else if(key == "signal")
                                    data.signal = value.toDouble();
                                else if(key == "network id" || key == "id")
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
    });
}

void LocationModel::openFile(QString fileName)
{
#ifdef Q_OS_WIN
    //damn windows
    if(fileName.startsWith("file:///", Qt::CaseInsensitive))
        fileName.remove(0,8);
#endif
#ifdef Q_OS_LINUX
    if(fileName.startsWith("file://", Qt::CaseInsensitive))
        fileName.remove(0,7);
#endif

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

void LocationModel::append(const LocationData &data, bool save)
{
    if(m_ids.contains(data.id))
    {
        m_ids[data.id] = m_ids[data.id] + 1;
        return;
    }

    m_ids.insert(data.id, 1);

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

    if(save)
    {
        m_databaseMutex.lock();
        addToDataBase(data);
        m_databaseMutex.unlock();
    }

    //set type stats
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

        m_databaseMutex.lock();
        QDir databaseDirectory = getDatabaseDirectory();
        QFile mpsFile(databaseDirectory.absolutePath() + QDir::separator() + "mps.dat");

        if(mpsFile.open(QFile::ReadWrite))
        {
            for(const qreal &value : std::as_const(m_mps))
                mpsFile.write(QString::number(value).toUtf8() + "\n");
        }

        mpsFile.close();

        if(!m_sqlDatabase.isOpen() && !m_sqlDatabase.open())
        {
            qDebug() << "Could not open database" << m_database;
            qDebug() << m_sqlDatabase.lastError();

            m_databaseMutex.unlock();
            return;
        }

        quint64 totalOps = 0;
        quint64 currentOp = 0;

        //get total ops
        for(int longitudeIndex = 0; longitudeIndex < 360; ++longitudeIndex)
        {
            for(int latitudeIndex = 0; latitudeIndex < 180; ++latitudeIndex)
            {
                if(m_sectors[longitudeIndex][latitudeIndex].updated)
                    totalOps += m_sectors[longitudeIndex][latitudeIndex].locations;
            }
        }

        QSqlQuery query(m_sqlDatabase);

        for(int longitudeIndex = 0; longitudeIndex < 360; ++longitudeIndex)
        {
            for(int latitudeIndex = 0; latitudeIndex < 180; ++latitudeIndex)
            {
                LocationDataNode *node = m_sectors[longitudeIndex][latitudeIndex].head;

                while(node)
                {
                    LocationData &data = node->data;

                    QString sanitizedDescription = QUrl::toPercentEncoding(data.description);
                    sanitizedDescription.replace('%', "\%");

                    QString command = QString("INSERT OR REPLACE INTO pois(id, accuracy, longitude, latitude, description, encryption, name, open, signal, style, type, timestamp, mfgid, frequency, capabilities, rois) VALUES(\"%1\",").arg(data.id);
                    command += QString(" \"%1\", \"%2\", \"%3\"").arg(QString::number(data.accuracy), QString::number(data.coordinates.longitude()), QString::number(data.coordinates.latitude()));
                    command += QString(", \"%1\", \"%2\", \"%3\"").arg(sanitizedDescription, data.encryption, QUrl::toPercentEncoding(data.name));
                    command += QString(", \"%1\", \"%2\", \"%3\"").arg(QString::number(data.open), QString::number(data.signal), data.styleTag);
                    command += QString(", \"%1\", \"%2\", \"%3\"").arg(data.type, data.timestamp.toString(), data.mfgid);
                    command += QString(", \"%1\", \"%2\", \"%3\")").arg(QString::number(data.frequency), data.capabilities.join(':'), data.rois.join(':'));

                    if(!query.exec(command))
                    {
                        qDebug() << "Failed to save database" << m_database;
                        qDebug() << query.lastError();
                        qDebug() << command;
                        m_databaseMutex.unlock();
                        return;
                    }

                    node = node->next;

                    setProgress(static_cast<qreal>(++currentOp) / totalOps);
                }
            }
        }

        m_databaseMutex.unlock();
    }));

    watcher.connect(&watcher, &QFutureWatcher<void>::finished, this, [this](){
        endLoading();
    });
}

void LocationModel::load(QString database)
{
    startLoading("Loading");

    resetSectorData();
    resetDataModel();

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

        if(!m_databaseMutex.tryLock(500))
            return;

        QDir databaseDirectory = getDatabaseDirectory(database);

        if(m_sqlDatabase.isOpen())
        {
            m_sqlDatabase.close();
            QSqlDatabase::removeDatabase(m_sqlDatabase.connectionName());
        }

        m_sqlDatabase = QSqlDatabase::addDatabase("QSQLITE");
        m_sqlDatabase.setDatabaseName(databaseDirectory.absoluteFilePath(database + ".db"));

        if(!m_sqlDatabase.open())
        {
            qDebug() << "Could not open database" << database << m_sqlDatabase.lastError();
            m_databaseMutex.unlock();
            return;
        }

        //get DB size
        QSqlQuery query(m_sqlDatabase);
        query.prepare(QString("SELECT COUNT(*) FROM pois"));
        query.exec();

        if (query.next()) {
            m_totalPointsOfInterestTemp = query.value(0).toInt();
        }

        setProgress(0);

        quint64 loadedDataCount = 0;
        quint64 loops = (m_totalPointsOfInterestTemp / 1000);

        if(m_totalPointsOfInterestTemp % 1000)
            ++loops;

        //read in chunks of 1000 entries
        for(quint64 i = 0; i < loops; ++i)
        {
            quint64 sqlStartIndex = (i * 1000);

            QString command = QString("SELECT * FROM pois LIMIT 1000 OFFSET %1").arg(QString::number(sqlStartIndex));

            query.prepare(command);

            if(!query.exec())
            {
                qDebug() << "Could not load database" << database << m_sqlDatabase.lastError();
                m_databaseMutex.unlock();
                return;
            }

            while(query.next())
            {
                LocationData data
                {
                    query.value("accuracy").toDouble(),
                    1,
                    QGeoCoordinate(query.value("latitude").toDouble(), query.value("longitude").toDouble()),
                    QUrl::fromPercentEncoding(query.value("description").toByteArray()),
                    query.value("encryption").toString(),
                    query.value("id").toString(),
                    QUrl::fromPercentEncoding(query.value("name").toByteArray()),
                    query.value("open").toString().toInt(),
                    query.value("signal").toDouble(),
                    query.value("style").toString(),
                    query.value("type").toString(),
                    query.value("timestamp").toDateTime(),
                    query.value("mfgid").toString(),
                    query.value("frequency").toDouble(),
                    query.value("capabilities").toString().split(':'),
                    query.value("rois").toString().split(':')
                };

                append(data, false);

                setProgress(++loadedDataCount / m_totalPointsOfInterestTemp);
            }
        }

        setLoadedDatabase(database);
        m_databaseMutex.unlock();
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

    while(viewportNodeIteratorA)
    {
        LocationDataNode *viewportNodeIteratorB = viewportNodeIteratorA->next;
        LocationDataNode *viewportNodeIteratorC = viewportNodeIteratorA; //last referenced for splicing
        QGeoCoordinate coordA(viewportNodeIteratorA->data.coordinates.latitude(), viewportNodeIteratorA->data.coordinates.longitude());

        while(viewportNodeIteratorB)
        {
            QGeoCoordinate coordB(viewportNodeIteratorB->data.coordinates.latitude(), viewportNodeIteratorB->data.coordinates.longitude());
            qreal coordDistance = coordA.distanceTo(coordB);

            if(coordDistance <= clusterDistance)
            {
                viewportNodeIteratorC->next = viewportNodeIteratorB->next;
                delete viewportNodeIteratorB;
                // if(!viewportNodeIteratorC->data.children)
                // {
                //     viewportNodeIteratorC->data.children = viewportNodeIteratorB;
                //     viewportNodeIteratorC->data.lastChild = viewportNodeIteratorB;
                // }
                // else
                // {
                //     viewportNodeIteratorC->data.lastChild->next = viewportNodeIteratorB;
                //     viewportNodeIteratorC->data.lastChild = viewportNodeIteratorC->data.lastChild->next;
                // }

                viewportNodeIteratorB = viewportNodeIteratorC->next;

                //set the size and color for the heatmap
                if(viewportNodeIteratorC->data.color.green() > 16)
                    viewportNodeIteratorC->data.color.setGreen(viewportNodeIteratorC->data.color.green() - 16);

                if(viewportNodeIteratorC->data.color.red() < (249))
                    viewportNodeIteratorC->data.color.setRed(viewportNodeIteratorC->data.color.red() + 16);
                else if(viewportNodeIteratorC->data.color.blue() < 249)
                    viewportNodeIteratorC->data.color.setBlue(viewportNodeIteratorC->data.color.blue() + 16);

                --count;

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
        if(!m_threadMutex.tryLock(QDeadlineTimer(250)))
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
        // beginInsertRows(QModelIndex(), 0, totalNodes - 1);
        while(headNode)
        {
            beginInsertRows(QModelIndex(), index, index + 1);

            LocationDataNode *nodeToDelete = headNode;
            m_filteredData.append(headNode->data);
            headNode = headNode->next;

            delete nodeToDelete;
            endInsertRows();
            ++index;
        }
        // endInsertRows();

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

QDir LocationModel::getDatabaseDirectory(QString name)
{
    if(name.isEmpty())
        name = m_loadedDatabase;

    QDir directory = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + name);

    if(!directory.exists())
        directory.mkpath(directory.absolutePath());

    return directory;
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
        m_databaseMutex.lock();

        //clear filtered data
        resetDataModel();
        resetSectorData();

        QFile::remove(getDatabaseDirectory().absoluteFilePath(m_loadedDatabase + ".db"));
        getDatabaseDirectory().rmdir(getDatabaseDirectory().absolutePath());

        m_databaseMutex.unlock();

    });

    watcher.disconnect();
    watcher.setFuture(result);

    connect(&watcher, &QFutureWatcher<void>::finished, this, [this](){
        endLoading();

        if(m_loadedDatabase == "default")
            createDatabase("default");
        else
            load("default");
    });
}

void LocationModel::createDatabase(QString name)
{
    startLoading("Creating Database");

    auto result = QtConcurrent::run([this, name](){
        QDir databaseDirectory = getDatabaseDirectory(name);

        if(QFile::exists(databaseDirectory.absoluteFilePath(name + ".db")))
        {
            errorOccurred("Create Database Error", "Database already exists");
            return;
        }

        QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
        database.setDatabaseName(databaseDirectory.absoluteFilePath(name + ".db"));

        if(!database.open())
        {
            qDebug() << "Could not open database" << database.lastError();
            return;
        }

        QString command = "CREATE TABLE IF NOT EXISTS pois (id TEXT PRIMARY KEY, name TEXT, timestamp TEXT, type TEXT";
        command += ", mfgid TEXT, description BLOB, longitude TEXT, latitude TEXT, rois TEXT, capabilities TEXT";
        command += ", style TEXT, encryption TEXT, open TEXT, signal TEXT, frequency TEXT, accuracy TEXT)";
        QSqlQuery query;

        query.prepare(command);
        if(!query.exec())
        {
            qDebug() << "Could not open database" << database.lastError();
            return;
        }

        database.close();

        qDebug() << "Created" << name;

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

void LocationModel::addToDataBase(const LocationData &data)
{
    bool close = false;
    if(!m_sqlDatabase.isOpen() && !(close = m_sqlDatabase.open()))
    {
        qDebug() << "Could not open database" << m_loadedDatabase << m_sqlDatabase.lastError();
        return;
    }

    QSqlQuery query(m_sqlDatabase);

    QString sanitizedDescription = QUrl::toPercentEncoding(data.description);
    sanitizedDescription.replace('%', "\%");

    QString command = QString("INSERT OR REPLACE INTO pois(id, accuracy, longitude, latitude, description, encryption, name, open, signal, style, type, timestamp, mfgid, frequency, capabilities, rois) VALUES(\"%1\",").arg(data.id);
    command += QString(" \"%1\", \"%2\", \"%3\"").arg(QString::number(data.accuracy), QString::number(data.coordinates.longitude()), QString::number(data.coordinates.latitude()));
    command += QString(", \"%1\", \"%2\", \"%3\"").arg(sanitizedDescription, data.encryption, QUrl::toPercentEncoding(data.name));
    command += QString(", \"%1\", \"%2\", \"%3\"").arg(QString::number(data.open), QString::number(data.signal), data.styleTag);
    command += QString(", \"%1\", \"%2\", \"%3\"").arg(data.type, data.timestamp.toString(), data.mfgid);
    command += QString(", \"%1\", \"%2\", \"%3\")").arg(QString::number(data.frequency), data.capabilities.join(':'), data.rois.join(':'));

    if(!query.exec(command))
    {
        qDebug() << "Failed to save database" << m_database;
        qDebug() << query.lastError();
        qDebug() << command;
        m_databaseMutex.unlock();
        return;
    }

    if(close)
        m_sqlDatabase.close();
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
