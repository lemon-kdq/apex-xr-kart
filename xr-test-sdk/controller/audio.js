// Original synthesized music and effects; no external audio downloads.
let audioCtx,engine,engineGain,musicBus,fxBus,beat=0,nextBeat=0,lastTelemetry=0,audioSpeed=0,previousLap=0;
let musicOn=true,fxOn=true;
try{musicOn=localStorage.getItem('apexMusic')!=='off';fxOn=localStorage.getItem('apexFx')!=='off'}catch{}
function labels(){document.getElementById('music').textContent=musicOn?'音乐 开':'音乐 关';document.getElementById('fx').textContent=fxOn?'音效 开':'音效 关'}
function tone(freq,time,duration,volume,bus,type='triangle'){
 const o=audioCtx.createOscillator(),g=audioCtx.createGain();o.type=type;o.frequency.value=freq;
 g.gain.setValueAtTime(0,time);g.gain.linearRampToValueAtTime(volume,time+.008);g.gain.exponentialRampToValueAtTime(.0001,time+duration);
 o.connect(g);g.connect(bus);o.start(time);o.stop(time+duration+.02);o.onended=()=>{o.disconnect();g.disconnect()};
}
function startAudio(){
 if(!audioCtx){audioCtx=new (window.AudioContext||window.webkitAudioContext)();musicBus=audioCtx.createGain();fxBus=audioCtx.createGain();musicBus.gain.value=musicOn?.38:0;fxBus.gain.value=fxOn?.45:0;musicBus.connect(audioCtx.destination);fxBus.connect(audioCtx.destination);
 engine=audioCtx.createOscillator();engine.type='sawtooth';const filter=audioCtx.createBiquadFilter();filter.type='lowpass';filter.frequency.value=420;engineGain=audioCtx.createGain();engineGain.gain.value=0;engine.connect(filter);filter.connect(engineGain);engineGain.connect(fxBus);engine.start();nextBeat=audioCtx.currentTime;
 }if(audioCtx.state==='suspended')audioCtx.resume().catch(()=>{});
}
function audioStop(){lastTelemetry=0;audioSpeed=0;if(audioCtx){engineGain.gain.setTargetAtTime(0,audioCtx.currentTime,.02);audioCtx.suspend().catch(()=>{})}}
function audioUpdate(speed,lap){lastTelemetry=performance.now();audioSpeed=Math.abs(speed);if(audioCtx&&lap>previousLap){[523,659,784,1047].forEach((n,i)=>tone(n,audioCtx.currentTime+i*.09,.18,.10,fxBus))}previousLap=lap;}
document.addEventListener('pointerdown',e=>{startAudio();if(e.target.closest('button'))tone(620,audioCtx.currentTime,.04,.06,fxBus);if(e.target.closest('#brake')&&audioSpeed>1){tone(1800,audioCtx.currentTime,.16,.025,fxBus,'sawtooth')}},true);
for(const [id,key] of [['music','apexMusic'],['fx','apexFx']])document.getElementById(id).onclick=()=>{startAudio();if(id==='music')musicOn=!musicOn;else fxOn=!fxOn;musicBus.gain.setTargetAtTime(musicOn?.38:0,audioCtx.currentTime,.03);fxBus.gain.setTargetAtTime(fxOn?.45:0,audioCtx.currentTime,.03);try{localStorage.setItem(key,(id==='music'?musicOn:fxOn)?'on':'off')}catch{}labels()};
setInterval(()=>{
 if(!audioCtx||audioCtx.state!=='running')return;
 const now=audioCtx.currentTime,live=performance.now()-lastTelemetry<1000&&!exited&&!document.hidden;
 engine.frequency.setTargetAtTime(40+audioSpeed*7+(state.gas?20:0),now,.08);engineGain.gain.setTargetAtTime(live?.025+Math.min(audioSpeed/13,1)*.035:0,now,.06);
 musicBus.gain.setTargetAtTime(musicOn&&live?.38:0,now,.1);
 if(nextBeat<now)nextBeat=now;
 const melody=[72,0,76,79,76,0,74,0,69,0,72,76,74,0,72,0,65,0,69,72,69,0,67,0,67,71,74,0,71,0,67,0];
 while(nextBeat<now+.12){let step=beat%32;const n=melody[step];if(live&&musicOn){if(n)tone(440*Math.pow(2,(n-69)/12),nextBeat,.18,.10,musicBus);if(step%4===0)tone(440*Math.pow(2,([48,45,41,43][Math.floor(step/8)]-69)/12),nextBeat,.33,.12,musicBus);if(step%4===0)tone(70,nextBeat,.09,.16,musicBus,'sine');if(step%2===1)tone(6500,nextBeat,.025,.012,musicBus,'square');}beat++;nextBeat+=.25;}
},50);
window.addEventListener('blur',audioStop);document.addEventListener('visibilitychange',()=>{if(document.hidden)audioStop()});window.addEventListener('pagehide',audioStop);labels();
