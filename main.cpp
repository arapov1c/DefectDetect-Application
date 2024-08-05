#include <opencv2/opencv.hpp>
#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QWidget>
#include <QToolButton>
#include <QObject>
#include <QEvent>
#include <QLineEdit>
#include <QIntValidator>
#include <QCheckBox>
#include <QString>
#include <QDir>
#include <QInputDialog>
#include <QFileInfo>
#include<iostream>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>

//globalne varijable koje se koriste u nekoliko funkcija, deklarisane ovako radi lakše manipulacije između funkcija
cv::Mat slika, originalnaSlika;
cv::Scalar boja;
cv::Point prethodnaTacka(-1, -1);
bool crtanjeAktivno = false;
int velicinaMarkera = 20;
int trenutniMarker = 1;
std::vector<int> dimenzije{4};
std::vector<int> indeksi;
std::vector<int> spasi;
std::vector<cv::Mat> sveSlike;
std::vector<std::vector<QCheckBox*>> checkboxes;
std::vector<QString> naziviUAplikaciji{10};
QString putanja;
QString putanjaSpasene;
std::vector<int> ocjene_d1, ocjene_d2, ocjene_d3, ocjene_d4, ocjene_d5, ocjene_rub, ocjene_podloga, ocjene_ispravno, ocjene_koza;
std::vector<cv::Mat> patchevi, patchevi_d1, patchevi_d2, patchevi_d3, patchevi_d4, patchevi_d5, patchevi_rub, patchevi_podloga, patchevi_ispravno, patchevi_koza;
QString imeSlike;

void readConfig(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file for reading:" << fileName;
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith("#") || line.trimmed().isEmpty()) {
            continue; // Preskoči komentare i prazne linije
        }

        QStringList parts = line.split("=");
        if (parts.size() != 2) {
            continue; // Preskoči neispravne linije
        }

        QString key = parts.at(0).trimmed();
        QString value = parts.at(1).trimmed();

        if (key == "velicinaMarkera") {
            velicinaMarkera = value.toInt();
        }
        else if(key == "x"){
            dimenzije[0] = value.toInt();
        }
        else if(key == "y"){
            dimenzije[1] = value.toInt();
        }
        else if(key == "sx"){
            dimenzije[2] = value.toInt();
        }
        else if(key == "sy"){
            dimenzije[3] = value.toInt();
        }
        else if(key=="nazivMarkera1"){
            naziviUAplikaciji[0] = value;
        }
        else if(key=="nazivMarkera2"){
            naziviUAplikaciji[1] = value;
        }
        else if(key=="nazivMarkera3"){
            naziviUAplikaciji[2] = value;
        }
        else if(key=="nazivMarkera4"){
            naziviUAplikaciji[3] = value;
        }
        else if(key=="nazivMarkera5"){
            naziviUAplikaciji[4] = value;
        }
        else if(key=="nazivGumice"){
            naziviUAplikaciji[5] = value;
        }
        else if(key=="nazivRuba"){
            naziviUAplikaciji[6] = value;
        }
        else if(key=="nazivPodloge"){
            naziviUAplikaciji[7] = value;
        }
        else if(key=="nazivKlaseIspravno"){
            naziviUAplikaciji[8] = value;
        }
        else if(key=="nazivKlaseKoza"){
            naziviUAplikaciji[9] = value;
        }

    }

    file.close();
}

/*void writeConfig(const QString &fileName, const QString &username, int windowWidth, int windowHeight) {
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file for writing:" << fileName;
        return;
    }

    QTextStream out(&file);
    out << "username=" << username << "\n";
    out << "windowWidth=" << windowWidth << "\n";
    out << "windowHeight=" << windowHeight << "\n";

    file.close();
}*/


void spasiSlike(std::vector<cv::Mat>& slike, QWidget* window2, QWidget* window3) {
    QFileInfo fileInfo(putanja);
    imeSlike = fileInfo.baseName();
    std::vector<std::string> naziviFoldera = {"Original", "Maske", "Maske", "Maske", "Maske", "Maske", "Maske", "Maske", "Maske", "Maske", "Anotacija"};
    std::vector<std::string> naziviSlika = {imeSlike.toStdString(), naziviUAplikaciji[0].toStdString(), naziviUAplikaciji[1].toStdString(),
                                            naziviUAplikaciji[2].toStdString(), naziviUAplikaciji[3].toStdString(), naziviUAplikaciji[4].toStdString(),
                                            naziviUAplikaciji[6].toStdString(), naziviUAplikaciji[7].toStdString(), naziviUAplikaciji[8].toStdString(), naziviUAplikaciji[9].toStdString(), "Anotacija_"+imeSlike.toStdString()};

    // Odabir glavnog foldera
    QString glavniFolder = QFileDialog::getExistingDirectory(window2, "Odaberi glavni direktorij za spašavanje", QDir::currentPath());
    if (glavniFolder.isEmpty()) {
        return;  // Ako korisnik nije odabrao glavni folder, prekidamo postupak
    }

    QString rezultatiFolder = glavniFolder + "/Rezultati_" + imeSlike;
    QDir dir(rezultatiFolder);
    if (!dir.exists()) {
        dir.mkpath(rezultatiFolder);
    }

    for (int i = 0; i < slike.size(); i++) {
        cv::Mat& slika = slike[i];  // Referenca na sliku iz vektora
        if (!slika.empty()) {
            std::string nazivSlike = naziviSlika[i];
            QString folderPath = rezultatiFolder + "/" + QString::fromStdString(naziviFoldera[i]);

            // Provjera postojanja podfoldera
            QDir poddir(folderPath);
            if (!poddir.exists()) {
                poddir.mkpath(folderPath);  // Kreiranje podfoldera ako ne postoji
            }

            putanjaSpasene = folderPath + "/" + QString::fromStdString(nazivSlike) + ".bmp";  // Spašavanje u BMP formatu
            cv::imwrite(putanjaSpasene.toStdString(), slika);
        }
    }
    sveSlike.clear();
    sveSlike.push_back(originalnaSlika);
    window3->show(); //Prikaz prozora da je slika spašena
}

void kopirajDioSlike(const cv::Mat& original, cv::Mat& trenutna, int x, int y, int velicina) {
    cv::Rect region(x - velicina / 2, y - velicina / 2, velicina, velicina);
    cv::Mat dioSlike = original(region);
    dioSlike.copyTo(trenutna(region));
}

void onMouseClick(int event, int x, int y, int flags, void* userData) {
    cv::Mat* trenutnaSlika = static_cast<cv::Mat*>(userData);
    if (!trenutnaSlika) {
        std::cerr << "Greška: Nedostupna trenutna slika!" << std::endl;
        return;
    }


    if (event == (flags & cv::EVENT_FLAG_ALTKEY) && crtanjeAktivno) {
        prethodnaTacka = cv::Point(x, y);
    } else if (event == cv::EVENT_MOUSEMOVE && (flags & cv::EVENT_FLAG_ALTKEY) && crtanjeAktivno) {
        cv::line(*trenutnaSlika, prethodnaTacka, cv::Point(x, y), boja, velicinaMarkera);
        prethodnaTacka = cv::Point(x, y);
        cv::imshow("Glavni pregled", *trenutnaSlika);
    } else if (event == cv::EVENT_MOUSEMOVE && (flags & cv::EVENT_FLAG_ALTKEY) && !crtanjeAktivno) { //Implementacija gumice tako što se kopira original preko
        kopirajDioSlike(originalnaSlika, *trenutnaSlika, x, y, velicinaMarkera);                       //anotirane slike
        cv::imshow("Glavni pregled", *trenutnaSlika);
    }
}


void onButtonClick(QWidget* window, QWidget* window2, int odabir) {
    if(odabir==1){
        QString filePath = QFileDialog::getOpenFileName(window, "Odaberi sliku", "", "Slike (*.png *.jpg *.jpeg *.bmp)");
        putanja = filePath;

        if (!filePath.isEmpty()) {
            sveSlike.clear();
            // Stvaranje novog prozora za prikaz slike
            originalnaSlika = cv::imread(putanja.toStdString());
            sveSlike.push_back(originalnaSlika);
            slika = originalnaSlika.clone();
        }
        else return;
    }
    else if(odabir==2){
        QString folderPath = QFileDialog::getExistingDirectory(window, "Učitaj sesiju", "");
        if (!folderPath.isEmpty()) {
            sveSlike.clear();
            std::string baseFolder = folderPath.toStdString();

            // Putanje do foldera Anotacije i Original
            std::string folderAnotacije = baseFolder + "/Anotacija";
            std::string folderOriginal = baseFolder + "/Original";

            // Učitavanje slika iz foldera Anotacije
            QDir dirAnotacije(QString::fromStdString(folderAnotacije));

            QStringList filter;
            filter << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp";

            QFileInfoList listAnotacije = dirAnotacije.entryInfoList(filter, QDir::Files);
            for (const QFileInfo& fileInfo : listAnotacije) {
                cv::Mat img = cv::imread(fileInfo.absoluteFilePath().toStdString());
                if (!img.empty()) {
                    slika = img;
                }
            }

            // Učitavanje slika iz foldera Original
            QDir dirOriginal(QString::fromStdString(folderOriginal));
            QFileInfoList listOriginal = dirOriginal.entryInfoList(filter, QDir::Files);
            for (const QFileInfo& fileInfo : listOriginal) {
                cv::Mat img = cv::imread(fileInfo.absoluteFilePath().toStdString());
                if (!img.empty()) {
                    putanja = fileInfo.absoluteFilePath();
                    originalnaSlika = img;
                }
            }
            sveSlike.push_back(originalnaSlika);
            //cv::waitKey(0);
        } else {
            std::cerr << "Nije odabran folder." << std::endl;
            return;
        }
    }
    cv::namedWindow("Glavni pregled", cv::WINDOW_NORMAL);
    cv::resizeWindow("Glavni pregled", 800, 600);
    cv::imshow("Glavni pregled", slika);
    cv::setMouseCallback("Glavni pregled", onMouseClick, &slika);

    // Prikaz alatne trake
    window2->show();
}

void onMarkerClick(int markerId){
    trenutniMarker = markerId;
    if (markerId == 7) { // Ako je odabrana gumica
        crtanjeAktivno = false; // Isključi crtanje
    } else {
        crtanjeAktivno = true; // Omogući crtanje
        switch(markerId){
        case 1:
            boja = cv::Scalar(255,182,56);
            break;
        case 2:
            boja = cv::Scalar(69, 200, 255);
            break;
        case 3:
            boja = cv::Scalar(49, 49, 255);
            break;
        case 4:
            boja = cv::Scalar(110, 193, 0);
            break;
        case 5:
            boja = cv::Scalar(255, 82, 140);
            break;
        case 6:
            boja = cv::Scalar(196, 102, 255);
            break;
        case 8:
            boja = cv::Scalar(36, 137, 244);
            break;
        }
    }
}

void spasiPatcheve(std::vector<std::vector<cv::Mat>>& slike, QWidget* window6, std::vector<QString> naziv) {
    QString directory = QFileDialog::getExistingDirectory(window6, "Odaberi direktorij za spašavanje", QDir::currentPath());
    if (directory.isEmpty()) {
        return;  // Ako korisnik nije odabrao direktorij, prekidamo postupak
    }

    QFileInfo fileInfo(putanja);
    imeSlike = fileInfo.baseName();
    QString patchesRootDirectory = QDir(directory).filePath("Patchevi_" + imeSlike);
    QStringList supportedFormats = {"BMP (*.bmp)", "JPEG (*.jpg *.jpeg)", "PNG (*.png)"};
    QString selectedFilter = "BMP (*.bmp)";  // Defaultni format



    for (int i = 0; i<slike.size(); i++){
        if (!slike[i].empty()) {  
            QString defaultFileName;
            // Kreiramo poddirektorij za spašavanje patcheva
            QString patchesDirectoryName = "Patchevi_" + imeSlike + "_" + naziv[i];
            QString patchesDirectoryPath = QDir(patchesRootDirectory).filePath(patchesDirectoryName);

            // Kreiramo direktorij ako ne postoji
            QDir().mkpath(patchesDirectoryPath);

            for (size_t j = 0; j < slike[i].size(); ++j) {
                defaultFileName = imeSlike + "_" + QString("Patch%1").arg(QString::number(j + 1).rightJustified(4, '0')) + "_" + naziv[i];  // Patch0001, Patch0002, ...

                QString putanjaSpasene = QDir(patchesDirectoryPath).filePath(defaultFileName);

                // Spašavanje slike u odabrani format bez dijaloga za spašavanje
                QString selectedFormat = supportedFormats[0];  // Uzimamo prvi podržani format (BMP) bez dijaloga
                QString selectedPath = putanjaSpasene + ".bmp";  // Dodajemo ekstenziju BMP

                // Spašavanje slike u odabrani format
                cv::imwrite(selectedPath.toStdString(), slike[i][j]);
            }
            //slike[i].clear();
        }
    }
    slike.clear();
}

std::vector<cv::Mat> kreirajMaske(){
    std::vector<cv::Mat> maske;
    cv::Mat hsvSlika;
    cv::cvtColor(slika, hsvSlika, cv::COLOR_BGR2HSV);

    cv::Mat defekt1;
    cv::inRange(hsvSlika, cv::Scalar(101, 199, 255), cv::Scalar(101, 199, 255), defekt1);
    maske.push_back(defekt1);
    sveSlike.push_back(defekt1);

    cv::Mat defekt2;
    cv::inRange(hsvSlika, cv::Scalar(21, 186, 255), cv::Scalar(21,186,255), defekt2);
    maske.push_back(defekt2);
    sveSlike.push_back(defekt2);

    cv::Mat defekt3;
    cv::inRange(hsvSlika, cv::Scalar(0, 206, 255), cv::Scalar(0,206,255), defekt3);
    maske.push_back(defekt3);
    sveSlike.push_back(defekt3);

    cv::Mat defekt4;
    cv::inRange(hsvSlika, cv::Scalar(77, 255, 193), cv::Scalar(77,255,193), defekt4);
    maske.push_back(defekt4);
    sveSlike.push_back(defekt4);

    cv::Mat defekt5;
    cv::inRange(hsvSlika, cv::Scalar(15, 217, 244), cv::Scalar(15,217,244), defekt5);
    maske.push_back(defekt5);
    sveSlike.push_back(defekt5);

    cv::Mat rub;
    cv::inRange(hsvSlika, cv::Scalar(130, 173, 255), cv::Scalar(130,173,255), rub);
    maske.push_back(rub);
    sveSlike.push_back(rub);

    cv::Mat podloga;
    cv::inRange(hsvSlika, cv::Scalar(162, 153, 255), cv::Scalar(162,153,255), podloga);
    maske.push_back(podloga);
    sveSlike.push_back(podloga);

    cv::Mat spojenaMaska = defekt1 | defekt2 | defekt3 | defekt4 | defekt5 |rub | podloga;
    cv::Mat ispravno;
    cv::bitwise_not(spojenaMaska, ispravno);
    maske.push_back(ispravno);
    sveSlike.push_back(ispravno);

    cv::Mat spojenaMaska2 = rub | podloga;
    cv::Mat koza;
    cv::bitwise_not(spojenaMaska2, koza);
    maske.push_back(koza);
    sveSlike.push_back(koza);

    /*std::vector<std::string> imena_prozora = {"Defekt 1", "Defekt 2", "Defekt 3", "Defekt 4", "Defekt 5", "Rub", "Podloga", "Klasa ispravno", "Koza"};
    for (int i = 0; i < maske.size(); ++i){
        cv::namedWindow(imena_prozora[i], cv::WINDOW_NORMAL);
        cv::imshow(imena_prozora[i], maske[i]);
        cv::resizeWindow(imena_prozora[i], 800, 600);
    }*/
    return maske;
}

int dajOcjenu(cv::Mat patch){
    int broj_bijelih = cv::countNonZero(patch);
    int broj_crnih = patch.total() - broj_bijelih;
    if(broj_bijelih==0){
        return 0;
    }
    else if(broj_crnih==0){
        return 2;
    }
    else
        return 1;
}

std::vector<std::vector<int>> kreirajPatch(std::vector<QLineEdit*> textboxes, std::vector<cv::Mat> maske, QWidget* window){
    window->close();
    std::vector<int> dimenzije;
    cv::Mat p1, p2, p3, p4, p5, p6, p7, p8, p9;

    if(dimenzije.size()==0){
        for (QLineEdit* textbox : textboxes) {
            int vrijednost = textbox->text().toInt();
            dimenzije.push_back(vrijednost); // Ažurirajte vrijednost u vektoru
        }
    }
    cv::Mat slikaSaPatchevima = slika.clone();
    int x = dimenzije[0], y = dimenzije[1], sx = dimenzije[2], sy = dimenzije[3];
    for(int i = 0; i<=slikaSaPatchevima.cols - x; i+=sx){
        for(int j = 0; j<=slikaSaPatchevima.rows - y; j+=sy){
            cv::Rect pravougaonik(i, j, x, y);
            patchevi.push_back(originalnaSlika(pravougaonik).clone());
            p1 = maske[0](pravougaonik).clone();
            p2 = maske[1](pravougaonik).clone();
            p3 = maske[2](pravougaonik).clone();
            p4 = maske[3](pravougaonik).clone();
            p9 = maske[4](pravougaonik).clone();
            p5 = maske[5](pravougaonik).clone();
            p6 = maske[6](pravougaonik).clone();
            p7 = maske[7](pravougaonik).clone();
            p8 = maske[8](pravougaonik).clone();

            patchevi_d1.push_back(p1);
            patchevi_d2.push_back(p2);
            patchevi_d3.push_back(p3);
            patchevi_d4.push_back(p4);
            patchevi_d5.push_back(p9);
            patchevi_rub.push_back(p5);
            patchevi_podloga.push_back(p6);
            patchevi_ispravno.push_back(p7);
            patchevi_koza.push_back(p8);
            cv::rectangle(slikaSaPatchevima, pravougaonik, cv::Scalar(0,255,0), 10);

            ocjene_d1.push_back(dajOcjenu(p1));
            ocjene_d2.push_back(dajOcjenu(p2));
            ocjene_d3.push_back(dajOcjenu(p3));
            ocjene_d4.push_back(dajOcjenu(p4));
            ocjene_d5.push_back(dajOcjenu(p9));
            ocjene_rub.push_back(dajOcjenu(p5));
            ocjene_podloga.push_back(dajOcjenu(p6));
            ocjene_ispravno.push_back(dajOcjenu(p7));
            ocjene_koza.push_back(dajOcjenu(p8));
        }
    }

    cv::namedWindow("Prikaz patcheva", cv::WINDOW_NORMAL);
    cv::resizeWindow("Prikaz patcheva", 800, 600);
    cv::imshow("Prikaz patcheva", slikaSaPatchevima);
    std::vector<std::vector<int>> ocjene = {ocjene_d1, ocjene_d2, ocjene_d3, ocjene_d4, ocjene_d5, ocjene_rub, ocjene_podloga, ocjene_ispravno, ocjene_koza};
    return ocjene;
}

void eksportujPojedinePatcheve(QWidget* window6, QCheckBox* checkBoxDa, QCheckBox* checkBoxNe){
    std::vector<std::vector<int>> trazene_ocjene(9, std::vector<int>(3, 3));
    for (int i=0; i<9; i++){
        for (int j=0; j<3; j++){
            if (checkboxes[i][j]->isChecked()) {
                trazene_ocjene[i][j] = j;
            } else {
                trazene_ocjene[i][j] = 3;
            }
        }
    }

    for(int i=0; i<patchevi.size(); i++){
        indeksi.push_back(i);
    }

    for(int i = 0; i<indeksi.size(); i++){
        int t = indeksi[i];
        std::cout<<ocjene_d1[t]<<" "<<ocjene_d2[t]<<" "<<ocjene_d3[t]<<" "<<ocjene_d4[t]<< " "<<ocjene_d5[t]<<" " <<ocjene_rub[t]<<" "<<ocjene_podloga[t]<<" "<<ocjene_ispravno[t]<<" "<<ocjene_koza[t]<<std::endl;
        if(ocjene_d1[t] == trazene_ocjene[0][0] || ocjene_d1[t] == trazene_ocjene[0][1] || ocjene_d1[t] == trazene_ocjene[0][2])
            if(ocjene_d2[t] == trazene_ocjene[1][0] || ocjene_d2[t] == trazene_ocjene[1][1] || ocjene_d2[t] == trazene_ocjene[1][2])
                if(ocjene_d3[t] == trazene_ocjene[2][0] || ocjene_d3[t] == trazene_ocjene[2][1] || ocjene_d3[t] == trazene_ocjene[2][2])
                    if(ocjene_d4[t] == trazene_ocjene[3][0] || ocjene_d4[t] == trazene_ocjene[3][1] || ocjene_d4[t] == trazene_ocjene[3][2])
                        if(ocjene_d5[t] == trazene_ocjene[4][0] || ocjene_d5[t] == trazene_ocjene[4][1] || ocjene_d5[t] == trazene_ocjene[4][2])
                            if(ocjene_rub[t] == trazene_ocjene[5][0] || ocjene_rub[t] == trazene_ocjene[5][1] || ocjene_rub[t] == trazene_ocjene[5][2])
                                if(ocjene_podloga[t] == trazene_ocjene[6][0] || ocjene_podloga[t] == trazene_ocjene[6][1] || ocjene_podloga[t] == trazene_ocjene[6][2])
                                    if(ocjene_ispravno[t] == trazene_ocjene[7][0] || ocjene_ispravno[t] == trazene_ocjene[7][1] || ocjene_ispravno[t] == trazene_ocjene[7][2])
                                        if(ocjene_koza[t] == trazene_ocjene[8][0] || ocjene_koza[t] == trazene_ocjene[8][1] || ocjene_koza[t] == trazene_ocjene[8][2]){
                                            spasi.push_back(t);
                                            indeksi.erase(std::remove(indeksi.begin(), indeksi.end(), t), indeksi.end());
                                            i--;
                                        }
    }

    std::vector<cv::Mat> spaseniPatchevi;
    std::vector<cv::Mat> spasenaMaskaD1;
    std::vector<cv::Mat> spasenaMaskaD2;
    std::vector<cv::Mat> spasenaMaskaD3;
    std::vector<cv::Mat> spasenaMaskaD4;
    std::vector<cv::Mat> spasenaMaskaD5;
    std::vector<cv::Mat> spasenaMaskaRub;
    std::vector<cv::Mat> spasenaMaskaPodloga;
    std::vector<cv::Mat> spasenaMaskaIspravno;
    std::vector<cv::Mat> spasenaMaskaKoza;


    for (int i=0; i<spasi.size(); i++){
        spaseniPatchevi.push_back(patchevi[spasi[i]]);
        spasenaMaskaD1.push_back(patchevi_d1[spasi[i]]);
        spasenaMaskaD2.push_back(patchevi_d2[spasi[i]]);
        spasenaMaskaD3.push_back(patchevi_d3[spasi[i]]);
        spasenaMaskaD4.push_back(patchevi_d4[spasi[i]]);
        spasenaMaskaD5.push_back(patchevi_d5[spasi[i]]);
        spasenaMaskaRub.push_back(patchevi_rub[spasi[i]]);
        spasenaMaskaPodloga.push_back(patchevi_podloga[spasi[i]]);
        spasenaMaskaIspravno.push_back(patchevi_ispravno[spasi[i]]);
        spasenaMaskaKoza.push_back(patchevi_koza[spasi[i]]);
    }

    std::vector<std::vector<cv::Mat>> spaseni;
    std::vector<QString> nazivi;
    if (checkBoxNe && checkBoxNe->isChecked()) {
        spaseni = {spaseniPatchevi};
        nazivi = {"Original"};
    } else if (checkBoxDa && checkBoxDa->isChecked()) {
        spaseni = {spaseniPatchevi, spasenaMaskaD1, spasenaMaskaD2, spasenaMaskaD3, spasenaMaskaD4, spasenaMaskaD5,
                   spasenaMaskaRub, spasenaMaskaPodloga, spasenaMaskaIspravno, spasenaMaskaKoza};
        nazivi = {"Original", "Maska "+naziviUAplikaciji[0], "Maska "+naziviUAplikaciji[1], "Maska "+naziviUAplikaciji[2],
                  "Maska "+naziviUAplikaciji[3], "Maska "+naziviUAplikaciji[4], "Maska "+naziviUAplikaciji[6],
                  "Maska "+naziviUAplikaciji[7], "Maska "+naziviUAplikaciji[8], "Maska "+naziviUAplikaciji[9]};
    }

    if(!spaseniPatchevi.empty())
        spasiPatcheve(spaseni, window6, nazivi);
    spasi.clear();
}


int main(int argc, char *argv[]) {

    QApplication app(argc, argv);

    QString configFileName = "release/config.txt"; //PRILIKOM RELEASA APLIKACIJE UKINUTI OVO RELEASE IZ PUTANJE

    // Čitanje konfiguracijskog fajla
    readConfig(configFileName);

    QWidget window;
    window.setWindowTitle("DefectDetect");
    QVBoxLayout *layout = new QVBoxLayout(&window);

    QLabel *imageLabel = new QLabel(&window);
    QPixmap pixmap(":/ikonice/logo.png");
    imageLabel->setPixmap(pixmap);
    imageLabel->setScaledContents(true);
    layout->addWidget(imageLabel);

    //ALATNA TRAKA
    QWidget window2;
    window2.setWindowTitle("DefectDetect");
    QVBoxLayout *layout_alatna_traka = new QVBoxLayout(&window2);

    //Kreiranje dugmeta za učitavanje slike
    QPushButton *button = new QPushButton("Odaberi sliku", &window);
    layout->addWidget(button);
    QObject::connect(button, &QPushButton::clicked, [&]() {
        onButtonClick(&window, &window2,1);
    });

    QPushButton *button2 = new QPushButton("Učitaj sesiju", &window);
    layout->addWidget(button2);
    QObject::connect(button2, &QPushButton::clicked, [&]() {
        onButtonClick(&window, &window2,2);
    });

    QHBoxLayout *markerLayout = new QHBoxLayout;
    layout_alatna_traka->addLayout(markerLayout);

    QToolButton *markerButton1 = new QToolButton(&window2);
    markerButton1->setIcon(QIcon(":/ikonice/1.png"));
    markerButton1->setToolTip(naziviUAplikaciji[0]);
    //markerButton1->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    markerLayout->addWidget(markerButton1);

    QObject::connect(markerButton1, &QToolButton::clicked, [&]() {
        onMarkerClick(1);
    });

    QToolButton *markerButton2 = new QToolButton(&window2);
    markerButton2->setIcon(QIcon(":/ikonice/2.png"));
    markerButton2->setToolTip(naziviUAplikaciji[1]);
    //markerButton2->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    markerLayout->addWidget(markerButton2);

    QObject::connect(markerButton2, &QToolButton::clicked, [&]() {
        onMarkerClick(2);
    });

    QToolButton *markerButton3 = new QToolButton(&window2);
    markerButton3->setIcon(QIcon(":/ikonice/3.png"));
    markerButton3->setToolTip(naziviUAplikaciji[2]);
    //markerButton3->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    markerLayout->addWidget(markerButton3);

    QObject::connect(markerButton3, &QToolButton::clicked, [&]() {
        onMarkerClick(3);
    });

    QToolButton *markerButton4 = new QToolButton(&window2);
    markerButton4->setIcon(QIcon(":/ikonice/4.png"));
    markerButton4->setToolTip(naziviUAplikaciji[3]);
    //markerButton4->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    markerLayout->addWidget(markerButton4);

    QObject::connect(markerButton4, &QToolButton::clicked, [&]() {
        onMarkerClick(4);
    });

    QToolButton *markerButton8 = new QToolButton(&window2);
    markerButton8->setIcon(QIcon(":/ikonice/8.png"));
    markerButton8->setToolTip(naziviUAplikaciji[4]);
    //markerButton8->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    markerLayout->addWidget(markerButton8);

    QObject::connect(markerButton8, &QToolButton::clicked, [&]() {
        onMarkerClick(8);
    });

    QToolButton *markerButton5 = new QToolButton(&window2);
    markerButton5->setIcon(QIcon(":/ikonice/5.png"));
    markerButton5->setText("Rub");
    markerButton5->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    markerLayout->addWidget(markerButton5);

    QObject::connect(markerButton5, &QToolButton::clicked, [&]() {
        onMarkerClick(5);
    });

    QToolButton *markerButton6 = new QToolButton(&window2);
    markerButton6->setIcon(QIcon(":/ikonice/6.png"));
    markerButton6->setText("Podloga");
    markerButton6->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    markerLayout->addWidget(markerButton6);

    QObject::connect(markerButton6, &QToolButton::clicked, [&]() {
        onMarkerClick(6);
    });

    QToolButton *gumica = new QToolButton(&window2);
    gumica->setIcon(QIcon(":/ikonice/7.png"));
    gumica->setText(naziviUAplikaciji[5]);
    gumica->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    markerLayout->addWidget(gumica);

    QObject::connect(gumica, &QToolButton::clicked, [&]() {
        onMarkerClick(7);
    });

  /*  QToolButton *mis = new QToolButton(&window2);
    mis->setIcon(QIcon(":/ikonice/mis.png"));
    markerLayout->addWidget(mis);

    QObject::connect(mis, &QToolButton::clicked, [&]() {
        onMarkerClick(9);
    });*/

    //POSTAVLJANJE DEBLJINE MARKERA
    QLabel *label3 = new QLabel("Debljina:", &window2);
    markerLayout->addWidget(label3);

    QToolButton *debljina1 = new QToolButton(&window2);
    debljina1->setIcon(QIcon(":/ikonice/najtanji.png"));
    debljina1->setToolTip("1");
    markerLayout->addWidget(debljina1);

    QObject::connect(debljina1, &QToolButton::clicked, [&]() {
        velicinaMarkera = 10;
    });

    QToolButton *debljina2 = new QToolButton(&window2);
    debljina2->setIcon(QIcon(":/ikonice/srednji.png"));
    debljina2->setToolTip("2");
    markerLayout->addWidget(debljina2);

    QObject::connect(debljina2, &QToolButton::clicked, [&]() {
        velicinaMarkera = 20;
    });

    QToolButton *debljina3 = new QToolButton(&window2);
    debljina3->setIcon(QIcon(":/ikonice/najdeblji.png"));
    debljina3->setToolTip("3");
    markerLayout->addWidget(debljina3);

    QObject::connect(debljina3, &QToolButton::clicked, [&]() {
        velicinaMarkera = 30;
    });

    QHBoxLayout *layout_kraj = new QHBoxLayout;
    layout_alatna_traka->addLayout(layout_kraj);
    //PRIVREMENO UKLANJANJE ANOTACIJA
    QPushButton *privremeniPregled = new QPushButton("Ukloni anotacije", &window2);
    layout_kraj->addWidget(privremeniPregled);
    window2.setLayout(layout_kraj);

    QObject::connect(privremeniPregled, &QPushButton::pressed, [&]() {
        //cv::Mat originalna_slika = cv::imread(putanja); //prikaz originala dok je dugme pritisnuto
        cv::imshow("Glavni pregled", originalnaSlika);
    });

    QObject::connect(privremeniPregled, &QPushButton::released, [&]() {
        cv::imshow("Glavni pregled", slika); // prikaz anotacija
    });

    //****************************************************************************//
    //EVENTUALNA IZMJENA AKO TREBA DA SE JEDNIM KLIKOM UKLONE, PA VRATE ANOTACIJE
    //****************************************************************************//

    /*    QPushButton *vratiAnotacije = new QPushButton("Prikaži anotacije", &window2);
    layout_alatna_traka->addWidget(vratiAnotacije);
    window2.setLayout(layout_alatna_traka);

    QObject::connect(vratiAnotacije, &QPushButton::clicked, [&]() {
        //slika = cv::imread(putanja); // Učitajte sliku ponovno (morate imati putanju do originalne slike)
        cv::imshow("Glavni pregled", slika); // Prikaz originalne slike
    });*/

    //PROZOR UPOZORENJA ZA BRISANJE SVIH ANOTACIJA
    QWidget window4;
    window4.setWindowTitle("Brisanje anotacija");
    QVBoxLayout *layout_brisanje_anotacija = new QVBoxLayout(&window4);
    window4.setLayout(layout_brisanje_anotacija);

    QLabel *label4 = new QLabel("Da li ste sigurni da želite obrisati sve anotacije?", &window4);
    label4->setAlignment(Qt::AlignCenter); // Centriranje teksta
    layout_brisanje_anotacija->addWidget(label4);

    QPushButton *Da = new QPushButton("Da", &window4);
    layout_brisanje_anotacija->addWidget(Da);

    QObject::connect(Da, &QPushButton::clicked, [&]() {
        slika = originalnaSlika.clone();
        cv::imshow("Glavni pregled", slika);
        window4.close();
    });

    QPushButton *Ne = new QPushButton("Ne", &window4);
    layout_brisanje_anotacija->addWidget(Ne);

    QObject::connect(Ne, &QPushButton::clicked, [&]() {
        window4.close();
    });

    QPushButton *obrisiSve = new QPushButton("Obriši anotacije", &window2);
    layout_kraj->addWidget(obrisiSve);

    QObject::connect(obrisiSve, &QPushButton::clicked, [&]() {
        window4.show();
    });

    //BUTTON ZA OZNAKU KRAJA ANOTACIJE, KAKO BI SE MOGLE KREIRATI MASKE I PATCHEVI
    QWidget prozorKrajAnotacija;
    prozorKrajAnotacija.setWindowTitle("Kraj anotacija");
    QVBoxLayout *layout_kraj_anotacija = new QVBoxLayout(&prozorKrajAnotacija);
    prozorKrajAnotacija.setLayout(layout_kraj_anotacija);

    QPushButton *krajAnotacije = new QPushButton("Završi anotiranje", &window2);
    layout_kraj->addWidget(krajAnotacije);

    std::vector<cv::Mat> maske;

    QObject::connect(krajAnotacije, &QPushButton::clicked, [&]() {
        prozorKrajAnotacija.show();
        patchevi.clear();
        patchevi_d1.clear();
        patchevi_d2.clear();
        patchevi_d3.clear();
        patchevi_d4.clear();
        patchevi_d5.clear();
        patchevi_rub.clear();
        patchevi_ispravno.clear();
        patchevi_podloga.clear();
        patchevi_koza.clear();
        ocjene_koza.clear();
        ocjene_d1.clear();
        ocjene_d2.clear();
        ocjene_d3.clear();
        ocjene_d4.clear();
        ocjene_d5.clear();
        ocjene_ispravno.clear();
        ocjene_podloga.clear();
        ocjene_rub.clear();
        spasi.clear();
        dimenzije.clear();
        indeksi.clear();
        sveSlike.clear();
        sveSlike.push_back(originalnaSlika);
        maske = kreirajMaske();
        //window2.close();
    });


    QPushButton *prikazMaski = new QPushButton("Prikaži maske", &prozorKrajAnotacija);
    layout_kraj_anotacija->addWidget(prikazMaski);

    QObject::connect(prikazMaski, &QPushButton::clicked, [&]() {
        std::vector<std::string> imena_prozora = {naziviUAplikaciji[0].toStdString(), naziviUAplikaciji[1].toStdString(), naziviUAplikaciji[2].toStdString(),
                                                  naziviUAplikaciji[3].toStdString(), naziviUAplikaciji[4].toStdString(), naziviUAplikaciji[6].toStdString(),
                                                  naziviUAplikaciji[7].toStdString(), naziviUAplikaciji[8].toStdString(), naziviUAplikaciji[9].toStdString()};
        for (int i = 0; i < maske.size(); ++i){
            cv::namedWindow(imena_prozora[i], cv::WINDOW_NORMAL);
            cv::imshow(imena_prozora[i], maske[i]);
            cv::resizeWindow(imena_prozora[i], 800, 600);
        }
    });

    //PROZOR UPOZORENJE DA SU SLIKE SPAŠENE
    QWidget window3;
    window3.setWindowTitle("Anotirana slika");
    QVBoxLayout *layout_spasiSlike = new QVBoxLayout(&window3);

    QLabel *label2 = new QLabel("Slika je spašena", &window3);
    label2->setAlignment(Qt::AlignCenter); // Centriranje teksta
    layout_spasiSlike->addWidget(label2);

    //PROZOR ZA UNOS DIMENZIJA PATCHA
    QWidget prozorDimenzijePatcha;
    prozorDimenzijePatcha.setWindowTitle("Izrada patcheva");
    QVBoxLayout *layout_patchevi = new QVBoxLayout(&prozorDimenzijePatcha);
    prozorDimenzijePatcha.setLayout(layout_patchevi);

    std::vector<QLineEdit*> textboxes;
    for (int i = 0; i < 4; ++i) {
        std::vector<QString> nazivi = {"Širina patcha", "Visina patcha", "Horizonatlni stride", "Vertikalni stride"};
        QString label_text =  nazivi[i] + ":";
        QLabel *label = new QLabel(label_text, &prozorDimenzijePatcha);
        layout_patchevi->addWidget(label);

        // Kreirajte textbox za unos cijelih brojeva
        QLineEdit *textbox = new QLineEdit(&prozorDimenzijePatcha);
        textbox->setValidator(new QIntValidator(textbox));
        layout_patchevi->addWidget(textbox);
        textboxes.push_back(textbox);
        textbox->setText(QString::number(dimenzije[i]));

    }
    QHBoxLayout *layout_patchevi2 = new QHBoxLayout(&prozorDimenzijePatcha);
    layout_patchevi->addLayout(layout_patchevi2);

    std::vector<std::vector<int>> ocjene;


    QPushButton *izradiPatch = new QPushButton("Izradi patcheve", &prozorDimenzijePatcha);
    layout_patchevi2->addWidget(izradiPatch);


    QObject::connect(izradiPatch, &QPushButton::clicked, [&]() {
        ocjene = kreirajPatch(textboxes, maske, &prozorDimenzijePatcha);
    });

    QPushButton *patcheviButton = new QPushButton("Izradi patcheve", &prozorKrajAnotacija);
    layout_kraj_anotacija->addWidget(patcheviButton);

    QObject::connect(patcheviButton, &QPushButton::clicked, [&]() {
        prozorDimenzijePatcha.show();
    });

    QPushButton *patcheviExport = new QPushButton("Eksportuj patcheve", &prozorKrajAnotacija);
    layout_kraj_anotacija->addWidget(patcheviExport);


    QWidget prozorOdabirEksporta;
    prozorOdabirEksporta.setWindowTitle("Odabir eksporta");
    QVBoxLayout *layout_odabir_eksporta = new QVBoxLayout(&prozorOdabirEksporta);
    prozorOdabirEksporta.setLayout(layout_odabir_eksporta);

    QPushButton *eksportSvih = new QPushButton("Eksportuj sve patcheve", &prozorOdabirEksporta);
    layout_odabir_eksporta->addWidget(eksportSvih);

    QPushButton *eksportPojedinih = new QPushButton("Eksportuj odabrane patcheve", &prozorOdabirEksporta);
    layout_odabir_eksporta->addWidget(eksportPojedinih);

    //PROZOR ZA EXPORT PATCHEVA PREKO OCJENA
    QWidget prozorOcjene;
    prozorOcjene.setWindowTitle("Eksport patcheva");
    QVBoxLayout *layout_export_glavni = new QVBoxLayout(&prozorOcjene);
    prozorOcjene.setLayout(layout_export_glavni);



    QCheckBox *checkBoxDa = nullptr;
    QCheckBox *checkBoxNe = nullptr;

    auto dodajEksportMaskeCheckBox = [&]() {
        QHBoxLayout *layout = new QHBoxLayout;
        QWidget *widget = new QWidget(&window); // Kontejner za checkbox i tekst
        QVBoxLayout *innerLayout = new QVBoxLayout(widget); // Layout unutar kontejnera

        QLabel *label = new QLabel("Eksport maski:", widget);
        label->setAlignment(Qt::AlignLeft);
        innerLayout->addWidget(label);

        QHBoxLayout *checkboxLayout = new QHBoxLayout;
        checkBoxDa = new QCheckBox("Da", widget);
        checkBoxNe = new QCheckBox("Ne", widget);

        checkBoxDa->setChecked(true);

        QObject::connect(checkBoxDa, &QCheckBox::toggled, [checkBoxNe](bool checked){
            if (checked) {
                checkBoxNe->setChecked(false);
            }
        });

        QObject::connect(checkBoxNe, &QCheckBox::toggled, [checkBoxDa](bool checked){
            if (checked) {
                checkBoxDa->setChecked(false);
            }
        });

        checkboxLayout->addWidget(checkBoxDa);
        checkboxLayout->addWidget(checkBoxNe);

        innerLayout->addLayout(checkboxLayout);
        layout->addWidget(widget);
        layout_odabir_eksporta->addLayout(layout);
    };

    // Pozovite funkciju
    dodajEksportMaskeCheckBox();

    QObject::connect(patcheviExport, &QPushButton::clicked, [&]() {
       prozorOdabirEksporta.show();
    });

    QObject::connect(eksportSvih, &QPushButton::clicked, [&]() {
        std::vector<std::vector<cv::Mat>> spasi;
        std::vector<QString> nazivi;
        if (checkBoxNe && checkBoxNe->isChecked()) {
            spasi = {patchevi};
            nazivi = {"Original"};
        } else if (checkBoxDa && checkBoxDa->isChecked()) {
            spasi = {patchevi, patchevi_d1, patchevi_d2, patchevi_d3, patchevi_d4, patchevi_d5,
                                                    patchevi_ispravno, patchevi_koza, patchevi_podloga, patchevi_rub};
            nazivi = {"Original", "Maska "+naziviUAplikaciji[0], "Maska "+naziviUAplikaciji[1], "Maska "+naziviUAplikaciji[2],
                      "Maska "+naziviUAplikaciji[3], "Maska "+naziviUAplikaciji[4], "Maska "+naziviUAplikaciji[8],
                      "Maska "+naziviUAplikaciji[9], "Maska "+naziviUAplikaciji[7], "Maska "+naziviUAplikaciji[6]};
        }

        if(!patchevi.empty())
            spasiPatcheve(spasi, &prozorOdabirEksporta, nazivi);

        prozorOdabirEksporta.close();
    });

    QObject::connect(eksportPojedinih, &QPushButton::clicked, [&]() {
        prozorOcjene.show();
    });

    /*QObject::connect(patcheviExport, &QPushButton::clicked, [&]() {
        window6.show();
    });*/

    auto dodajCheckBoxove = [&](const QString& tekst, QWidget* eksport) {
        QHBoxLayout *layout = new QHBoxLayout;
        QWidget *widget = new QWidget(&window); // Kontejner za checkbox i tekst
        QVBoxLayout *innerLayout = new QVBoxLayout(widget); // Layout unutar kontejnera

        QLabel *label = new QLabel(tekst + ":", widget);
        label->setAlignment(Qt::AlignLeft);
        innerLayout->addWidget(label);

        QHBoxLayout *checkboxLayout = new QHBoxLayout;
        std::vector<QCheckBox*> chbpomocni;
        for (int i = 0; i <= 2; ++i) {
            QString naziv = QString::number(i);
            QCheckBox *checkBox = new QCheckBox(naziv, widget);
            chbpomocni.push_back(checkBox);
            checkboxLayout->addWidget(checkBox);
        }

        checkboxes.push_back(chbpomocni);
        innerLayout->addLayout(checkboxLayout);
        layout->addWidget(widget);
        layout_export_glavni->addLayout(layout);
        layout_export_glavni->addWidget(eksport);
    };

    QPushButton *eksport = new QPushButton("Eksportuj");

    std::vector<QString> tekstovi = {"Odaberite ocjene za "+naziviUAplikaciji[0], "Odaberite ocjene za "+naziviUAplikaciji[1],
                                     "Odaberite ocjene za "+naziviUAplikaciji[2], "Odaberite ocjene za "+naziviUAplikaciji[3],
                                     "Odaberite ocjene za "+naziviUAplikaciji[4], "Odaberite ocjene za "+naziviUAplikaciji[6],
                                     "Odaberite ocjene za "+naziviUAplikaciji[7], "Odaberite ocjene za "+naziviUAplikaciji[8],
                                     "Odaberite ocjene za "+naziviUAplikaciji[9]};

    for (int i=0; i<tekstovi.size(); i++){
        dodajCheckBoxove(tekstovi[i], eksport);
    }

    QObject::connect(eksport, &QPushButton::clicked, [&]() {
        eksportujPojedinePatcheve(&prozorOcjene, checkBoxDa, checkBoxNe);
        prozorOcjene.close();
        prozorOdabirEksporta.close();
    });

    QPushButton *spasi_sliku = new QPushButton("Spasi sliku", &prozorKrajAnotacija);
    layout_kraj_anotacija->addWidget(spasi_sliku);

    QObject::connect(spasi_sliku, &QPushButton::clicked, [&]() {
        sveSlike.push_back(slika);
        std::vector<cv::Mat>& referenca_slike = sveSlike;
        spasiSlike(referenca_slike, &window2, &window3);
    });
    QPushButton *krajPrograma = new QPushButton("Kraj sesije", &window2);
    layout_kraj->addWidget(krajPrograma);

    QObject::connect(krajPrograma, &QPushButton::clicked, [&]() {
        QWidgetList allWidgets = qApp->topLevelWidgets();
        for (int i = 0; i < allWidgets.size(); ++i) {
            allWidgets.at(i)->close();
        }
        cv::destroyAllWindows();
    });

    window.show();
    cv::waitKey(0);
    return app.exec();
}
