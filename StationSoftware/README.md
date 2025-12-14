# Train Station Sound Player

A Python Flask web server that provides HTTP endpoints for playing station sounds and announcements.

## Features

- **Train Arriving**: Plays random sounds from train arriving sound library
- **Order Ready Announcements**: Plays customized delivery announcements with names and colors
- **Text-to-Speech Alternative**: Option to use TTS instead of pre-recorded audio files
- **Multi-threaded Audio**: Non-blocking audio playback that doesn't freeze HTTP responses

## Installation

### Option 1: Using Virtual Environment (Recommended)

1. Create a virtual environment:
```bash
# On Windows
python -m venv venv

# On macOS/Linux
python3 -m venv venv
```

2. Activate the virtual environment:
```bash
# On Windows
venv\Scripts\activate

# On macOS/Linux
source venv/bin/activate
```

3. Install the required dependencies:
```bash
pip install -r requirements.txt
```

4. When you're done, deactivate the virtual environment:
```bash
deactivate
```

### Option 2: Global Installation

1. Install the required dependencies globally:
```bash
pip install -r requirements.txt
```

**Note**: Using a virtual environment is recommended to avoid conflicts with other Python projects.

## Directory Structure

```
StationSoftware/
├── main.py                    # Main Flask application
├── requirements.txt           # Python dependencies
├── test_client.py            # Test script for API endpoints
├── sample_order.json         # Example JSON for order requests
└── soundfiles/               # Audio files directory
    ├── trainarriving/        # Random train arrival sounds
    ├── delivery/
    │   └── deliveryReady.mp3 # "Delivery ready" announcement
    ├── names/                # Individual name audio files
    │   ├── oliver.mp3
    │   ├── emma.mp3
    │   └── max.mp3
    └── colors/               # Individual color audio files
        ├── green.mp3
        ├── blue.mp3
        └── red.mp3
```

## Usage

### Start the Server

1. If using a virtual environment, make sure it's activated:
```bash
# On Windows
venv\Scripts\activate

# On macOS/Linux
source venv/bin/activate
```

2. Start the Flask server:
```bash
python main.py
```

The server will start on `http://localhost:5000`

### API Endpoints

#### 1. Train Arriving
- **URL**: `/trainArriving`
- **Method**: `GET` or `POST`
- **Description**: Plays a random sound from the `soundfiles/trainarriving/` directory

**Example**:
```bash
curl -X POST http://localhost:5000/trainArriving
```

#### 2. Order Ready
- **URL**: `/orderReady`
- **Method**: `POST`
- **Content-Type**: `application/json`
- **Description**: Plays delivery announcement followed by names and colors

**JSON Format**:
```json
{
  "name1": "Oliver",
  "color1": "green",
  "name2": "Emma", 
  "color2": "blue",
  "name3": "Max",
  "color3": "red",
  "use_tts": false
}
```

**Parameters**:
- `name1`-`name6`: Names for up to 6 orders
- `color1`-`color6`: Colors for up to 6 orders  
- `use_tts`: `true` for text-to-speech, `false` for audio files (optional, default: false)

**Example**:
```bash
curl -X POST http://localhost:5000/orderReady \
  -H "Content-Type: application/json" \
  -d '{"name1":"Oliver","color1":"green","name2":"Emma","color2":"blue","use_tts":false}'
```

#### 3. Order Ready (TTS Only)
- **URL**: `/orderReadyTTS`
- **Method**: `POST`
- **Description**: Same as `/orderReady` but always uses text-to-speech

#### 4. Status
- **URL**: `/status`
- **Method**: `GET`
- **Description**: Returns information about available sound files

#### 5. Test
- **URL**: `/test`
- **Method**: `GET`
- **Description**: Simple test endpoint to verify server is running

## Audio File Requirements

### Supported Formats
- MP3, WAV, OGG, FLAC

### Required Files
- `soundfiles/delivery/deliveryReady.mp3` - Main delivery announcement
- `soundfiles/names/[name].mp3` - Individual name files (e.g., oliver.mp3, emma.mp3)
- `soundfiles/colors/[color].mp3` - Individual color files (e.g., green.mp3, blue.mp3)
- `soundfiles/trainarriving/` - Directory with various train arrival sounds

### File Naming
- All names and colors should be lowercase in filenames
- Use `.mp3` extension
- Example: `oliver.mp3`, `green.mp3`

## Testing

Run the test script to verify all endpoints:

```bash
python test_client.py
```

## Text-to-Speech

The system includes pyttsx3 for text-to-speech functionality as an alternative to audio files. This is useful for:
- Testing without having all audio files
- Dynamic content that changes frequently
- Multi-language support
- Accessibility features

## Dependencies

- **Flask**: Web server framework
- **pygame**: Audio playback
- **pyttsx3**: Text-to-speech engine

## Error Handling

The server includes comprehensive error handling for:
- Missing audio files
- Invalid JSON requests
- Audio playback failures
- File system errors

All endpoints return JSON responses with status and error information.

## Deployment
The application can be deployed on any server that supports Python and Flask. Consider using a production-ready server like Gunicorn or uWSGI for better performance and reliability.

Device: Raspberry Pi Zero 2 W
OS: Raspberry Pi OS (32-bit) with Desktop

Connect to Desktop via VNC.