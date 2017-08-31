#include "sound.hpp"
#include <QDebug>

Sound::Sound( QString name, int folderIndex, QObject *parent ) : QObject( parent ),
                                                                 name( name ),
                                                                 isDetected( false ),
                                                                 player( new QMediaPlayer( this ) ),
                                                                 audioProbe( new QAudioProbe( this ) ),
                                                                 leftSpectrum( 0 ),
                                                                 rightSpectrum( 0 )
{
    QString localFile = QDir::currentPath() + "/../sounds/track_" + QString::number( folderIndex ) + "/" + name;
    qDebug() << "Constructor de Sound" << localFile;

    player->setMedia( QUrl::fromLocalFile( localFile ) );
    player->setVolume( 0 );
    audioProbe->setSource( player );

    connect( audioProbe, SIGNAL( audioBufferProbed( QAudioBuffer ) ), SLOT( slot_calculateLevel( QAudioBuffer ) ) );
}

void Sound::slot_calculateLevel( QAudioBuffer buffer )
{
    qreal peakValue;

    if( buffer.frameCount() < 512 || buffer.format().channelCount() != 2 )
    {
        return;
    }

    double leftLevel = 0, rightLevel = 0;

    QVector< double > sample;
    sample.resize( buffer.frameCount() );

    if( buffer.format().sampleType() == QAudioFormat::SignedInt )
    {
        QAudioBuffer::S16S *data = buffer.data< QAudioBuffer::S16S >();

        if ( buffer.format().sampleSize() == 32 ) peakValue = INT_MAX;
        else if ( buffer.format().sampleSize() == 16 ) peakValue = SHRT_MAX;
        else peakValue = CHAR_MAX;

        for( int i = 0; i < buffer.frameCount(); i++ )
        {
            sample[i] = data[i].left / peakValue;
            leftLevel += abs( data[i].left ) / peakValue;
            rightLevel += abs( data[i].right ) / peakValue;
        }
    }

    leftSpectrum = leftLevel / ( float )buffer.frameCount();
    rightSpectrum = rightLevel / ( float )buffer.frameCount();
}
