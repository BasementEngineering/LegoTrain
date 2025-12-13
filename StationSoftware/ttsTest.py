import multiprocessing
import pyttsx3
import time

def tts_worker(text_queue, done_queue):
    engine = pyttsx3.init()
    engine.setProperty('rate', 150)  # Set speech rate
    engine.setProperty('volume', 1.0)  # Set volume level
    engine.setProperty('voice', 'de')  # Set to German voice if available
    engine.setProperty('pitch', 50)  # Set pitch level (may not be supported by all engines)
    while True:
        try:
            text = text_queue.get()
            if text is None:  # Shutdown signal
                break
            engine.say(text)
            engine.runAndWait()
            done_queue.put(True)
        except Exception as e:
            print(f"TTS Error: {e}")
            done_queue.put(False)

class TTSProcessManager:
    def __init__(self):
        self.text_queue = multiprocessing.Queue()
        self.done_queue = multiprocessing.Queue()
        self.process = None
        self.start_worker()

    def start_worker(self):
        if self.process is None or not self.process.is_alive():
            self.process = multiprocessing.Process(target=tts_worker, args=(self.text_queue, self.done_queue))
            self.process.start()

    def speak(self, text):
        self.text_queue.put(text)
        return self.done_queue

    def wait_for_current(self):
        """Wait for the current message to finish"""
        return self.done_queue.get()
    
    def wait_for_all(self):
        """Wait for all queued messages to finish"""
        while not self.text_queue.empty():
            self.wait_for_current()

    def shutdown(self):
        """Shutdown the worker process"""
        if self.process and self.process.is_alive():
            self.text_queue.put(None)  # Send shutdown signal
            self.process.join()

if __name__ == "__main__":
    # Initialize the TTS manager
    tts = TTSProcessManager()

    # Array of German strings
    german_strings = [
        "Guten Tag, willkommen am Bahnhof",
        "Der Zug fährt gleich ab", 
        "Bitte einsteigen und Türen schließen",
        "Nächster Halt: Hauptbahnhof",
        "Vielen Dank für Ihre Reise"
    ]

    # Add German strings to queue and wait for each to complete
    for i, text in enumerate(german_strings):
        tts.speak(text)
        print(f"Queued message {i+1}: {text}")
        time.sleep(1)  # Optional pause between messages
    
    print("Waiting for all messages to complete...")
    tts.wait_for_all()

    print("All messages spoken.")
    tts.shutdown()