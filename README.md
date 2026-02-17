# modemy
![Language](https://img.shields.io/badge/C++%2D20-blue?logo=cplusplus&logoColor=white)
![MIT License](https://img.shields.io/badge/License-MIT-green)

A minimal audio-based software modulator.
It uses a musical scale to create sound out of a given file's data.

## Features

- Supports 3 main musical scales
    - Major
    - Minor
    - Pentatonic
- Customizable BPM
- Supports arbitrary root notes
- Supports any file.

## Usage

Example command:
```
./modemy 48000 1 300 PENTATONIC 240 "./data/test.txt" 
```

Format:
```
./modemy <SAMPLE RATE> <CHANNELS> <ROOT FREQUENCY> <SCALE> <BPM> <PATH> 
```

## License

This project is licensed under the MIT License - see LICENSE for more details.

## Author

a22Dv - a22dev.gl@gmail.com
