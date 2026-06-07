# Edge AI Speech Recognition & Cellular IoT System

This project is an **Edge AI and IoT** application built on an **STM32F407** microcontroller. Audio is listened to from the environment, an offline machine learning model is used to detect the keywords **"YES"** and **"NO"**, and the result is sent to a cloud broker through an LTE network.

The firmware is not written as one large file. Instead, a layered structure is used so the project can be easier to read, test, and maintain. **FreeRTOS** is used to manage multiple tasks, and **CppUTest** is used for unit testing on a computer without flashing the board every time

## Architecture Overview

The system is divided into different layers so each part can have a clear responsibility.

1. **Hardware Layer**  
   This layer includes the STM32F407 microcontroller, I2S digital microphone, UART interface, and Quectel EG25-G cellular modem.

2. **Driver Layer (`Drivers/`)**
   - `drv_microphone`: The I2S configuration and audio data streaming are handled here.
   - `drv_atcommand`: A thread-safe AT command parser is used to communicate with the modem with controlled timeouts.

3. **Middleware Layer**
   - **FreeRTOS** is used to run tasks at the same time and manage task priorities.
   - **CMSIS-DSP** is used for mathematical operations such as audio filtering.
   - **ST Audio Preprocessing** and **X-CUBE-AI** are used to prepare audio features and run the neural network model.

4. **Application Layer**  
   The main application flow is controlled by `main.c` and `mqtt.c`. Keyword detection results are processed here and sent to the cloud.

## How It Works

### 1. Audio Streaming with I2S DMA

Audio data is collected continuously from the microphone. Since 16,000 audio samples per second are processed, the CPU should not be used for every single data transfer.

For this reason, **DMA** and a **Ping-Pong Buffer** are used.

- Audio samples are moved from the microphone to RAM automatically by DMA.
- When the first half of the buffer is filled, a DMA interrupt is triggered.
- A **FreeRTOS Binary Semaphore** is then released to wake up the audio processing task.
- While the CPU processes the first half of the buffer, the second half continues to be filled by DMA.

With this method, audio streaming can continue smoothly while processing is being done in parallel.

### 2. Audio Signal Processing

Raw audio samples are not sent directly to the neural network. They are too large and not very useful in raw form. Because of this, the audio is converted into a smaller and more meaningful feature representation.

The audio feature extraction pipeline is handled in `audio_dsp.c`.

1. **FIR Decimation**  
   The raw microphone data is filtered and downsampled into 16-bit PCM data by using an optimized **CMSIS-DSP FIR filter**.

2. **Windowing and FFT**  
   A **Hanning Window** is applied to the audio frame. Then, an **FFT** is used to convert the signal from the time domain into the frequency domain.

3. **Mel Filterbank**  
   The frequency data is converted into the Mel scale. This scale is closer to how human hearing works.

As a result, one second of audio is converted into a compact **122x40 Mel-Spectrogram** matrix. This matrix is used as the input of the AI model.

### 3. Edge AI Model Execution

The Mel-Spectrogram is passed to a Convolutional Neural Network inside `ai_app.c`.

The model weights are converted into C arrays by using **ST X-CUBE-AI**. These arrays are stored directly in the Flash memory of the microcontroller.

The model gives an output probability array:

```text
[NOISE, NO, YES]
```

A confidence threshold of **75%** is used before a final decision is accepted. This helps reduce false triggers caused by background noise or unclear speech.

### 4. Multi-Tasking and Thread Safety

**FreeRTOS** is used so different parts of the system can work independently.

- `MicTask`  
  This task is woken up by semaphores. Audio filtering, DSP processing, and AI inference are handled here.

- `NetworkTask`  
  This task checks the network status and manages periodic keep-alive operations.

- `at_driver_mutex`  
  Both AI-triggered messages and network keep-alive commands need to use the same UART connection to communicate with the modem. A mutex is used to protect this shared UART resource.

Because of this mutex, only one task can communicate with the modem at a time. This prevents AT commands from being mixed or corrupted.

### 5. AT Parsing and MQTT Telemetry

Cellular communication may sometimes be unstable. Because of this, the modem communication code is written defensively in `mqtt.c` and `cellular.c`.

- Network registration is checked with `+CGREG` before a data socket is opened.
- During MQTT publishing, the parser waits for the exact `>` prompt from the modem before the message payload is sent.
- Special cases are handled safely, such as ignoring the `0x1A` CTRL+Z termination character when it should not be stored in string buffers.

This makes the communication flow more reliable during real network conditions.

## Unit Testing Setup

This project was developed with a **Test-Driven Development** approach by using **CppUTest**.

The tests are built and run directly on a host computer such as **macOS** or **Linux**. This means that most logic can be tested without connecting or flashing the STM32 board.

### Mocking Strategy

Since a normal computer does not have the same STM32 peripherals, hardware dependencies are mocked inside the `tests/mocks/` folder.

- **HAL Mocks**  
  Fake structures are used for STM32 types such as `I2S_HandleTypeDef`. This allows the tests to manually call functions like `HAL_I2S_RxHalfCpltCallback()` and check how the system reacts.

- **Modem Spies**  
  The mock AT layer can read the payload sent by the application and inject fake modem responses. For example, a fake network registration response such as `+CGREG: 0,1` can be used to test the cellular state machine.

To run the unit tests locally:

```bash
cd tests
make
```

## Project Structure

```text
speech_recognition_project/
├── Core/
│   ├── App/            <-- Processing logic: audio_dsp.c, ai_app.c, mqtt.c
│   ├── Inc/            <-- FreeRTOS and HAL headers
│   └── Src/            <-- Main loop and hooks: main.c, freertos.c
├── Drivers/
│   ├── drv_microphone/ <-- Ping-Pong I2S and CMSIS FIR filter driver
│   ├── drv_atcommand/  <-- Thread-safe AT command execution driver
│   └── CMSIS/STM32...  <-- STM32 hardware abstraction layers
├── Middlewares/
│   ├── Custom/Cellular <-- Custom cellular logic built over the AT driver
│   ├── ST/AI           <-- X-CUBE-AI generated files
│   └── Third_Party/    <-- FreeRTOS kernel source code
├── tests/              <-- Host-based unit testing folder
│   ├── Makefile        <-- Build rules for native compilation
│   ├── mocks/          <-- Hardware and modem mocks
│   └── test_*.cpp      <-- Module unit tests
└── .gitignore          <-- Ignored binaries, object files, and IDE configs
```

## Acknowledgment

The Speech Recognition AI model was taken from the repository below, which also inspired the embedded speech recognition approach used in this project.

<https://github.com/Med-Karim-Ben-Boubaker/Embedded-Speech-Recognition-STM32F407>

AI tools were also used as development assistance during documentation, debugging, and code review. However, the implementation, integration, testing, and final project decisions were carried out by us.
