/*
 * Standardowa dyrektywa zapobiegająca przez załączeniem tego samego pliku
 * nagłówkoego przez pliki źródłowe kilkukrotnie.
 */
#ifndef MBED_JSNSR04_H
#define MBED_JSNSR04_H

/*
 * Załączenie pliku nagłówkowego mbed.h zawierajacego definicje obiektów
 * wykorzystywanych przez poniższą klasę. Wszystkie sterowniki przeznaczone do
 * współpracy ze śrdowoiskiem mbed powinny załączać ten plik nagłówkowy
 */
#include "mbed.h"

/*
 * Czas trwania impulsu wyzwalającego pomiar przez sensor ultradźwiękowy [us]
 */
#define TRIGGER_DELAY 15

class JSNSR04 {

    /*
     * Częśc prywatna klasy opisującej sensor ultradźwiękowy
     */
private:

    /*
     * Wskaźnik na obiekt klasy InterruptIn. Reprezentuje on pin
     * mikrokontrolera, którego zadaniem jest odbiór sygnału ECHO z sensora.
     */
    InterruptIn *echoIn;

    /*
     * Wskaźnik na obiekt klasy DigitalOut. Reprezentuje on pin
     * mikrokontrolera, którego zadaniem jest wytworzenie sygnału wyzwalającgo
     * pomiar przez sensor ultradźwiękowy
     */
    DigitalOut *triggerOut;

    /*
     * Obiekt klasy Timer. Licznik, który posłuży do pomiaru czasu trwania
     * sygnału zwrotnego ECHO emitowanego przez sensor ultradźwiękowy.
     */
    Timer timer;

    /* Obiekty klasy Timeout. triggerTimeout wykorzysytywany jest do
     * zakończenia nadawania sygnału trigger po założonym czasie TRIGGER_DELAY.
     * echoTimeout wykorzystywany jest do zgłoszenia zdarzenia "timeout", gdy
     * czas oczekiwania na wynik pomiaru będzie dłuższy niż wynikałoby to z
     * maksymalnego zasięgu sensora ultradźwiękowego
     */
    Timeout echoTimeout, triggerTimeout;

    /*
     * Flaga sygnalizująca zdarzenie typu "timeout" wystawiana gdy czas
     * oczekiwania na zbocze opadające sygnału ECHO jest dłuższy niż wynikałoby
     * to z maksymalnego zasięgu sensora
     */
    bool _isTimeout;

    /*
     * Pole pulseDuration przechowuje wartość zmierzonego czasu trwania impulsu
     * na linii ECHO
     */
    int _pulseDuration;

    /*
     * Flaga sygnalizująca że w danej chwili przes sensor przeprowadzany jest
     * pomiar odległośći
     */
    bool _measuring;

    /*
     * Metoda służąca do inicjalizacji pól wartościami początkowymi. Wywoływana
     * w konstruktorze klasy.
     */
    void init(void);

    /*
     * Metoda zatrzymująca licznik. Jej zadaniem jest również obliczenie czasu
     * trwania impulsu, który pojawił się na linii ECHO
     */
    void timerStop(void);

    /*
     * Metoda uruchamiająca licznik wykorzystywany do pomiaru czasu trwania
     * impulsu na lnii ECHO.
     */
    void timerStart(void);

    /* Metoda wywoływana tylko jako wywołanie zwrotne przy pomocy obiektu
     * triggerTimeout. Jej jedynym zadaniem jest sprowadzenie linii TRIG do
     * stanu niskiego po upływie czasu wyznaczonego przez stałą TRIGGER_DELAY.
     */
    void triggerOff(void);

    /*
     * Metoda wywoływana tylko jako wywołanie zwrotne przy pomocy obiektu
     * echoTimeout. Jej zadaniem jst ustawienie flagi "_isTimeout" jeżeli
     * czas oczekiwania na zbocze opadające na linii ECHO jest zbyt długi.
     */
    void notifyTimeout(void);
public:

    /*
     * Konstruktor inicjalizujący obiekt klasy reprezentującej sensor
     * ultradźwiękowy JSNSR04
     */
    JSNSR04(PinName trigger, PinName echo);

    /*
     * Metoda publczna używana to wyzwolenia pomiaru przez sensor
     * ultradźwiękowy
     */
    void triggerMeasurement(void);

    /*
     * Metoda publiczna, zwracjąca ostatnią wartość pola "pulseDuration", czyli
     * czasu trwania impulsu na linii ECHO, emitowanego przez sensor
     * ultradźwiękowy
     */
    int getPulseDuration(void);
};

#endif
