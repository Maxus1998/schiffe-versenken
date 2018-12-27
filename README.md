# schiffe-versenken
Aufgabe aus der C++ Vorlesung 2018

Nachrichten:<br/>
Jede Nachricht hat einen 1 Byte großen Header. Das erste Bit ist entweder 0 wenn es eine vom Client ausgehende Nachricht ist
und 1 wenn sie vom Server ausgeht. Die restlichen Bits dtehen für die Bedeutung der Nachricht, damit der Client und der Server wissen,
was sie mit der Nachricht machen müssen.<br/>
Nachrichten vom Client:<br/>
    1. Create Game:<br/>
        - Beschreibung: Der Client stellt eine Anfrage, ein neues Spiel zu erstellen.<br/>
        - Header: 00000000<br/>
        - Body: leer<br/>
    2. Request players:<br/>
        - Beschreibung: Der Client stellt eine Anfrage, eine Liste an Spielern zu erhalten, die ein Spiel erstellt haben.<br/>
        - Header: 00000001<br/>
        - Body: leer<br/>
    3. Join Game:<br/>
        - Beschreibung: Der Client teilt dem Server mit, welchem Spieler er beitreten will.<br/>
        - Header: 00000010<br/>
        - Body: Chararray<br/>
    4. Set Ships:<br/>
        - Beschreibung: Der Client sendet Koordinaten des Teiles des Schiffs, das am weitesten oben links ist sowie die<br/>
          Ausrichtung des Schiffs(horizontal/vertikal)<br/>
        - Header: 00000011<br/>
        - Body: 1 Byte (Zahl 0-99) + 1 Byte (horizontal/vertikal) (mehrere Nachrichten)<br/>
    5. Zug:<br/>
        - Bewschreibung: Der Spieler sendet Koordinaten, auf die er schießen will.<br/>
        - Header: 00000100<br/>
        - Body: 1 Byte (Zahl 0-99)<br/>
<br/>
Nachrichten vom Server:<br/>
    1. Confirm Create:<br/>
        - Beschreibung: Gibt dem Client bescheid, dass das Spiel erfolgreich erstellt wurde.<br/>
        - Header: 10000000<br/>
        - Body: leer<br/>
    2. Send Games:<br/>
        - Beschreibung: Sendet Namen von Clients, die gerade einen zweiten Spieler suchen.<br/>
        - Header: 10000001<br/>
        - Body: Chararray (mehrere Nachrichten)<br/>
    3. Game joined:<br/>
        - Beschreibung: Gibt dem suchenden Client Bescheid, dass ein Spieler seinem Spiel beigetreten ist und sagt<br/>
          dem Spieler, der einem Spiel beitreten will, dass er erfolgreich beigetreten ist.<br/>
        - Header: 10000010<br/>
        - Body: leer<br/>
    4. Schiffe gesetzt:<br/>
        - Beschreibung: Gibt beiden Clients Bescheid, dass ein Client alle Schiffe gesetzt hat.<br/>
        - Header: 10000011<br/>
        - Body: 1 Byte (1 oder 0 um den Spieler zu identifizieren)<br/>
    5.  Alle Schiffe gesetzt:<br/>
        - Beschreibung: Gibt beiden Clients Bescheid, dass alle Schiffe von beiden Clients erfolgreich gesetzt wurden.<br/>
        - Header: 10000100<br/>
        - Body: leer<br/>
    6. Zugergebnis (1)<br/>
        - Beschreibung: Gibt beiden Clients Bescheid, wohin und von welchem Spieler geschossen wurde und ob es ein Treffer war oder nicht.<br/>
        - Header: 10000101<br/>
        - Body: 1 Byte (Zahl 0-99) + 1 Byte (1. Bit: welcher Spieler
                                             2. Bit: Treffer oder kein Treffer)<br/>
    7. Zugergebnis (2)<br/>
        - Beschreibung: Gleich wie Zugergebnis (1), allerdins ist das die Endnachricht. Sie signalisiert, dass der Spieler, der gerade
          geschossen hat das letzte Schiff des andern Spielers versenkt hat. Hiermit hat dieser Spieler gewonnen.<br/>
        - Header: 10000101<br/>
        - Body: 1 Byte (Zahl 0-99) + 1 Byte (1. Bit: welcher Spieler
                                             2. Bit: Treffer oder kein Treffer)<br/>