#include "JSNSR04.h"

/*
 * Konstruktor inicjaliujący klasy repreentującej sensor ultradźwiękowy JSNSR04
 */
JSNSR04::JSNSR04(PinName trigger, PinName echo) {
    /*
     * Utworzenie instancji obiektów reprezentujących piny mikrokontrolera
     */

    /*
     * Konfiguracja pinu mikrokontrolera  połączonego do linii TRIG jako wyjście
     */
    triggerOut = new DigitalOut(trigger);

    /*
     * Konfiguracja pinu mikrokontrolera połączonego do linii ECHO jako wejście
     * z możliwością rejestracji przerwań.
     */
    echoIn = new InterruptIn(echo);

    /*
     * Inicjaliacja pól wartościami początkowymi
     */
    init();
}

void JSNSR04::init(void) {

    /*
     * Aktywacja wewnętrznego rezystora podciągającego pin ECHO mikrokontrolera
     * do napięcia zasilającego.
     */
    echoIn->mode(PullUp);

    /*
     * Inicjaliacja pól wartościami początkowymi
     */
    _measuring = false;
    printf("measuring = false\n\r");
    _isTimeout = false;
}

void JSNSR04::triggerMeasurement(void) {
    printf("\n\rtrigger start\n\r");

    /*
     * Sygnał wyzwalający pomiar ma postać impulsu o stanie wysokim trwającego
     * TRIGGER_DELAY [us]. Wystawienie stanu niskiego na 2 us w celu upewnienia
     * się, że mamy stan niski na linii TRIG przed rozpoczęciem formowania
     * impulsu wyzwalającego.
     */
    triggerOut->write(0);
    wait_us(2);

    /*
     * Wystawienie stanu wysokiego przez pin TRIG mikrokontrolera
     */
    triggerOut->write(1);


    /*
     * Przypisanie wywołania zwrotnego metody "timerStart" do zdarzenia typu
     * "rise" powiązanego z pinem ECHO mikrokontrolera. Zdarzenie to nastąpi
     * jeżeli na pinie tym zostanie zarejestrowane zbocze opadające sygnału
     */
    echoIn->rise(callback(this, &JSNSR04::timerStart));

    /*
     * Powiązanie wywołania zwrotnego metody "turnOffTrigger" ze zdarzeniem
     * typu "timeout", które nastąpi po upływie czasu TRIGGER_DELAY. Zalętą
     * wykorzystania obiektu klasy "Timeout" w stosunku do wywołania
     * "wait_us()" jest to, że w pierwszym przypadku program może być dalej
     * wykonywany, a odliczanie czasu oraz wywołanie zwrotne metody odbywa się
     * w kolejnym wątku.
     */
    triggerTimeout.attach_us(callback(this, &JSNSR04::triggerOff), TRIGGER_DELAY);
    printf("\n\rtrigger end\n\r");

    /*
     * Wystawienie flagi "_measuring". Sygnalizuje ona, że sensor jest w
     * trakcie wykonywania pomiaru.
     */
    _measuring = true;
    printf("measuring = true\n\r");
}

void JSNSR04::triggerOff(void) {

    /*
     * Wystawienie stanu niskiego przez pin TRIG mikrokontrolera. Jest to krok
     * kończący formowanie sygnału wyzwalącego pomiar odległośći przez sensor
     * ultradźwiękowy.
     */
    triggerOut->write(0);
}

void JSNSR04::timerStart(void) {
    printf("\n\rtimer start\n\r");

    /*
     * Uruchomienie licznika służącego do pomiaru czasu trwania impulsu na
     * linii ECHO
     */
    this->timer.start();

    /*
     * Przypisanie wywołania zwrotnego metody "timerStop" do zdarzenia typu
     * "fall" powiązanego z pinem ECHO mikrokontrolera. Zdarzenia do nastąpi
     * jeżeli na pinie tym zostanie zarejestrowane zbocze opadające sygnału
     */
    echoIn->fall(callback(this, &JSNSR04::timerStop));

    /*
     * Deaktywacja wywołania zwrotnego powiązanego ze zdarzeniem spowodowanym
     * wystąpieniem zbocza narastającego zarejestrowanego na pinie ECHO
     * mikrokontrolera.
     */
    echoIn->rise(NULL);
}

void JSNSR04::timerStop(void) {
    printf("\n\rtimer stop\n\r");

    /*
     * Odczyt zawartości licznika. Jest to jednocześnie czas trwania impulsu
     * ECHO.
     */
    _pulseDuration = timer.read_us();

    /*
     * Zatrzymanie licznika
     */
    timer.stop();

    /*
     * Wyzerowanie zawartości licznika
     */
    timer.reset();

    /*
     * Wyczyszczenie flagi sygnalizującej aktualny pomiar
     */
    _measuring = false;
    printf("measuring = false\n\r");

    /*
     * Deaktywacja wywołania zwrotnego powiązanego ze zdarzeniem spowodowanym
     * wystąpieniem zbocza narastającego zarejestrowanego na pinie ECHO
     * mikrokontrolera.
     */
    echoIn->fall(NULL);
}

void JSNSR04::notifyTimeout(void) {

    /*
     * Zatrzymanie licznika
     */
    timer.stop();

    /*
     * Wyzerowanie zawartości licznika
     */
    timer.reset();

    /*
     * Wyzerowanie flagi sygnalizującej trwający pomiar. Jeżeli zostało
     * zgłoszone zdarzenie "timeout" (zbyt dług czas oczwkiwania na wynik
     * pomiaru), to przyjmujemy że pomiar nie jest aktualnie wykonywany
     */
    _measuring = false;
    printf("measuring = false\n\r");

    /*
     * Ustawienie flagi sygnalizującej wystąpienie zdarzenia typu timeout.
     */
    _isTimeout = true;

    /*
     * Ustawienie wartości pola "pulseDuration" na "-1". Pozwala to na
     * rozróżnienie pomiędzy poprawnym wynikiem pomiaru a wystąpieniem błędu
     */
    _pulseDuration = -1;
    printf("notify timeout!");
}

int JSNSR04::getPulseDuration(void) {

    /*
     * Przypisanie wywołania zwrotnego metody "notifyTimeout" do zdarzenia typu
     * Timeout, które nastąpi jeżeli czas oczekiwania na odpowiedź przekroczy
     * czas maksymalnej poprawnej odpowiedzi wynikającej z maksymalnego zasięgu
     * sensora. Za maksymalny zasięg sensora przyjęto na podstawie pomiarów
     * wartość 600 [cm]. 35000 / 58 = 603,4
     */
    echoTimeout.attach_us(callback(this, &JSNSR04::notifyTimeout), 35000);

    /*
     * Oczekiwanie na zmierzony czas trwania impulsu na linii ECHO. Czekamy tak
     * długo, dopóki zasygnalizowany zostanie koniec pomiaru poprzez zdjęcie
     * flagi "_measuring" (ustawienie _measuring = false). Drugi warunek
     * zapobiega utknięciu mikrokontrolera w tej pętli. Nigdy nie będziemy
     * czekać dłużej niż czas potrzebny na otrzymanie wyniku najdalszej,
     * poprawnie mierzonej odległości
     */
    while (_measuring && !_isTimeout) {
        printf("measuring: %d timeout: %d\n\r", _measuring, _isTimeout);
    };

    /*
     * Jeżeli pomiar został wykonany poprawnie (wynik uzyskany przed upływem
     * 35 [ms]), anulujemy powiązanie metody "notifyTimeout" ze zdarzeniem
     * timeout. Jeżeli znaleźliśmy się tu wyniku wywołania metody
     * "notifyTimeout" poniższa intrukcja nie ma żadnego efektu. W związku z
     * czym nie ma konieczności sprawdzania flagi "_isTimeout" przed jej
     * wykonaniem
     */
    echoTimeout.detach();
    /*
     * Jeżeli wykonywanie programu dotarło do tego miejsca to albo zostało
     * zgłoszone zdarzenie typu timeout, albo został poprawnie wykonany pomiar.
     * W obu przypadkach można wyzerować "_isTimeout"
     */
    _isTimeout = false;

    /*
     * Zwrócenie zawartości pola prywatnego "_pulseDuration"
     */
    return _pulseDuration;
}
