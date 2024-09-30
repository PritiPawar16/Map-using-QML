#ifndef CUSTOMTABLEMODEL_H
#define CUSTOMTABLEMODEL_H
#include <QAbstractTableModel>
#include <QVector>
#include<QDateTime>

struct RowData
{
    QDateTime dateTime;
};
class CustomTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit CustomTableModel(QObject *parent = nullptr);

    bool loadDataFromFile(const QString &fileName);
    void clearData();
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    void updateSpeeds(const QList<double> &speeds);

signals:
    void dataLoaded();
    void dataReset();

private:
    QVector<QStringList> m_data;
    QStringList headers;
    QVector<QStringList> m_filteredData;  
};

#endif // CUSTOMTABLEMODEL_H
