function formatTime(date, is24Hour) {
  const hours24 = date.getHours();
  const minutes = String(date.getMinutes()).padStart(2, '0');

  if (is24Hour) {
    return `${hours24}:${minutes}`;
  }

  const hours12 = hours24 % 12 || 12;
  return `${hours12}:${minutes}`;
}

function formatDate(date) {
  const weekdays = ['SUN', 'MON', 'TUE', 'WED', 'THU', 'FRI', 'SAT'];
  const months = ['JAN', 'FEB', 'MAR', 'APR', 'MAY', 'JUN', 'JUL', 'AUG', 'SEP', 'OCT', 'NOV', 'DEC'];

  return `${weekdays[date.getDay()]} ${date.getDate()} ${months[date.getMonth()]}`;
}

function formatBattery(percent) {
  return `${Math.round(percent)}%`;
}

function formatConnection(isConnected) {
  return isConnected ? 'CONNECTED' : 'OFFLINE';
}

module.exports = {
  formatBattery,
  formatConnection,
  formatDate,
  formatTime
};
