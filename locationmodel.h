#ifndef LOCATIONMODEL_H
#define LOCATIONMODEL_H

#include <QObject>
#include <QFuture>
#include <QFutureSynchronizer>
#include <QHash>
#include <QVector>
#include <QAbstractListModel>
#include <QPointF>
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QStringView>
#include <QTimer>
#include <QtPositioning>
#include <QtLocation>
#include <QGeoLocation>
#include <QGeoCoordinate>
#include <QtConcurrent/QtConcurrentRun>
#include <QRegularExpression>
#include <QVariantHash>

/*
 * Location data memory mapping
 *
 * In order to efficiently store and sort potentially millions of POIs we should be able to break it
 * up into multiple smaller steps.
 *
 *
 */
struct LocationDataNode;

struct Sector
{
    LocationDataNode *head = nullptr;
    LocationDataNode *last = nullptr;

    quint64 locations;
    bool updated = false;
    QMutex mutex;
};

struct LocationData
{
    qreal accuracy = 0;
    qint64 clusterCount = 1;
    QGeoCoordinate coordinates;
    QString description;
    QString encryption;
    QString id;
    QString name;
    int open = 0;
    qreal signal = 0;
    QString styleTag;
    QString type;
    QDateTime timestamp;
    QString mfgid;
    qreal frequency;
    QStringList capabilities;
    QStringList rois;

    QColor color = QColor(0,0,128,200);
    qreal dotSize = 20;

    LocationDataNode *children = nullptr;
    LocationDataNode *lastChild = nullptr;

    ~LocationData();

    inline bool operator > (const LocationData &other)
    {
        return coordinates.distanceTo(QGeoCoordinate(-90,-180)) > other.coordinates.distanceTo(QGeoCoordinate(-90,-180));
    }
    inline bool operator < (const LocationData &other)
    {
        return coordinates.distanceTo(QGeoCoordinate(-90,-180)) < other.coordinates.distanceTo(QGeoCoordinate(-90,-180));
    }
};

Q_DECLARE_METATYPE(LocationData)

struct LocationDataNode
{
    LocationData data;
    LocationDataNode *next = nullptr;
    LocationDataNode() {}
    LocationDataNode(const LocationData &data) { this->data = data; }
};

Q_DECLARE_METATYPE(LocationDataNode)

class LocationModel : public QAbstractListModel
{
    QML_ELEMENT
    Q_OBJECT
public:
    enum LocationDataRole
    {
        LocationRole = Qt::UserRole,
        NameRole,
        StyleRole,
        DescriptionRole,
        OpenNetworksRole,
        TypeRole,
        EncryptionRole,
        TimestampRole,
        AccuracyRole,
        SignalRole,
        ColorRole,
        SizeRole
    };

    Q_ENUM(LocationDataRole)

    explicit LocationModel(QObject *parent = nullptr);
    ~LocationModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index = QModelIndex(), int role = Qt::DisplayRole)const override;

    QHash<int, QByteArray> roleNames() const override;

    void clear();
    double logScale(double percentage, double min = 10, double max = 500000.0);

    void parseKML(QString fileName);
    void parseCSV(QString fileName);
    Q_INVOKABLE void openFile(QString fileName);

    qreal progress() const;
    void setProgress(qreal progress);

    void append(const LocationData &data);
    void sort();
    Q_INVOKABLE void save();
    Q_INVOKABLE void load(QString database);

    quint64 totalPointsOfInterest() const;
    void setTotalPointsOfInterest(quint64 totalPointsOfInterest);

    quint64 bluetoothPointsOfInterest() const;
    void setBluetoothPointsOfInterest(quint64 bluetoothPointsOfInterest);

    quint64 cellularPointsOfInterest() const;
    void setCellularPointsOfInterest(quint64 cellularPointsOfInterest);

    quint64 wifiPointsOfInterest() const;
    void setWifiPointsOfInterest(quint64 wifiPointsOfInterest);

    QString database() const;
    void setDatabase(const QString &database);

    QStringList availableDatabases() const;
    void setAvailableDatabases(const QStringList &availableDatabases);

    Q_INVOKABLE void resetDatabase();
    Q_INVOKABLE void createDatabase(QString name);

    QString errorMessage() const;
    void setErrorMessage(const QString &errorMessage);

    QString errorTitle() const;
    void setErrorTitle(const QString &errorTitle);

    QString loadingTitle() const;
    void setLoadingTitle(const QString &loadingTitle);

    QString loadedDatabase() const;
    void setLoadedDatabase(const QString &loadedDatabase);

    QString currentPage() const;
    void setCurrentPage(const QString &currentPage);

    quint64 bluetoothStats() const;
    void setBluetoothStats(quint64 bluetoothStats);

    quint64 bluetoothLEStats() const;
    void setBluetoothLEStats(quint64 bluetoothLEStats);

    quint64 gsmStats() const;
    void setGsmStats(quint64 gsmStats);

    quint64 cdmaStats() const;
    void setCdmaStats(quint64 cdmaStats);

    quint64 wcdmaStats() const;
    void setWcdmaStats(quint64 wcdmaStats);

    quint64 lteStats() const;
    void setLteStats(quint64 lteStats);

    quint64 nrStats() const;
    void setNrStats(quint64 nrStats);

    quint64 wifiStats() const;
    void setWifiStats(quint64 wifiStats);

    qreal mpsAverage() const;
    void setMpsAverage(qreal mpsAverage);

public slots:
    Q_INVOKABLE void getPointsInRect(QGeoShape area, qreal zoomLevel);

private slots:
    void updateProgress();
    void errorOccurred(QString title, QString message);

signals:
    void error();
    void progressChanged();

    void totalPointsOfInterestChanged();

    void bluetoothPointsOfInterestChanged();

    void cellularPointsOfInterestChanged();

    void wifiPointsOfInterestChanged();
    void loadingFinished();
    void loadingStarted();

    void databaseChanged();

    void availableDatabasesChanged();

    void errorMessageChanged();

    void errorTitleChanged();

    void loadingTitleChanged();

    void loadedDatabaseChanged();

    void currentPageChanged();

    void bluetoothStatsChanged();

    void bluetoothLEStatsChanged();

    void gsmStatsChanged();

    void cdmaStatsChanged();

    void wcdmaStatsChanged();

    void lteStatsChanged();

    void nrStatsChanged();

    void wifiStatsChanged();

    void mpsAverageChanged();

private:

    const QStringList wifiTypeKeys {
        "WIFI"
    };

    const QStringList bluetoothTypeKeys {
        "BT",
        "BLE"
    };

    const QStringList cellTypeKeys {
        "GSM",
        "CDMA",
        "WCDMA",
        "LTE",
        "NR"
    };

    QVector<QString> m_ids;

    void calculateMPS();

    QRegularExpression m_kmlDescriptionSeparator = QRegularExpression("(?<!:)\\s");
    QString m_database = "default";
    QString m_loadedDatabase = "default";
    QStringList m_availableDatabases { "default" };
    QList<qreal> m_mps;
    qreal m_mpsAverage = 0;

    QString m_currentPage = "map";

    Sector m_sectors[360][180];
    bool m_loading = false;

    QFutureWatcher<void> watcher;

    bool m_debug = false;
    void resetDataModel();
    void resetSectorData();
    void startLoading(QString title);
    void endLoading();
    void startUpdateTimer();
    void stopUpdateTimer();
    QTimer *createUpdateTimer();

    QMutex m_threadMutex;
    QVector<LocationData> m_filteredData;
    qreal m_progress = 0;
    QString m_loadingTitle = "Loading";
    QTimer *m_updateTimer = nullptr;

    LocationDataNode *m_headNode = nullptr;
    LocationDataNode *m_lastNode = nullptr;
    quint64 m_nodeLength = 0;

    quint64 m_totalPointsOfInterest = 0;
    quint64 m_bluetoothPointsOfInterest = 0;
    quint64 m_cellularPointsOfInterest = 0;
    quint64 m_wifiPointsOfInterest = 0;

    quint64 m_bluetoothStats = 0;
    quint64 m_bluetoothLEStats = 0;
    quint64 m_gsmStats = 0;
    quint64 m_cdmaStats = 0;
    quint64 m_wcdmaStats = 0;
    quint64 m_lteStats = 0;
    quint64 m_nrStats = 0;
    quint64 m_wifiStats = 0;

    quint64 m_totalPointsOfInterestTemp = 0;
    quint64 m_bluetoothPointsOfInterestTemp = 0;
    quint64 m_cellularPointsOfInterestTemp = 0;
    quint64 m_wifiPointsOfInterestTemp = 0;

    QString m_errorMessage;
    QString m_errorTitle;

    Q_PROPERTY(qreal progress READ progress WRITE setProgress NOTIFY progressChanged FINAL)
    Q_PROPERTY(quint64 totalPointsOfInterest READ totalPointsOfInterest WRITE setTotalPointsOfInterest NOTIFY totalPointsOfInterestChanged FINAL)
    Q_PROPERTY(quint64 bluetoothPointsOfInterest READ bluetoothPointsOfInterest WRITE setBluetoothPointsOfInterest NOTIFY bluetoothPointsOfInterestChanged FINAL)
    Q_PROPERTY(quint64 cellularPointsOfInterest READ cellularPointsOfInterest WRITE setCellularPointsOfInterest NOTIFY cellularPointsOfInterestChanged FINAL)
    Q_PROPERTY(quint64 wifiPointsOfInterest READ wifiPointsOfInterest WRITE setWifiPointsOfInterest NOTIFY wifiPointsOfInterestChanged FINAL)
    Q_PROPERTY(QString database READ database WRITE setDatabase NOTIFY databaseChanged FINAL)
    Q_PROPERTY(QStringList availableDatabases READ availableDatabases WRITE setAvailableDatabases NOTIFY availableDatabasesChanged FINAL)
    Q_PROPERTY(QString errorMessage READ errorMessage WRITE setErrorMessage NOTIFY errorMessageChanged FINAL)
    Q_PROPERTY(QString errorTitle READ errorTitle WRITE setErrorTitle NOTIFY errorTitleChanged FINAL)
    Q_PROPERTY(QString loadingTitle READ loadingTitle WRITE setLoadingTitle NOTIFY loadingTitleChanged FINAL)
    Q_PROPERTY(QString loadedDatabase READ loadedDatabase WRITE setLoadedDatabase NOTIFY loadedDatabaseChanged FINAL)
    Q_PROPERTY(QString currentPage READ currentPage WRITE setCurrentPage NOTIFY currentPageChanged FINAL)
    Q_PROPERTY(quint64 bluetoothStats READ bluetoothStats WRITE setBluetoothStats NOTIFY bluetoothStatsChanged FINAL)
    Q_PROPERTY(quint64 bluetoothLEStats READ bluetoothLEStats WRITE setBluetoothLEStats NOTIFY bluetoothLEStatsChanged FINAL)
    Q_PROPERTY(quint64 gsmStats READ gsmStats WRITE setGsmStats NOTIFY gsmStatsChanged FINAL)
    Q_PROPERTY(quint64 cdmaStats READ cdmaStats WRITE setCdmaStats NOTIFY cdmaStatsChanged FINAL)
    Q_PROPERTY(quint64 wcdmaStats READ wcdmaStats WRITE setWcdmaStats NOTIFY wcdmaStatsChanged FINAL)
    Q_PROPERTY(quint64 lteStats READ lteStats WRITE setLteStats NOTIFY lteStatsChanged FINAL)
    Q_PROPERTY(quint64 nrStats READ nrStats WRITE setNrStats NOTIFY nrStatsChanged FINAL)
    Q_PROPERTY(quint64 wifiStats READ wifiStats WRITE setWifiStats NOTIFY wifiStatsChanged FINAL)
    Q_PROPERTY(qreal mpsAverage READ mpsAverage WRITE setMpsAverage NOTIFY mpsAverageChanged FINAL)
};

Q_DECLARE_METATYPE(LocationModel)

#endif // LOCATIONMODEL_H
