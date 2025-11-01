# Amigus Aufbauanleitung

## Warum das Ganze?

* Das ist ein Mit-Mach-Projekt, also macht gerne mit ;),
* Ich möchte gerne erklären, wie es gehen könnte, aber
* ich möchte Support-Aufwand begrenzen. Wir alle haben Jobs zum Brot-Verdienst, das ist ein Hobby-Projekt, ich (und Oli vermutlich auch) haben keine Chance, alle verbastelten Karten zu retten.
* Die Bauteilkosten sind schon ordentlich hoch, wäre besser, wenn wenig schief geht.

## Schritt 1: Platinen bestellen

Zum Bestellen verwendet man die Gerbers
aus [https://github.com/necronomfive/AmiGUS-pub/tree/main/PCB/Gerbers/Version_13]() und
[https://github.com/necronomfive/AmiGUS-pub/tree/main/PCB/Gerbers/Audiot]().

Nicht als Werbung gemeint... ich hab hier mal die Settings für JLCPCB angehängt...

![settings JLCPCB](assets/images/AmiGUS-JLPCB-1.png)

![settings II](assets/images/AmiGUS-JLPCB-2.png)

Geht es anders?
Bestimmt, aber so hat es für mich prima geklappt.

Bestellerfahrung und Laufzeiten;

* Platinen bestellt am 09.09.,
* AUDIOTs geblockt wegen Design-Regel Verletzungen,
* Platinen fertig am 19.09.,
* Billigster Versand via FedEx und via Paris begann am gleichen Tag, Soll-Laufzeit 7-10 Tage,
* Ankunft am 24.09.,
* Platine fertig bestückt am 27.09.,
* Am 29.09. herausgefunden, dass sie sich nicht programmieren lässt weil der Stecker am Kabel des Programmers kaputt ist,
* Getestet am 30.09.

## Schritt 2: Bauteile bestellen

Hier sind die Beispiel-Warenkörbe für Reichelt und Mouser verlinkt:

[https://github.com/necronomfive/AmiGUS-pub/tree/main/PCB/BOM/AmiGUS_Rev13-example]()

Alinea bietet das Slotblech einzeln an, für extrem faire 10€ - [https://www.amiga-shop.net/Amiga-Ha.../Slotblende-fuer-AmiGUS-Soundkarte::1437.html]().

Wenn man sich kein Slotblech selber schnitzt, kann aus der Mouser Liste das Keystone Blech entfernt werden, sonst entsprechend drin lassen.
Wenn man es selber schnitzt... Bohrer, Dremel, viel Spaß!

## Schritt 3: Der Aufbau

So sehen die Platinen aus, wenn sie ankommen:

![settings JLCPCB](assets/images/AmiGUS-PCBs.jpg)

Leider muss man den AUDIOT noch etwas vorbereiten:

![settings JLCPCB](assets/images/Audiot-PCB.jpg)

Der obere Teil muss abgebrochen werden.
Ich habe die Platine dazu im Schraubstock eingespannt:

![settings JLCPCB](assets/images/Audiot-Sanding.jpg)

Diese Version war nicht so gut, ich hab da nochmal herumgebastelt, wenn das nach wie vor nicht gut klappt, bitte gerne melden.

Nuja, dann also bündig einspannen (dazu die Karte) und dagegen drücken. Danach Abfeilen:

![settings JLCPCB](assets/images/Audiot-Sanding-II.jpg)

Kanten nachbearbeiten:

![settings JLCPCB](assets/images/Audiot-Sanding-III.jpg)

Beide Seiten:

![settings JLCPCB](assets/images/Audiot-Sanding-IV.jpg)

Fertig:

![settings JLCPCB](assets/images/Audiot-Front.jpg)
![settings JLCPCB](assets/images/Audiot-Back.jpg)

Generelle Anmerkungen

Während dem Aufbau lässt sich alles gut mit der [interaktiven BOM](https://htmlpreview.github.io/?https://github.com/necronomfive/AmiGUS-pub/blob/main/PCB/BOM/interactiveBOM.html) ab haken.

Ich hab folgendes Equipment verwendet:

![settings JLCPCB](assets/images/Solder-Workshop.jpg)


* ELV LF-8800 mit 1.6mm Meißelspitze, passendem Entlötkolben und Lötzange,
* Stereomikroskop,
* gewinkelte Pinzette,
* Felder 0.5mm, 3.5% FLUX Sn60PB40 Lötzinn,
* Flussmittel, amtech von eBay, klebt wie Bulle, sieht aktiviert schnell braun aus, fließt schnell überall hin, nervt, weiss nicht, ob ich das nochmal kaufe, tat aber den Job, vielleicht sogar besser als das von Reichelt,
* Entlötlitze, falls mal was scheiße aussieht,
* Zahnbürste, IPA und Wattestäbchen zum Saubermachen,
* Kapton-Tape, damit man nix versaut, das billige von Amazon tut's,
* und die GANZ helle Lampe, 200W LED äquivalent, die macht so hell, das mein Handy den Weißabgleich nicht mehr hinbekommt. Deshalb sieht vieles so gelb aus. Und wenn man danach ans Tageslicht kommt denkt man ständig, es wäre voll finster, :D
* Zeit, ausreichend viel Zeit. Rechnet mal 12h, aber vielleicht bin ich einfach langsam. :rolleyes:
* Erfahrungsgemäß bringt Stress und Zeitdruck gar nichts, man hat nur mehr Fehler verbaut.
* Ich lasse die Fehler einfach weg und spare mir die Fehlersuche. Aber vielleicht ist das Geschmackssache? Egal.
	* Schleifen und Abbrechen der 5 AUDIOTs: 0.5h
	* Session 1 - 5h - Chips und Elkos
	* Session 2 - 3h - Hühnerfutter
	* Session 3 - 3.5h - Mehr Hühnerfutter, Elkos, Spannungsregler, Buchsen, Pinheader
* Temperatur... auf verbleit
	* meist 365°C, ob das nun viel ist oder nicht :nixwissen:, ich finde es nicht zu viel,
	* manchmal hab ich damit nicht genug Hitze, Masseflächen u.ä., dann geh ich auf 390°C,
	* vermutlich ginge auch, stattdessen mit der Heizplatte auf 100°C oder so vorzuheizen, aber...
	* die Zange auch bei 370°C,
	* Entlötkolben, den ich vor allem für die Header missbrauche - 390°C, weil ey, die sind nicht empfindlich!

## Schritt 3: AmiGUS löten

### 1. Alle Chips, außer U16

Ohne besonderen Grund hab ich rechts angefangen und mich nach links vorgearbeitet.

Also die OPAs zuerst:

![settings JLCPCB](assets/images/OPA.jpg)

U16 / DAC skippen, statt dessen U13 / ADC weiter machen:

![settings JLCPCB](assets/images/U13.jpg)

Dann U10 / der Codec:

![settings JLCPCB](assets/images/U10.jpg)

U11 / das RAM vor U24, weil eng und U11 die kleineren Beinabstände hat:

![settings JLCPCB](assets/images/U11.jpg)

Jetzt erst U24 / das Gatter:

![settings JLCPCB](assets/images/U24.jpg)

U14, weil man ihn ja eh schon in der Hand hat:

![settings JLCPCB](assets/images/U14.jpg)

Oh, hier sieht man es gerade; Kapton-Tape. Wenn man sich die diversen Goldflächen nicht abklebt hat man die schneller versaut, als man kucken kann. Kein Tape, kein Mitleid! :P

U7, der FPGA kommt dann:

![settings JLCPCB](assets/images/FPGA.jpg)

Dann die Bustreiber von rechts nach links draufdübeln:

![settings JLCPCB](assets/images/Bus-Driver.jpg)

Hier dann auch schön sauber gemacht.

### 2. C37, C39, C44, C42

Vorher:

![settings JLCPCB](assets/images/before.jpg)

Nachher:

![settings JLCPCB](assets/images/after.jpg)


### 3. U16

Ich hefte immer erst einen Pin an, dann Flussmittel druff, dann ...


![settings JLCPCB](assets/images/U16.jpg)

... den Rest anlöten:

![settings JLCPCB](assets/images/rest.jpg)

Hier sieht man nun auch, warum ich die Elkos vorgezogen habe - ich möchte den ADC nicht beschädigen, da ist echt eng.

### 4. C40, C111, C112

Wie man sieht, wird es hier gleich auch eng, einmal stören 2 andere Elkos, das andere mal das Hühnerfutter:

![settings JLCPCB](assets/images/C40.jpg)

### 5. C97

Bei dem auch, Hühnerfutter rechts:

![settings JLCPCB](assets/images/C97.jpg)

### 6. Quarz

Lustiges Ding! Pads verzinnen...

![settings JLCPCB](assets/images/crystal.jpg)

... Quarz mit der Entlötzange ...

![settings JLCPCB](assets/images/crystal-II.jpg)

... aufsetzen...

![settings JLCPCB](assets/images/crystal-III.jpg)

... fertig!

![settings JLCPCB](assets/images/crystal-IV.jpg)

Ich mache das so, weil die Lötpad-Flächen mir sonst zu klein sind. Mit der Zange kriege ich alle 4 gleichzeitig auf Temperatur und muss nur noch aufsetzen. Läuft!

### 7. Hühnerfutter

Djoa, das unausweichliche... ich tupf immer je ein Pad eines Postens mit Zinn an, kleb alle Bauteil damit auf, Flussmittel druff, beide Seiten gescheit unterm Mikroskop löten, erst die blanke, dann die vorgeklebte.

75x 100nF... als Zwischenschritt wie oben beschrieben:

![settings JLCPCB](assets/images/birdseeds.jpg)

Und danach fertig und zwischengereinigt:

![settings JLCPCB](assets/images/cleaned.jpg)

Im Grunde kann man die interaktive BOM nun von oben nach unten durch das Hühnerfutter abarbeiten. Um nichts zu übersehen, kann man alle Positionen einzeln abhaken. Is besser...

![settings JLCPCB](assets/images/birdseeds-II.jpg)

Bei Bauteilen mit großen Masseflächen kann es selbst mit 390⁰ bei der 1.6mm Meißelspitze eng werden. Muss gehen...

![settings JLCPCB](assets/images/birdseeds-III.jpg)

Gerne immer wieder mal zwischendurch reinigen, sonst klebt man sich alles, und ich meine wirklich alles, mit Flussmittelresten ein.
Beim Hühnerfutter ist das sogar noch schlimmer, als bei den großen Käfern, so sparsam kann man kaum dosieren...

### 8. U3 und U23

Die SMD Spannungsregler hätte man bestimmt mit den großen Käfern zusammen verbauen können, jetzt aber ganz sicher.
Warnung: große Flächen, evtl. dickere Lötspitze und mehr Temperatur.

![settings JLCPCB](assets/images/U3.jpg)
![settings JLCPCB](assets/images/U3-II.jpg)
![settings JLCPCB](assets/images/U3-III.jpg)

### 9. Alle verbliebenen Elkos

![settings JLCPCB](assets/images/capacitors.jpg)

Hier sieht man nun ganz gut, was ich gegen die SMD Elkos habe. Obwohl ich schon die kleinsten Polymer Elkos mit passender Kapazität und Spannung gewählt habe, passt die Lötspitze nachher nur knapp dazwischen.
Zusammen mit den gigantischen Kupferflächen ist das eine Gefahrenquelle für kalte Lötstellen.

![settings JLCPCB](assets/images/capacitors-II.jpg)

Die weiteren klappen ganz gut, auch hier wieder große Kupferflächen. Und "oben rechts", C98 und C99 sind wieder sehr dicht beieinander, das war bedrahtet schöner, finde ich.

![settings JLCPCB](assets/images/capacitors-III.jpg)

## 10. Bedrahtete Linearregler U20, U21 und U22

Als Pedant (wie ich) alle schön gleichmäßig biegen - und das ist schwerer als es klingt, weil unterschiedliche Hersteller.

Pro-Tipp: zuerst anschrauben, mit Nylonschrauben, niemals (!!!) mit Metallschrauben, denn auf der Rückseite laufen Leiterbahnen, dann lieber auch die "Fahne" anlöten.

Wenn der 7909 von Reichelt ist, lieber nochmal auf dem Regler nachlesen, ob da auch der richtige geliefert wurde. Wir haben hier so ca 30% falsche Regler (7806 für +6V) beobachtet. Das ist doof, wenn die Fahne angelötet war, sieht das auch nicht so schön aus. Bisher haben wir zwar noch von keinen dadurch zerstörten Karten gehört, doof ist das dennoch.

![settings JLCPCB](assets/images/U20.jpg)

Hier ist wieder viel Hitze nötig. Die Beine einkürzen macht es besser. Ich löte die mit dem Entlötkolben ein, so kriege ich mehr Kontaktfläche hin.

### 11. Alle Buchsen

![settings JLCPCB](assets/images/jacks.jpg)

### 12. Alle Pinheader

Auch hier: viel Hitze und der Entlötkolben.

![settings JLCPCB](assets/images/pinheader.jpg)

Gleiches mit dem AUDIOT.

![settings JLCPCB](assets/images/pinheader-II.jpg)

Pin 1 oben rechts, also Aussparung oben.
Vorsichtig sein, damit sich die Pins nicht herausdrücken.

![settings JLCPCB](assets/images/pinheader-III.jpg)

Elektrisch ist die Karte nun fertig!

![settings JLCPCB](assets/images/ready.jpg)
![settings JLCPCB](assets/images/ready-II.jpg)

### 13. Slotblech schnitzen

Tja, ich habe es einfach bei Alinea gekauft. Falls jemand mag, gerne eine Dremel-Beschreibung liefern.

### 14. Slotblech anschrauben

Schafft ihr, oder?

## Schritt 4: Inbetriebnahme

### 1. Messen

Ich finde es schlimm, gute alte Hardware kaputt zu machen.
Also schauen wir mal, dass zumindest keine schlimmen Kurzschlüsse auf dem Board sind.

![settings JLCPCB](assets/images/regulators.png)

Das sind die Linearregler, so angeordnet wie oben rechts auf der AmiGUS.

Da kann man zumindest mal messen, ob nix was nicht gegeneinander verbunden sein sollte, verbunden ist:

1. GND und +12V
2. GND und -12V
3. GND und +5VA
4. GND und -5VA
5. GND und -9VA
6. +12V und -12V
7. +12V und +5VA
8. +12V und -5VA
9. +12V und -9VA
10. -12V und +5VA
11. -12V und -5VA
12. -12V und -9VA
13. +5VA und -5VA
14. +5VA und -9VA
15. -5VA und -9VA

Am Zorro findet sich unterhalb von C94 ein kleiner Pfeil, auf der Rückseite der Platine ist dort Pin 1.
Auf der Vorderseite ist da Pin 2, nach links sind aufsteigend die geraden Pins (also 2, 4, 6, 8, 10 ...)

* Pin 2, 4, 100, 90 und 88 sind GND
* Pin 6 ist +5V
* Pin 10 ist +12V
* Pin 20 ist -12V

Hier kann man gerne wieder auf unerwünschte Verbindungen prüfen.
Zwischen +5V und GND messe ich ca. 2MOhm,
zwischen +12V und GND sind es ca. 50MOhm,
zwischen GND und -12V sind es ca. 60MOhm.

Die +/-12V kommen entstprechend bei den Linearreglern an, GND auch,
+5V führt z.B. zu den TLV1117-33 und TLV1117-18, jeweils rechtes Bein.

![settings JLCPCB](assets/images/regulators-ii.png)

Wo ihr schonmal dabei seid...
messt doch kurz die +3.3V bzw +1.8V der beiden TLV1117 Outputs auf Durchgang gegen GND, linkes Bein gegen mittleres Bein,
nur um sicherzugehen, die armen Regler nicht kurz zu schließen.
Ein Krümel Lot an der falschen Stelle reicht dazu schon, hab ich gehört @TurricanA1200 ;).

U3 - Pin 1 (GND) nach Pin 2 (3,3V) - 400 bis 460 Ohm
U3 - Pin 1 (GND) nach Pin 3 (5V) - ~50 kOhm
U23 - Pin 1 (GND) nach Pin 2 (1,8V) - ~360 kOhm
U23 - Pin 1 (GND) nach Pin 3 (5V) - ~50 kOhm

Jagt euch bitte nichts hoch!

### 2. Einstecken

Die Karte kann dann nun in einen Amiga wandern. Funktionierendes Netzteil wäre zum Programmieren ganz nett.
Restliche Funktionalität ist hier noch optional. ;)

Ja, das kann man, die AmiGUS Bustreiber sind zu diesem Zeitpunkt hochohmig, da passiert nix und der Rechner sollte normal booten.

### 3. Programmieren

Es führt soweit ich weiß nichts um die Altera Quartus Software herum. @botfixer hat eine aktuelle Quartus Prime Lite versucht, ich nehme die olle 15.1 unter Windows.

So sieht das nach dem Start aus:

![settings JLCPCB](assets/images/quartus.png)

Einfach File -> Open und die XyZ_AmiGUS_FPGA.pof aus [https://github.com/necronomfive/AmiGUS-pub/tree/main/FPGA/Releases/Quartus]() öffnen.
Dann bei Programm / Configure ein paar Haken setzen und start drücken.

![settings JLCPCB](assets/images/quartus-II.png)

Tja, leider ist Quartus ziemlich egal, was man in Verify anhakt oder nicht.
Mein Kabel war inne Fritten, aber dennoch bekam ich eine schöne "100% (Successful)" Nachricht.

![settings JLCPCB](assets/images/quartus-III.png)


Vermutlich bin ich der einzige, dem sowas auffällt, aber an dieser Stelle ist der Amiga schon eingefroren, die Floppy klackert nicht mehr.
Zeit für einen Kaltstart, "Did you try turning it off and on again?".

Beide Maustasten gedrückt halten und in Expansion Board Diagnostics...

![settings JLCPCB](assets/images/boot.jpg)

5, 6 und 7 sind die AmiGUS, sieht gut aus. Auch hier wieder die unteren drei...

![settings JLCPCB](assets/images/showconfig.png)

### 4. Software

Zeit, etwas Software herunterzuladen - aktuell ist das release_250907-rc2 von [https://github.com/necronomfive/AmiGUS-pub/releases]().
Es gab wohl auch Leute, bei denen das LHA-Archiv klemmt. Keine Ahnung, wie ihr die entpackt, mit lha 2.15 tun die.
Ansonsten sind da auch schöne ADFs.
Für die Software-Installation und ihre Varianten... [RTFM](https://github.com/necronomfive/AmiGUS-pub/blob/main/Documentation/AmiGUS/AmiGUS_User_Manual.pdf) - dafür haben wir das geschrieben.

Und nun das FlashFPGA Tool starten und "Info" drücken...

![settings JLCPCB](assets/images/flash.png)

... und auch das ist leider normal, also beherzt auf "Init" drücken ...

![settings JLCPCB](assets/images/flash-II.png)

... und unten der Balken wird blau. Sollte nun der "Quit" Knopf nicht funktionieren, ist etwas nicht OK.
Ein schnelles Reset später ist die Karte einsatzbereit.

Zeit, bisserl was zu testen. Der Mixer ...

![settings JLCPCB](assets/images/mixer.png)

... zeigt in den "Levels" an, was an den DAC bzw. TOSLINK gesendet wird.
Paula richtig verkabelt? Dann sollte sie nicht nur zu hören sein, sondern es auch hier Ausschlag der Pegel geben.
LineIn richtig verkabelt? Es gilt das gleiche.

Beides teste ich mit [SmartPlay](https://aminet.net/mus/play/SmartPlay.lha), einem einfachen MOD-Player für Paula.

Jetzt geht es ans Eingemachte.
[LHA](https://aminet.net/util/arc/lha.run) habt ihr?
[AHI 4.18](https://aminet.net/driver/audio/ahiusr_4.18.lha) installieren, mit BGUI Oberfläche oder [MUI](https://aminet.net/util/libs/mui38usr.lha) so vorhanden.
[HippoPlayer](https://aminet.net/mus/play/hippoplayer.lha) installieren und auf neueste Version [updaten](https://aminet.net/mus/play/hippoplayerupdate.lha).
Ein paar MOD files und MP3s wären nun auch ganz gut zu haben...

HippoPlayer starten... Mit "Add" kann man Musikstücke hinzufügen.

In den Prefs (unter "Pr") finden sich auf den letzten 3 Tabs die Settings für MHI...

![settings JLCPCB](assets/images/prefs.png)

... AmiGUS direkt ...

![settings JLCPCB](assets/images/prefs-II.png)

... und AHI.

![settings JLCPCB](assets/images/prefs-III.png)

Regeln:

* Ist AmiGUS aktiv, sind AHI und Paula aus.
* MHI an schaltet die mpega.library ab.

Was bedeutet das nun?

* Will ich AHI testen, schalte ich den nativen AmiGUS Support im vorletzten Tab aus, stelle AHI im letzten Tab an und drücke save oder use,
* Will ich mhi testen, vorvorletzter Tab, mhi Haken rein, save oder use drücken,
* AmiGUS Modus nativ: AmiGUS Support im vorletzten Tab an, dabei kann AHI an bleiben, ist egal,
* Will ich Paula hören, müssen AHI und AmiGUS aus.

So sieht es dann aus, wenn MHI spielt...

![settings JLCPCB](assets/images/MHI.png)

... AGUS output aktiv ...

![settings JLCPCB](assets/images/AGUS.png)

... und AHI output aktiv.

![settings JLCPCB](assets/images/AHI.png)

Alle drei tun?
Paula und LineIn laufen auch?

Prima, Glückwunsch, geschafft!




