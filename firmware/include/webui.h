// =====================================================================
//  AquaFeeder -- interface web (pagina unica, servida da flash)
// =====================================================================
#pragma once
#include <Arduino.h>

static const char INDEX_HTML[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="pt-BR"><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>AquaFeeder</title>
<style>
:root{--bg:#11151a;--card:#1a2027;--line:#2a333d;--fg:#e6edf3;--dim:#8b98a5;
--acc:#3ddc97;--acc2:#2b8cff;--warn:#ffb454;--err:#ff6b6b}
*{box-sizing:border-box}
body{margin:0;font:15px/1.45 system-ui,-apple-system,Segoe UI,Roboto,sans-serif;
background:var(--bg);color:var(--fg)}
header{display:flex;align-items:center;gap:10px;padding:12px 14px;
background:var(--card);border-bottom:1px solid var(--line);position:sticky;top:0;z-index:5}
header h1{font-size:16px;margin:0;font-weight:600;letter-spacing:.3px}
.dot{width:9px;height:9px;border-radius:50%;background:var(--err);flex:0 0 auto}
.dot.on{background:var(--acc);box-shadow:0 0 8px var(--acc)}
main{padding:12px;max-width:760px;margin:0 auto}
nav{display:flex;gap:6px;margin-bottom:12px;overflow:auto}
nav button{flex:1;min-width:78px;padding:9px 6px;border:1px solid var(--line);
background:var(--card);color:var(--dim);border-radius:9px;font-size:13px;cursor:pointer}
nav button.sel{color:var(--fg);border-color:var(--acc);background:#16241f}
.card{background:var(--card);border:1px solid var(--line);border-radius:12px;
padding:14px;margin-bottom:12px}
.row{display:flex;gap:8px;align-items:center;flex-wrap:wrap}
.grid{display:grid;grid-template-columns:1fr 1fr;gap:10px}
label{font-size:12px;color:var(--dim);display:block;margin-bottom:3px}
input,select{width:100%;padding:8px;background:#0e1216;color:var(--fg);
border:1px solid var(--line);border-radius:8px;font-size:14px}
input[type=checkbox]{width:auto}
button.b{padding:11px 14px;border:0;border-radius:10px;font-size:14px;font-weight:600;
cursor:pointer;background:#243040;color:var(--fg)}
button.p{background:var(--acc);color:#08150f}
button.s{background:#243040}
button.w{background:#3a2f1c;color:var(--warn)}
button.b:disabled{opacity:.45;cursor:not-allowed}
.big{font-size:26px;font-weight:700}
.dim{color:var(--dim);font-size:12px}
table{width:100%;border-collapse:collapse;font-size:13px}
td,th{padding:6px 4px;border-bottom:1px solid var(--line);text-align:left}
th{color:var(--dim);font-weight:500;font-size:11px;text-transform:uppercase}
img#cam{width:100%;border-radius:10px;background:#000;display:block;min-height:120px}
.dow{display:flex;gap:3px}
.dow b{flex:1;text-align:center;padding:5px 0;font-size:11px;border-radius:6px;
background:#0e1216;color:var(--dim);cursor:pointer;border:1px solid var(--line);font-weight:500}
.dow b.on{background:#16241f;color:var(--acc);border-color:var(--acc)}
.slot{border-bottom:1px solid var(--line);padding:10px 0}
.slot:last-child{border:0}
#toast{position:fixed;left:50%;bottom:18px;transform:translateX(-50%);
background:#0b0f13;border:1px solid var(--line);padding:10px 16px;border-radius:10px;
font-size:13px;opacity:0;transition:.25s;pointer-events:none;max-width:90vw;z-index:20}
#toast.on{opacity:1}
#gate{position:fixed;inset:0;background:var(--bg);display:none;place-items:center;z-index:30}
#gate.on{display:grid}
.hint{font-size:11px;color:var(--dim);margin-top:6px;line-height:1.4}
.netitem{display:flex;justify-content:space-between;align-items:center;gap:8px;
padding:9px 10px;border:1px solid var(--line);border-radius:8px;margin-top:6px;
background:#0e1216;cursor:pointer;font-size:14px}
.netitem.sel{border-color:var(--acc);background:#16241f}
.chip{font-size:10px;border:1px solid var(--line);border-radius:20px;padding:1px 8px;
margin-left:6px;vertical-align:middle;color:var(--dim);font-weight:500}
.chip.ok{color:var(--acc);border-color:var(--acc);background:#16241f}
</style></head><body>

<header><span class="dot" id="dot"></span><h1>AquaFeeder</h1>
<span class="dim" id="hdr" style="margin-left:auto"></span></header>

<main>
<nav>
 <button class="sel" data-t="ag">Agora</button>
 <button data-t="sc">Agenda</button>
 <button data-t="cf">Ajustes</button>
 <button data-t="lg">Histórico</button>
</nav>

<!-- ================= AGORA ================= -->
<section id="ag">
 <div class="card">
  <div class="row" style="justify-content:space-between">
   <div><div class="dim">Porções hoje</div><div class="big" id="fed">-</div></div>
   <div><div class="dim">Última</div><div id="last" style="font-size:15px">-</div></div>
   <div><div class="dim">Relógio</div><div id="clock" style="font-size:15px">-</div></div>
  </div>
 </div>
 <div class="card">
  <label>Alimentar agora</label>
  <div class="row">
   <button class="b p" style="flex:1" onclick="feed(1)">1 porção</button>
   <button class="b p" style="flex:1" onclick="feed(2)">2 porções</button>
   <button class="b p" style="flex:1" onclick="feed(3)">3 porções</button>
  </div>
  <div class="hint">1 porção &asymp; 0,23 ml (1/4 de volta da rosca). Calibre em Ajustes.</div>
 </div>
 <div class="card">
  <div class="row" style="justify-content:space-between">
   <label style="margin:0">Câmera</label>
   <div class="row">
    <button class="b s" onclick="snap()">Foto</button>
    <button class="b s" id="btnLive" onclick="live()">Ligar vídeo</button>
   </div>
  </div>
  <img id="cam" alt="sem imagem">
  <div class="hint" id="camHint"></div>
 </div>
 <div class="card">
  <label>Manutenção da rosca</label>
  <div class="row">
   <button class="b w" style="flex:1" onclick="jog(-1024)">&#8630; Recuar</button>
   <button class="b w" style="flex:1" onclick="jog(1024)">Avançar &#8631;</button>
  </div>
  <div class="hint">Use para desentupir ou para encher a rosca depois de limpar.
  Avançar joga ração no aquário e NÃO conta no limite diário.</div>
 </div>
</section>

<!-- ================= AGENDA ================= -->
<section id="sc" hidden>
 <div class="card" id="slots"></div>
 <button class="b p" style="width:100%" onclick="saveSlots()">Salvar agenda</button>
 <div class="hint">Se faltar luz na hora marcada, o alimentador recupera a refeição
 quando voltar (dentro da janela configurada em Ajustes).</div>
</section>

<!-- ================= AJUSTES ================= -->
<section id="cf" hidden>
 <div class="card">
  <label>Dosagem</label>
  <div class="grid">
   <div><label>Passos por porção</label><input id="c_steps" type="number" min="40" max="20000"></div>
   <div><label>µs por passo</label><input id="c_us" type="number" min="1000" max="8000"></div>
   <div><label>Inverter sentido</label><select id="c_rev"><option value="0">Não</option><option value="1">Sim</option></select></div>
   <div><label>Sensor de grãos</label><select id="c_sens"><option value="0">Desligado</option><option value="1">Ligado</option></select></div>
  </div>
  <div class="hint">Meia volta = 2048 passos. Se a ração sair para o lado errado,
  inverta o sentido. Mudar o sensor exige reiniciar.</div>
 </div>
 <div class="card">
  <label>Segurança</label>
  <div class="grid">
   <div><label>Máx. porções/dia</label><input id="c_day" type="number" min="1" max="60"></div>
   <div><label>Máx. por vez</label><input id="c_req" type="number" min="1" max="20"></div>
   <div><label>Intervalo mín. (s)</label><input id="c_int" type="number" min="0" max="3600"></div>
   <div><label>Janela de recuperação (min)</label><input id="c_cat" type="number" min="0" max="720"></div>
  </div>
 </div>
 <div class="card">
  <label>Câmera</label>
  <div class="grid">
   <div><label>Resolução</label><select id="c_csz">
     <option value="0">320x240</option><option value="1">640x480</option>
     <option value="2">800x600</option><option value="3">1280x720</option></select></div>
   <div><label>Qualidade (10=melhor)</label><input id="c_cq" type="number" min="10" max="30"></div>
   <div><label>Girar vertical</label><select id="c_cv"><option value="0">Não</option><option value="1">Sim</option></select></div>
   <div><label>Espelhar</label><select id="c_ch"><option value="0">Não</option><option value="1">Sim</option></select></div>
  </div>
  <div style="margin-top:10px"><label>Câmera externa (URL de imagem/MJPEG)</label>
   <input id="c_cext" placeholder="vazio = usar a câmera do ESP32"></div>
  <div class="hint">Não tem câmera no ESP32? Use um celular velho com o app
  <b>IP Webcam</b>, ou qualquer câmera Wi-Fi que sirva MJPEG/JPEG, e cole a URL aqui
  (ex.: <code>http://192.168.0.50:8080/video</code>). O navegador busca a imagem
  direto — o alimentador não faz proxy.</div>
 </div>
 <div class="card">
  <label>Telegram (controle fora de casa)</label>
  <div><label>Token do bot <span id="tgSaved" class="chip"></span></label>
   <div class="row" style="flex-wrap:nowrap">
    <input id="c_tgt" placeholder="deixe vazio para desligar">
    <button class="b s" type="button" style="flex:0 0 auto" onclick="copySecret('tgToken','#c_tgt',this)">Copiar</button>
   </div></div>
  <div style="margin-top:8px"><label>Chat ID autorizado</label><input id="c_tgc"></div>
  <div style="margin-top:8px"><label>Avisar a cada alimentação</label>
   <select id="c_tgn"><option value="0">Não</option><option value="1">Sim</option></select></div>
  <div class="hint">Comandos: /alimentar [n] &middot; /foto &middot; /status &middot; /agenda</div>
 </div>
 <div class="card">
  <label>MQTT (comando pela nuvem)</label>
  <div class="grid">
   <div><label>Broker</label><input id="c_mqh" placeholder="vazio = desligado"></div>
   <div><label>Porta</label><input id="c_mqp" type="number" min="1" max="65535"></div>
   <div><label>Usuario</label><input id="c_mqu"></div>
   <div><label>Senha <span id="mqwSaved" class="chip"></span></label><input id="c_mqw" type="password" placeholder="(nao alterar)"></div>
   <div><label>Prefixo dos topicos</label><input id="c_mqx"></div>
   <div><label>TLS</label><select id="c_mqt"><option value="1">Sim (8883)</option><option value="0">Nao (1883)</option></select></div>
  </div>
  <div class="hint">O aparelho conecta <b>para fora</b> no broker e fica escutando
  <code>prefixo/cmd</code>. Publique <code>feed 2</code> ali e ele alimenta. O broker
  publica <code>prefixo/online</code> = 0 sozinho se o alimentador cair — é o alarme
  de "parou de funcionar" que o Telegram não te dá.</div>
 </div>
 <div class="card">
  <label>Rede e sistema</label>
  <div class="grid">
   <div><label>Wi-Fi (SSID)</label><input id="c_ssid"></div>
   <div><label>Senha do Wi-Fi <span id="wpSaved" class="chip"></span></label><input id="c_wpass" type="password" placeholder="(não alterar)"></div>
   <div><label>Nome na rede</label><input id="c_host"></div>
   <div><label>Fuso (TZ POSIX)</label><input id="c_tz"></div>
   <div><label>Senha da interface <span id="uiSaved" class="chip"></span></label><input id="c_ui" type="password" placeholder="(não alterar)"></div>
  </div>
  <button class="b s" style="width:100%;margin-top:8px" onclick="scanWifi(this)">Procurar redes 2,4 GHz</button>
  <div id="netlist"></div>
  <div class="hint">Toque em <b>Procurar redes</b> e escolha a sua na lista (só aparecem redes de 2,4 GHz,
  as únicas que o alimentador consegue usar) — depois é só digitar a senha.
  Fuso do Brasil: <code>&lt;-03&gt;3</code>. Trocar Wi-Fi ou nome reinicia o aparelho.</div>
 </div>
 <div class="row">
  <button class="b p" style="flex:2" onclick="saveCfg()">Salvar ajustes</button>
  <button class="b s" style="flex:1" onclick="reboot()">Reiniciar</button>
 </div>
</section>

<!-- ================= HISTORICO ================= -->
<section id="lg" hidden>
 <div class="card"><table><thead><tr><th>Quando</th><th>Porções</th><th>Origem</th><th>Grãos</th></tr></thead>
 <tbody id="logb"></tbody></table></div>
</section>
</main>

<div id="toast"></div>
<div id="gate"><div class="card" style="width:300px">
 <h3 style="margin:0 0 10px">Entrar</h3>
 <input id="pw" type="password" placeholder="senha" onkeydown="if(event.key=='Enter')login()">
 <button class="b p" style="width:100%;margin-top:10px" onclick="login()">Entrar</button>
</div></div>

<script>
const $=s=>document.querySelector(s), $$=s=>document.querySelectorAll(s);
const esc=s=>String(s).replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;').replace(/"/g,'&quot;');
const DOW=['D','S','T','Q','Q','S','S'];
let streaming=false, cfgCache=null;

function toast(m,bad){const t=$('#toast');t.textContent=m;t.style.borderColor=bad?'var(--err)':'var(--acc)';
 t.classList.add('on');clearTimeout(t._h);t._h=setTimeout(()=>t.classList.remove('on'),2800)}

async function api(path,opt){
 const r=await fetch(path,Object.assign({cache:'no-store'},opt));
 if(r.status==401){$('#gate').classList.add('on');throw new Error('auth')}
 const txt=await r.text();
 let j={}; try{j=JSON.parse(txt)}catch(e){}
 if(!r.ok) throw new Error(j.err||('HTTP '+r.status));
 return j;
}
async function login(){
 try{await api('/api/login',{method:'POST',body:JSON.stringify({pass:$('#pw').value})});
  $('#gate').classList.remove('on');boot()}catch(e){toast('Senha incorreta',1)}
}

$$('nav button').forEach(b=>b.onclick=()=>{
 $$('nav button').forEach(x=>x.classList.remove('sel'));b.classList.add('sel');
 ['ag','sc','cf','lg'].forEach(id=>$('#'+id).hidden=(id!=b.dataset.t));
 if(b.dataset.t=='lg')loadLog();
});

async function feed(n){
 try{await api('/api/feed?p='+n,{method:'POST'});toast('Alimentando '+n+' porção(ões)');
  setTimeout(status,1500)}catch(e){toast(e.message,1)}
}
async function jog(s){
 try{await api('/api/jog?s='+s,{method:'POST'});toast('Movendo rosca...')}
 catch(e){toast(e.message,1)}
}
async function reboot(){
 if(!confirm('Reiniciar o alimentador?'))return;
 try{await api('/api/reboot',{method:'POST'})}catch(e){}
 toast('Reiniciando...');
}
function extUrl(){return (cfgCache&&cfgCache.camExtUrl)?cfgCache.camExtUrl:''}
function snap(){
 const u=extUrl();
 $('#cam').src=(u?u+(u.indexOf('?')<0?'?':'&'):'/snapshot.jpg?')+'t='+Date.now();
 streaming=false;$('#btnLive').textContent='Ligar vídeo';
}
function live(){
 const img=$('#cam'), u=extUrl();
 if(streaming){img.src='';streaming=false;$('#btnLive').textContent='Ligar vídeo';return}
 img.src=u ? u+(u.indexOf('?')<0?'?':'&')+'t='+Date.now()
           : location.protocol+'//'+location.hostname+':81/stream?t='+Date.now();
 streaming=true;$('#btnLive').textContent='Parar vídeo';
}

async function status(){
 try{
  const s=await api('/api/status');
  $('#dot').classList.toggle('on',!!s.wifi);
  $('#fed').textContent=s.fedToday+' / '+s.maxPerDay;
  $('#last').textContent=s.lastFeed||'nunca';
  $('#clock').textContent=s.time||'--:--';
  $('#hdr').textContent=(s.ip||'')+' · '+s.rssi+'dBm';
  $('#camHint').textContent = extUrl() ? ('câmera externa: '+extUrl())
     : (s.cam ? 'câmera ok' : ('câmera indisponível: '+(s.camErr||'')));
 }catch(e){$('#dot').classList.remove('on')}
}

function slotRow(i,s){
 return `<div class="slot"><div class="row">
  <input type="checkbox" ${s.en?'checked':''} data-k="en" data-i="${i}">
  <input type="time" style="width:110px" value="${String(s.h).padStart(2,'0')}:${String(s.m).padStart(2,'0')}" data-k="t" data-i="${i}">
  <input type="number" style="width:64px" min="1" max="20" value="${s.p}" data-k="p" data-i="${i}">
  <span class="dim">porções</span></div>
  <div class="dow" style="margin-top:6px" data-i="${i}">
  ${DOW.map((d,x)=>`<b class="${(s.dow>>x)&1?'on':''}" data-b="${x}">${d}</b>`).join('')}
  </div></div>`;
}
function renderSlots(sl){
 $('#slots').innerHTML=sl.map((s,i)=>slotRow(i,s)).join('');
 $$('.dow b').forEach(b=>b.onclick=()=>b.classList.toggle('on'));
}
async function saveSlots(){
 const sl=[];
 for(let i=0;i<8;i++){
  const t=document.querySelector(`[data-k="t"][data-i="${i}"]`).value.split(':');
  let dow=0;
  document.querySelectorAll(`.dow[data-i="${i}"] b`).forEach(b=>{if(b.classList.contains('on'))dow|=1<<(+b.dataset.b)});
  sl.push({en:document.querySelector(`[data-k="en"][data-i="${i}"]`).checked,
   h:+t[0],m:+t[1],dow:dow,
   p:+document.querySelector(`[data-k="p"][data-i="${i}"]`).value});
 }
 try{await api('/api/config',{method:'POST',body:JSON.stringify({slots:sl})});
  toast('Agenda salva')}catch(e){toast(e.message,1)}
}

function fillCfg(c){
 cfgCache=c;
 $('#c_steps').value=c.stepsPerPortion; $('#c_us').value=c.stepUs;
 $('#c_rev').value=c.reverse?1:0;       $('#c_sens').value=c.sensorEnabled?1:0;
 $('#c_day').value=c.maxPerDay;         $('#c_req').value=c.maxPerRequest;
 $('#c_int').value=c.minIntervalS;      $('#c_cat').value=c.catchUpMin;
 $('#c_csz').value=c.camSize;           $('#c_cq').value=c.camQuality;
 $('#c_cv').value=c.camVflip?1:0;       $('#c_ch').value=c.camHmirror?1:0;
 $('#c_cext').value=c.camExtUrl||'';
 $('#c_mqh').value=c.mqttHost||''; $('#c_mqp').value=c.mqttPort||8883;
 $('#c_mqu').value=c.mqttUser||''; $('#c_mqx').value=c.mqttPrefix||'aquafeeder';
 $('#c_mqt').value=c.mqttTls?1:0;
 $('#c_tgc').value=c.tgChat||'';        $('#c_tgn').value=c.tgNotify?1:0;
 $('#c_ssid').value=c.ssid||'';         $('#c_host').value=c.host||'';
 $('#c_tz').value=c.tz||'';
 setSaved('#tgSaved', c.tgEnabled, '#c_tgt');
 setSaved('#uiSaved', c.hasUiPass, '#c_ui');
 setSaved('#wpSaved', c.hasWifiPass, '#c_wpass');
 setSaved('#mqwSaved', c.mqttHasPass, '#c_mqw');
 renderSlots(c.slots);
}
// mostra "salvo" ao lado do campo -- o firmware nunca devolve o segredo em si
function setSaved(chipSel, on, inputSel){
 const c=$(chipSel); if(c){c.textContent=on?'✓ salvo':'vazio';c.className='chip'+(on?' ok':'')}
 const i=inputSel&&$(inputSel); if(i&&on)i.placeholder='•••••••• salvo — deixe em branco p/ manter';
}
// busca o segredo (autenticado) so na hora de copiar, revela no campo e copia
async function copySecret(key,sel,btn){
 const el=$(sel);
 try{
  const c=await api('/api/config?secrets=1');
  const v=(c[key]||'').toString();
  if(!v){toast('Nada salvo para copiar',1);return}
  el.type='text'; el.value=v;
  let ok=false;
  try{await navigator.clipboard.writeText(v);ok=true}catch(e){}
  if(!ok){el.select();try{ok=document.execCommand('copy')}catch(e){}}
  toast(ok?'Copiado ✓':'Revelado no campo — selecione e copie');
 }catch(e){toast(e.message,1)}
}
async function saveCfg(){
 const b={stepsPerPortion:+$('#c_steps').value,stepUs:+$('#c_us').value,
  reverse:$('#c_rev').value=='1',sensorEnabled:$('#c_sens').value=='1',
  maxPerDay:+$('#c_day').value,maxPerRequest:+$('#c_req').value,
  minIntervalS:+$('#c_int').value,catchUpMin:+$('#c_cat').value,
  camSize:+$('#c_csz').value,camQuality:+$('#c_cq').value,
  camVflip:$('#c_cv').value=='1',camHmirror:$('#c_ch').value=='1',
  camExtUrl:$('#c_cext').value,
  mqttHost:$('#c_mqh').value,mqttPort:+$('#c_mqp').value,
  mqttUser:$('#c_mqu').value,mqttPrefix:$('#c_mqx').value,
  mqttTls:$('#c_mqt').value=='1',
  tgChat:$('#c_tgc').value,tgNotify:$('#c_tgn').value=='1',
  ssid:$('#c_ssid').value,host:$('#c_host').value,tz:$('#c_tz').value};
 if($('#c_tgt').value) b.tgToken=$('#c_tgt').value;
 if($('#c_wpass').value) b.pass=$('#c_wpass').value;
 if($('#c_ui').value) b.uiPass=$('#c_ui').value;
 if($('#c_mqw').value) b.mqttPass=$('#c_mqw').value;
 try{const r=await api('/api/config',{method:'POST',body:JSON.stringify(b)});
  toast(r.reboot?'Salvo. Reiniciando...':'Ajustes salvos')}catch(e){toast(e.message,1)}
}

async function scanWifi(btn){
 const box=$('#netlist'), old=btn?btn.textContent:'';
 if(btn){btn.disabled=true;btn.textContent='Procurando...'}
 box.innerHTML='<div class="dim" style="padding:8px 2px">procurando redes 2,4 GHz...</div>';
 try{
  const r=await api('/api/scan');
  const nets=(r.nets||[]).sort((a,b)=>b.rssi-a.rssi);
  box.innerHTML = nets.length ? nets.map(n=>{
   const q=n.rssi>=-60?4:n.rssi>=-70?3:n.rssi>=-78?2:1;
   const dots='●'.repeat(q)+'<span style="opacity:.3">'+'●'.repeat(4-q)+'</span>';
   return `<div class="netitem" onclick="pickNet(this)" data-ssid="${esc(n.ssid)}">
    <span>${n.lock?'🔒 ':''}${esc(n.ssid)}</span><span class="dim">${dots}</span></div>`;
  }).join('') : '<div class="dim" style="padding:8px 2px">nenhuma rede encontrada — chegue mais perto do roteador e tente de novo</div>';
 }catch(e){box.innerHTML='<div class="dim" style="padding:8px 2px">'+e.message+'</div>'}
 if(btn){btn.disabled=false;btn.textContent=old}
}
function pickNet(el){
 $('#c_ssid').value=el.dataset.ssid;
 $$('#netlist .netitem').forEach(x=>x.classList.remove('sel'));
 el.classList.add('sel');
 $('#c_wpass').focus();
 toast('Rede: '+el.dataset.ssid+' — agora digite a senha');
}

async function loadLog(){
 try{const l=await api('/api/log');
  $('#logb').innerHTML=l.items.length?l.items.map(i=>
   `<tr><td>${i.when}</td><td>${i.p}</td><td>${i.src}</td>
    <td>${i.ok?(i.pulses||'-'):'<span style="color:var(--err)">falhou</span>'}</td></tr>`).join('')
   :'<tr><td colspan="4" class="dim">nada ainda</td></tr>';
 }catch(e){}
}

async function boot(){
 try{fillCfg(await api('/api/config'))}catch(e){return}
 status();
}
boot();
setInterval(status,5000);
</script></body></html>
)HTML";
