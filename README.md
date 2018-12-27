# schiffe-versenken
Aufgabe aus der C++ Vorlesung 2018

Nachrichten:
Jede Nachricht hat einen 1 Byte großen Header. Das erste Bit ist entweder 0 wenn es eine vom Client ausgehende Nachricht ist
und 1 wenn sie vom Server ausgeht. Die restlichen Bits dtehen für die Bedeutung der Nachricht, damit der Client und der Server wissen,
was sie mit der Nachricht machen müssen.
Nachrichten vom Client:
    1. Create Game:
        - Beschreibung: Der Client stellt eine Anfrage, ein neues Spiel zu erstellen.
        - Header: 00000000
        - Body: leer
    2. Request players:
        - Beschreibung: Der Client stellt eine Anfrage, eine Liste an Spielern zu erhalten, die ein Spiel erstellt haben.
        - Header: 00000001
        - Body: leer
    3. Join Game:
        - Beschreibung: Der Client teilt dem Server mit, welchem Spieler er beitreten will.
        - Header: 00000010
        - Body: Chararray
    4. Set Ships:
        - Beschreibung: Der Client sendet Koordinaten des Teiles des Schiffs, das am weitesten oben links ist sowie die
          Ausrichtung des Schiffs(horizontal/vertikal)
        - Header: 00000011
        - Body: 1 Byte (Zahl 0-99) + 1 Byte (horizontal/vertikal) (mehrere Nachrichten)
    5. Zug:
        - Bewschreibung: Der Spieler sendet Koordinaten, auf die er schießen will.
        - Header: 00000100
        - Body: 1 Byte (Zahl 0-99)

Nachrichten vom Server:
    1. Confirm Create:
        - Beschreibung: Gibt dem Client bescheid, dass das Spiel erfolgreich erstellt wurde.
        - Header: 10000000
        - Body: leer
    2. Send Games:
        - Beschreibung: Sendet Namen von Clients, die gerade einen zweiten Spieler suchen.
        - Header: 10000001
        - Body: Chararray (mehrere Nachrichten)
    3. Game joined:
        - Beschreibung: Gibt dem suchenden Client Bescheid, dass ein Spieler seinem Spiel beigetreten ist und sagt
          dem Spieler, der einem Spiel beitreten will, dass er erfolgreich beigetreten ist.
        - Header: 10000010
        - Body: leer
    4. Schiffe gesetzt:
        - Beschreibung: Gibt beiden Clients Bescheid, dass ein Client alle Schiffe gesetzt hat.
        - Header: 10000011
        - Body: 1 Byte (1 oder 0 um den Spieler zu identifizieren)
    5.  Alle Schiffe gesetzt:
        - Beschreibung: Gibt beiden Clients Bescheid, dass alle Schiffe von beiden Clients erfolgreich gesetzt wurden.
        - Header: 10000100
        - Body: leer
    6. Zugergebnis (1)
        - Beschreibung: Gibt beiden Clients Bescheid, wohin und von welchem Spieler geschossen wurde und ob es ein Treffer war oder nicht.
        - Header: 10000101
        - Body: 1 Byte (Zahl 0-99) + 1 Byte (1. Bit: welcher Spieler
                                             2. Bit: Treffer oder kein Treffer)
    7. Zugergebnis (2)
        - Beschreibung: Gleich wie Zugergebnis (1), allerdins ist das die Endnachricht. Sie signalisiert, dass der Spieler, der gerade
          geschossen hat das letzte Schiff des andern Spielers versenkt hat. Hiermit hat dieser Spieler gewonnen.
        - Header: 10000101
        - Body: 1 Byte (Zahl 0-99) + 1 Byte (1. Bit: welcher Spieler
                                             2. Bit: Treffer oder kein Treffer)  