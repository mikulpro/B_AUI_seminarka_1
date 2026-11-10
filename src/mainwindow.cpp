#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonParseError>
#include <QGridLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), netManager(new QNetworkAccessManager(this))
{
    setWindowTitle("Code Translator – Qt 6");
    resize(900, 700);
    setupUi();
    setupConnections();
}

void MainWindow::setupUi()
{
    auto central = new QWidget(this);
    auto mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(10,10,10,10);
    mainLayout->setSpacing(10);

    // Language selection
    auto langGroup = new QGroupBox("Languages");
    auto langLayout = new QHBoxLayout(langGroup);
    langLayout->setSpacing(10);
    sourceLangCombo = new QComboBox;
    targetLangCombo = new QComboBox;
    auto langs = QStringList{
        "C++", "Python", "JavaScript", "Java", "C#", "Go", "Rust", "TypeScript", "Ruby"
    };
    sourceLangCombo->addItems(langs);
    targetLangCombo->addItems(langs);
    sourceLangCombo->setCurrentText("C++");
    targetLangCombo->setCurrentText("Python");
    langLayout->addWidget(new QLabel("Source:"));
    langLayout->addWidget(sourceLangCombo);
    langLayout->addSpacing(20);
    langLayout->addWidget(new QLabel("Target:"));
    langLayout->addWidget(targetLangCombo);
    mainLayout->addWidget(langGroup);

    // Text areas
    auto editGroup = new QGroupBox("Code");
    auto editLayout = new QVBoxLayout(editGroup);
    editLayout->setSpacing(10);
    sourceEdit = new QTextEdit;
    targetEdit = new QTextEdit;
    targetEdit->setReadOnly(true);
    sourceEdit->setPlaceholderText("Enter source code here…");
    targetEdit->setPlaceholderText("Translated code will appear here…");
    editLayout->addWidget(new QLabel("Source Code:"));
    editLayout->addWidget(sourceEdit, 1);
    editLayout->addWidget(new QLabel("Translated Code:"));
    editLayout->addWidget(targetEdit, 1);
    mainLayout->addWidget(editGroup);

    // Provider settings
    auto providerGroup = new QGroupBox("Provider");
    auto providerLayout = new QGridLayout(providerGroup);
    providerLayout->setSpacing(8);

    providerCombo = new QComboBox;
    providerCombo->addItems(QStringList{
        "Local (gtp-oss:20b)",
        "Google AI Studio"
    });

    localUrlEdit = new QLineEdit;
    localUrlEdit->setPlaceholderText("e.g. http://localhost:8080/v1/chat/completions");
    localUrlEdit->setText("http://localhost:8080/v1/chat/completions");

    apiKeyEdit = new QLineEdit;
    apiKeyEdit->setPlaceholderText("Google API Key");
    apiKeyEdit->setEchoMode(QLineEdit::Password);

    providerLayout->addWidget(new QLabel("Select provider:"), 0, 0);
    providerLayout->addWidget(providerCombo, 0, 1, 1, 2);
    providerLayout->addWidget(new QLabel("Local URL:"), 1, 0);
    providerLayout->addWidget(localUrlEdit, 1, 1, 1, 2);
    providerLayout->addWidget(new QLabel("API Key:"), 2, 0);
    providerLayout->addWidget(apiKeyEdit, 2, 1, 1, 2);
    mainLayout->addWidget(providerGroup);

    // Translate button
    translateButton = new QPushButton("Translate");
    translateButton->setMinimumHeight(48);
    mainLayout->addWidget(translateButton);

    // Status bar
    statusLabel = new QLabel;
    statusLabel->setText("");
    statusLabel->setStyleSheet("color: #555;");
    mainLayout->addWidget(statusLabel);

    setCentralWidget(central);

    // Initial enable/disable
    onProviderChanged(providerCombo->currentText());
}

void MainWindow::setupConnections()
{
    connect(translateButton, &QPushButton::clicked,
            this, &MainWindow::onTranslate);
    connect(providerCombo, &QComboBox::currentTextChanged,
            this, &MainWindow::onProviderChanged);
    connect(netManager, &QNetworkAccessManager::finished,
            this, &MainWindow::onReplyFinished);
}

void MainWindow::onProviderChanged(const QString &text)
{
    bool isLocal = text.startsWith("Local");
    localUrlEdit->setEnabled(isLocal);
    apiKeyEdit->setEnabled(!isLocal);
}

void MainWindow::onTranslate()
{
    const QString sourceCode = sourceEdit->toPlainText();
    if (sourceCode.isEmpty()) {
        QMessageBox::warning(this, "Empty source", "Please enter code to translate.");
        return;
    }

    translateButton->setEnabled(false);
    statusLabel->setText("Translating…");

    QString prompt;
    buildPrompt(prompt);

    sendRequest(prompt);
}

void MainWindow::buildPrompt(QString &prompt)
{
    const QString srcLang = sourceLangCombo->currentText();
    const QString tgtLang = targetLangCombo->currentText();
    const QString sourceCode = sourceEdit->toPlainText();

    prompt = QStringLiteral(
        "Translate the following code from %1 to %2:\n\n%3")
        .arg(srcLang, tgtLang, sourceCode);
}

void MainWindow::sendRequest(const QString &prompt)
{
    m_currentProvider = providerCombo->currentText().startsWith("Local")
                        ? Provider::Local : Provider::Google;

    QNetworkRequest request;
    QJsonObject body;

    if (m_currentProvider == Provider::Local) {
        const QUrl url(localUrlEdit->text());
        if (!url.isValid()) {
            showError("Invalid local URL.");
            return;
        }
        request.setUrl(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          "application/json");

        body["model"] = "gtp-oss:20b";
        QJsonArray msgs;
        QJsonObject userMsg;
        userMsg["role"] = "user";
        userMsg["content"] = prompt;
        msgs.append(userMsg);
        body["messages"] = msgs;
        body["temperature"] = 0.0;
    } else { // Google AI Studio
        const QString apiKey = apiKeyEdit->text().trimmed();
        if (apiKey.isEmpty()) {
            showError("Google API key is required.");
            return;
        }
        const QString model = "text-bison-001";
        const QUrl url(QString("https://generativelanguage.googleapis.com/v1beta/models/%1:generateMessage?key=%2")
                        .arg(model, apiKey));
        request.setUrl(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          "application/json");

        QJsonObject contentsObj;
        QJsonArray partsArr;
        QJsonObject partObj;
        partObj["text"] = prompt;
        partsArr.append(partObj);
        contentsObj["parts"] = partsArr;

        QJsonArray contentsArr;
        contentsArr.append(contentsObj);
        body["contents"] = contentsArr;

        QJsonObject genConf;
        genConf["temperature"] = 0.0;
        genConf["topP"] = 1;
        genConf["topK"] = 64;
        body["generationConfig"] = genConf;
    }

    const QByteArray json = QJsonDocument(body).toJson(QJsonDocument::Compact);
    netManager->post(request, json);
}

void MainWindow::onReplyFinished(QNetworkReply *reply)
{
    translateButton->setEnabled(true);
    statusLabel->setText("");

    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status != 200) {
        QString err = QString("HTTP %1 error: %2")
                      .arg(status)
                      .arg(reply->readAll());
        showError(err);
        reply->deleteLater();
        return;
    }

    const QByteArray data = reply->readAll();
    QJsonParseError parseErr;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &parseErr);
    if (parseErr.error != QJsonParseError::NoError) {
        showError("Failed to parse JSON response.");
        reply->deleteLater();
        return;
    }

    const QJsonObject json = doc.object();
    QString translated;

    if (m_currentProvider == Provider::Local) {
        if (json.contains("choices")) {
            const QJsonArray choices = json["choices"].toArray();
            if (!choices.isEmpty()) {
                const QJsonObject choice = choices[0].toObject();
                const QJsonObject msg = choice["message"].toObject();
                translated = msg["content"].toString();
            }
        }
    } else {
        if (json.contains("candidates")) {
            const QJsonArray cand = json["candidates"].toArray();
            if (!cand.isEmpty()) {
                const QJsonObject candObj = cand[0].toObject();
                const QJsonObject contentObj = candObj["content"].toObject();
                const QJsonArray parts = contentObj["parts"].toArray();
                if (!parts.isEmpty()) {
                    const QJsonObject partObj = parts[0].toObject();
                    translated = partObj["text"].toString();
                }
            }
        }
    }

    if (translated.isEmpty()) {
        showError("No translation found in the response.");
    } else {
        targetEdit->setPlainText(translated);
    }

    reply->deleteLater();
}

void MainWindow::showError(const QString &msg)
{
    QMessageBox::critical(this, "Error", msg);
    statusLabel->setText(msg);
}
