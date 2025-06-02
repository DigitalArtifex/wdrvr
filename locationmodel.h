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

struct LocationData
{
    QString name;
    QString styleTag;
    QString description;
    int open = 0;
    QPointF coordinates;
    qint64 clusterCount = 1;

    // LocationData();
    // LocationData(const LocationData &other)
    // {

    // }

    // LocationData &operator=(const LocationData &other)
    // {

    // }

    // bool operator==(const LocationData &data)
    // {
    //     return coordinates == data.coordinates;
    // }

    // bool operator!=(const LocationData &data)
    // {
    //     return !(coordinates == data.coordinates);
    // }
};

Q_DECLARE_METATYPE(LocationData)

struct LocationDataNode
{
    LocationData data;
    LocationDataNode *next = nullptr;
    LocationDataNode() {}
};

class LocationModel : public QAbstractListModel
{
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
    double logScale(double percentage, double min = 2, double max = 500000.0, double base = 10);

    Q_INVOKABLE void parseKML(QString fileName, bool append = false);

    qreal progress() const;
    void setProgress(qreal progress);

signals:
    void error(QString title, QString message);
    void progressChanged();

private:
    bool m_debug = false;
    void resetDataModel();
    void startUpdateTimer();
    QVector<LocationData> m_filteredData;
    qreal m_progress = 0;
    QTimer *m_updateTimer = nullptr;

    LocationDataNode *m_headNode = nullptr;
    LocationDataNode *m_lastNode = nullptr;
    quint64 m_nodeLength = 0;

    Q_PROPERTY(qreal progress READ progress WRITE setProgress NOTIFY progressChanged FINAL)
};

Q_DECLARE_METATYPE(LocationModel)

#endif // LOCATIONMODEL_H
