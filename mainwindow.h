#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <random>
#include <string>


#include <QMediaDevices>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QStyle>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_button_play_pressed();

    void on_button_shuffle_pressed();

    void on_button_backward_pressed();

    void on_button_forward_pressed();

    void on_button_download_pressed();

private:
    Ui::MainWindow *ui;
    QList<QString> songs;
    QMediaPlayer *player = new QMediaPlayer();
    std::default_random_engine gen;

    void next_song();
    void update_text();
    void update_songs(const std::string PATH);
    std::string get_qrc(const std::string PATH);
};
#endif // MAINWINDOW_H
