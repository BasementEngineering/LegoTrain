const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Pump Control</title>
    <style>
        body {
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            font-family: Arial, sans-serif;
        }
        button {
            padding: 20px 40px;
            font-size: 24px;
            cursor: pointer;
        }
    </style>
</head>
<body>
    <button id="fillButton">Fill a Shot</button>

    <script>
        document.getElementById('fillButton').addEventListener('click', function() {
            fetch('/fill', {
                method: 'POST'
            }).then(response => {
                if (response.ok) {
                    alert('Shot filled successfully!');
                } else {
                    alert('Failed to fill the shot.');
                }
            }).catch(error => {
                alert('Error: ' + error.message);
            });
        });
    </script>
</body>
</html>
)rawliteral";