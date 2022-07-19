//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2021 Juan-Pablo Caceres, Chris Chafe.
  SoundWIRE group at CCRMA, Stanford University.

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation
  files (the "Software"), to deal in the Software without
  restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following
  conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.
*/
//*****************************************************************

/**
 * \file vsPinger.cpp
 * \author Dominick Hing
 * \date July 2022
 */

#include "vsPinger.h"

#include <QDateTime>
#include <QHostInfo>
#include <QString>
#include <QTimer>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>

using std::cout;
using std::endl;

// NOTE: It's better not to use
// using namespace std;
// because some functions (like exit()) get confused with QT functions

//*******************************************************************************
VsPinger::VsPinger(QString scheme, QString host, QString path, QString token)
    : mToken(token)
{
    mURL.setScheme(scheme);
    mURL.setHost(host);
    mURL.setPath(path);

    mTimer.setTimerType(Qt::PreciseTimer);

    connect(&mSocket, &QWebSocket::binaryMessageReceived, this,
            &VsPinger::receivePingMessage);
    connect(&mSocket, &QWebSocket::connected, this, &VsPinger::connected);
    connect(&mSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            this, &VsPinger::onError);
    connect(&mTimer, &QTimer::timeout, this, &VsPinger::onPingTimer);
}

//*******************************************************************************
void VsPinger::start()
{
    mTimer.setInterval(mPingInterval);

    QString authVal = "Bearer ";
    authVal.append(mToken);

    QNetworkRequest req = QNetworkRequest(QUrl(mURL));
    req.setRawHeader(QByteArray("Upgrade"), QByteArray("websocket"));
    req.setRawHeader(QByteArray("Connection"), QByteArray("upgrade"));
    req.setRawHeader(QByteArray("Authorization"), authVal.toUtf8());
    mSocket.open(req);

    mStarted = true;
}

//*******************************************************************************
void VsPinger::stop()
{
    mStarted = false;
    mTimer.stop();
    mSocket.close(QWebSocketProtocol::CloseCodeNormal, NULL);
}

//*******************************************************************************
void VsPinger::sendPingMessage(const QByteArray& message)
{
    mSocket.sendBinaryMessage(message);
}

//*******************************************************************************
void VsPinger::updateStats()
{
    PingStat stat;
    stat.packetsReceived = 0;
    stat.packetsSent     = 0;

    uint32_t count = 0;

    std::vector<uint32_t> vec_expired;
    std::vector<qint64> vec_rtt;
    std::map<uint32_t, VsPing*>::reverse_iterator it;
    for (it = mPings.rbegin(); it != mPings.rend(); ++it) {
        VsPing* ping = it->second;

        // Only include in statistics pings that have timed out or been received.
        // All others are pending and are not considered in statistics
        if (ping->timedOut() || ping->receivedReply()) {
            stat.packetsSent++;
            if (ping->receivedReply()) {
                stat.packetsReceived++;
            }

            QDateTime sent     = ping->sentTimestamp();
            QDateTime received = ping->receivedTimestamp();
            qint64 diff        = sent.msecsTo(received);
            vec_rtt.push_back(diff);

            count++;
        } else {
            continue;
        }

        // mark this ping as ready to delete, since it will no longer be used in stats
        if (count > mPingNumPerInterval) {
            vec_expired.push_back(ping->pingNumber());
        }
    }

    // Update RTT stats
    double min_rtt    = std::numeric_limits<qint64>::max();
    double max_rtt    = std::numeric_limits<qint64>::min();
    double avg_rtt    = 0;
    double stddev_rtt = 0;
    for (std::vector<qint64>::iterator it_rtt = vec_rtt.begin(); it_rtt != vec_rtt.end();
         it_rtt++) {
        double rtt = (double)*it_rtt;
        if (rtt < min_rtt) {
            min_rtt = rtt;
        }
        if (rtt > max_rtt) {
            max_rtt = rtt;
        }

        avg_rtt += rtt / vec_rtt.size();
    }

    for (std::vector<qint64>::iterator it_rtt = vec_rtt.begin(); it_rtt != vec_rtt.end();
         it_rtt++) {
        double rtt = (double)*it_rtt;
        stddev_rtt += (rtt - avg_rtt) * (rtt - avg_rtt);
    }
    stddev_rtt /= vec_rtt.size();

    stat.maxRtt    = max_rtt;
    stat.minRtt    = min_rtt;
    stat.avgRtt    = avg_rtt;
    stat.stdDevRtt = stddev_rtt;

    // Deleted pings marked as expired by freeing the Ping object and clearing the map
    // item
    for (std::vector<uint32_t>::iterator it_expired = vec_expired.begin();
         it_expired != vec_expired.end(); it_expired++) {
        uint32_t expiredPingNum = *it_expired;
        delete mPings.at(expiredPingNum);
        mPings.erase(expiredPingNum);
    }

    // Update mStats
    mStats = stat;
    return;
}

//*******************************************************************************
VsPinger::PingStat VsPinger::getPingStats()
{
    return mStats;
}

//*******************************************************************************
void VsPinger::onError(QAbstractSocket::SocketError error)
{
    cout << "WebSocket Error: " << error << endl;
    mStarted = false;
    mTimer.stop();
}

//*******************************************************************************
void VsPinger::connected()
{
    mTimer.start();
}

//*******************************************************************************
void VsPinger::onPingTimer()
{
    updateStats();

    QByteArray bytes = QByteArray::number(mPingCount);
    QDateTime now    = QDateTime::currentDateTime();
    this->sendPingMessage(bytes);

    VsPing* ping = new VsPing(mPingCount, mPingInterval);
    ping->send();
    mPings[mPingCount] = ping;

    connect(ping, &VsPing::timeout, this, &VsPinger::onPingTimeout);

    mLastPacketSent = mPingCount;
    mPingCount++;
}

//*******************************************************************************
void VsPinger::onPingTimeout(uint32_t pingNum)
{
    std::map<uint32_t, VsPing*>::iterator it = mPings.find(pingNum);
    if (it == mPings.end()) {
        return;
    }

    updateStats();
}

//*******************************************************************************
void VsPinger::receivePingMessage(const QByteArray& message)
{
    QDateTime now    = QDateTime::currentDateTime();
    uint32_t pingNum = message.toUInt();

    // locate the appropriate corresponding ping message
    std::map<uint32_t, VsPing*>::iterator it = mPings.find(pingNum);
    if (it == mPings.end()) {
        return;
    }

    VsPing* ping = (*it).second;

    // do not apply to pings that have timed out
    if (!ping->timedOut()) {
        // update ping data
        ping->receive();

        // update vsPinger
        mHasReceivedPing    = true;
        mLastPacketReceived = pingNum;
        if (pingNum > mLargestPingNumReceived) {
            mLargestPingNumReceived = pingNum;
        }
    }

    updateStats();
}