# Si7021_tempsensor_system

ZAsada działania biblioteki:

pliki Si7021:
  Część biblioteki odpowiadająca za bezpośrednie operacje na czujniku
  
pliki Si7021_service:
  Część biblioteki odpowiadająca za kontrolę pracy czujnika
  
Główna idea:

Główna funkcja serwisu wykonywana jest cyklicznie i to ona steruje zadaniami oraz stanem czujnika.
Komunikacja z czujnikiem po I2c wykonana z wykorzystaniem przerwań.
Dwa serwisy umieszczone w przerwaniach zakończeń konwersji I2c, zmieniające jedynie stan czujnika.
Dane moga być czytane w tle, odbierane na wezwanie


Etapy wykonania operacji:
- otrzymanie komendy z programu głównego -> serwis główny -> stan czujnika zmieniony na prace,
- wysłanie zapytania do czujnika -> przerwanie Tx -> informacja o wysłaniu danych,
- odebranie informacji z czujnika -> przerwanie RX -> informacja o odebraniu danych,
- przetworzenie danych i wykonanie zadanej komendy -> serwis główny -> informacja o gotowości danych do odbioru, 
- odbiór danych przez użytkonika, lub kolejna komenda

W tej wersji programu pamiętana jest jedynie jedna zadana komenda.

Do zrobienia:

- Obsługa błędów,
- Funkcja inicjalizacyjna jako jedna z komend,rozszerzenie jej funkcjonalności (odczyt ID, sprawdzenie VDD podczas startu)

