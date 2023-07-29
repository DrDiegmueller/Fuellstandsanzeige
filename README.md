# Fuellstandsanzeige
Füllstandsanzeige mit Arduino Uno Juli 2023

Der Füllstand einer 10.000 Liter- Zisterne wird mit einem "water pressure level sensor" ermittelt,
der die standardisierte 4mA/20mA-Schnittstelle besitzt.

Als Geometrie der Zisterne ist die Zylinderform benutzt worden.
Ausgabe am vier-zeiligen LCD:
1) Wert des Analog/Digital-Wandlers (ADC-Wert)
2) Wasserhöhe (messbar sensormäßig) bis 5m Höhe
3) Restinhalt Zisterne in Liter
4) Prozentualer Füllgrad
5) Error-Codes
6) Datum und Uhrzeit

Mit einer Auflösung von 10Bit (interner ADC) kann man ca. 4mm Höhenunterschied auflösen.
Fehlerfälle: Sensor nicht angeschlossen / Kontaktfehler (0mA)
unb Zisterne voll
