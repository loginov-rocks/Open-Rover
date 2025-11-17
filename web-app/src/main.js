// UI elements.
const deviceNameLabel = document.getElementById('device-name');
const connectButton = document.getElementById('connect');
const disconnectButton = document.getElementById('disconnect');
const joystickArea = document.getElementById('joystick-area');
const joystickContainer = document.getElementById('joystick');
const joystickHandle = document.getElementById('joystick-handle');
const terminalContainer = document.getElementById('terminal');
const toggleJoystickButton = document.getElementById('toggle-joystick');
const messageForm = document.getElementById('message-form');
const messageInput = document.getElementById('message-input');

// Helpers.
const defaultDeviceName = 'Web Bluetooth Terminal';
const terminalAutoScrollingLimit = terminalContainer.offsetHeight / 2;
let isTerminalAutoScrolling = true;
let isJoystickVisible = false;

// Echo resolver for send chain.
let echoResolver = null;

// Timing measurement.
const pendingMessages = new Map(); // messageId -> sendTimestamp
let messageIdCounter = 0;
const timingSamples = []; // Keep all samples for average

const logToTerminal = (message, type = '') => {
  terminalContainer.insertAdjacentHTML('beforeend', `<div${type && ` class="${type}"`}>${message}</div>`);

  if (isTerminalAutoScrolling) {
    const scrollTop = terminalContainer.scrollHeight - terminalContainer.offsetHeight;

    if (scrollTop > 0) {
      terminalContainer.scrollTop = scrollTop;
    }
  }
};

// Create a BluetoothTerminal instance with the default configuration.
const bluetoothTerminal = new BluetoothTerminal({
  // serviceUuid: 0xFFE0,
  // characteristicUuid: 0xFFE1,
  // characteristicValueSize: 20,
  // receiveSeparator: '\n',
  // sendSeparator: '\n',
  // logLevel: 'log',
});

// Helper function to send a message with timing measurement
const sendMessageWithTiming = async (message) => {
  const messageId = messageIdCounter++;
  const sendTime = performance.now();

  try {
    await bluetoothTerminal.send(message);
    // Only track if send succeeded
    pendingMessages.set(messageId, sendTime);
  } catch (error) {
    // Decrement counter since this ID won't be used
    messageIdCounter--;
    throw error;
  }
};

// Set a callback that will be called when an incoming message from the connected device is received.
bluetoothTerminal.onReceive((message) => {
  const receiveTime = performance.now();

  // Measure round-trip time if we have pending messages.
  if (pendingMessages.size > 0) {
    // Get the oldest pending message (FIFO).
    const [messageId, sendTime] = pendingMessages.entries().next().value;
    pendingMessages.delete(messageId);

    const roundTripTime = Math.round(receiveTime - sendTime);
    timingSamples.push(roundTripTime);

    // Calculate average across all samples.
    const average = Math.round(timingSamples.reduce((a, b) => a + b, 0) / timingSamples.length);

    logToTerminal(`Round-trip: ${roundTripTime}ms, average: ${average}ms (${timingSamples.length} samples)`, 'timing');
  }

  // Signal waiting sender that echo was received.
  if (echoResolver) {
    echoResolver(message);
    echoResolver = null;
  }

  logToTerminal(message, 'incoming');
});

// Set a callback that will be called every time any log message is produced by the class, regardless of the log level
// set.
bluetoothTerminal.onLog((logLevel, method, message) => {
  // Ignore debug messages.
  if (logLevel === 'debug') {
    return;
  }

  logToTerminal(message);
});

const joystick = new Joystick({
  area: joystickArea,
  document,
  fractionDigits: 2,
  handle: joystickHandle,
});

// Wait for echo response from rover.
const waitForEcho = () => {
  return new Promise((resolve, reject) => {
    echoResolver = resolve;

    // Timeout safety net.
    setTimeout(() => {
      if (echoResolver === resolve) {
        echoResolver = null;
        reject(new Error('Echo timeout'));
      }
    }, 2000);
  });
};

// Continuously send joystick position while visible.
const processSendChain = async () => {
  while (isJoystickVisible) {
    const x = joystick.getX();
    const y = joystick.getY();
    const message = JSON.stringify({x, y});

    try {
      await sendMessageWithTiming(message);
      await waitForEcho();
      logToTerminal(message, 'outgoing');
    } catch (error) {
      logToTerminal(error, 'error');
      break;
    }
  }
};


// Bind event listeners to the UI elements.
connectButton.addEventListener('click', async () => {
  try {
    // Open the browser Bluetooth device picker to select a device if none was previously selected, establish a
    // connection with the selected device, and initiate communication.
    await bluetoothTerminal.connect();
  } catch (error) {
    logToTerminal(error, 'error');

    return;
  }

  // Retrieve the name of the currently connected device.
  deviceNameLabel.textContent = bluetoothTerminal.getDeviceName() || defaultDeviceName;
});

disconnectButton.addEventListener('click', () => {
  try {
    // Disconnect from the currently connected device and clean up associated resources.
    bluetoothTerminal.disconnect();
  } catch (error) {
    logToTerminal(error, 'error');

    return;
  }

  deviceNameLabel.textContent = defaultDeviceName;
});

messageForm.addEventListener('submit', async (event) => {
  event.preventDefault();

  try {
    // Send a message to the connected device.
    await sendMessageWithTiming(messageInput.value);
  } catch (error) {
    logToTerminal(error, 'error');

    return;
  }

  logToTerminal(messageInput.value, 'outgoing');

  messageInput.value = '';
  messageInput.focus();
});

// Enable terminal auto-scrolling if it scrolls beyond the bottom.
terminalContainer.addEventListener('scroll', () => {
  const scrollTopOffset = terminalContainer.scrollHeight - terminalContainer.offsetHeight - terminalAutoScrollingLimit;

  isTerminalAutoScrolling = (scrollTopOffset < terminalContainer.scrollTop);
});

toggleJoystickButton.addEventListener('click', () => {
  isJoystickVisible = !isJoystickVisible;

  if (isJoystickVisible) {
    joystickContainer.classList.add('visible');
    processSendChain();
  } else {
    joystickContainer.classList.remove('visible');
  }
});
