#ifndef ORDER_PAGE_H
#define ORDER_PAGE_H

const char orderPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Schnapsglas Bestellungen</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            padding: 20px;
            background-color: #f0f0f0;
        }
        .container {
            display: flex;
            gap: 20px;
            flex-wrap: wrap;
        }
        .tile {
            padding: 20px;
            border-radius: 10px;
            min-width: 250px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
        }
        .tile-red {
            background-color: #ffebeb;
            border: 3px solid #ff4444;
        }
        .tile-green {
            background-color: #ebffeb;
            border: 3px solid #44ff44;
        }
        .tile-blue {
            background-color: #ebebff;
            border: 3px solid #4444ff;
        }
        .field-group {
            margin-bottom: 15px;
        }
        label {
            display: block;
            margin-bottom: 5px;
            font-weight: bold;
        }
        input, textarea {
            width: 100%;
            padding: 8px;
            border: 1px solid #ccc;
            border-radius: 4px;
            box-sizing: border-box;
        }
        textarea {
            min-height: 80px;
            resize: vertical;
        }
        h2 {
            margin-top: 0;
            text-align: center;
        }
        .deliver-btn {
            background-color: #4CAF50;
            color: white;
            padding: 20px 40px;
            font-size: 18px;
            font-weight: bold;
            border: none;
            border-radius: 8px;
            cursor: pointer;
            margin: 30px auto;
            display: block;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
        }
        .deliver-btn:hover {
            background-color: #45a049;
        }
    </style>
</head>
<body>
    <h1>Schnapsglas Bestellungen</h1>
    
    <div class="container">
        <div class="tile tile-red">
            <h2>Glas 1</h2>
            <div class="field-group">
                <label for="drink1">Getränketyp:</label>
                <input type="text" id="drink1" value="Alkohol">
            </div>
            <div class="field-group">
                <label for="person1">Name der Person:</label>
                <textarea id="person1" placeholder="Name oder Beschreibung eingeben..."></textarea>
            </div>
        </div>

        <div class="tile tile-green">
            <h2>Glas 2</h2>
            <div class="field-group">
                <label for="drink2">Getränketyp:</label>
                <input type="text" id="drink2" value="Alkohol">
            </div>
            <div class="field-group">
                <label for="person2">Name der Person:</label>
                <textarea id="person2" placeholder="Name oder Beschreibung eingeben..."></textarea>
            </div>
        </div>

        <div class="tile tile-blue">
            <h2>Glas 3</h2>
            <div class="field-group">
                <label for="drink3">Getränketyp:</label>
                <input type="text" id="drink3" value="Alkohol">
            </div>
            <div class="field-group">
                <label for="person3">Name der Person:</label>
                <textarea id="person3" placeholder="Name oder Beschreibung eingeben..."></textarea>
            </div>
        </div>
    </div>

    <button class="deliver-btn" onclick="deliverOrder()">Bestellung ausliefern</button>

    <script>
        function deliverOrder() {
            const orders = [];
            
            // Glass 1 (red)
            const name1 = document.getElementById('person1').value.trim();
            if (name1) {
                orders.push({
                    name: name1,
                    drink: document.getElementById('drink1').value,
                    color: "rot"
                });
            }
            
            // Glass 2 (green)
            const name2 = document.getElementById('person2').value.trim();
            if (name2) {
                orders.push({
                    name: name2,
                    drink: document.getElementById('drink2').value,
                    color: "grün"
                });
            }
            
            // Glass 3 (blue)
            const name3 = document.getElementById('person3').value.trim();
            if (name3) {
                orders.push({
                    name: name3,
                    drink: document.getElementById('drink3').value,
                    color: "blau"
                });
            }
            
            if (orders.length === 0) {
                alert("Bitte geben Sie mindestens einen Namen ein!");
                return;
            }
            
            console.log("Bestellung:", JSON.stringify(orders, null, 2));
            fetch(window.location.origin + '/order', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(orders)
            })
            .then(response => response.json())
            .then(data => {
                console.log('Server response:', data);
            })
            .catch(error => {
                console.error('Error sending order:', error);
            });
            alert(`Bestellung wurde erstellt!\n${orders.length} Glas/Gläser bestellt.`);
        }
    </script>
</body>
</html>
)rawliteral";

#endif