# schiffe-versenken
Aufgabe aus der C++ Vorlesung 2018

Nachrichten:
Jede Nachricht hat einen 1 Byte großen Header. Das erste Bit ist entweder 0 wenn es eine vom Client ausgehende Nachricht ist
und 1 wenn sie vom Server ausgeht. Die restlichen Bits dtehen für die Bedeutung der Nachricht, damit der Client und der Server wissen,
was sie mit der Nachricht machen müssen.\n
Nachrichten vom Client:\n
    1. Create Game:\n
        - Beschreibung: Der Client stellt eine Anfrage, ein neues Spiel zu erstellen.\n
        - Header: 00000000\n
        - Body: leer\n
    2. Request players:\n
        - Beschreibung: Der Client stellt eine Anfrage, eine Liste an Spielern zu erhalten, die ein Spiel erstellt haben.\n
        - Header: 00000001\n
        - Body: leer\n
    3. Join Game:\n
        - Beschreibung: Der Client teilt dem Server mit, welchem Spieler er beitreten will.\n
        - Header: 00000010\n
        - Body: Chararray\n
    4. Set Ships:\n
        - Beschreibung: Der Client sendet Koordinaten des Teiles des Schiffs, das am weitesten oben links ist sowie die\n
          Ausrichtung des Schiffs(horizontal/vertikal)\n
        - Header: 00000011\n
        - Body: 1 Byte (Zahl 0-99) + 1 Byte (horizontal/vertikal) (mehrere Nachrichten)\n
    5. Zug:\n
        - Bewschreibung: Der Spieler sendet Koordinaten, auf die er schießen will.\n
        - Header: 00000100\n
        - Body: 1 Byte (Zahl 0-99)\n
\n
Nachrichten vom Server:\n
    1. Confirm Create:\n
        - Beschreibung: Gibt dem Client bescheid, dass das Spiel erfolgreich erstellt wurde.\n
        - Header: 10000000\n
        - Body: leer\n
    2. Send Games:\n
        - Beschreibung: Sendet Namen von Clients, die gerade einen zweiten Spieler suchen.\n
        - Header: 10000001\n
        - Body: Chararray (mehrere Nachrichten)\n
    3. Game joined:\n
        - Beschreibung: Gibt dem suchenden Client Bescheid, dass ein Spieler seinem Spiel beigetreten ist und sagt
          dem Spieler, der einem Spiel beitreten will, dass er erfolgreich beigetreten ist.\n
        - Header: 10000010\n
        - Body: leer\n
    4. Schiffe gesetzt:\n
        - Beschreibung: Gibt beiden Clients Bescheid, dass ein Client alle Schiffe gesetzt hat.\n
        - Header: 10000011\n
        - Body: 1 Byte (1 oder 0 um den Spieler zu identifizieren)\n
    5.  Alle Schiffe gesetzt:\n
        - Beschreibung: Gibt beiden Clients Bescheid, dass alle Schiffe von beiden Clients erfolgreich gesetzt wurden.\n
        - Header: 10000100\n
        - Body: leer\n
    6. Zugergebnis (1)\n
        - Beschreibung: Gibt beiden Clients Bescheid, wohin und von welchem Spieler geschossen wurde und ob es ein Treffer war oder nicht.\n
        - Header: 10000101\n
        - Body: 1 Byte (Zahl 0-99) + 1 Byte (1. Bit: welcher Spieler
                                             2. Bit: Treffer oder kein Treffer)\n
    7. Zugergebnis (2)\n
        - Beschreibung: Gleich wie Zugergebnis (1), allerdins ist das die Endnachricht. Sie signalisiert, dass der Spieler, der gerade
          geschossen hat das letzte Schiff des andern Spielers versenkt hat. Hiermit hat dieser Spieler gewonnen.\n
        - Header: 10000101\n
        - Body: 1 Byte (Zahl 0-99) + 1 Byte (1. Bit: welcher Spieler
                                             2. Bit: Treffer oder kein Treffer)\n