#include "app.h"

#include "httpClient.h"
#include <QJsonObject>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>

HttpClient::HttpClient(QObject *parent) : QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
}

HttpClient::~HttpClient()
{
    m_manager->deleteLater();
}

// Set headers for the request
void HttpClient::setRequestHeaders(QNetworkRequest &request, const QVariantMap &headers)
{
    for (auto it = headers.begin(); it != headers.end(); ++it)
    {
        request.setRawHeader(it.key().toUtf8(), it.value().toByteArray());
    }
}

// GET request
void HttpClient::get(const QUrl &url, const QVariantMap &headers)
{
    QNetworkRequest request(url);
    setRequestHeaders(request, headers);

    QNetworkReply *reply = m_manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { onReplyFinished(reply); });
}

// POST request (JSON data)
void HttpClient::post(const QUrl &url, const QVariantMap &data, const QVariantMap &headers)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    setRequestHeaders(request, headers);

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(data);
    QNetworkReply *reply = m_manager->post(request, jsonDoc.toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { onReplyFinished(reply); });
}

// Handle finished requests
void HttpClient::onReplyFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray data = reply->readAll();
        emit responseReceived(data);
    }
    else
    {
        emit errorOccurred(reply->errorString());
    }

    reply->deleteLater();
}

QByteArray HttpClient::getBlocking(const QUrl &url, const QVariantMap &headers, int timeoutMs) {
    QNetworkRequest request(url);
    setRequestHeaders(request, headers);

    QNetworkReply *reply = m_manager->get(request);
    QEventLoop loop;
    QTimer timer;

    // Timeout handling
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    timer.start(timeoutMs);
    loop.exec();  // Blocks until finished or timeout

    QByteArray data;
    if (reply->error() == QNetworkReply::NoError && timer.isActive()) {
        data = reply->readAll();
    } else {
        qDebug() << "Error or timeout:" << reply->errorString();
    }

    reply->deleteLater();
    return data;
}