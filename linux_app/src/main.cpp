#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <vector>
#include <string>
#include <memory>
#include <iostream>

#include "LinuxPlayer.h"
#include "PulseAudioSource.h"
#include "OutputComponent.h"
#include "OutputFormat.h"
#include "OutputMetadata.h"
#include "DeviceManager.h" 

class AirplayApp : public QApplication {
public:
    AirplayApp(int &argc, char **argv) : QApplication(argc, argv) {
        setQuitOnLastWindowClosed(false);
    }

    void init() {
        setupTrayIcon();
        startBackend();
    }

private:
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QMenu *devicesMenu;
    
    std::unique_ptr<LinuxPlayer> player;
    std::unique_ptr<OutputComponent> output;
    std::unique_ptr<PulseAudioSource> audioSource;
    
    QMutex audioMutex;

    void setupTrayIcon() {
        trayIcon = new QSystemTrayIcon(this);
        // Use a standard icon
        if (QIcon::hasThemeIcon("audio-card")) {
            trayIcon->setIcon(QIcon::fromTheme("audio-card"));
        } else {
            QPixmap pixmap(16, 16);
            pixmap.fill(Qt::blue);
            trayIcon->setIcon(QIcon(pixmap));
        }

        trayMenu = new QMenu();
        
        devicesMenu = trayMenu->addMenu("Devices (Auto-Connect)");
        devicesMenu->setEnabled(false); 

        trayMenu->addSeparator();
        
        QAction *quitAction = new QAction("Quit", this);
        connect(quitAction, &QAction::triggered, this, &QApplication::quit);
        trayMenu->addAction(quitAction);

        trayIcon->setContextMenu(trayMenu);
        trayIcon->show();
        
        trayIcon->setToolTip("AirPlay Free (Linux)");
    }

    void startBackend() {
        try {
            player = std::make_unique<LinuxPlayer>();
            output = std::make_unique<OutputComponent>(*player);
            
            // Configure Output Format (CD Quality)
            OutputFormat fmt(SampleRate(44100), SampleSize(16), ChannelCount(2));
            output->open(fmt);
            
            // Start PulseAudio Capture
            audioSource = std::make_unique<PulseAudioSource>();
            bool started = audioSource->start([this](const uint8_t* data, size_t size) {
                QMutexLocker locker(&audioMutex);
                if (output) {
                    output->write(data, size);
                }
            });

            if (!started) {
                QMessageBox::critical(nullptr, "Error", "Failed to start PulseAudio capture.");
            } else {
                std::cout << "Backend started. Listening for AirPlay devices..." << std::endl;
            }

        } catch (const std::exception& e) {
            QMessageBox::critical(nullptr, "Error", QString("Backend initialization failed: %1").arg(e.what()));
        }
    }
};

int main(int argc, char *argv[]) {
    AirplayApp app(argc, argv);
    app.init();
    return app.exec();
}
