#ifndef HttpClient_H
#define HttpClient_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>

class HttpClient : public QObject
{
    Q_OBJECT

public:
    explicit HttpClient(QObject *parent = nullptr);
    ~HttpClient();

    void get(const QUrl &url, const QVariantMap &headers = {});
    void post(const QUrl &url, const QVariantMap &data, const QVariantMap &headers = {});

signals:
    void responseReceived(const QByteArray &data);
    void errorOccurred(const QString &error);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_manager;
    void setRequestHeaders(QNetworkRequest &request, const QVariantMap &headers);
};

#endif // HttpClient_H