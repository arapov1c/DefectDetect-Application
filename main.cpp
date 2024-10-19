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
#include <QTabWidget>
#include <QFormLayout>
#include <QComboBox>
#include <QDoubleValidator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QKeyEvent>
#include <QObject>
#include <QEvent>

//globalne varijable koje se koriste u nekoliko funkcija, deklarisane ovako radi lakše manipulacije između funkcija
cv::Mat slika, originalnaSlika;
cv::Scalar boja;
cv::Point prethodnaTacka(-1, -1);
bool crtanjeAktivno = false;
int trenutnaSlikaIndex=0;
int defaultVelicinaMarkera = 20;
int velicinaMarkera = defaultVelicinaMarkera, trenutniMarker = 1;
std::vector<double> defaultDimenzije = {200,200,50,50};
std::vector<double> dimenzije = defaultDimenzije;
std::vector<int> indeksi, spasi;
std::vector<std::vector<QCheckBox*>> checkboxes;
std::vector<QString> defaultNaziviUAplikaciji = {"Defect 1", "Defect 2", "Defect 3", "Defect 4", "Defect 5", "Eraser",
                                                 "Edge", "Surface", "Class Correct", "Leather"};
std::vector<QString> naziviUAplikaciji = defaultNaziviUAplikaciji;
QString putanja, putanjaSpasene;
std::vector<int> ocjene_d1, ocjene_d2, ocjene_d3, ocjene_d4, ocjene_d5, ocjene_rub, ocjene_podloga, ocjene_ispravno, ocjene_koza;
std::vector<cv::Mat> sveSlike, patchevi, patchevi_d1, patchevi_d2, patchevi_d3, patchevi_d4, patchevi_d5, patchevi_rub, patchevi_podloga, patchevi_ispravno, patchevi_koza;
QString imeSlike, directory, patchesRootDirectory;
int defaultOdstupanje = 95;
int odstupanjeZaOcjenu1 = defaultOdstupanje;
std::vector<std::vector<int>> koordinate;
QFileInfoList fileList;

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

        if (key == "MarkerThickness") {
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
        else if(key=="Marker1Name"){
            naziviUAplikaciji[0] = value;
        }
        else if(key=="Marker2Name"){
            naziviUAplikaciji[1] = value;
        }
        else if(key=="Marker3Name"){
            naziviUAplikaciji[2] = value;
        }
        else if(key=="Marker4Name"){
            naziviUAplikaciji[3] = value;
        }
        else if(key=="Marker5Name"){
            naziviUAplikaciji[4] = value;
        }
        else if(key=="EraserName"){
            naziviUAplikaciji[5] = value;
        }
        else if(key=="EdgeName"){
            naziviUAplikaciji[6] = value;
        }
        else if(key=="SurfaceName"){
            naziviUAplikaciji[7] = value;
        }
        else if(key=="ClassCorrectName"){
            naziviUAplikaciji[8] = value;
        }
        else if(key=="ClassLeatherName"){
            naziviUAplikaciji[9] = value;
        }
        else if(key=="Rating1Tolerance"){
            odstupanjeZaOcjenu1 = value.toInt();
        }

    }

    file.close();
}

void writeConfig(const QString &fileName, int velicinaMarkera, const std::vector<QString> &nazivi, const std::vector<double> &dimenzije, int odstupanjeZaOcjenu1) {
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file for writing:" << fileName;
        return;
    }

    QTextStream out(&file);

    // Upisivanje svih vrednosti u fajl
    out << "MarkerThickness=" << velicinaMarkera << "\n";

    // Pretpostavljamo da su dimenzije i nazivi uvek iste dužine kao u readConfig
    out << "x=" << dimenzije[0] << "\n";
    out << "y=" << dimenzije[1] << "\n";
    out << "sx=" << dimenzije[2] << "\n";
    out << "sy=" << dimenzije[3] << "\n";

    out << "Marker1Name=" << nazivi[0] << "\n";
    out << "Marker2Name=" << nazivi[1] << "\n";
    out << "Marker3Name=" << nazivi[2] << "\n";
    out << "Marker4Name=" << nazivi[3] << "\n";
    out << "Marker5Name=" << nazivi[4] << "\n";
    out << "EraserName=" << nazivi[5] << "\n";
    out << "EdgeName=" << nazivi[6] << "\n";
    out << "SurfaceName=" << nazivi[7] << "\n";
    out << "ClassCorrectName=" << nazivi[8] << "\n";
    out << "ClasssLeatherName=" << nazivi[9] << "\n";

    out << "Rating1Tolerance=" << odstupanjeZaOcjenu1 << "\n";


    file.close();
}

void spasiSlike(std::vector<cv::Mat>& slike, QWidget* window2, QWidget* window3) {
    QFileInfo fileInfo(putanja);
    imeSlike = fileInfo.baseName();
    std::vector<std::string> naziviFoldera = {"Original", "Masks", "Masks", "Masks", "Masks", "Masks", "Masks", "Masks", "Masks", "Masks", "Annotation"};
    std::vector<std::string> naziviSlika = {imeSlike.toStdString(), naziviUAplikaciji[0].toStdString(), naziviUAplikaciji[1].toStdString(),
                                            naziviUAplikaciji[2].toStdString(), naziviUAplikaciji[3].toStdString(), naziviUAplikaciji[4].toStdString(),
                                            naziviUAplikaciji[6].toStdString(), naziviUAplikaciji[7].toStdString(), naziviUAplikaciji[8].toStdString(), naziviUAplikaciji[9].toStdString(), "Anotacija_"+imeSlike.toStdString()};

    // Odabir glavnog foldera
    QString glavniFolder = QFileDialog::getExistingDirectory(window2, "Choose main directory", QDir::currentPath());
    if (glavniFolder.isEmpty()) {
        return;  // Ako korisnik nije odabrao glavni folder, prekidamo postupak
    }

    QString rezultatiFolder = glavniFolder + "/Results_" + imeSlike;
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
        cv::imshow("Main View", *trenutnaSlika);
    } else if (event == cv::EVENT_MOUSEMOVE && (flags & cv::EVENT_FLAG_ALTKEY) && !crtanjeAktivno) { //Implementacija gumice tako što se kopira original preko
        kopirajDioSlike(originalnaSlika, *trenutnaSlika, x, y, velicinaMarkera);                       //anotirane slike
        cv::imshow("Main View", *trenutnaSlika);
    }
}

void loadImage(int index) {
    if (index >= 0 && index < fileList.size()) {
        QString imagePath = fileList[index].absoluteFilePath();
        putanja = imagePath;
        originalnaSlika = cv::imread(putanja.toStdString());
        slika = originalnaSlika.clone();

        cv::namedWindow("Main View", cv::WINDOW_NORMAL);
        cv::resizeWindow("Main View", 800, 600);
        cv::imshow("Main View", slika);
        cv::setMouseCallback("Main View", onMouseClick, &slika);
    }

}

void onButtonClick(QWidget* window, QWidget* window2, int odabir) {
    if(odabir==1){
        QString filePath = QFileDialog::getOpenFileName(window, "Choose image", "", "Images (*.png *.jpg *.jpeg *.bmp)");
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
        QString folderPath = QFileDialog::getExistingDirectory(window, "Load session", "");
        if (!folderPath.isEmpty()) {
            sveSlike.clear();
            std::string baseFolder = folderPath.toStdString();

            // Putanje do foldera Anotacije i Original
            std::string folderAnotacije = baseFolder + "/Annoatation";
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
    else if(odabir==3){
        // Otvaranje dijaloga za odabir foldera
        QString folderSlika = QFileDialog::getExistingDirectory(window, "Choose folder", "");

        if (!folderSlika.isEmpty()) {
            sveSlike.clear();
            trenutnaSlikaIndex = 0;

            // Pronađi sve slike u folderu
            QDir dir(folderSlika);
            QStringList filters;
            filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp";  // Filtriraj slike po ekstenziji
            dir.setNameFilters(filters);

            fileList = dir.entryInfoList(QDir::Files);

            // Provjera da li ima slika u folderu
            if (!fileList.isEmpty()) {
                loadImage(trenutnaSlikaIndex);
            }
        }
    }

    if(odabir!=3){
        cv::namedWindow("Main View", cv::WINDOW_NORMAL);
        cv::resizeWindow("Main View", 800, 600);
        cv::imshow("Main View", slika);
        cv::setMouseCallback("Main View", onMouseClick, &slika);
    }

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
    directory = QFileDialog::getExistingDirectory(window6, "Choose directory", QDir::currentPath());
    if (directory.isEmpty()) {
        return;  // Ako korisnik nije odabrao direktorij, prekidamo postupak
    }

    QFileInfo fileInfo(putanja);
    imeSlike = fileInfo.baseName();
    patchesRootDirectory = QDir(directory).filePath("Patches_" + imeSlike);
    std::cout<<patchesRootDirectory.toStdString()<<std::endl;
    QStringList supportedFormats = {"BMP (*.bmp)", "JPEG (*.jpg *.jpeg)", "PNG (*.png)"};
    QString selectedFilter = "BMP (*.bmp)";  // Defaultni format



    for (int i = 0; i<slike.size(); i++){
        if (!slike[i].empty()) {  
            QString defaultFileName;
            // Kreiramo poddirektorij za spašavanje patcheva
            QString patchesDirectoryName = "Patches_" + imeSlike + "_" + naziv[i];
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

    return maske;
}

int dajOcjenu(cv::Mat patch){
    int broj_bijelih = cv::countNonZero(patch);
    int broj_crnih = patch.total() - broj_bijelih;
    std::cout<<"bijeli"<<broj_bijelih<<" crni"<<broj_crnih<<std::endl<<"ukupno"<<patch.total()<<" provjera1 "<<odstupanjeZaOcjenu1*0.01*patch.total()<<" provjera2 "<<patch.total() - odstupanjeZaOcjenu1*0.01*patch.total()<<std::endl;
    if(broj_bijelih<=patch.total() - odstupanjeZaOcjenu1*0.01*patch.total()){
        return 0;
    }
    else if(broj_bijelih>odstupanjeZaOcjenu1*0.01*patch.total()){
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
            std::vector<int> koordinata = {i,j};

            patchevi_d1.push_back(p1);
            patchevi_d2.push_back(p2);
            patchevi_d3.push_back(p3);
            patchevi_d4.push_back(p4);
            patchevi_d5.push_back(p9);
            patchevi_rub.push_back(p5);
            patchevi_podloga.push_back(p6);
            patchevi_ispravno.push_back(p7);
            patchevi_koza.push_back(p8);
            koordinate.push_back(koordinata);
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

    cv::namedWindow("Patches view", cv::WINDOW_NORMAL);
    cv::resizeWindow("Patches view", 800, 600);
    cv::imshow("Patches view", slikaSaPatchevima);
    std::vector<std::vector<int>> ocjene = {ocjene_d1, ocjene_d2, ocjene_d3, ocjene_d4, ocjene_d5, ocjene_rub, ocjene_podloga, ocjene_ispravno, ocjene_koza};
    return ocjene;
}

void exportJson(std::vector<std::vector<cv::Mat>>& slike, std::vector<QString> naziv,
                QCheckBox* includeMasks, std::vector<std::vector<int>>& ocjene, QCheckBox* kolektivni) {

    if(kolektivni && kolektivni->isChecked()){
        QJsonObject root;

        // 1. org_patchevi sekcija
        QJsonArray orgPatchesArray;
        for(int j=0; j < slike[0].size(); j++){
            QJsonObject patch;
            patch["file_name"] = imeSlike + "_" + QString("Patch%1").arg(QString::number(j + 1).rightJustified(4, '0')) + "_" + naziv[0];
            patch["id"] = QString("%1").arg(QString::number(j + 1).rightJustified(4, '0'));
            patch["height"] = dimenzije[1];
            patch["width"] = dimenzije[0];
            patch["x_koor"] = koordinate[j][0];
            patch["y_koor"] = koordinate[j][1];

            orgPatchesArray.append(patch);

        }
        root["org_patches"] = orgPatchesArray;

        // 2. klase sekcija
        std::vector<QString> naziv_klase = naziviUAplikaciji;
        naziv_klase.erase(naziv_klase.begin() + 5); //brisanje gumice iz vektora naziva klasa
        QJsonArray klaseArray;
        for (int i = 0; i < 9; ++i) {
            QJsonObject klasa;
            klasa["class_id"] = i;
            klasa["name"] = naziv_klase[i];
            klaseArray.append(klasa);
        }
        root["classes"] = klaseArray;

        // 3. annotation sekcija
        QJsonArray annotationArray;
        for (int i = 0; i < slike[0].size(); ++i) {
            QJsonObject annotation;
            annotation["id"] = QString("Ann-%1").arg(i+1);
            annotation["patch_id"] = QString("%1").arg(QString::number(i + 1).rightJustified(4, '0'));
            if (includeMasks && includeMasks->isChecked()) {
                QJsonArray maskIdsArray;
                for (int j = 1; j <= 9; j++) {
                    QString maskId = QString("%1 - mask %2").arg(j).arg(QString("%1").arg(QString::number(i + 1).rightJustified(4, '0')));
                    maskIdsArray.append(maskId);
                }
                annotation["mask_ids"] = maskIdsArray;
            }


            // class_ocjene niz
            QJsonArray classOcjeneArray;
            classOcjeneArray.append(ocjene[0][i]);
            classOcjeneArray.append(ocjene[1][i]);
            classOcjeneArray.append(ocjene[2][i]);
            classOcjeneArray.append(ocjene[3][i]);
            classOcjeneArray.append(ocjene[4][i]);
            classOcjeneArray.append(ocjene[5][i]);
            classOcjeneArray.append(ocjene[6][i]);
            classOcjeneArray.append(ocjene[7][i]);
            classOcjeneArray.append(ocjene[8][i]);

            annotation["class_ratings_ids"] = classOcjeneArray;

            annotationArray.append(annotation);
        }
        root["annotation"] = annotationArray;

        // 4. masks sekcija (ako je potrebno)
        if (includeMasks && includeMasks->isChecked()) {
            QJsonArray masksArray;
            for (int j = 1; j<slike.size(); j++){
                for (int i = 0; i < slike[j].size(); i++) {
                    QJsonObject mask;
                    mask["file_name"] = imeSlike + "_" + QString("Patch%1").arg(QString::number(i + 1).rightJustified(4, '0')) + "_" + naziv[j];
                    mask["id"] =  QString("%1 - mask %2").arg(j).arg(QString("%1")).arg(QString::number(i + 1).rightJustified(4, '0'));
                    mask["patch_id"] = QString("%1").arg(QString::number(i + 1).rightJustified(4, '0'));
                    mask["rating_id"] = QString::number(ocjene[j-1][i]);
                    masksArray.append(mask);
                }
            }
            root["masks"] = masksArray;
        }

        QJsonArray ocjeneArray;
        QJsonObject ocjena1;
        ocjena1["rating_id"] = 0;
        ocjena1["name"] = "No presence";
        ocjeneArray.append(ocjena1);

        QJsonObject ocjena2;
        ocjena2["ocjena_id"] = 1;
        ocjena2["name"] = "Partial presence";
        ocjeneArray.append(ocjena2);

        QJsonObject ocjena3;
        ocjena3["ocjena_id"] = 2;
        ocjena3["name"] = "Full presence";
        ocjeneArray.append(ocjena3);

        root["ratings"] = ocjeneArray;

        // Kreiramo JSON dokument
        QJsonDocument jsonDoc(root);

        // Određujemo putanju do fajla koristeći patchesRootDirectory
        QString jsonFilePath = patchesRootDirectory + "/output.json";
        // Zapišemo JSON dokument u fajl
        QFile file(jsonFilePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(jsonDoc.toJson(QJsonDocument::JsonFormat())); // Koristi Indented za leže čitanje
            file.close();
        } else {
            qWarning("Could not open file for writing");
        }
    }
    else {
        // Kreiranje podfoldera "json" unutar glavnog direktorijuma
        QString jsonFolderPath = patchesRootDirectory + "/json";
        QDir().mkdir(jsonFolderPath);



        // Petlja kroz sve patcheve
        for (int i = 0; i < slike[0].size(); ++i) {
            QJsonObject root;

            // 1. org_patchevi sekcija (za ovaj patch)
            QJsonArray orgPatchesArray;
            QJsonObject patch;

            patch["file_name"] = imeSlike + "_" + QString("Patch%1").arg(QString::number(i + 1).rightJustified(4, '0')) + "_" + naziv[0];
            patch["id"] = QString("%1").arg(QString::number(i + 1).rightJustified(4, '0'));
            patch["height"] = dimenzije[1];
            patch["width"] = dimenzije[0];
            patch["x_koor"] = koordinate[i][0];
            patch["y_koor"] = koordinate[i][1];
            orgPatchesArray.append(patch);
            root["org_patches"] = orgPatchesArray;

            // 2. klase sekcija
            std::vector<QString> naziv_klase = naziviUAplikaciji;
            naziv_klase.erase(naziv_klase.begin() + 5); //brisanje gumice iz vektora naziva klasa
            QJsonArray klaseArray;
            for (int j = 0; j < 9; ++j) {
                QJsonObject klasa;
                klasa["class_id"] = j;
                klasa["name"] = naziv_klase[j];
                klaseArray.append(klasa);
            }
            root["classes"] = klaseArray;

            // 3. annotation sekcija (za ovaj patch)
            QJsonObject annotation;
            annotation["id"] = QString("Ann-%1").arg(i+1);
            annotation["patch_id"] = QString("%1").arg(QString::number(i + 1).rightJustified(4, '0'));

            if (includeMasks && includeMasks->isChecked()) {
                QJsonArray maskIdsArray;
                for (int j = 1; j <= 9; j++) {
                    QString maskId = QString("%1 - mask %2").arg(j).arg(QString("%1").arg(QString::number(i + 1).rightJustified(4, '0')));
                    maskIdsArray.append(maskId);
                }
                annotation["mask_ids"] = maskIdsArray;
            }

            // class_ocjene niz
            QJsonArray classOcjeneArray;
            for (int j = 0; j < 9; ++j) {
                classOcjeneArray.append(ocjene[j][i]);
            }
            annotation["class_ratings_ids"] = classOcjeneArray;

            QJsonArray annotationArray;
            annotationArray.append(annotation);
            root["annotation"] = annotationArray;

            // 4. masks sekcija (ako je potrebno)
            if (includeMasks && includeMasks->isChecked()) {
                QJsonArray masksArray;
                for (int j = 1; j < slike.size(); j++) {
                    QJsonObject mask;
                    mask["file_name"] = imeSlike + "_" + QString("Patch%1").arg(QString::number(i + 1).rightJustified(4, '0')) + "_" + "Mask " + naziv[j];
                    mask["id"] = QString("%1 - mask %2").arg(j).arg(QString("%1")).arg(QString::number(i + 1).rightJustified(4, '0'));
                    mask["patch_id"] = QString("%1").arg(QString::number(i + 1).rightJustified(4, '0'));
                    mask["rating_id"] = QString::number(ocjene[j - 1][i]);
                    masksArray.append(mask);
                }
                root["masks"] = masksArray;
            }

            // Ocjene sekcija
            QJsonArray ocjeneArray;
            QJsonObject ocjena1;
            ocjena1["ocjena_id"] = 0;
            ocjena1["name"] = "No presence";
            ocjeneArray.append(ocjena1);

            QJsonObject ocjena2;
            ocjena2["ocjena_id"] = 1;
            ocjena2["name"] = "Partial presence";
            ocjeneArray.append(ocjena2);

            QJsonObject ocjena3;
            ocjena3["ocjena_id"] = 2;
            ocjena3["name"] = "Full presence";
            ocjeneArray.append(ocjena3);

            root["ratings"] = ocjeneArray;

            // Kreiramo JSON dokument
            QJsonDocument jsonDoc(root);

            // Određujemo putanju do fajla koristeći patchesRootDirectory
            QString jsonFileName = QString("output_%1.json").arg(QString::number(i + 1).rightJustified(4, '0'));
            QString jsonFilePath = jsonFolderPath + "/" + jsonFileName;

            // Zapišemo JSON dokument u fajl
            QFile file(jsonFilePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(jsonDoc.toJson(QJsonDocument::Indented)); // Koristi Indented za lakše čitanje
                file.close();
            } else {
                qWarning("Could not open file for writing");
            }
        }

    }
}

void eksportujPojedinePatcheve(QWidget* window6, QCheckBox* checkBoxDa, QCheckBox* checkBoxNe, QCheckBox* checkBoxKolektivni){
    std::vector<std::vector<int>> trazene_ocjene(9, std::vector<int>(3, 3)), ocjene;
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

    std::vector<cv::Mat> spaseniPatchevi, spasenaMaskaD1, spasenaMaskaD2, spasenaMaskaD3, spasenaMaskaD4, spasenaMaskaD5, spasenaMaskaRub,
        spasenaMaskaPodloga, spasenaMaskaIspravno, spasenaMaskaKoza;

    std::vector<int> spasenaOcjenaD1, spasenaOcjenaD2, spasenaOcjenaD3, spasenaOcjenaD4, spasenaOcjenaD5, spasenaOcjenaRub, spasenaOcjenaPodloga,
        spasenaOcjenaIspravno, spasenaOcjenaKoza;

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

        spasenaOcjenaD1.push_back(ocjene_d1[spasi[i]]);
        spasenaOcjenaD2.push_back(ocjene_d2[spasi[i]]);
        spasenaOcjenaD3.push_back(ocjene_d3[spasi[i]]);
        spasenaOcjenaD4.push_back(ocjene_d4[spasi[i]]);
        spasenaOcjenaD5.push_back(ocjene_d5[spasi[i]]);
        spasenaOcjenaRub.push_back(ocjene_rub[spasi[i]]);
        spasenaOcjenaPodloga.push_back(ocjene_podloga[spasi[i]]);
        spasenaOcjenaIspravno.push_back(ocjene_ispravno[spasi[i]]);
        spasenaOcjenaKoza.push_back(ocjene_koza[spasi[i]]);

    }

    std::vector<std::vector<cv::Mat>> spaseni, slikeJson;
    std::vector<QString> nazivi;
    if (checkBoxNe && checkBoxNe->isChecked()) {
        spaseni = {spaseniPatchevi};
        nazivi = {"Original"};
    } else if (checkBoxDa && checkBoxDa->isChecked()) {
        spaseni = {spaseniPatchevi, spasenaMaskaD1, spasenaMaskaD2, spasenaMaskaD3, spasenaMaskaD4, spasenaMaskaD5,
                   spasenaMaskaRub, spasenaMaskaPodloga, spasenaMaskaIspravno, spasenaMaskaKoza};
        nazivi = {"Original", "Mask "+naziviUAplikaciji[0], "Mask "+naziviUAplikaciji[1], "Mask "+naziviUAplikaciji[2],
                  "Mask "+naziviUAplikaciji[3], "Mask "+naziviUAplikaciji[4], "Mask "+naziviUAplikaciji[6],
                  "Mask "+naziviUAplikaciji[7], "Mask "+naziviUAplikaciji[8], "Mask "+naziviUAplikaciji[9]};
    }
    slikeJson = spaseni;
    ocjene = {spasenaOcjenaD1, spasenaOcjenaD2, spasenaOcjenaD3, spasenaOcjenaD4, spasenaOcjenaD5, spasenaOcjenaRub, spasenaOcjenaPodloga,
              spasenaOcjenaIspravno, spasenaOcjenaKoza};

    if(!spaseniPatchevi.empty()){

        spasiPatcheve(spaseni, window6, nazivi);
        exportJson(slikeJson, nazivi, checkBoxDa, ocjene, checkBoxKolektivni);
    }
    spasi.clear();
}

void updateUI(QToolButton *markerButton1, QToolButton *markerButton2, QToolButton *markerButton3, QToolButton *markerButton4,
              QToolButton *markerButton8, QToolButton *markerButton5, QToolButton *markerButton6, QToolButton *gumica,
              std::vector<QLineEdit*> textboxes) {
    markerButton1->setToolTip(naziviUAplikaciji[0]);
    markerButton2->setToolTip(naziviUAplikaciji[1]);
    markerButton3->setToolTip(naziviUAplikaciji[2]);
    markerButton4->setToolTip(naziviUAplikaciji[3]);
    markerButton8->setToolTip(naziviUAplikaciji[4]);
    markerButton5->setText(naziviUAplikaciji[6]);
    markerButton6->setText(naziviUAplikaciji[7]);
    gumica->setText(naziviUAplikaciji[5]);

    for(int i=0; i<4; i++){
        textboxes[i]->setText(QString::number(dimenzije[i]));
    }
}



int main(int argc, char *argv[]) {

    QApplication app(argc, argv);

    QString configFileName = "config.txt"; //PRILIKOM RELEASA APLIKACIJE UKINUTI OVO RELEASE IZ PUTANJE

    // Čitanje konfiguracijskog fajla
    readConfig(configFileName);

    QWidget mainWindow;
    QIcon logoMini(":/ikonice/logo-mini.png");
    mainWindow.setWindowTitle("DefectDetect");
    mainWindow.setWindowIcon(logoMini);
    QVBoxLayout *layout = new QVBoxLayout(&mainWindow);

    QLabel *imageLabel = new QLabel(&mainWindow);
    QPixmap pixmap(":/ikonice/logo.png");
    imageLabel->setPixmap(pixmap);
    imageLabel->setScaledContents(true);
    layout->addWidget(imageLabel);

    QWidget toolsWindow;
    toolsWindow.setWindowTitle("DefectDetect");
    toolsWindow.setWindowIcon(logoMini);
    QVBoxLayout *layout_alatna_traka = new QVBoxLayout(&toolsWindow);

    //Kreiranje dugmeta za učitavanje slike
    QPushButton *buttonOdabirSlike = new QPushButton("Load Image", &mainWindow);
    layout->addWidget(buttonOdabirSlike);
    QObject::connect(buttonOdabirSlike, &QPushButton::clicked, [&]() {
        onButtonClick(&mainWindow, &toolsWindow,1);
    });


    QPushButton *buttonOdabirFoldera = new QPushButton("Load Multiple Images", &mainWindow);
    layout->addWidget(buttonOdabirFoldera);
    QObject::connect(buttonOdabirFoldera, &QPushButton::clicked, [&]() {
        onButtonClick(&mainWindow, &toolsWindow,3);
    });

    QPushButton *buttonUcitajSesiju = new QPushButton("Load Session", &mainWindow);
    layout->addWidget(buttonUcitajSesiju);
    QObject::connect(buttonUcitajSesiju, &QPushButton::clicked, [&]() {
        onButtonClick(&mainWindow, &toolsWindow,2);
    });

    QHBoxLayout *markerLayout = new QHBoxLayout;
    layout_alatna_traka->addLayout(markerLayout);

    QToolButton *marker1 = new QToolButton(&toolsWindow);
    marker1->setIcon(QIcon(":/ikonice/1.png"));
    marker1->setToolTip(naziviUAplikaciji[0]);
    //markerButton1->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    markerLayout->addWidget(marker1);

    QObject::connect(marker1, &QToolButton::clicked, [&]() {
        onMarkerClick(1);
    });

    QToolButton *marker2 = new QToolButton(&toolsWindow);
    marker2->setIcon(QIcon(":/ikonice/2.png"));
    marker2->setToolTip(naziviUAplikaciji[1]);
    //markerButton2->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    markerLayout->addWidget(marker2);

    QObject::connect(marker2, &QToolButton::clicked, [&]() {
        onMarkerClick(2);
    });

    QToolButton *marker3 = new QToolButton(&toolsWindow);
    marker3->setIcon(QIcon(":/ikonice/3.png"));
    marker3->setToolTip(naziviUAplikaciji[2]);
    //markerButton3->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    markerLayout->addWidget(marker3);

    QObject::connect(marker3, &QToolButton::clicked, [&]() {
        onMarkerClick(3);
    });

    QToolButton *marker4 = new QToolButton(&toolsWindow);
    marker4->setIcon(QIcon(":/ikonice/4.png"));
    marker4->setToolTip(naziviUAplikaciji[3]);
    //markerButton4->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    markerLayout->addWidget(marker4);

    QObject::connect(marker4, &QToolButton::clicked, [&]() {
        onMarkerClick(4);
    });

    QToolButton *marker5 = new QToolButton(&toolsWindow);
    marker5->setIcon(QIcon(":/ikonice/8.png"));
    marker5->setToolTip(naziviUAplikaciji[4]);
    //markerButton8->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    markerLayout->addWidget(marker5);

    QObject::connect(marker5, &QToolButton::clicked, [&]() {
        onMarkerClick(8);
    });

    QToolButton *markerRub = new QToolButton(&toolsWindow);
    markerRub->setIcon(QIcon(":/ikonice/5.png"));
    markerRub->setText(naziviUAplikaciji[6]);
    markerRub->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    markerLayout->addWidget(markerRub);

    QObject::connect(markerRub, &QToolButton::clicked, [&]() {
        onMarkerClick(5);
    });

    QToolButton *markerPodloga = new QToolButton(&toolsWindow);
    markerPodloga->setIcon(QIcon(":/ikonice/6.png"));
    markerPodloga->setText(naziviUAplikaciji[7]);
    markerPodloga->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    markerLayout->addWidget(markerPodloga);

    QObject::connect(markerPodloga, &QToolButton::clicked, [&]() {
        onMarkerClick(6);
    });

    QToolButton *gumica = new QToolButton(&toolsWindow);
    gumica->setIcon(QIcon(":/ikonice/7.png"));
    gumica->setText(naziviUAplikaciji[5]);
    gumica->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    markerLayout->addWidget(gumica);

    QObject::connect(gumica, &QToolButton::clicked, [&]() {
        onMarkerClick(7);
    });


    //POSTAVKE
    QWidget settingsWindow;
    settingsWindow.setWindowTitle("Settings");
    settingsWindow.setWindowIcon(logoMini);

    QVBoxLayout *layout_postavke = new QVBoxLayout(&settingsWindow);

    QTabWidget *tabWidget = new QTabWidget(&settingsWindow);
    layout_postavke->addWidget(tabWidget);


    // Prvi tab - Nazivi
    QWidget *tabNazivi = new QWidget();
    QFormLayout *naziviLayout = new QFormLayout(tabNazivi);
    naziviLayout->addRow("Marker 1 name:", new QLineEdit(naziviUAplikaciji[0]));
    naziviLayout->addRow("Marker 2 name:", new QLineEdit(naziviUAplikaciji[1]));
    naziviLayout->addRow("Marker 3 name:", new QLineEdit(naziviUAplikaciji[2]));
    naziviLayout->addRow("Marker 4 name:", new QLineEdit(naziviUAplikaciji[3]));
    naziviLayout->addRow("Marker 5 name:", new QLineEdit(naziviUAplikaciji[4]));
    naziviLayout->addRow("Eraser name:", new QLineEdit(naziviUAplikaciji[5]));
    naziviLayout->addRow("Edge name:", new QLineEdit(naziviUAplikaciji[6]));
    naziviLayout->addRow("Surface name:", new QLineEdit(naziviUAplikaciji[7]));
    naziviLayout->addRow("Class Correct name:", new QLineEdit(naziviUAplikaciji[8]));
    naziviLayout->addRow("Class Leather name:", new QLineEdit(naziviUAplikaciji[9]));
    tabWidget->addTab(tabNazivi, "Names");

    // Drugi tab - Patchevi
    QWidget *tabPatchevi = new QWidget();
    QFormLayout *patcheviLayout = new QFormLayout(tabPatchevi);

    // Dodavanje QLineEdit sa QDoubleValidator za ograničenje unosa na float brojeve
    QLineEdit *sirinaEdit = new QLineEdit(QString::number(dimenzije[0]));
    QLineEdit *visinaEdit = new QLineEdit(QString::number(dimenzije[1]));
    QLineEdit *hStrideEdit = new QLineEdit(QString::number(dimenzije[2]));
    QLineEdit *vStrideEdit = new QLineEdit(QString::number(dimenzije[3]));

    QDoubleValidator *floatValidator = new QDoubleValidator(0, 100, 2); // Minimum 0, Maximum 100, 2 decimale
    floatValidator->setNotation(QDoubleValidator::StandardNotation);

    sirinaEdit->setValidator(floatValidator);
    visinaEdit->setValidator(floatValidator);
    hStrideEdit->setValidator(floatValidator);
    vStrideEdit->setValidator(floatValidator);

    patcheviLayout->addRow("Width:", sirinaEdit);
    patcheviLayout->addRow("Height:", visinaEdit);
    patcheviLayout->addRow("Horizontal stride:", hStrideEdit);
    patcheviLayout->addRow("Vertical stride:", vStrideEdit);

    tabWidget->addTab(tabPatchevi, "Patches");

    // Treći tab - Ostalo
    QWidget *tabOstalo = new QWidget();
    QFormLayout *ostaloLayout = new QFormLayout(tabOstalo);

    // Dodavanje dropdown menija za odabir debljine markera
    QComboBox *markerThicknessComboBox = new QComboBox();
    for (int i = 10; i <= 70; i += 5) {
        markerThicknessComboBox->addItem(QString::number(i));
    }

    markerThicknessComboBox->setCurrentText(QString::number(velicinaMarkera));

    QLineEdit *dozvoljenoOdstupanjeZaOcjenu1 = new QLineEdit();
    QIntValidator *validator = new QIntValidator(1, 100);  // Postavljamo minimum na 1 i maksimum na 100
    dozvoljenoOdstupanjeZaOcjenu1->setValidator(validator);

    // Postavi inicijalnu vrednost
    dozvoljenoOdstupanjeZaOcjenu1->setText(QString::number(odstupanjeZaOcjenu1));

    //POSTAVLJANJE DEBLJINE MARKERA
    QLabel *labelDebljina = new QLabel("Thickness:", &toolsWindow);
    markerLayout->addWidget(labelDebljina);

    QToolButton *debljina1 = new QToolButton(&toolsWindow);
    debljina1->setIcon(QIcon(":/ikonice/najtanji.png"));
    debljina1->setToolTip("10");
    markerLayout->addWidget(debljina1);

    QObject::connect(debljina1, &QToolButton::clicked, [&]() {
        velicinaMarkera = 10;
        markerThicknessComboBox->setCurrentText(QString::number(velicinaMarkera));
        writeConfig(configFileName, velicinaMarkera, naziviUAplikaciji, dimenzije, odstupanjeZaOcjenu1);
    });
    ostaloLayout->addRow("Marker Thickness:", markerThicknessComboBox);
    QLabel *label = new QLabel("Setting tolerance to assign rating 1 to patch.<br>"
                               "This tolerance represents the percentage of the patch <br>"
                               "covered by the annotation in relation to its surface.<br>"
                               "It also limits the minimum presence of the annotation <br>"
                               "for assigning a rating of 1.");
    label->setWordWrap(true);  // Da osiguraš prelamanje linija kada je potrebno

    ostaloLayout->addRow(label);
    ostaloLayout->addRow("Choose this ratio as a percentage between 1 and 100.", dozvoljenoOdstupanjeZaOcjenu1);


    tabWidget->addTab(tabOstalo, "Other");


    QToolButton *debljina2 = new QToolButton(&toolsWindow);
    debljina2->setIcon(QIcon(":/ikonice/srednji.png"));
    debljina2->setToolTip("20");
    markerLayout->addWidget(debljina2);

    QObject::connect(debljina2, &QToolButton::clicked, [&]() {
        velicinaMarkera = 20;
        markerThicknessComboBox->setCurrentText(QString::number(velicinaMarkera));
        writeConfig(configFileName, velicinaMarkera, naziviUAplikaciji, dimenzije, odstupanjeZaOcjenu1);
    });

    QToolButton *debljina3 = new QToolButton(&toolsWindow);
    debljina3->setIcon(QIcon(":/ikonice/najdeblji.png"));
    debljina3->setToolTip("30");
    markerLayout->addWidget(debljina3);

    QObject::connect(debljina3, &QToolButton::clicked, [&]() {
        velicinaMarkera = 30;
        markerThicknessComboBox->setCurrentText(QString::number(velicinaMarkera));
        writeConfig(configFileName, velicinaMarkera, naziviUAplikaciji, dimenzije, odstupanjeZaOcjenu1);
    });

    QHBoxLayout *layout_kraj = new QHBoxLayout;
    layout_alatna_traka->addLayout(layout_kraj);
    //PRIVREMENO UKLANJANJE ANOTACIJA
    QPushButton *privremeniPregled = new QPushButton("Remove Annotations", &toolsWindow);
    layout_kraj->addWidget(privremeniPregled);
    toolsWindow.setLayout(layout_kraj);

    QObject::connect(privremeniPregled, &QPushButton::pressed, [&]() {
        //cv::Mat originalna_slika = cv::imread(putanja); //prikaz originala dok je dugme pritisnuto
        cv::imshow("Main View", originalnaSlika);
    });

    QObject::connect(privremeniPregled, &QPushButton::released, [&]() {
        cv::imshow("Main View", slika); // prikaz anotacija
    });

    /*----------------------------------------------------------------------------//
    //EVENTUALNA IZMJENA AKO TREBA DA SE JEDNIM KLIKOM UKLONE, PA VRATE ANOTACIJE
    //----------------------------------------------------------------------------//

        QPushButton *vratiAnotacije = new QPushButton("Prikaži anotacije", &window2);
    layout_alatna_traka->addWidget(vratiAnotacije);
    window2.setLayout(layout_alatna_traka);

    QObject::connect(vratiAnotacije, &QPushButton::clicked, [&]() {
        //slika = cv::imread(putanja); // Učitajte sliku ponovno (morate imati putanju do originalne slike)
        cv::imshow("Glavni pregled", slika); // Prikaz originalne slike
    });*/

    //PROZOR UPOZORENJA ZA BRISANJE SVIH ANOTACIJA
    QWidget warningWindow;
    warningWindow.setWindowTitle("Deleting Annotations");
    warningWindow.setWindowIcon(logoMini);

    QVBoxLayout *layout_brisanje_anotacija = new QVBoxLayout(&warningWindow);
    warningWindow.setLayout(layout_brisanje_anotacija);

    QLabel *warningLabel = new QLabel("Are you sure you want to delete all annotations?", &warningWindow);
    warningLabel->setAlignment(Qt::AlignCenter); // Centriranje teksta
    layout_brisanje_anotacija->addWidget(warningLabel);

    QPushButton *Da = new QPushButton("Yes", &warningWindow);
    layout_brisanje_anotacija->addWidget(Da);

    QObject::connect(Da, &QPushButton::clicked, [&]() {
        slika = originalnaSlika.clone();
        cv::imshow("Main View", slika);
        warningWindow.close();
    });

    QPushButton *Ne = new QPushButton("No", &warningWindow);
    layout_brisanje_anotacija->addWidget(Ne);

    QObject::connect(Ne, &QPushButton::clicked, [&]() {
        warningWindow.close();
    });

    QPushButton *obrisiSve = new QPushButton("Delete Annotations...", &toolsWindow);
    layout_kraj->addWidget(obrisiSve);

    QObject::connect(obrisiSve, &QPushButton::clicked, [&]() {
        warningWindow.show();
    });

    //BUTTON ZA OZNAKU KRAJA ANOTACIJE, KAKO BI SE MOGLE KREIRATI MASKE I PATCHEVI
    QWidget endAnnWindow;
    endAnnWindow.setWindowTitle("End of annotating");
    endAnnWindow.setWindowIcon(logoMini);

    QVBoxLayout *layout_kraj_anotacija = new QVBoxLayout(&endAnnWindow);
    endAnnWindow.setLayout(layout_kraj_anotacija);

    QPushButton *krajAnotacije = new QPushButton("End Annotation...", &toolsWindow);
    layout_kraj->addWidget(krajAnotacije);

    std::vector<cv::Mat> maske;

    QObject::connect(krajAnotacije, &QPushButton::clicked, [&]() {
        endAnnWindow.show();
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
        koordinate.clear();
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


    QPushButton *prikazMaski = new QPushButton("Show Masks", &endAnnWindow);
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
    QWidget savedWindow;
    savedWindow.setWindowTitle("Saving");
    savedWindow.setWindowIcon(logoMini);

    QVBoxLayout *layout_spasiSlike = new QVBoxLayout(&savedWindow);

    QLabel *savedLabel = new QLabel("Image is saved.", &savedWindow);
    savedLabel->setAlignment(Qt::AlignCenter); // Centriranje teksta
    layout_spasiSlike->addWidget(savedLabel);

    //PROZOR ZA UNOS DIMENZIJA PATCHA
    QWidget patchDimWindow;
    patchDimWindow.setWindowTitle("Creating patches");
    patchDimWindow.setWindowIcon(logoMini);

    QVBoxLayout *layout_patchevi = new QVBoxLayout(&patchDimWindow);
    patchDimWindow.setLayout(layout_patchevi);

    std::vector<QLineEdit*> textboxes;
    for (int i = 0; i < 4; ++i) {
        std::vector<QString> nazivi = {"Width", "Height", "Horizontal stride", "Vertical stride"};
        QString label_text =  nazivi[i] + ":";
        QLabel *label = new QLabel(label_text, &patchDimWindow);
        layout_patchevi->addWidget(label);

        // Kreirajte textbox za unos cijelih brojeva
        QLineEdit *textbox = new QLineEdit(&patchDimWindow);
        textbox->setValidator(new QIntValidator(textbox));
        layout_patchevi->addWidget(textbox);
        textboxes.push_back(textbox);
        textbox->setText(QString::number(dimenzije[i]));
    }
    QHBoxLayout *layout_patchevi2 = new QHBoxLayout(&patchDimWindow);
    layout_patchevi->addLayout(layout_patchevi2);

    std::vector<std::vector<int>> ocjene;


    QPushButton *izradiPatch = new QPushButton("Create", &patchDimWindow);
    layout_patchevi2->addWidget(izradiPatch);


    QObject::connect(izradiPatch, &QPushButton::clicked, [&]() {
        ocjene = kreirajPatch(textboxes, maske, &patchDimWindow);
    });

    QPushButton *patcheviButton = new QPushButton("Create Patches...", &endAnnWindow);
    layout_kraj_anotacija->addWidget(patcheviButton);

    QObject::connect(patcheviButton, &QPushButton::clicked, [&]() {
        patchDimWindow.show();
    });

    QPushButton *patcheviExport = new QPushButton("Export patches...", &endAnnWindow);
    layout_kraj_anotacija->addWidget(patcheviExport);


    QWidget chooseExpWindow;
    chooseExpWindow.setWindowTitle("Export type selection");
    chooseExpWindow.setWindowIcon(logoMini);

    QVBoxLayout *layout_odabir_eksporta = new QVBoxLayout(&chooseExpWindow);
    chooseExpWindow.setLayout(layout_odabir_eksporta);

    QPushButton *eksportSvih = new QPushButton("Export all patches", &chooseExpWindow);
    layout_odabir_eksporta->addWidget(eksportSvih);

    QPushButton *eksportPojedinih = new QPushButton("Export selected patches...", &chooseExpWindow);
    layout_odabir_eksporta->addWidget(eksportPojedinih);

    //PROZOR ZA EXPORT PATCHEVA PREKO OCJENA
    QWidget gradesWindow;
    gradesWindow.setWindowTitle("Exporting patches");
    gradesWindow.setWindowIcon(logoMini);

    QVBoxLayout *layout_export_glavni = new QVBoxLayout(&gradesWindow);
    gradesWindow.setLayout(layout_export_glavni);

    QCheckBox *checkBoxDa = nullptr;
    QCheckBox *checkBoxNe = nullptr;

    auto dodajEksportMaskeCheckBox = [&]() {
        QHBoxLayout *layout = new QHBoxLayout;
        QWidget *widget = new QWidget(&mainWindow); // Kontejner za checkbox i tekst
        QVBoxLayout *innerLayout = new QVBoxLayout(widget); // Layout unutar kontejnera

        QLabel *label = new QLabel("Mask export:", widget);
        label->setAlignment(Qt::AlignLeft);
        innerLayout->addWidget(label);

        QHBoxLayout *checkboxLayout = new QHBoxLayout;
        checkBoxDa = new QCheckBox("Yes", widget);
        checkBoxNe = new QCheckBox("No", widget);

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

    QCheckBox *checkBoxKolektivni = nullptr;
    QCheckBox *checkBoxIndividualni = nullptr;

    auto dodajEksportJsonCheckBox = [&]() {
        QHBoxLayout *layout = new QHBoxLayout;
        QWidget *widget = new QWidget(&mainWindow); // Kontejner za checkbox i tekst
        QVBoxLayout *innerLayout = new QVBoxLayout(widget); // Layout unutar kontejnera

        QLabel *label = new QLabel("JSON file export:", widget);
        label->setAlignment(Qt::AlignLeft);
        innerLayout->addWidget(label);

        QHBoxLayout *checkboxLayout = new QHBoxLayout;
        checkBoxKolektivni = new QCheckBox("Collective", widget);
        checkBoxIndividualni = new QCheckBox("Individual", widget);

        checkBoxKolektivni->setChecked(true);

        QObject::connect(checkBoxKolektivni, &QCheckBox::toggled, [checkBoxIndividualni](bool checked){
            if (checked) {
                checkBoxIndividualni->setChecked(false);
            }
        });

        QObject::connect(checkBoxIndividualni, &QCheckBox::toggled, [checkBoxKolektivni](bool checked){
            if (checked) {
                checkBoxKolektivni->setChecked(false);
            }
        });

        checkboxLayout->addWidget(checkBoxKolektivni);
        checkboxLayout->addWidget(checkBoxIndividualni);

        innerLayout->addLayout(checkboxLayout);
        layout->addWidget(widget);
        layout_odabir_eksporta->addLayout(layout);
    };

    // Pozovite funkciju
    dodajEksportJsonCheckBox();

    QObject::connect(patcheviExport, &QPushButton::clicked, [&]() {
       chooseExpWindow.show();
    });

    QObject::connect(eksportSvih, &QPushButton::clicked, [&]() {
        std::vector<std::vector<cv::Mat>> spasi;
        std::vector<QString> nazivi;
        if (checkBoxNe && checkBoxNe->isChecked()) {
            spasi = {patchevi};
            nazivi = {"Original"};
        } else if (checkBoxDa && checkBoxDa->isChecked()) {
            spasi = {patchevi, patchevi_d1, patchevi_d2, patchevi_d3, patchevi_d4, patchevi_d5, patchevi_rub, patchevi_podloga,
                                                                   patchevi_ispravno, patchevi_koza};
            nazivi = {"Original", "Mask "+naziviUAplikaciji[0], "Mask "+naziviUAplikaciji[1], "Mask "+naziviUAplikaciji[2],
                      "Mask "+naziviUAplikaciji[3], "Mask "+naziviUAplikaciji[4], "Mask "+naziviUAplikaciji[6],
                      "Mask "+naziviUAplikaciji[7], "Mask "+naziviUAplikaciji[8], "Mask "+naziviUAplikaciji[9]};
        }

        std::vector<std::vector<cv::Mat>> slikeJson = spasi;

        if(!spasi.empty())
        {
            std::vector<std::vector<int>> ocjene = {ocjene_d1, ocjene_d2, ocjene_d3, ocjene_d4, ocjene_d5, ocjene_rub, ocjene_podloga, ocjene_ispravno, ocjene_koza};
            spasiPatcheve(spasi, &chooseExpWindow, nazivi);
            exportJson(slikeJson, nazivi, checkBoxDa, ocjene, checkBoxKolektivni);

        }
        chooseExpWindow.close();
    });

    QObject::connect(eksportPojedinih, &QPushButton::clicked, [&]() {
        gradesWindow.show();
    });

    /*QObject::connect(patcheviExport, &QPushButton::clicked, [&]() {
        window6.show();
    });*/

    auto dodajCheckBoxove = [&](const QString& tekst, QWidget* eksport) {
        QHBoxLayout *layout = new QHBoxLayout;
        QWidget *widget = new QWidget(&mainWindow); // Kontejner za checkbox i tekst
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

    QPushButton *eksport = new QPushButton("Export");

    std::vector<QString> tekstovi = {"Choose ratings for "+naziviUAplikaciji[0], "Choose ratings for "+naziviUAplikaciji[1],
                                     "Choose ratings for "+naziviUAplikaciji[2], "Choose ratings for "+naziviUAplikaciji[3],
                                     "Choose ratings for "+naziviUAplikaciji[4], "Choose ratings for "+naziviUAplikaciji[6],
                                     "Choose ratings for "+naziviUAplikaciji[7], "Choose ratings for "+naziviUAplikaciji[8],
                                     "Choose ratings for "+naziviUAplikaciji[9]};

    for (int i=0; i<tekstovi.size(); i++){
        dodajCheckBoxove(tekstovi[i], eksport);
    }

    QObject::connect(eksport, &QPushButton::clicked, [&]() {
        eksportujPojedinePatcheve(&gradesWindow, checkBoxDa, checkBoxNe, checkBoxKolektivni);
        gradesWindow.close();
        chooseExpWindow.close();
    });

    QPushButton *spasi_sliku = new QPushButton("Save image", &endAnnWindow);
    layout_kraj_anotacija->addWidget(spasi_sliku);

    QObject::connect(spasi_sliku, &QPushButton::clicked, [&]() {
        sveSlike.push_back(slika);
        std::vector<cv::Mat>& referenca_slike = sveSlike;
        spasiSlike(referenca_slike, &toolsWindow, &savedWindow);
    });

    // Dodavanje donjeg layout-a za dugmad
    QHBoxLayout *bottomLayout = new QHBoxLayout();

    // Dugme "Default" u donjem levom uglu
    QPushButton *defaultButton = new QPushButton("Default");
    bottomLayout->addWidget(defaultButton, 0, Qt::AlignLeft);
    bottomLayout->addStretch();

    // Dugme "Spasi" u donjem desnom uglu
    QPushButton *spasiButton = new QPushButton("Save");
    bottomLayout->addWidget(spasiButton, 0, Qt::AlignRight);

    // Povezivanje signala sa slotovima (ovde ćeš dodati šta treba da se desi)
    QObject::connect(spasiButton, &QPushButton::clicked, [&]() {
        // Prikupljanje vrednosti iz tabova i ažuriranje varijabli
        velicinaMarkera = markerThicknessComboBox->currentText().toInt();
        odstupanjeZaOcjenu1 = dozvoljenoOdstupanjeZaOcjenu1->text().toInt();

        // Ažuriranje naziva markera
        for (int i=0; i<10; i++){
            naziviUAplikaciji[i] = naziviLayout->itemAt(i, QFormLayout::FieldRole)->widget()->property("text").toString();
        }

        // Ažuriranje dimenzija
        dimenzije[0] = sirinaEdit->text().toDouble();
        dimenzije[1] = visinaEdit->text().toDouble();
        dimenzije[2] = hStrideEdit->text().toDouble();
        dimenzije[3] = vStrideEdit->text().toDouble();

        writeConfig(configFileName, velicinaMarkera, naziviUAplikaciji, dimenzije, odstupanjeZaOcjenu1);
        updateUI(marker1, marker2, marker3, marker4, marker5, markerRub, markerPodloga, gumica, textboxes);
    });


    QObject::connect(defaultButton, &QPushButton::clicked, [&]() {
        dimenzije = defaultDimenzije;
        velicinaMarkera = defaultVelicinaMarkera;
        naziviUAplikaciji = defaultNaziviUAplikaciji;
        odstupanjeZaOcjenu1 = defaultOdstupanje;
        // Ažuriranje QLineEdit-ova u tabu "Patchevi"
        sirinaEdit->setText(QString::number(defaultDimenzije[0]));
        visinaEdit->setText(QString::number(defaultDimenzije[1]));
        hStrideEdit->setText(QString::number(defaultDimenzije[2]));
        vStrideEdit->setText(QString::number(defaultDimenzije[3]));

        // Ažuriranje QLineEdit-ova u tabu "Nazivi"
        for (int i=0; i<10; i++){
            naziviLayout->itemAt(i, QFormLayout::FieldRole)->widget()->setProperty("text", defaultNaziviUAplikaciji[i]);
        }
        // Ažuriranje QComboBox-a u tabu "Ostalo"
        markerThicknessComboBox->setCurrentText(QString::number(defaultVelicinaMarkera));
        dozvoljenoOdstupanjeZaOcjenu1->setText(QString::number(defaultOdstupanje));

        writeConfig(configFileName, velicinaMarkera, naziviUAplikaciji, dimenzije, odstupanjeZaOcjenu1);
        updateUI(marker1, marker2, marker3, marker4, marker5, markerRub, markerPodloga, gumica, textboxes);

    });

    // Dodavanje donjeg layout-a u glavni layout prozora
    layout_postavke->addLayout(bottomLayout);

    // Dodaj QTabWidget u layout prozora za postavke

    QPushButton *postavke = new QPushButton("Settings", &mainWindow);
    layout->addWidget(postavke);
    QObject::connect(postavke, &QPushButton::clicked, [&]() {
        settingsWindow.show();
    });


    QPushButton *krajPrograma = new QPushButton("End Session", &toolsWindow);
    layout_kraj->addWidget(krajPrograma);

    QObject::connect(krajPrograma, &QPushButton::clicked, [&]() {
        QWidgetList allWidgets = qApp->topLevelWidgets();
        for (int i = 0; i < allWidgets.size(); ++i) {
            allWidgets.at(i)->close();
        }
        cv::destroyAllWindows();
    });

    QHBoxLayout *layout_iteration = new QHBoxLayout;
    layout_alatna_traka->addLayout(layout_iteration);

    QToolButton *previous = new QToolButton(&toolsWindow);
    previous->setText("Previous");
    previous->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    layout_iteration->addWidget(previous);

    QObject::connect(previous, &QToolButton::clicked, [&]() {
        if (trenutnaSlikaIndex > 0) {
            trenutnaSlikaIndex--;
            loadImage(trenutnaSlikaIndex);
        }
    });

    QToolButton *next = new QToolButton(&toolsWindow);
    next->setText("Next");
    next->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    layout_iteration->addWidget(next);

    QObject::connect(next, &QToolButton::clicked, [&]() {
        if (trenutnaSlikaIndex < fileList.size() - 1) {
            trenutnaSlikaIndex++;
            loadImage(trenutnaSlikaIndex);
        }
    });


    mainWindow.show();
    cv::waitKey(0);
    return app.exec();
}
