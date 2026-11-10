#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMessageBox>
QT_BEGIN_NAMESPACE
class QComboBox;
class QLineEdit;
class QPushButton;
class QTextEdit;
class QLabel;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onTranslate();
    void onProviderChanged(const QString &);
    void onReplyFinished(QNetworkReply *);

private:
    void setupUi();
    void setupConnections();
    void buildPrompt(QString &prompt);
    void sendRequest(const QString &prompt);
    void showError(const QString &msg);

    // UI widgets
    QTextEdit *sourceEdit;
    QTextEdit *targetEdit;
    QComboBox *sourceLangCombo;
    QComboBox *targetLangCombo;
    QComboBox *providerCombo;
    QLineEdit *localUrlEdit;
    QLineEdit *apiKeyEdit;
    QPushButton *translateButton;
    QLabel *statusLabel;

    QNetworkAccessManager *netManager;
    enum class Provider { Local, Google };
    Provider m_currentProvider = Provider::Local;
};

#endif // MAINWINDOW_H
