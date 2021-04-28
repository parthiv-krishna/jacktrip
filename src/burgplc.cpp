#include "burgplc.h"
#include <QDebug>
#include <sstream>
#include "jacktrip_globals.h"

using namespace std;
#define TWOTOTHESIXTEENTH 1024
//65536
#define OUT(x,ch,s) sampleToBits(x,ch,s)
#define PACKETSAMP ( int s = 0; s < mFPP; s++ )

BurgPLC::BurgPLC(int sample_rate, int channels, int bit_res, int FPP, int qLen, int hist) :
    RingBuffer(0, 0),
    mSampleRate (sample_rate),
    mNumChannels (channels),
    mAudioBitRes (bit_res),
    mFPP (FPP),
    mQLen (qLen),
    mHist (hist)
{
    switch (mAudioBitRes) { // int from JitterBuffer to AudioInterface enum
    case 1: mBitResolutionMode = AudioInterface::audioBitResolutionT::BIT8;
        break;
    case 2: mBitResolutionMode = AudioInterface::audioBitResolutionT::BIT16;
        break;
    case 3: mBitResolutionMode = AudioInterface::audioBitResolutionT::BIT24;
        break;
    case 4: mBitResolutionMode = AudioInterface::audioBitResolutionT::BIT32;
        break;
    }
    mTotalSize = mSampleRate * mNumChannels * mAudioBitRes * 2;  // 2 secs of audio
    mXfrBuffer   = new int8_t[mTotalSize];
    mPacketCnt = 0; // burg
#define TRAINSAMPS (mHist*mFPP)
#define ORDER (TRAINSAMPS-1)
    mTrain.resize( TRAINSAMPS, 0.0 );
    mPrediction.resize( TRAINSAMPS-1, 0.0 ); // ORDER
    mCoeffs.resize( TRAINSAMPS-2, 0.0 );
    mTruth.resize( mFPP, 0.0 );
    mTruthCh1.resize( mFPP, 0.0 );
    mXfadedPred.resize( mFPP, 0.0 );
    mNextPred.resize( mFPP, 0.0 );
    mLastGoodPacket.resize( mFPP, 0.0 );
    for ( int i = 0; i < mHist; i++ ) {
        vector<sample_t> tmp( mFPP, 0.0 );
        mLastPackets.push_back(tmp);
    }
    mFadeUp.resize( mFPP, 0.0 );
    mFadeDown.resize( mFPP, 0.0 );
    for ( int i = 0; i < mFPP; i++ ) {
        mFadeUp[i] = (double)i/(double)mFPP;
        mFadeDown[i] = 1.0 - mFadeUp[i];
    }
    mLastWasGlitch = false;
    mPhasor.resize( mNumChannels, 0.0 );
    mIncomingSeq = -1;
    mOutgoingCnt = 0;
    mLastIncomingSeq = 0;
    mOutgoingCntWraps = 0;
    mIncomingSeqWrap.resize(TWOTOTHESIXTEENTH);
    for ( int i = 0; i < TWOTOTHESIXTEENTH; i++ ) mIncomingSeqWrap[i] = -1;
    mBytes = mFPP*mNumChannels*mBitResolutionMode;
    for ( int i = 0; i < TWOTOTHESIXTEENTH; i++ ) {
        int8_t* tmp = new int8_t[mBytes];
        mIncomingDat.push_back(tmp);
    }
    mLastOutgoingSeq = 0;
    mUnderrunCtr = 0;
    mZeros = new int8_t[mTotalSize];
    for PACKETSAMP OUT(0.0, 0, s);
    for PACKETSAMP OUT(0.0, 1, s);
    memcpy(mZeros, mXfrBuffer, mBytes);
    mUnderSig = new int8_t[mTotalSize];
    for PACKETSAMP OUT(((s==0)||(s==mFPP-1))?-0.5:-0.3, 0, s);
    for PACKETSAMP OUT(-0.3, 1, s);
    memcpy(mUnderSig, mXfrBuffer, mBytes);
    mOverSig = new int8_t[mTotalSize];
    for PACKETSAMP OUT(((s==0)||(s==mFPP-1))?0.5:0.3, 0, s);
    for PACKETSAMP OUT(0.3, 1, s);
    memcpy(mOverSig, mXfrBuffer, mBytes);
    mBalance = 0;
    mTimer0.start();
    mTimer1.start();
    mTimer2.start();
    mElapsedAcc = 0.0;
    mExpectedOutgoingSeq = 0;
    mLastOutgoingCnt = 0;
    start();
}

void BurgPLC::run()
{
    setRealtimeProcessPriority();
    while (true) {
        usleep(15); // = 17 usec
        plot();
    }
}

void BurgPLC::plot()
{
        QMutexLocker locker(&mMutex); // lock the mutex here and pullPacket but not pushPacket
    double elapsed0 = (double)mTimer0.nsecsElapsed() / 1000000.0;
    //            mTimer0.start(); // histogram
    if (mLastIncomingSeq != mIncomingSeq) {
        //        fprintf(stdout,"%f\ttimer\t%d\t%d\n",elapsed,mLastIncomingSeq%10,mIncomingSeq%10); // > /tmp/xxx.dat // tail -n +18 xxx.dat | head -n -11 > xx.dat
        mLastIncomingSeq = mIncomingSeq;
        if (mOutgoingCntWraps) {
            memcpy(mIncomingDat[mIncomingSeq], mUDPbuf, mBytes);
            memcpy(mXfrBuffer, mIncomingDat[mIncomingSeq], mBytes);
            inputPacket();
        }
        double push = (double)mTimer1.nsecsElapsed() / 1000000.0;
        double pull = (double)mTimer2.nsecsElapsed() / 1000000.0;
        if(push < pull) qDebug() << "hi governator--over" << push << pull;
        mTimer1.start();
    }
    if (mLastOutgoingCnt != mOutgoingCnt) {
        mLastOutgoingCnt = mOutgoingCnt;
        double push = (double)mTimer1.nsecsElapsed() / 1000000.0;
        double pull = (double)mTimer2.nsecsElapsed() / 1000000.0;
        if(pull < push) qDebug() << "hi governator--under" << push << pull;
        //        if (mOutgoingCntWraps) fprintf(stdout,"%f\t%f\t%f\n",elapsed0,elapsed1,elapsed2); // > /tmp/xxx.dat // tail -n +18 xxx.dat | head -n -11 > xx.dat
        mTimer2.start();
    }
}

bool BurgPLC::pushPacket (const int8_t *buf, int len, int seq) {
    //    QMutexLocker locker(&mMutex); // lock the mutex
    seq %= TWOTOTHESIXTEENTH;
    mIncomingSeq = seq;
    if (!mOutgoingCntWraps)mUDPbuf=buf;
    ////    double elapsed = (double)mTimer0.nsecsElapsed() / 1000000.0;
    ////    //    if (!mOutgoingCntWraps) elapsed = 0.0;
    ////    //    else
    ////    fprintf(stdout,"%f\tpush\t%d\t%d\n",elapsed,10,10); // > /tmp/xxx.dat // tail -n +18 xxx.dat | head -n -11 > xx.dat
    ////    //        mTimer2.start();

    int nextSeq = mLastIncomingSeq+1;
    nextSeq %= TWOTOTHESIXTEENTH;
    if (mIncomingSeq != nextSeq) {
        qDebug() << "hi pushPacket LOST PACKET" << mIncomingSeq << nextSeq;
    }
    //    mLastIncomingSeq = mIncomingSeq;
    //    mIncomingSeqWrap[mIncomingSeq] = mOutgoingCntWraps;
    //    memcpy(mIncomingDat[mIncomingSeq], buf, mBytes);
    //    //    qDebug() << "hi pushPacket .........." << mLastIncomingSeq;
    //    // TODO: does not suppport mixed buf sizes
    //    mExpectedOutgoingSeq = mIncomingSeq-3;
    //    if(mExpectedOutgoingSeq<0) mExpectedOutgoingSeq+=TWOTOTHESIXTEENTH;
    usleep(50); // 25 usec @ 32FPP // 100 usec @ 128FPP
    return true;
};

void BurgPLC::pullPacket (int8_t* buf) {
        QMutexLocker locker(&mMutex); // lock the mutex
        if (!mOutgoingCntWraps)mJACKbuf=buf;

//    double elapsed = (double)mTimer1.nsecsElapsed() / 1000000.0;
    //    qDebug() << (double)mTimer.nsecsElapsed() / 1000000.0;
    if (!mOutgoingCntWraps) {
//        elapsed = 0.0;
        mExpectedOutgoingSeq = mIncomingSeq;
        mExpectedOutgoingSeq %= TWOTOTHESIXTEENTH;
    }
    else {
        //        mExpectedOutgoingSeq = mIncomingSeq;
        //        fprintf(stderr,"%f\tpull\t%d\t%d\n",elapsed,mExpectedOutgoingSeq%10,10); // > /tmp/xxx.dat // tail -n +18 xxx.dat | head -n -11 > xx.dat
        //        memcpy(mXfrBuffer, mIncomingDat[mExpectedOutgoingSeq], mBytes);
        //        memcpy(buf, mXfrBuffer, mBytes);
        //        mExpectedOutgoingSeq++;
        //        mExpectedOutgoingSeq = mIncomingSeq;
        //        mExpectedOutgoingSeq %= TWOTOTHESIXTEENTH;
    }
    //    mTimer1.start();

    //    mElapsedAcc += elapsed;
    //    if (mOutgoingCntWraps && (!mOutgoingCnt)) {
    //        fprintf(stderr,"%f\n",  mElapsedAcc / (double)TWOTOTHESIXTEENTH);
    //        mElapsedAcc = 0.0;
    //    }

    //    int delta = mLastIncomingSeq - mLastOutgoingSeq;
    //    if (lastWasOK) delta--;
    //    mLastOutgoingSeq = mLastIncomingSeq;
    //    if (delta < 0) delta+=TWOTOTHESIXTEENTH;
    //    //    if ((delta)&&(mUnderrunCtr)) qDebug() << "hi pullPacket under" <<  mUnderrunCtr;
    //    mUnderrunCtr++;
    //    if (delta) mUnderrunCtr = 0;
    //    lastWasOK = false;
    //    switch (delta) {
    //    case 0: // under
    //        memcpy(mXfrBuffer, mZeros, mBytes);
    //        inputPacket();
    //        processPacket( false ); // assume earlier have arrived
    //        //        qDebug() << "hi pullPacket under" << mUnderrunCtr;
    //        mBalance--;
    //        break;
    //    case 1: {
    //        int tmp = mLastOutgoingSeq-1;
    //        if (tmp < 0) tmp+=TWOTOTHESIXTEENTH;
    //        memcpy(mXfrBuffer, mIncomingDat[tmp], mBytes);
    //        inputPacket();
    //        processPacket( false );
    //        lastWasOK = true;
    //    }
    //        break;
    //    default: // over
    //        memcpy(mXfrBuffer, mZeros, mBytes);
    //        inputPacket();
    //        processPacket( false ); // assume earlier have arrived
    //        //        qDebug() << "hi pullPacket over" << delta-1;
    //        mBalance += (delta-1);
    //    }
    usleep(150); // 75 usec @ 32FPP // 300 usec @ 128FPP
    processPacket( false );
    memcpy(mJACKbuf, mXfrBuffer, mBytes);
    ////    if (!mOutgoingCntWraps) mBalance = 0;
    ////    else fprintf(stdout,"%d\n",mBalance); // > /tmp/xxx.dat // tail -n +18 xxx.dat | head -n -11 > xx.dat

    mOutgoingCnt++;
    mOutgoingCnt %= TWOTOTHESIXTEENTH;
    if (!mOutgoingCnt) mOutgoingCntWraps++;
};

#define RUN -1

void BurgPLC::inputPacket ()
{
#define INCh0(x,s) mTruth[s] = x
#define INCh1(x,s) mTruthCh1[s] = x
    for PACKETSAMP {
        INCh0(bitsToSample(0, s), s);
        INCh1(bitsToSample(1, s), s);
    }
    //        for PACKETSAMP { // screw case here but not for sim
    //            INCh0(0.3*sin(mPhasor[0]), s);
    //            INCh1(0.3*sin(mPhasor[1]), s);
    //            mPhasor[0] += 0.1;
    //            mPhasor[1] += 0.11;
    //        }
    if(mPacketCnt) {
        if(RUN > 2) {

            for ( int i = 0; i < mHist; i++ ) {
                for PACKETSAMP mTrain[s+((mHist-(i+1))*mFPP)] =
                        mLastPackets[i][s];
            }
        }
    }
}

void BurgPLC::processPacket (bool glitch)
{
    //    QMutexLocker locker(&mMutex);  // lock the mutex
    // debugSequenceDelta = bytesToInt(mXfrBuffer) - debugSequenceNumber;

    //    if (debugSequenceNumber != bytesToInt(mXfrBuffer))
    //        qDebug() << "expected" << debugSequenceNumber << "got " << bytesToInt(mXfrBuffer);

    //if(!overrun)
    //    debugSequenceNumber = bytesToInt(mXfrBuffer)+1;
    //    if (debugSequenceDelta != 1) qDebug() << debugSequenceNumber << "\t" << debugSequenceDelta;


    //    glitch = !(mPacketCnt%100);
    if(mPacketCnt) {
        if(RUN > 2) {
            //            if (glitch) qDebug() << "glitch";

            // GET LINEAR PREDICTION COEFFICIENTS
            ba.train( mCoeffs, mTrain, mPacketCnt );

            // LINEAR PREDICT DATA
            vector<sample_t> tail( mTrain );

            ba.predict( mCoeffs, tail ); // resizes to TRAINSAMPS-2 + TRAINSAMPS

            for ( int i = 0; i < ORDER; i++ )
                mPrediction[i] = tail[i+TRAINSAMPS];
            ///////////////////////////////////////////// // CALCULATE AND DISPLAY ERROR

            for PACKETSAMP mXfadedPred[s] = mTruth[s] * mFadeUp[s] + mNextPred[s] * mFadeDown[s];
        }
        for PACKETSAMP {
            switch(RUN)
            {
            case -1  :
                OUT(0.3*sin(mPhasor[0]), 0, s);
                OUT(0.3*sin(mPhasor[1]), 1, s);
                mPhasor[0] += 0.1;
                mPhasor[1] += 0.11;
                break;
            case 0  :
                OUT(mTruth[s], 0, s);
                OUT(mTruthCh1[s], 1, s);
                break;
            case 1  : OUT((glitch) ?
                              mLastGoodPacket[s] : mTruth[s], 0, s);
                OUT(mTruthCh1[s], 1, s);
                break;
            case 2  : {
                double tmp = (glitch) ? 0.0 : mTruth[s];
                OUT(tmp, 0, s);
                OUT(mTruthCh1[s], 1, s);
            }
                break;
            case 3  :
                OUT((glitch) ? mPrediction[s] : ( (mLastWasGlitch) ?
                                                      mXfadedPred[ s ] : mTruth[s] ), 0, s);
                OUT(mTruthCh1[s], 1, s);
                break;
            case 4  :
                OUT((glitch) ? mPrediction[s] : mTruth[s], 0, s);
                break;
            case 5  : OUT(mPrediction[s], 0, s);
                break;
            case 6  :
                OUT(0.3*sin(mPhasor[0]), 0, s);
                OUT(0.3*sin(mPhasor[1]), 1, s);
                mPhasor[0] += 0.1;
                mPhasor[1] += 0.11;
                break;
            }
        }
        mLastWasGlitch = glitch;
        for PACKETSAMP
                mNextPred[s] = mPrediction[ s + mFPP];
    }

    // if mPacketCnt==0 initialization follows
    for ( int i = mHist-1; i>0; i-- ) {
        for PACKETSAMP mLastPackets[i][s] = mLastPackets[i-1][s];
    }

    // will only be able to glitch if mPacketCnt>0
    for PACKETSAMP mLastPackets[0][s] =
            ((!glitch)||(mPacketCnt<mHist)) ? mTruth[s] : mPrediction[s];

    if (!glitch)
        for PACKETSAMP mLastGoodPacket[s] = mTruth[s];
    mPacketCnt++;
}


// copped from AudioInterface.cpp

sample_t BurgPLC::bitsToSample(int ch, int frame) {
    sample_t sample = 0.0;
    AudioInterface::fromBitToSampleConversion(
                &mXfrBuffer[(frame * mBitResolutionMode * mNumChannels)
            + (ch * mBitResolutionMode)],
            &sample, mBitResolutionMode);
    return sample;
}

void BurgPLC::sampleToBits(sample_t sample, int ch, int frame) {
    AudioInterface::fromSampleToBitConversion(
                &sample,
                &mXfrBuffer[(frame * mBitResolutionMode * mNumChannels)
            + (ch * mBitResolutionMode)],
            mBitResolutionMode);
}

QString BurgPLC::qStringFromLongDouble(const long double myLongDouble)
{
    stringstream ss;
    ss << myLongDouble;
    return QString::fromStdString(ss.str());
}

// returns the first stereo frame of a buffer as an int32
int BurgPLC::bytesToInt(const int8_t *buf)
{
    int output = 0;
    memcpy(&output, buf, sizeof(int));  // 4 bytes
    return output;
}
