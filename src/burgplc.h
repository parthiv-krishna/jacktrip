#ifndef BURGPLC_H
#define BURGPLC_H

#include "burgalgorithm.h"
#include "AudioInterface.h"
#include <QStack>
#include <QMutexLocker>

class BurgPLC
{
public:
    BurgPLC(int sample_rate, int channels, int bit_res, int FPP, int qLen, int hist);
    int8_t* getBufferPtr() { return mXfrBuffer; };
    void processPacket (bool glitch);
    int bytesToInt(const int8_t *buf);
    void pushPacket (const int8_t* buf, int seq);
    void pullPacket (int8_t* buf);
private:
    QMutex mMutex;                     ///< Mutex to protect read and write operations
    int mSampleRate;
    int mNumChannels;
    int mAudioBitRes;
    int mFPP;
    int mQLen;
    int mHist;
    int mTotalSize;  ///< Total size of mXfrBuffer
    AudioInterface::audioBitResolutionT mBitResolutionMode;
    BurgAlgorithm ba;
    int8_t* mXfrBuffer;
    int mPacketCnt;
    sample_t bitsToSample(int ch, int frame);
void sampleToBits(sample_t sample, int ch, int frame);
    QString qStringFromLongDouble(const long double myLongDouble);
    vector<sample_t> mTrain;
    vector<sample_t> mPrediction; // ORDER
    vector<long double> mCoeffs;
    vector<sample_t> mTruth;
    vector<sample_t> mTruthCh1;
    vector<sample_t> mXfadedPred;
    vector<sample_t> mNextPred;
    vector<sample_t> mLastGoodPacket;
    vector<vector<sample_t>> mLastPackets;
    vector<sample_t> mFadeUp;
    vector<sample_t> mFadeDown;
    bool mLastWasGlitch;
    vector<double> mPhasor;
    vector<int8_t*> mIncomingPacket;
    int mIdealOneSecondsWorthOfPackets;
    int mOneSecondPacketCounter;
    vector<int> mLastPush;
    vector<int> mLastPull; // for debug
    int mLastFrame;
    int mOverrunCounter;
    int mUnderrunCounter;
};

#endif // BURGPLC_H