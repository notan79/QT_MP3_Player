#include <random>
#include <iostream>
#include <cstdlib>
#include <filesystem>
#include <thread>
//#include <fstream>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMediaDevices>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QStyle>
#include <QFileDialog>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QAudioOutput *audio_output = new QAudioOutput();
    player->setAudioOutput(audio_output);


    ui->button_play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->button_shuffle->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    ui->button_backward->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    ui->button_forward->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));


    const std::string PATH = "/Users/nc/song_downloads";
    update_songs(PATH);

    if(songs.length() == 0)
    {
        ui->label->setText("Download a song before playing");
        ui->horizontalSlider->setEnabled(false);
    }

    else{
        player->setSource(QUrl(QString::fromStdString(PATH+"/")+songs[0]));
        update_text();
    }
    connect(player, &QMediaPlayer::durationChanged, ui->horizontalSlider, &QSlider::setMaximum);
    connect(player, &QMediaPlayer::positionChanged, ui->horizontalSlider, &QSlider::setValue);
    connect(ui->horizontalSlider, &QSlider::sliderMoved, this, [=](){
        player->setPosition(ui->horizontalSlider->sliderPosition());
        if(ui->horizontalSlider->sliderPosition() == ui->horizontalSlider->maximum())
        {
            ui->horizontalSlider->setEnabled(false);
            this->next_song();
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            ui->horizontalSlider->setValue(0);
            ui->horizontalSlider->setEnabled(true);
            player->play();
        }
    });

    connect(player, &QMediaPlayer::mediaStatusChanged, this, [=](){
        if(player->mediaStatus() == QMediaPlayer::EndOfMedia){
            this->next_song();
            player->play();
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_button_play_pressed()
{
    if(songs.length() == 0)
        return;
    if(player->isPlaying()){
        player->pause();
        ui->button_play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    }else{
        player->play();
        ui->button_play->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    }

}


// shuffle button
void MainWindow::on_button_shuffle_pressed()
{
    if(songs.length() == 0)
        return;

    player->stop();
    ui->button_play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

    QList<QString> arr;

    unsigned songs_size = songs.size();
    for(unsigned i = 0; i<songs_size-1; ++i){
        std::uniform_int_distribution<int> distribution(0, songs.size()-1-i);
        unsigned index = distribution(gen);
        arr.prepend(songs[index]);
        songs.remove(index);
    }
    arr.append(songs[0]);
    songs = arr;

    const QString PATH = "/Users/nc/song_downloads/";
    player->setSource(QUrl(PATH+songs[0]));
    update_text();
}

void MainWindow::next_song()
{
    QString cur = songs[0];
    songs.remove(0);
    songs.append(cur);

    const QString PATH = "/Users/nc/song_downloads/";
    player->setSource(QUrl(PATH+songs[0]));

    update_text();
}

void MainWindow::update_text()
{
    QString text;
    if(songs.length() == 0)
        return;
    else if(songs.length() == 1)
        text = "Current Song: " + songs[0] +"\nNext Song: " + songs[0];
    else
        text = "Current Song: " + songs[0] +"\nNext Song: " + songs[1];
    ui->label->setText(text);
}


void MainWindow::on_button_backward_pressed()
{
    if(songs.length() == 0)
        return;
    bool wasPlaying = player->isPlaying();
    QString last = songs.last();
    songs.removeLast();
    songs.prepend(last);

    const QString PATH = "/Users/nc/song_downloads/";
    player->setSource(QUrl(PATH+songs[0]));
    if(wasPlaying)
        player->play();
    update_text();
}


void MainWindow::on_button_forward_pressed()
{
    if(songs.length() == 0)
        return;
    bool wasPlaying = player->isPlaying();
    next_song();
    if(wasPlaying)
        player->play();
}

// bool MainWindow::youtube_download(std::string url){
//     const std::string PATH = "/qrc:/sounds/";
//     const std::string COMMAND = $ yt-dlp "https://www.youtube.com/watch?v=taWjtvyhWOw" -xciw -f "bestaudio/best" --audio-format mp3 --ffmpeg-location "audio-orchestrator-ffmpeg/bin" -P "song_downloads"
//     return system(COMMAND.c_str());
// }


void MainWindow::on_button_download_pressed()
{
    const std::string YT_URL = ui->text_url->toPlainText().toStdString();
    const std::string PATH = "/Users/nc/song_downloads";
    const std::string COMMAND = "/Users/nc/anaconda3/bin/yt-dlp \"" + YT_URL + "\" -xciw -f \"bestaudio/best\" --audio-format mp3 --ffmpeg-location \"/Users/nc/audio-orchestrator-ffmpeg/bin\" -P \"" + PATH + "\"";
    system(COMMAND.c_str());
    update_songs(PATH);

    // this stuff was for updating a resource folder//

    //std::string qrc = get_qrc(PATH);
    // const std::string res = "/Users/nc/QT_Projects/MP3_Player/res.qrc";
    // std::ofstream file_write(res, std::ios::trunc);
    // file_write << qrc;
    // file_write.close();

}

void MainWindow::update_songs(const std::string PATH)
{


    QList<QString> temp;
    std::string cur = "";
    if(songs.length() != 0)
        std::string cur = songs[0].toStdString();

    for (const auto & entry : std::filesystem::directory_iterator(PATH)){
        std::string str = entry.path();
        str.erase(0, PATH.length()+1); // removes the path for the string
        std::cout << str;
        std::string extension = str;
        extension.erase(0,extension.length()-3); // gets the file type

        if(extension == "mp3" && str != cur) // checks if the ending is mp3
        {
            temp.prepend(QString::fromStdString(str));
        }
    }
    if(cur != "")
        temp.prepend(QString::fromStdString(cur));

    songs = temp;
    if(songs.length() != 0)
        ui->horizontalSlider->setEnabled(true);
    update_text();
}


std::string MainWindow::get_qrc(const std::string PATH)
{
/*
    Method to return a string representation of a qrc resource file. Also
    updates the 'songs' instance variable to have all the mp3 files.

    Parameters: 'const std::string PATH': where to search for files

    Returns: 'std::string text': representation of the qrc structure
*/

    QList<QString> temp;
    std::string text;
    for (const auto & entry : std::filesystem::directory_iterator(PATH)){
        std::string str = entry.path();
        str.erase(0, PATH.length()+1); // removes the path for the string

        std::string extension = str;
        extension.erase(0,extension.length()-3); // gets the file type

        if(extension == "mp3") // checks if the ending is mp3
        {
            text += "        <file>" + str + "</file>\n";
            temp.prepend(QString::fromStdString(str));
        }
    }

    text = "<RCC>\n    <qresource prefix=\"/sounds\">\n" + text + "    </qresource>\n</RCC>"; // formatting for qrc

    songs = temp;
    return text;
}

// // Prints out the strings of the names in the songs directory
// for (const auto & entry : std::filesystem::directory_iterator(PATH)){
//     str = entry.path();
//     std::cout << str.erase(0,PATH.length()+1) << std::endl;
// }
