#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QQueue>
#include <QMap>
#include <QTimer>
#include <QNetworkReply>
#include "output.h"

class Downloader : public QObject
{
  Q_OBJECT
  int lastLength;
  QTimer timer;
  QQueue<QUrl> downloadQueue;
  QNetworkAccessManager* manager;
  QMap<QNetworkReply*,QUrl> replyUrlMap;
  QMap<QUrl,QUrl> redirectMapping;
  int maxConnections;
  int numConnections;
  QSet<QNetworkReply*> replies;
  void continueQueue();
  void printProgress();
  QUrl getUnredirectedUrl(QUrl url);
public:
  explicit Downloader(QObject *parent = 0);
  void load(const QUrl& url);
  int getMaxConnections() const;
  void setMaxConnections(int value);
signals:
  void downloadSuccess(const QUrl& url, const QByteArray& data);
  void downloadFailed(const QUrl& url, QNetworkReply::NetworkError error);
  void queueEmpty();
private slots:
  void downloadFinished(QNetworkReply* reply);
};

#endif // DOWNLOADER_H
