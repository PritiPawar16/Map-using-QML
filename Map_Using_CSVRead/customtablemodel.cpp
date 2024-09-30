#include "customtablemodel.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

CustomTableModel::CustomTableModel(QObject *parent) : QAbstractTableModel(parent)
{
}

bool CustomTableModel::loadDataFromFile(const QString &fileName)
{
    beginResetModel();
    m_data.clear();

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    QString headerLine = in.readLine();
    headers = headerLine.split(",");

    // Add "Speed" column header
    headers.append("CalculatedSpeed");

    setHeaderData(0, Qt::Horizontal, headers);

    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList fields = line.split(",");

        // qDebug() << "Fields count:" << fields.size() << "Fields:" << fields;

        // Adjust the size of the fields list to match the expected number of columns
        while (fields.size() < headers.size() - 1) // exclude the new "Speed" column for now
        {
            fields.append("");
        }
        while (fields.size() > headers.size() - 1)
        {
            fields.removeLast();
        }

        // Add an empty string for the "Speed" column
        fields.append("");

        m_data.append(fields);
    }

    endResetModel(); // Notify views of updated model
    emit dataLoaded();
    return true;
}

void CustomTableModel::clearData()
{
    beginResetModel();
    m_data.clear();
    m_filteredData.clear();
    endResetModel();
}

int CustomTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    const QVector<QStringList> &dataToUse = m_filteredData.isEmpty() ? m_data : m_filteredData;
    return dataToUse.size();
}

int CustomTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    const QVector<QStringList> &dataToUse = m_filteredData.isEmpty() ? m_data : m_filteredData;
    if (dataToUse.isEmpty())
        return 0;
    return headers.size(); // Return the number of headers to ensure consistency
}

QVariant CustomTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const QVector<QStringList> &dataToUse = m_filteredData.isEmpty() ? m_data : m_filteredData;

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        int row = index.row();
        int col = index.column();
        if (row < 0 || row >= dataToUse.size() || col < 0 || col >= dataToUse.first().size())
            return QVariant();
        return dataToUse[row][col];
    }
    else if (role == Qt::TextAlignmentRole)
    {
        return Qt::AlignCenter;
    }

    return QVariant();
}

QVariant CustomTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            if (section >= 0 && section < headers.size())
            {
                return headers.at(section);
            }
        }
        else if (orientation == Qt::Vertical)
        {
            if (section >= 0 && section < m_data.size())
            {
                return QString::number(section + 1);
            }
        }
    }
    return QVariant(); // Return an invalid variant if data not available
}

void CustomTableModel::updateSpeeds(const QList<double> &speeds)
{
    // qDebug() << "Calculated speed size" << speeds.size();
    // qDebug() << "Data size" << m_data.size();

    // Ensure that we have one extra slot in speeds to account for the initial zero
    if (speeds.size() != m_data.size() - 1)
    {
        qDebug() << "Speed data size does not match with the table data size.";
        return;
    }

    // Create a new speeds list with the first speed as zero
    QList<double> adjustedSpeeds = {0.0};
    adjustedSpeeds.append(speeds);

    beginResetModel();
    for (int i = 0; i < m_data.size(); ++i)
    {
        if (i < adjustedSpeeds.size())
        {
            m_data[i].last() = QString::number(adjustedSpeeds[i]);
            // qDebug() << "Speed for point" << i << ":" << adjustedSpeeds[i];
        }
    }
    endResetModel();
}
