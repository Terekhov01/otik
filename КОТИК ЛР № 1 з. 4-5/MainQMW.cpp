#include "MainQMW.h"
#include "ui_MainQMW.h"
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QStandardItem>

MainQMW::MainQMW(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainQMW) {
    ui->setupUi(this);

    file = new QFile();
    in.setDevice(file);

    SetSignals();

    WriteKotikTableView();
}

MainQMW::~MainQMW() {
    delete ui;

    if (file->isOpen()) {
        file->close();
    }
    delete file;
}

void MainQMW::SetSignals() {
    connect(ui->openFileButton, &QPushButton::clicked, this, &MainQMW::OnFileOpenButtonClicked);
    connect(ui->kotikFormatConvertButton, &QPushButton::clicked, this, &MainQMW::OnKotikFormatConvertButtonClicked);
    connect(ui->sourceFormatButton, &QPushButton::clicked, this, &MainQMW::OnSourceFormatButtonClicked);
    connect(ui->saveButton, &QPushButton::clicked, this, &MainQMW::OnSaveButtonClicked);
}

void MainQMW::SaveFileContentStrings() {
    fileHexContent = "";

    fileContent.clear();
    int fileSize = file->size();

    ui->fileOpenProgressBar->setMinimum(0);
    ui->fileOpenProgressBar->setMaximum(fileSize);

    for (int i = 0; i < fileSize; i++) {
        QByteArray byteArray = file->read(1);
        fileContent.append(byteArray[0]);
        ui->fileOpenProgressBar->setValue(i);
    }

    // Перевод в hex строку с разделителем \t
    fileHexContent = fileContent.toHex();
    for(int i = 0; i < fileHexContent.size(); i++) {
        if(i % 3 == 0) {
            fileHexContent.insert(i, '\t');
        }
    }
    fileHexContent.remove(0, 1);

    // Вставка переносов строк
    for(int i = 0; i < fileHexContent.size(); i++) {
        // 16 + 11 + 1
        if(i % 24 == 0) {
            fileHexContent.insert(i, '\n');
            if(i != 0) {
                fileHexContent.remove(i + 1, 1);
            }
        }
    }
    fileHexContent.remove(0, 1);

    file->close();

    ui->fileOpenProgressBar->setValue(0);
}

void MainQMW::WriteKotikTableView() {
    QStandardItemModel* model = new QStandardItemModel();

    QList<QStandardItem*> itemsRow1;
    itemsRow1.append(new QStandardItem("Сигнатура"));
    itemsRow1.append(new QStandardItem("12 3C 75 00"));
    itemsRow1.append(new QStandardItem("4"));

    QList<QStandardItem*> itemsRow2;
    itemsRow2.append(new QStandardItem("Номер версии"));
    itemsRow2.append(new QStandardItem("00 00 00 01"));
    itemsRow2.append(new QStandardItem("4"));

    QList<QStandardItem*> itemsRow3;
    itemsRow3.append(new QStandardItem("Размер исходного файла в байтах"));
    itemsRow3.append(new QStandardItem("00 00 00 01"));
    itemsRow3.append(new QStandardItem("4"));

    QList<QStandardItem*> itemsRow4;
    itemsRow4.append(new QStandardItem("Защита от помех"));
    itemsRow4.append(new QStandardItem("00"));
    itemsRow4.append(new QStandardItem("1"));

    QList<QStandardItem*> itemsRow5;
    itemsRow5.append(new QStandardItem("Алгоритм сжатия с контекстом"));
    itemsRow5.append(new QStandardItem("00"));
    itemsRow5.append(new QStandardItem("1"));

    QList<QStandardItem*> itemsRow6;
    itemsRow6.append(new QStandardItem("Алгоритм сжатия без контекста"));
    itemsRow6.append(new QStandardItem("00"));
    itemsRow6.append(new QStandardItem("1"));

    QList<QStandardItem*> itemsRow7;
    itemsRow7.append(new QStandardItem("Шифрование"));
    itemsRow7.append(new QStandardItem("00"));
    itemsRow7.append(new QStandardItem("1"));

    QList<QStandardItem*> itemsRow8;
    itemsRow8.append(new QStandardItem("Окончание файла"));
    itemsRow8.append(new QStandardItem("00"));
    itemsRow8.append(new QStandardItem("1"));

    model->appendRow(itemsRow1);
    model->appendRow(itemsRow2);
    model->appendRow(itemsRow3);
    model->appendRow(itemsRow4);
    model->appendRow(itemsRow5);
    model->appendRow(itemsRow6);
    model->appendRow(itemsRow7);
    model->appendRow(itemsRow8);

    ui->kotikTableView->setModel(model);
    ui->kotikTableView->horizontalHeader()->setStretchLastSection(true);
    ui->kotikTableView->resizeRowsToContents();
}

void MainQMW::OnFileOpenButtonClicked() {
    ui->fileHexContentTextEdit->clear();
    // Получение имени файла
    fileName = QFileDialog::getOpenFileName(this, "Открытие файла", QDir::currentPath(), "All files (*.*)");

    if(fileName == "") {
        QMessageBox::critical(this, "Ошибка", "Файл не открыт");
        return;
    }

    // Открытие файла
    file->setFileName(fileName);
    file->open(QIODevice::ReadOnly);

    if(!file->isOpen()) {
        QMessageBox::critical(this, "Ошибка", "Файл не открыт");
        return;
    }

    // Запись размера файла в таблицу
    int size = file->size();
    QString sizeStr = QString::number(size, 16);

    while(sizeStr.size() < 8) {
        sizeStr = "0" + sizeStr;
    }

    for(int i = 0; i < sizeStr.size(); i++) {
        if((i + 1) % 3 == 0) {
            sizeStr.insert(i, ' ');
        }
    }
    ui->kotikTableView->model()->setData(ui->kotikTableView->model()->index(2, 1), sizeStr);

    SaveFileContentStrings();

    // Сохранение hex строки в fileHexContentTextEdit
    ui->fileHexContentTextEdit->setText(fileHexContent);

    QMessageBox::information(this, "Успех", "Файл открыт");
}

void MainQMW::OnCloseFileButtonClicked() {
    if(file->isOpen()) {
        file->close();
    }

    QMessageBox::information(this, "Успех", "Файл закрыт");
}

void MainQMW::OnSaveButtonClicked() {
    fileName = QFileDialog::getSaveFileName(this, "Открытие файла", QDir::currentPath(), "All files (*.*)");

    if(fileName == "") {
        QMessageBox::critical(this, "Ошибка", "Файл не сохранён");
        return;
    }

    file->setFileName(fileName);
    file->open(QFile::WriteOnly);

    if(!file->isOpen()) {
        QMessageBox::critical(this, "Ошибка", "Файл не сохранён");
        return;
    }

    QByteArray fileContentStr = ui->fileHexContentTextEdit->toPlainText().toLocal8Bit();

    file->write(QByteArray::fromHex(fileContentStr));

    file->close();

    QMessageBox::information(this, "Успех", "Файл сохранён");
}

void MainQMW::OnKotikFormatConvertButtonClicked() {
    QString fileContentStr = "";

    // Сигнатура
    QString data = ui->kotikTableView->model()->data(ui->kotikTableView->model()->index(0, 1)).toString();
    fileContentStr = fileContentStr + data;

    // Номер версии
    data = ui->kotikTableView->model()->data(ui->kotikTableView->model()->index(1, 1)).toString();
    fileContentStr = fileContentStr + data;

    // Размер исходного файла в байтах
    data = ui->kotikTableView->model()->data(ui->kotikTableView->model()->index(2, 1)).toString();
    fileContentStr = fileContentStr + data;

    // Защита от помех
    data = ui->kotikTableView->model()->data(ui->kotikTableView->model()->index(3, 1)).toString();
    fileContentStr = fileContentStr + data;

    // Алгоритм сжатия с контекстом
    data = ui->kotikTableView->model()->data(ui->kotikTableView->model()->index(4, 1)).toString();
    fileContentStr = fileContentStr + data;

    // Алгоритм сжатия без контекста
    data = ui->kotikTableView->model()->data(ui->kotikTableView->model()->index(5, 1)).toString();
    fileContentStr = fileContentStr + data;

    // Шифрование
    data = ui->kotikTableView->model()->data(ui->kotikTableView->model()->index(6, 1)).toString();
    fileContentStr = fileContentStr + data;

    // Запись файла
    fileContentStr = fileContentStr + ui->fileHexContentTextEdit->toPlainText();

    // Конец файла
    data = ui->kotikTableView->model()->data(ui->kotikTableView->model()->index(7, 1)).toString();
    fileContentStr = fileContentStr + data;

    // Удаление переносов строк, табуляций и пробелов
    for(int i = 0; i < fileContentStr.size(); i++) {
        if((fileContentStr.at(i) == '\n') || (fileContentStr.at(i) == '\t') || (fileContentStr.at(i) == ' ')) {
            fileContentStr = fileContentStr.remove(i, 1);
        }
    }

    // Перевод в hex строку с разделителем \t
    for(int i = 0; i < fileContentStr.size(); i++) {
        if(i % 3 == 0) {
            fileContentStr.insert(i, '\t');
        }
    }
    fileContentStr = fileContentStr.remove(0, 1);

    // Вставка переносов строк
    for(int i = 1; i < fileContentStr.size(); i++) {
        // 16 + 11 + 1
        if(i % 24 == 0) {
            fileContentStr.insert(i - 1, '\n');
            fileContentStr.remove(i, 1);
        }
    }

    ui->fileHexContentTextEdit->setText(fileContentStr);

    QMessageBox::information(this, "Успех", "Формат изменён (kotik)");
}

void MainQMW::OnSourceFormatButtonClicked() {
    QString fileContentStr = ui->fileHexContentTextEdit->toPlainText();

    fileContentStr = fileContentStr.remove(0, 48);
    fileContentStr = fileContentStr.remove(fileContentStr.size() - 3, 3);

    ui->fileHexContentTextEdit->setText(fileContentStr);

    QMessageBox::information(this, "Успех", "Формат изменён (исходный)");
}
