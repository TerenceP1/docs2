from pydub import AudioSegment
from pydub.playback import play

song = AudioSegment.from_wav("louder.mp3")
play(song)