#ifndef MAINQMW_H
#define MAINQMW_H

#include <QMainWindow>
#include <QFile>
#include <QDataStream>
#include <QByteArray>

QT_BEGIN_NAMESPACE
namespace Ui { class MainQMW; }
QT_END_NAMESPACE

class MainQMW : public QMainWindow
{
    Q_OBJECT

public:
    MainQMW(QWidget *parent = nullptr);
    ~MainQMW();

private:
    Ui::MainQMW *ui;
    QFile* file;
    QDataStream in;
    QByteArray fileContent;
    QString fileName;

    QString fileHexContent;
    QString fileIntContent;

    void SetSignals();

    void SaveFileContentStrings();
    void WriteKotikTableView();

private slots:
    void OnFileOpenButtonClicked();    
    void OnCloseFileButtonClicked();
    void OnSaveButtonClicked();
    void OnKotikFormatConvertButtonClicked();
    void OnSourceFormatButtonClicked();
};
#endif // MAINQMW_H
