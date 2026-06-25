var PRESETS = {
  'early':    { wake: 5,  sleep: 21 },
  'standard': { wake: 7,  sleep: 23 },
  'nightowl': { wake: 10, sleep: 2 }
};

Pebble.addEventListener('showConfiguration', function() {
  var wake = localStorage.getItem('wake_hour') || '7';
  var sleep = localStorage.getItem('sleep_hour') || '23';

  var html = '<!DOCTYPE html><html><head>' +
    '<meta name="viewport" content="width=device-width,initial-scale=1">' +
    '<title>Energy Config</title>' +
    '<style>' +
    'body{font-family:-apple-system,sans-serif;background:#1a1a1a;color:#fff;' +
    'margin:0;padding:20px;text-align:center}' +
    'h2{color:#4CAF50;margin-bottom:5px}' +
    'p.sub{color:#888;font-size:14px;margin-top:0}' +
    '.presets{display:flex;gap:10px;justify-content:center;margin:20px 0}' +
    '.preset{flex:1;padding:12px 8px;border:2px solid #333;border-radius:8px;' +
    'background:#222;color:#fff;font-size:14px;cursor:pointer}' +
    '.preset.active{border-color:#4CAF50;background:#1b3a1b}' +
    '.custom{margin:20px auto;max-width:280px}' +
    'label{display:block;color:#aaa;font-size:14px;margin:12px 0 4px;text-align:left}' +
    'select{width:100%;padding:10px;border:1px solid #333;border-radius:6px;' +
    'background:#222;color:#fff;font-size:16px;appearance:auto}' +
    '.save{margin-top:24px;padding:14px 40px;background:#4CAF50;color:#fff;' +
    'border:none;border-radius:8px;font-size:16px;font-weight:bold;cursor:pointer}' +
    '</style></head><body>' +
    '<h2>Energy</h2><p class="sub">Configure your schedule</p>' +
    '<div class="presets">' +
    '<button class="preset" onclick="setPreset(\'early\')">Early Bird<br><small>5am-9pm</small></button>' +
    '<button class="preset" onclick="setPreset(\'standard\')">Standard<br><small>7am-11pm</small></button>' +
    '<button class="preset" onclick="setPreset(\'nightowl\')">Night Owl<br><small>10am-2am</small></button>' +
    '</div>' +
    '<div class="custom">' +
    '<label>Wake Hour</label><select id="wake">' + buildOptions(0, 23, parseInt(wake)) + '</select>' +
    '<label>Sleep Hour</label><select id="sleep">' + buildOptions(0, 23, parseInt(sleep)) + '</select>' +
    '</div>' +
    '<button class="save" onclick="save()">Save</button>' +
    '<script>' +
    'var presets=' + JSON.stringify(PRESETS) + ';' +
    'function setPreset(k){' +
    'document.getElementById("wake").value=presets[k].wake;' +
    'document.getElementById("sleep").value=presets[k].sleep;' +
    'document.querySelectorAll(".preset").forEach(function(b){b.classList.remove("active")});' +
    'event.target.closest(".preset").classList.add("active");}' +
    'document.querySelectorAll("select").forEach(function(s){' +
    's.addEventListener("change",function(){' +
    'document.querySelectorAll(".preset").forEach(function(b){b.classList.remove("active")})})});' +
    'function save(){' +
    'var w=document.getElementById("wake").value;' +
    'var s=document.getElementById("sleep").value;' +
    'var result=encodeURIComponent(JSON.stringify({wake_hour:parseInt(w),sleep_hour:parseInt(s)}));' +
    'document.location="pebblejs://close#"+result;}' +
    'function highlightPreset(){var w=parseInt(document.getElementById("wake").value);' +
    'var s=parseInt(document.getElementById("sleep").value);' +
    'var btns=document.querySelectorAll(".preset");' +
    'btns.forEach(function(b){b.classList.remove("active")});' +
    'for(var k in presets){if(presets[k].wake===w&&presets[k].sleep===s){btns[' +
    'k==="early"?0:k==="standard"?1:2].classList.add("active");break;}}}' +
    'highlightPreset();' +
    '</script></body></html>';

  Pebble.openURL('data:text/html,' + encodeURIComponent(html));
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (e && e.response && e.response.length > 0) {
    try {
      var config = JSON.parse(decodeURIComponent(e.response));
      localStorage.setItem('wake_hour', config.wake_hour.toString());
      localStorage.setItem('sleep_hour', config.sleep_hour.toString());

      Pebble.sendAppMessage({
        'wake_hour': config.wake_hour,
        'sleep_hour': config.sleep_hour
      }, function() {
        console.log('Config sent successfully');
      }, function() {
        console.log('Config send failed');
      });
    } catch (err) {
      console.log('Config parse error: ' + err);
    }
  }
});

function buildOptions(min, max, selected) {
  var opts = '';
  for (var i = min; i <= max; i++) {
    var label = (i < 10 ? '0' : '') + i + ':00';
    opts += '<option value="' + i + '"' + (i === selected ? ' selected' : '') + '>' + label + '</option>';
  }
  return opts;
}
