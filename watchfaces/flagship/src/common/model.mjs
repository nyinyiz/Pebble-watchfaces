import { formatBattery, formatConnection, formatDate, formatTime } from './format.mjs';

function pad2(n) {
  return String(n).padStart(2, '0');
}

function getAccentName(hours) {
  if (hours < 6) {
    return 'night';
  }

  if (hours < 12) {
    return 'morning';
  }

  if (hours < 18) {
    return 'day';
  }

  return 'night';
}

export function buildFaceModel(state) {
  const accentName = getAccentName(state.date.getHours());
  const meridiem = state.is24Hour ? '' : state.date.getHours() >= 12 ? 'PM' : 'AM';
  const batteryBase = formatBattery(state.batteryPercent);
  let batteryText = batteryBase;

  if (state.isCharging) {
    batteryText = `${batteryBase} CHG`;
  } else if (state.batteryPercent <= 20) {
    batteryText = `${batteryBase} LOW`;
  }

  return {
    palette: {
      accentName
    },
    time: {
      text: formatTime(state.date, state.is24Hour),
      meridiem
    },
    date: {
      text: formatDate(state.date)
    },
    statusLeft: {
      text: batteryText
    },
    statusCenter: {
      text: `SEC ${pad2(state.date.getSeconds())}`
    },
    statusRight: {
      text: formatConnection(state.isConnected)
    }
  };
}
