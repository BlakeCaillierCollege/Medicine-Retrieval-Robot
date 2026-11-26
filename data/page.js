const statusText = document.getElementById('status');

async function refreshStatus() {
  try {
    const response = await fetch('/status');
    if (!response.ok) throw new Error('Status request failed');
    const command = (await response.text()).trim();
    statusText.textContent = command === 'F' ? 'Last command: Forward' : 'Last command: Stop';
  } catch (error) {
    statusText.textContent = 'Could not read robot status';
  }
}

async function sendCommand(command) {
  try {
    const response = await fetch('/command', { method: 'POST', body: command });
    if (!response.ok) throw new Error('Command request failed');
    await refreshStatus();
  } catch (error) {
    statusText.textContent = 'Command failed; check the robot connection';
  }
}

document.querySelectorAll('[data-command]').forEach(button => {
  button.addEventListener('click', () => sendCommand(button.dataset.command));
});

refreshStatus();
