#ifndef SOUND_HPP
#define SOUND_HPP

#include <QMediaPlayer>
#include <QAudioProbe>
#include <QAudioBuffer>
#include <QDebug>
#include <QUrl>
#include <QDir>

class Sound : public QObject
{
    Q_OBJECT

public:

    QString name;
    bool isDetected;

    QMediaPlayer *player;
    QAudioProbe *audioProbe;

    float leftSpectrum;
    float rightSpectrum;

    explicit Sound( QString name = "", int folderIndex = 0, QObject *parent = 0 );

private slots:

    void slot_calculateLevel( QAudioBuffer buffer );
};

#endif // SOUND_HPP

