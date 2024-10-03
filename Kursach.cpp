#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <regex>
#include <sstream>

using namespace std;

// Класс для обработки данных сенсоров
class SensorDataProcessor {
private:
    ifstream inputFile;                     // Поток для чтения входного файла
    map<string, ofstream> sensorFiles;      // Словарь файлов для каждого датчика
    string currentDateTime;                 // Текущая дата и время

    // Приватная функция для преобразования шестнадцатеричной строки в число с плавающей точкой
    float hexToFloat(const string& hexStr) {
        uint32_t num;
        stringstream ss;
        ss << hex << hexStr;
        ss >> num;

        float result;
        memcpy(&result, &num, sizeof(result));

        return result;
    }

public:
    // Конструктор, открывает входной файл для чтения
    SensorDataProcessor(const string& inputFilename) {
        inputFile.open(inputFilename, ios::in);
    }

    // Деструктор, закрывает все открытые файлы
    ~SensorDataProcessor() {
        inputFile.close();
        for (auto& pair : sensorFiles) {
            pair.second.close();
        }
    }

    // Проверяет, является ли строка датой и временем в формате "YYYYMMDD-hh:mm:ss"
    bool isDateTime(const string& line) {
        regex dateTimePattern("\\d{8}-\\d{2}:\\d{2}:\\d{2}");
        return regex_match(line, dateTimePattern);
    }

    // Обрабатывает данные сенсоров
    void processSensorData() {
        string line;

        // Построчное чтение данных из входного файла
        while (getline(inputFile, line)) {
            // Если строка содержит дату и время
            if (isDateTime(line)) {
                currentDateTime = line;
                // Записываем текущую дату и время во все открытые файлы датчиков
                for (auto& pair : sensorFiles) {
                    pair.second << currentDateTime << '\n';
                }
            }
            // Если строка начинается с символа '#', то это данные от датчика
            else if (line[0] == '#') {
                // Выделяем номер датчика из строки
                string sensorNumber = line.substr(1, line.find(' ') - 1);
                // Если файл для данного датчика еще не открыт, открываем его
                if (sensorFiles.find(sensorNumber) == sensorFiles.end()) {
                    sensorFiles[sensorNumber] = ofstream(sensorNumber + ".txt", ios::app);
                    // Если есть текущая дата и время, записываем их в файл
                    if (!currentDateTime.empty()) {
                        sensorFiles[sensorNumber] << currentDateTime << '\n';
                    }
                }

                // Извлекаем X, Y, Z координаты из строки
                string xcoord;
                string ycoord;
                string zcoord;

                float Xcoord;
                float Ycoord;
                float Zcoord;

                int index = line.find_first_of(">");
                index = index + 25;
                xcoord = "0x" + line.substr(index, 8);
                index = index + 8;
                ycoord = "0x" + line.substr(index, 8);
                index = index + 8;
                zcoord = "0x" + line.substr(index, 8);

                // Преобразуем шестнадцатеричные координаты в числа с плавающей точкой
                Xcoord = hexToFloat(xcoord);
                Ycoord = hexToFloat(ycoord);
                Zcoord = hexToFloat(zcoord);

                // Записываем координаты в файл датчика
                sensorFiles[sensorNumber] << "X: " << Xcoord << ", Y: " << Ycoord << ", Z: " << Zcoord << '\n';
            }
            // В противном случае записываем текущую дату и время во все файлы датчиков
            else {
                for (auto& pair : sensorFiles) {
                    pair.second << currentDateTime << " " << line << '\n';
                }
            }
        }
    }
};

// Точка входа в программу
int main() {
    // Создаем объект класса SensorDataProcessor, передаем ему имя входного файла
    SensorDataProcessor processor("data.MON");
    // Запускаем обработку данных сенсоров
    processor.processSensorData();
    // Возвращаем 0 в качестве кода завершения программы
    return 0;
}
