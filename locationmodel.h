#ifndef LOCATIONMODEL_H
#define LOCATIONMODEL_H

#include <QObject>
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
};

struct LocationData
{
    QString name;
    QString styleTag;
    QString description;
    int open = 0;
    QGeoCoordinate coordinates;
    qint64 clusterCount = 1;

    QString type;
    QString encryption;
    QDateTime timestamp;
    qreal accuracy = 0;
    qreal signal = 0;
};

Q_DECLARE_METATYPE(LocationData)

struct LocationDataNode
{
    LocationData data;
    LocationDataNode *next = nullptr;
    LocationDataNode() {}
    LocationDataNode(const LocationData &data) { this->data = data; }
};

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
        OpenNetworksRole
    };

    Q_ENUM(LocationDataRole)

    explicit LocationModel(QObject *parent = nullptr);
    ~LocationModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index = QModelIndex(), int role = Qt::DisplayRole)const override;

    QHash<int, QByteArray> roleNames() const override;

    void setLocations(const QVector<LocationData> &locations);
    void setLocation(const LocationData &location);
    Q_INVOKABLE void getLocations(const QGeoCoordinate &center, const qreal &distanceFrom, const qreal &precision);
    void clear();
    double logScale(double percentage, double min = 10, double max = 500000.0);

    Q_INVOKABLE void parseKML(QString fileName, bool append = false);
    Q_INVOKABLE void openFile(QString fileName, bool append = false);

    qreal progress() const;
    void setProgress(qreal progress);

    void append(const LocationData &data);

    quint64 totalPointsOfInterest() const;
    void setTotalPointsOfInterest(quint64 totalPointsOfInterest);

    quint64 bluetoothPointsOfInterest() const;
    void setBluetoothPointsOfInterest(quint64 bluetoothPointsOfInterest);

    quint64 cellularPointsOfInterest() const;
    void setCellularPointsOfInterest(quint64 cellularPointsOfInterest);

    quint64 wifiPointsOfInterest() const;
    void setWifiPointsOfInterest(quint64 wifiPointsOfInterest);

signals:
    void error(QString title, QString message);
    void progressChanged();

    void totalPointsOfInterestChanged();

    void bluetoothPointsOfInterestChanged();

    void cellularPointsOfInterestChanged();

    void wifiPointsOfInterestChanged();

private:
    //Sector m_sectors[180][360];

    bool m_debug = false;
    void resetDataModel();
    void startUpdateTimer();

    QMutex m_threadMutex;
    QVector<LocationData> m_filteredData;
    qreal m_progress = 0;
    QTimer *m_updateTimer = nullptr;

    LocationDataNode *m_headNode = nullptr;
    LocationDataNode *m_lastNode = nullptr;
    quint64 m_nodeLength = 0;

    quint64 m_totalPointsOfInterest = 0;
    quint64 m_bluetoothPointsOfInterest = 0;
    quint64 m_cellularPointsOfInterest = 0;
    quint64 m_wifiPointsOfInterest = 0;

    Q_PROPERTY(qreal progress READ progress WRITE setProgress NOTIFY progressChanged FINAL)
    Q_PROPERTY(quint64 totalPointsOfInterest READ totalPointsOfInterest WRITE setTotalPointsOfInterest NOTIFY totalPointsOfInterestChanged FINAL)
    Q_PROPERTY(quint64 bluetoothPointsOfInterest READ bluetoothPointsOfInterest WRITE setBluetoothPointsOfInterest NOTIFY bluetoothPointsOfInterestChanged FINAL)
    Q_PROPERTY(quint64 cellularPointsOfInterest READ cellularPointsOfInterest WRITE setCellularPointsOfInterest NOTIFY cellularPointsOfInterestChanged FINAL)
    Q_PROPERTY(quint64 wifiPointsOfInterest READ wifiPointsOfInterest WRITE setWifiPointsOfInterest NOTIFY wifiPointsOfInterestChanged FINAL)
};

Q_DECLARE_METATYPE(LocationModel)

#endif // LOCATIONMODEL_H
